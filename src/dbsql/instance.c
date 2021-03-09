/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: instance.c,v 9.0 2001/01/09 02:55:48 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Instance_sid[] = "@(#) $Id: instance.c,v 9.0 2001/01/09 02:55:48 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * instance - functions to do the instance matrix to determine
 *	connectivity...
 */
 
static mat_head h_matrix;
static var_in_list();
static v_insert();

/*
 * make_instance - make the initial instance matrix using the
 *	root where_cform (C1...CN) tree and the selection list (T).
 */

   v_matrix *make_instance(r, t)
      node *r, *t;

      {
         v_matrix *vrow, *make_matrix();
         node *vbase, *vn, *q;
         int pos;

         vbase = r->n_list;
         clear_mat_head(&h_matrix);

         /* for the sel_list, build a row, link int place */
         vrow = make_matrix();
         vrow->v_list[0] = -1;	/* target list id is -1 */
         /* foreach base element, if var is here, place a 1, else 0 */
         for (pos = 0, vn = vbase; vn != NULL; vn = vn->n_list, pos++)
            vrow->v_array[pos] = var_in_list(vn, t);
         add_to_matrix(&h_matrix, vrow);

         /* for each AND node in the canonical tree, build a row */
         for (q = r->n_narg[1]; q != NULL; q = q->n_narg[1]){
            vrow = make_matrix();

            /* store the current id */
            vrow->v_list[0] = q->n_id;

            /* foreach base element, if var is here, place a 1, else 0 */
            for (pos = 0, vn = vbase; vn != NULL; vn = vn->n_list, pos++)
               vrow->v_array[pos] = var_in_list(vn, q->n_list);

            /* link into place */
            add_to_matrix(&h_matrix, vrow);
            }
         return(h_matrix.m_head);
      }

/*
 * var_in_list - determine if variable node v is somewhere in node list n
 *	returns: 0 = no, 1 = yes
 */

   static var_in_list(v, n)
      node *v, *n;

      {
         cell *vc, *nc;

         vc = (cell *) v->n_obj;
         for (; n != NULL; n = n->n_list){
            nc = (cell *) n->n_obj;
            if (equal(nc->c_sval, vc->c_sval))
               return(1);
            }
         return(0);
      }

/*
 * make_matrix - the v_matrix allocation routine
 */

   v_matrix *make_matrix()

      {
         v_matrix *v;
         int i;

         v = (v_matrix *) fb_malloc(sizeof(v_matrix));
         for (i = 0; i < MAXVARIABLES; i++)
            v->v_array[i] = 0;
         for (i = 0; i < MAXCLAUSES; i++)
            v->v_list[i] = 0;
         v->v_next = v->v_prev = NULL;
         v->v_type = 0;
         v->v_overlap = -1;
         v->v_glink = v_ghead;
         v_ghead = v;
         return(v);
      }

/*
 * converge_instance - converge the instance matrix using the
 *	connectivity algorithm from Wong, et al.
 */

   v_matrix *converge_instance(mlist, n)
      node *n;
      v_matrix *mlist;

      {
         int i, j, ncols, nrows, rep_flag;
         v_matrix *v, *nv, *v_tmp, *make_matrix();

         ncols = countlist(n);
         for (i = 0; i < ncols; i++){
            if ((nrows = v_countmatrix(mlist)) == 1)
               break;

            /* make a matrix element of all the rows with a 1 in column i */
            v_tmp = make_matrix();
            for (v = mlist, j = 0; j < nrows; j++, v = v->v_next){
               /* if a 1 in column i, OR this in place */
               if (v->v_array[i] == 1)
                  v_combine(v, v_tmp, ncols);
               }
            /* replace this element where first i==1 appears, fb_delete rest */
            rep_flag = 0;
            for (v = mlist, j = 0; v != NULL && j < nrows; j++){
               /* if a 0, cont. if a 1 in column i, OR this in place */
               if (v->v_array[i] == 0)
                  v = v->v_next;
               else {
                  if (rep_flag == 0){
                     /* either replace ... */
                     rep_flag = 1;
                     v_insert(v_tmp, v);
                     if (v == mlist)
                        mlist = v_tmp;
                     v_delete(v);
                     v = v_tmp->v_next;
                     }
                  else{
                     /* or fb_delete */
                     nv = v->v_next;
                     v_delete(v);
                     v = nv;
                     }
                  }
               }
            /* trace_instance(mlist, n); */
            }
         return(mlist);
      }

/*
 * v_combine - do logical OR and add/combine list of CLAUSE IDs
 */

   v_combine(e, vt, ncols)
      v_matrix *e, *vt;
      int ncols;

      {
         int j, k;

         for (j = 0; j < ncols; j++)
            if (vt->v_array[j] || e->v_array[j])
               vt->v_array[j] = 1;
         for (j = 0; j < MAXCLAUSES; j++)
            if (vt->v_list[j] == 0)
               break;
         if (j >= MAXCLAUSES)
            fb_xerror(FB_MESSAGE, "Too many clauses: v_combine.\n", NIL);
         for (k = 0; k < MAXVARIABLES && j < MAXVARIABLES; k++, j++){
            if (e->v_list[k] == 0)
               break;
            vt->v_list[j] = e->v_list[k];
            }
      }

