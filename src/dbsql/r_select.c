/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: r_select.c,v 9.0 2001/01/09 02:55:49 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char R_select_sid[] = "@(#) $Id: r_select.c,v 9.0 2001/01/09 02:55:49 john Exp $";
#endif

#include "dbsql_e.h"

#if DEBUG
static char msg[FB_MAXLINE];
#endif /* DEBUG */

static int n_rels;			/* number of distinct Rs to cross */

static check_optimize();
static sub_check_optimize();
static test_join_is_complex();
static is_complex();

/*
 * r_select - select a relation into r by parsing c_root.
 *	use any existing relations in r_head chain to limit parse iterations.
 */

   relation *r_select(c_root)
      node *c_root;

      {
         int nvars, pos, nrecs, s_nvars, slot, found, t_slot, i, j, k;
         relation *create_relation(), *r, *tr, *pr;
         node *n, *nn, e_tree, *et, *pn, *s_vars, *tn, *dn, *a, v_tree;
         fb_database *dp;
         v_matrix *vm, *make_matrix();

         /* clear all the relations -- get set for this sub query */
         r_clear_relations();
         nvars = countlist(c_root->n_list);		/* gen Q var count */
         et = &e_tree;
         clearnode(et);
         et->n_list = c_root->n_list;
         et->n_type = C_ROOT;

         /*
          * do single variable processing - store in rel_single slots
          * this is done by gathering all same single variables into
          * a single e_tree and passing that for preprocessing.
          */

         for (pos = 0; pos < nvars; pos++){
            et = &e_tree;
            et->n_narg[1] = NULL;
            pn = c_root;
            for (n = c_root->n_narg[1]; n != NULL; n = nn){
               nn = n->n_narg[1];
               if (countlist(n->n_list) == 1){
                  if (n->n_list->n_id == pos){
                     /* unlink from c_root */
                     pn->n_narg[1] = nn;
                     n->n_narg[1] = NULL;
                     /* link onto e_tree */
                     et->n_narg[1] = n;
                     et = n;
                     }
                  else
                     pn = n;
                  }
               else
                  break;
               }
            /*
             * e_tree now is either empty or it has all the one
             * clause variables for variable "pos"
             */
            et = &e_tree;
            if (et->n_narg[1] != NULL)
               r_preprocess(et, nvars);
            }

         /* now, all that is left in c_root are non single var components */

         /* use et as a temp root for the next tree structure */
         clearnode(et);
         et->n_type = C_ROOT;
         et->n_narg[1] = c_root->n_narg[1];

         /* set the roots n_list with the variables from this 2+ part */
         for (n = c_root->n_narg[1]; n != NULL; n = n->n_narg[1])
            for (a = n->n_list; a != NULL; a = a->n_list)
               add_var(et, a);

         /*
          * et has the list of vars for this subQ element.
          * c_root has the entire list.
          */

#if DEBUG
         if (traceflag >= 6){
            fprintf(stderr, "r_select - 2+ var part of subq (c_root)\n");
            trace_canon(c_root);
            fprintf(stderr, "r_select - 2+ var part of subq (et)\n");
            trace_canon(et);
            }
#endif /* DEBUG */

         /*
          * the rest depends on the stuff thats been done.
          * the idea is to set up rel_depth with proper relations.
          * for each variable "slot"
          *    find a r_head element slot OR a single var slot that matches
          *       or generate a new one with a indicator for FULL DBASE
          *       store this R in rel_depth[slot]
          *    
          * then examine rel_depth.
          * if two relations are the EXACT same, fold it up. repeat.
          *	(BUT, could THIS exactness ever happen? i dont think so!)
          * count relations to do cross product of.
          * calculate width (nrecs across) for this cross product
          * do cross product
          */

         /*
          * sometimes, a 2 var sub Q will be disconnected into
          * solely single var subQs. in this case, assume there
          * is only one var per, and that the first one is
          * the one to return. Next little clause does this.
          */
         if (c_root->n_narg[1] == NULL){
            for (slot = 0; slot < MAXVARIABLES; slot++)
               if (rel_single[slot] != NULL){
                  r = rel_single[slot];
                  rel_single[slot] = NULL;
                  return(r);
                  }
            fb_xerror(FB_MESSAGE, "r_select - Error: No Single Rel Found.", NIL);
            }

         s_vars = et->n_list;
         s_nvars = countlist(s_vars);		 /* num vars this subQ */
         n = s_vars;
         r = NULL;
#if DEBUG
         if (traceflag >= 10){
            fprintf(stderr, "r_select SINGLES TRACE-\n");
            for (slot = 0; slot < MAXVARIABLES; slot++){
               if (rel_single[slot] == NULL)
                  continue;
               sprintf(msg, "r_select - singles slot %d", slot);
               trace_relation(rel_single[slot], msg);
               }
            }
#endif /* DEBUG */
         for (slot = 0; slot < s_nvars; slot++, n = n->n_list){
            found = 0;
            pr = NULL;
            for (r = r_head; r != NULL; r = r->r_next){
               if (var_in_relation(n, r)){
                  found = 1;
                  r->r_used = 1;
                  /* now unlink r from the r_head list */
                  if (pr == NULL)
                     r_head = r->r_next;
                  else
                     pr->r_next = r->r_next;
                  break;
                  }
               pr = r;
               }
            if (!found){
               r = rel_single[n->n_id];
               rel_single[n->n_id] = NULL;
               }
            if (r == NULL){
               /* set up a relation representing entire dbase */
               t_slot = n->n_id;
               vm = make_matrix();
               vm->v_array[t_slot] = 1;
               dn = c_root->n_list;
               for (i = 0; i < t_slot; i++, dn = dn->n_list)
                  ;
               dp = (fb_database *) dn->n_p1;
               if (dp == NULL)
                  fb_xerror(FB_MESSAGE, "r_select - Null database pointer", NIL);
               r = create_relation(1, vm, nvars);
               r->r_dbase[0] = dp;
               tn = makenode();
               *tn = *n;
               tn->n_list = NULL;
               r->r_vars = tn;
               if (init_optimize(r, et) == FB_AOK)
                  r->r_reccnt = -2;	/* flag shows to OPTIMIZE */
               else
                  r->r_reccnt = -1;	/* flag shows to use whole dbase */
               }
            rel_depth[slot] = r;
            if (r == NULL)
               fb_xerror(FB_MESSAGE, "r_select: invalid (null) relation.", NIL);
            }

         /*
          * examine here for EXACT same relations. fold up if so. repeat.
          *	(BUT, could THIS exactness ever happen? i dont think so!)
          */

         /* count number of distinct relations in the cross product to do */
         for (nrecs = 0, slot = 0; slot < s_nvars; slot++)
            nrecs += rel_depth[slot]->r_nrecs;
         n_rels = s_nvars;		/* number of actual Rs to cross */


         /*
          * if any of these is represented by a complex sub component
          * make it a -1 type, not a -2 type.
          */
         for (slot = 0; slot < s_nvars; slot++){
            r = rel_depth[slot];
            if (r->r_reccnt == -2){
               if (test_join_is_complex(r, r->r_join_fld, c_root))
                  r->r_reccnt = -1;
               }
            }

         /* attempt to bubble the -2 type reccnts to the inner core */
         for (slot = s_nvars - 1; slot > 0; slot--){
            r = rel_depth[slot];
            pr = rel_depth[slot - 1];
            if (pr->r_reccnt == -2 && r->r_reccnt != -2){
               rel_depth[slot] = pr;
               rel_depth[slot - 1] = r;
               }
            }

#if DEBUG
         if (traceflag >= 10){
            fprintf(stderr, "r_select DEPTH TRACE- maxdepth=%d, nrecs=%d\n",
               s_nvars, nrecs);
            for (slot = 0; slot < s_nvars; slot++){
               sprintf(msg, "r_select - depth level %d", slot);
               trace_relation(rel_depth[slot], msg);
               }
            }
#endif /* DEBUG */

         vm = make_matrix();
         r = create_relation(nrecs, vm, nvars);
         clearnode(&v_tree);
         /* copy in each list of dbase pointers -- fixed 0 ... nrecs-1 */
         for (i = 0, k = 0; i < nrecs; k++){
            tr = rel_depth[k];
            r->r_offset[k] = i;
            r->r_isize[k] = tr->r_nrecs;
            for (j = 0; j < tr->r_nrecs && i < nrecs; j++)
               r->r_dbase[i++] = tr->r_dbase[j];
            for (a = tr->r_vars; a != NULL; a = a->n_list)
               add_var(&v_tree, a);
            tr->r_vm->v_list[0] = 0;
            v_combine(tr->r_vm, r->r_vm, nvars);
            }
         r->r_vars = v_tree.n_list;
         v_tree.n_list = NULL;

         if (set_where(et) != FB_AOK)
            fb_xerror(FB_MESSAGE, "r_select - set_where() failed.", NIL);
         /* get recursion going with a depth of 1 */
         rr_select(r, et, 1);
         return(r);
      }

