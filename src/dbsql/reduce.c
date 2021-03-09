/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: reduce.c,v 9.0 2001/01/09 02:55:50 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Reduce_sid[] = "@(#) $Id: reduce.c,v 9.0 2001/01/09 02:55:50 john Exp $";
#endif

#include "dbsql_e.h"

static v_matrix *s_mlist;			/* save mlist */
static node *s_mn;				/* save node list */
static short int first_reduction_done;		/* for a hueristic */

static void do_reduce();
static connected();
static irreduce();
static void store_output();
static test_overlap();

/*
 * reduce - the driver for the reduction phase of incidence matrix operation.
 */

   reduce(mlist, mn)
      v_matrix *mlist;
      node *mn;

      {
         v_matrix *copy_vmatrix();
         node *copy_nodelist();

         clear_mat_head(&h_output);
         clear_mat_head(&h_single);
         clear_mat_head(&h_double);
         clear_mat_head(&h_reduced);
         s_mn = copy_nodelist(mn);
         s_mlist = copy_vmatrix(mlist);
         first_reduction_done = 0;	 /* used in heuristic below */
         do_reduce(mlist, mn);
         store_output(h_single.m_head, V_SINGLE, -1);
#if DEBUG
         if (traceflag >= 5)
            trace_instance(h_output.m_head, mn,
               "(PRE SORT) Reduced Incidence Matrix");
#endif /* DEBUG */
         h_output.m_vars = mn;
         sort_matrix(&h_output);
      }

/*
 * do_reduce - recursive portion of the reduction process
 */

   static void do_reduce(mlist, mn)
      v_matrix *mlist;
      node *mn;

      {
         int vrows, i, j;
         v_matrix *v, *vlist, *cv, *copy_one_vmatrix();
         node *pn;

         /* if number of variables (columns) is 1, then add and go */
         if (countlist(mn) <= 1){
            v = mlist;
            for (i = 0; i < MAXCLAUSES && v->v_list[i] != 0; i++){
               cv = copy_one_vmatrix(v->v_list[i], s_mlist);
               add_to_matrix(&h_single, cv);
               }
            return;
            }

         if (connected(&mlist, mn))
            irreduce(mlist, mn);
         else {
#if DEBUG
            if (traceflag >= 2)
               trace_instance(mlist, mn, "UnConnected Parts");
#endif /* DEBUG */
            vrows = v_countmatrix(mlist);
            /*
            * hueristic: if these first parts are unconnected, then
            * 	all parts except the Target parts are dropped.
            */
            if (first_reduction_done == 0){
               for (v = mlist, i = 0; i < vrows; i++, v = v->v_next){
                  for (j = 0; j < MAXCLAUSES; j++)
                     if (v->v_list[j] == -1){
                        mlist = v;
                        mlist->v_next = NULL;
                        vrows = 1;
                        break;
                        }
                  }
               }
            first_reduction_done = 1;
            for (v = mlist, i = 0; i < vrows; i++, v = v->v_next){
               sub_make_instance(v, mn, &vlist, &pn);
               do_reduce(vlist, pn);
               }
            }
      }

/*
 * connected - the connectivity test. if not connected, leave components
 *	in the new list, mlist, as pointed to by mptr.
 */

   static connected(mptr, mn)
      v_matrix **mptr;
      node *mn;

      {
         v_matrix *converge_instance(), *mlist;
         int i, vcols;

         mlist = *mptr;
         vcols = countlist(mn);
         /* converge the matrix into as few rows as possible */
         mlist = converge_instance(mlist, mn);
         *mptr = mlist;
         /* if Nrows == 1, && and all 1's then its connected */
         if (v_countmatrix(mlist) == 1){
            for (i = 0; i < vcols; i++)
               if (mlist->v_array[i] == 0)
                  return(0);
            return(1);
            }
         return(0);
      }

/*
 * irreduce - break apart the remaining elements into a reduced incidance
 *	matrix, as defined by Wong and Youssefi.
 *	- rebuild a large matrix (from s_mlist)
 *	- output lines with one variable and remove
 *	- do Wong column tests
 */
   static irreduce(mlist, mn)
      v_matrix *mlist;
      node *mn;

      {
         v_matrix *cv, *copy_one_vmatrix(), *v, *converge_instance();
         int i, j, var_cnt, s_ncols, ov, vcount;
         mat_head h_local, h_lreduce;

         clear_mat_head(&h_double);
#if DEBUG
         if (traceflag >= 3)
            trace_instance(mlist, mn, "IRREDUCEABLE PART");
#endif /* DEBUG */
         /* build the double matrix ... only the 2+ variable lines
          * foreach clause element, locate the saved element
          *    if a one var line, add to single
          *    else add to double
          */
         s_ncols = countlist(s_mn);
         v = mlist;
         for (i = 0; i < MAXCLAUSES && v->v_list[i] != 0; i++){
            cv = copy_one_vmatrix(v->v_list[i], s_mlist);
            for (j = 0, var_cnt = 0; j < s_ncols; j++)
               if (cv->v_array[j] == 1)
                  var_cnt++;
            if (var_cnt == 1)
               add_to_matrix(&h_single, cv);
            else
               add_to_matrix(&h_double, cv);
            }
         if (h_double.m_head != NULL){
#if DEBUG
            if (traceflag >= 3)
               trace_instance(h_double.m_head, s_mn,
                  "IRREDUCEABLE PART - two+Var lines");
#endif /* DEBUG */
            h_double.m_vars = s_mn;
            vcount = v_countmatrix(h_double.m_head);
            /* test_overlap returns 0...nvar-1 or -1 on error */
            if (vcount > 1)
               ov = test_overlap(&h_double, &h_reduced);
            else
               ov = -1;
            if (ov == -1){
               /* no overlap or multiple overlap */
               /* converge the matrix into as few rows as possible */
               h_double.m_head = converge_instance(h_double.m_head,
                  h_double.m_vars);
               store_output(h_double.m_head, V_NOREDUCE, -1);
#if DEBUG
               if (traceflag >= 3)
                  trace_instance(h_double.m_head, h_double.m_vars,
                     "Type 3 reductions");
#endif /* DEBUG */
               }
            else{
#if DEBUG
               if (traceflag >= 3)
                  trace_instance(h_reduced.m_head, h_reduced.m_vars,
                     "Use This to Build Type 2 Reduction");
#endif /* DEBUG */
               clear_mat_head(&h_lreduce);
               /* for each reduced element */
               for (v = h_reduced.m_head; v != NULL; v = v->v_next){
                  clear_mat_head(&h_local);
                  /* make a local matrix of just those vars */
                  for (i = 0; i < MAXCLAUSES && v->v_list[i] != 0; i++){
                     cv = copy_one_vmatrix(v->v_list[i], s_mlist);
                     add_to_matrix(&h_local, cv);
                     }
                  /* converge the matrix into as few rows as possible */
                  h_local.m_head = converge_instance(h_local.m_head, s_mn);
                  /* quick error check -- by definition needs to be == 1 */
                  if ((var_cnt = v_countmatrix(h_local.m_head)) != 1)
                     fprintf(stderr, "(101) Reduction state error: count=%d\n",
                        var_cnt);
                  add_to_matrix(&h_lreduce, h_local.m_head);
                  }
               store_output(h_lreduce.m_head, V_DOUBLE, ov);
               }
            }
      }