/*
 * v_insert - insert e in front of v. assume e and v well defined.
 */

   static v_insert(e, v)
      v_matrix *e, *v;

      {
         e->v_next = v;
         e->v_prev = v->v_prev;
         if (e->v_prev != NULL)
            e->v_prev->v_next = e;
         v->v_prev = e;
      }

/*
 * v_delete - fb_delete e and fb_free up the memory.
 */

   v_delete(e)
      v_matrix *e;

      {
         if (e->v_prev != NULL)
            e->v_prev->v_next = e->v_next;
         if (e->v_next != NULL)
            e->v_next->v_prev = e->v_prev;
         e->v_prev = NULL;
         e->v_next = NULL;
      }

/*
 * v_unlink - unlinke e
 */

   v_unlink(e)
      v_matrix *e;

      {
         if (e->v_prev != NULL)
            e->v_prev->v_next = e->v_next;
         if (e->v_next != NULL)
            e->v_next->v_prev = e->v_prev;
         e->v_prev = NULL;
         e->v_next = NULL;
      }

/*
 * v_countmatrix - count the v_matrix es in v's next list
 */

   v_countmatrix(v)
      v_matrix *v;

      {
         int i;

         for (i = 0; v != NULL; v = v->v_next, i++)
            ;
         return(i);
      }

/*
 * copy_vmatrix - duplicate a v_matrix and return it
 */

   v_matrix *copy_vmatrix(f)
      v_matrix *f;

      {
         v_matrix *t, *pt, *make_matrix(), *vroot;

         pt = NULL;
         vroot = (v_matrix *) NULL;
         for (; f != (v_matrix *) NULL; f = f->v_next){
            t = make_matrix();
            *t = *f;
            t->v_next = t->v_prev = NULL;
            if (vroot == (v_matrix *) NULL)
               vroot = t;
            if (pt != NULL)
               pt->v_next = t;
            t->v_prev = pt;
            pt = t;
            }
         return(vroot);
      }

/*
 * copy_one_vmatrix - duplicate a single v_matrix element and return it
 *	the matrix element copied is the one with matrix id m_id
 */

   v_matrix *copy_one_vmatrix(m_id, f)
      int m_id;
      v_matrix *f;

      {
         v_matrix *t, *make_matrix();

         t = make_matrix();
         for (; f != (v_matrix *) NULL; f = f->v_next){
            if (f->v_list[0] == m_id)
               break;
            }
         if (f != (v_matrix *) NULL)
            *t = *f;
         t->v_next = t->v_prev = NULL;
         return(t);
      }

/*
 * sub_make_instance - make a sub instance array of a single line.
 *	remove any uneeded vars from the vn list; i.e. remove columns
 *	if there is a column of 0.
 */

   sub_make_instance(v, mn, vlist_ptr, vn_ptr)
      v_matrix *v, **vlist_ptr;
      node **vn_ptr, *mn;

      {
         v_matrix *vt, *make_matrix();
         node *f, *t, *pt, *nroot, *makenode();
         cell *tc, *fc, *makecell();
         int i, j;

         vt = make_matrix();
         for (i = 0; i < MAXCLAUSES; i++)
            vt->v_list[i] = v->v_list[i];
         *vlist_ptr = vt;
         *vn_ptr = NULL;
         pt = NULL;
         nroot = (node *) NULL;
         /* remove any 0 entries in the nlist as it is copied */
         for (f = mn, i = j = 0; f != NULL; f = f->n_list, i++){
            if (v->v_array[i] == 1){
               vt->v_array[j] = 1;
               t = makenode();
               tc = makecell();
               fc = (cell *) f->n_obj;
               fb_mkstr(&(tc->c_sval), fc->c_sval);
               t->n_obj = (int) tc;
               if (nroot == (node *) NULL)
                  nroot = t;
               if (pt != NULL)
                  pt->n_list = t;
               pt = t;
               j++;
               }
            }
         *vn_ptr = nroot;
      }

/*
 * gcollect_instance - garbage collect all of the vmatrix that have been made
 */

   gcollect_instance()
      {
         v_matrix *v, *nv;

         for (v = v_ghead; v != NULL; v = nv){
            nv = v->v_glink;
            fb_free((char *) v);
            }
         v_ghead = NULL;
      }

#if DEBUG
   trace_instance(mlist, n, str)
      node *n;
      v_matrix *mlist;
      char *str;

      {
         cell *c;
         int vcount, i;
         v_matrix *v;

         fprintf(stderr, " *** %s\n", str);
         vcount = countlist(n);
         for (; n != NULL; n = n->n_list){
            c = (cell *) n->n_obj;
            fprintf(stderr, "%s ", c->c_sval);
            }
         fprintf(stderr, "\n");

         for (v = mlist; v != NULL; v = v->v_next){
            for (i = 0; i < vcount; i++)
               fprintf(stderr, "%d ", v->v_array[i]);
            fprintf(stderr, " ... ");
            for (i = 0; i < MAXCLAUSES && v->v_list[i] != 0; i++)
               fprintf(stderr, "%d ", v->v_list[i]);
            fprintf(stderr, "   (t=%d, o=%d)", v->v_type, v->v_overlap);
            fprintf(stderr, "\n");
            }
      }
#endif /* DEBUG */