/*
 * rr_select - does actual deposit to relation r if depth+1 matches
 *	the nrecs of the relation r.
 */

   rr_select(r, c_root, depth)
      relation *r;
      node *c_root;
      int depth;

      {
#if DEBUG
         if (traceflag >= 6)
            fprintf(stderr, "rr_select - depth %d\n", depth);
#endif /* DEBUG */
         if (depth >= n_rels){		/* should be == */
            /* foreach r[depth], deposit all elements of "depth" */
            relatetest(r, c_root, depth);
            }
         else{
            /* foreach r[depth], load and recurse with depth + 1 */
            recurseeach(r, c_root, depth);
            }
      }

/*
 * init_optimize - if the fields in this part of the query are autoindexed,
 *	then its possible to optimize query
 */

   init_optimize(r, et)
      relation *r;
      node *et;

      {
         int st = FB_AOK, i;
         node *n;
         fb_database *db = NULL;

         for (i = 0; i < r->r_nrecs; i++)
            if (r->r_dbase[i] != NULL){
               db = r->r_dbase[i];
               break;
               }
         for (n = et; n != NULL && st != FB_ERROR; n = n->n_narg[1]){
            switch(n->n_type){
               case C_AND:
                  /* follow the code - its in n_narg[0] */
                  st = check_optimize(db, n->n_narg[0], r);
                  break;
               }
            }
         return(st);
      }

   static check_optimize(db, n, r)
      fb_database *db;
      node *n;
      relation *r;

      {
         int st = FB_AOK, i;

         if (n == NULL)
            return(FB_AOK);

         r->r_join_op = n->n_type;
         /* for any fields of db, if not autoindexed, return FB_ERROR */
         for (i = 0; i < NARGS && st != FB_ERROR; i++)
            if (n->n_narg[i] != NULL)
               st = sub_check_optimize(db, n->n_narg[i], r);
         return(st);
      }

   static sub_check_optimize(db, n, r)
      fb_database *db;
      node *n;
      relation *r;

      {
         int st = FB_AOK, i;
         fb_database *tdb;
         fb_field *tf;

         if (n == NULL)
            return(FB_AOK);

         for (i = 0; i < NARGS && st != FB_ERROR; i++)
            if (n->n_narg[i] != NULL)
               st = sub_check_optimize(db, n->n_narg[i], r);
         if (st == FB_ERROR)
            return(FB_ERROR);

         switch(n->n_type){
            case V_ID:
               tdb = NULL;
               tf = NULL;
               if (n->n_p1 != 0)
                  tdb = (fb_database *) n->n_p1;
               if (n->n_p2 != 0)
                  tf  = (fb_field *) n->n_p2;
               if (tdb != NULL && tf != NULL && tdb == db){
                  if (tf->aid == NULL)
                     st = FB_ERROR;
                  else{
                     /*
                      * this should catch multiple field join requests
                      * and refuse to optimize such a thing, no matter auto ix.
                      */
                     if (r->r_aid != NULL && r->r_aid != tf->aid)
                        st = FB_ERROR;
                     r->r_aid = tf->aid;
                     r->r_join_fld = tf;
                     if (r->r_join_op != R_EQ)
                        st = FB_ERROR;
                     }
                  }
               break;
            }
         return(st);
      }

   static test_join_is_complex(r, f, n)
      relation *r;
      fb_field *f;
      node *n;

      {
         node *eq, *eq_L, *eq_R;

         /* first get to eq */
         n = n->n_narg[1];
         n = n->n_narg[0];
         eq = n;
         eq_L = n->n_narg[0];
         eq_R = n->n_narg[1];
         /* find the *current* tree and determine it *it* is complex */
         if (tree_contains_field(eq_L, f)){
            if (is_complex(eq_L))
               return(1);
            }
         else if (is_complex(eq_R))
            return(1);
         return(0);
      }

   static is_complex(n)
      node *n;

      {
         if (n == NULL)
            return(0);
         if (n->n_type != V_ID)
            return(1);
         return(0);
      }