/*
 * store_output - store v_matrix element v into the output,
 *	with type and overlap flag.
 */

   static void store_output(v, type, ov)
      v_matrix *v;
      int type, ov;

      {
         v_matrix *t, *lt;

         if (v == NULL)
            return;
         for (t = v; t != NULL; t = t->v_next){
            t->v_type = type;
            t->v_overlap = ov;
            lt = t;
            }
         add_to_matrix(&h_output, v);
         h_output.m_tail = lt;
      }

/*
 * add_to_matrix - generic append mechanism for matrices via mat_head structs
 */

   add_to_matrix(h, v)
      mat_head *h;
      v_matrix *v;

      {
         if (h->m_head == NULL)
            h->m_head = v;
         else if (h->m_tail != NULL){
            h->m_tail->v_next = v;
            v->v_prev = h->m_tail;
            }
         h->m_tail = v;
      }

/*
 * delete_from_matrix - generic fb_delete for matrices via mat_head structs
 */

   delete_from_matrix(h, v)
      mat_head *h;
      v_matrix *v;

      {
         if (h->m_head == v)
            h->m_head = v->v_next;
         if (h->m_tail == v)
            h->m_tail = v->v_prev;
         v_delete(v);
      }

/*
 * unlink_from_matrix - generic unlink for matrices via mat_head structs
 */

   unlink_from_matrix(h, v)
      mat_head *h;
      v_matrix *v;

      {
         if (h->m_head == v)
            h->m_head = v->v_next;
         if (h->m_tail == v)
            h->m_tail = v->v_prev;
         v_unlink(v);
      }

/*
 * clear_mat_head - clear out a matrix header and its vars
 */

   clear_mat_head(h)
      mat_head *h;

      {
         h->m_head = NULL;
         h->m_tail = NULL;
         h->m_vars = NULL;
      }

/*
 * test_overlap - test the overlap by duplicating d into r, except for
 *	some column m ... if r is then disconnected, overlap is on m.
 *      return count indicating m or -1
 */

   static test_overlap(d, r)
      mat_head *d, *r;

      {
         int m = -1, col, nvars, j, pos;
         v_matrix *vr, *vd, *make_matrix();
         node *vars, *tn, *fn, *pt;
         cell *fc, *tc;

         nvars = countlist(d->m_vars);
         for (col = 0; col < nvars; col++){
            /* Build d into r without column col ... */

            /*
             * foreach matrix node in d ...
             *    copy parts to new node for r
             */
            clear_mat_head(r);
            for (vd = d->m_head; vd != NULL; vd = vd->v_next){
               vr = make_matrix();
               /* copy in proper columns of information */
               for (j = 0, pos = 0; j < nvars; j++){
                  if (j != col){
                     vr->v_array[pos] = vd->v_array[j];
                     pos++;
                     }
                  vr->v_list[j] = vd->v_list[j];
                  }
               /* copy in list of vars */
               for (j = 0; j < MAXCLAUSES; j++){
                  if (vd->v_list[j] == 0)
                     break;
                  vr->v_list[j] = vd->v_list[j];
                  }
               add_to_matrix(r, vr);
               }

            /* create a new node list without column col */
            vars = pt = NULL;
            for (j = 0, fn = d->m_vars; j < nvars; j++, fn = fn->n_list)
               if (j != col){
                  tn = makenode();
                  tc = makecell();
                  fc = (cell *) fn->n_obj;
                  fb_mkstr(&(tc->c_sval), fc->c_sval);
                  tn->n_obj = (int) tc;
                  if (vars == NULL)
                     vars = tn;
                  if (pt != NULL)
                     pt->n_list = tn;
                  pt = tn;
                  }

            /* vars is set now with new list */
            r->m_vars = vars;

            /* trace_instance(r->m_head, r->m_vars, "test overlap - r:"); */

            /* if not connected (disconnected) overlap has occured */
            if (!connected(&(r->m_head), r->m_vars)){
               m = col;
               break;
               }

            /* fb_free up r -- fb_free up r->m-vars also -- or do garbage collect */
            }
         return(m);
      }
