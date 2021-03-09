/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: u_mulvar.c,v 9.0 2001/01/09 02:55:52 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char U_multivars_sid[] = "@(#) $Id: u_mulvar.c,v 9.0 2001/01/09 02:55:52 john Exp $";
#endif

#include "dbsql_e.h"

static verify_join_predicate();

/*
 * u_multivars - provide a qselect utility for multiple variable requests
 */

   u_multivars(selq, sel_list, from, where_cform, group_by, having,
         sub_flag, n, order_by)
      node *selq, *sel_list, *from, *where_cform, *group_by, *having, *n;
      node *order_by;
      int sub_flag;

      {
         int st = FB_AOK;
         v_matrix *make_instance(), *mlist;
         node *sn, *tn, *makenode(), *copy_nodelist(), *q;
         mat_head h_query;

         if (st != FB_ERROR)
            st = u_opentables(from);
         if (st != FB_ERROR)
            st = u_verify_select(sel_list, from);
         if (st != FB_ERROR)
            st = u_verify_select(group_by, from);
         if (st != FB_ERROR)
            st = u_verify_tables(where_cform->n_list, from);
         if (st != FB_ERROR)
            st = u_verify_cform(where_cform, from);
         if (st != FB_ERROR)
            st = set_where(where_cform);
         if (st == FB_ERROR)
            return(FB_ERROR);

         relation_function = 1;
         /* convert the sel_list into a simple variable list */
         sn = copy_nodelist(where_cform->n_list);

         /* building the vars is needed so as to get full obj name expansion */
         tn = makenode();
         for (q = sel_list; q != NULL; q = q->n_list)
            build_vars(tn, q);

         /* make the instance matrix */
         mlist = make_instance(where_cform, tn->n_list);

#if DEBUG
         if (traceflag >= 1)
            trace_instance(mlist, sn, "Incidence Matrix");
#endif /* DEBUG */

         /* produce the reduced matrix into global h_output area */
         reduce(mlist, sn);

#if DEBUG
         if (traceflag >= 1)
            trace_instance(h_output.m_head, sn, "Reduced Incidence Matrix");
#endif /* DEBUG */

         if (verify_join_predicate(&h_output) == FB_ERROR){
            fb_serror(FB_MESSAGE, "Invalid Query: No Join predicate specified.",NIL);
            return(FB_ERROR);
            }

         /* sequence the ordered irreducible components into sub queries */
         r_head = NULL;
         while (h_output.m_head != NULL){
            sequence(&h_query, &h_output);
#if DEBUG
            if (traceflag >= 1)
               trace_instance(h_query.m_head, sn, "Sub Query");
#endif /* DEBUG */
            make_rel(&h_query, where_cform, from);
#if DEBUG
            if (traceflag >= 6)
               trace_relation(r_head, "Sub Query Relation Trace");
#endif /* DEBUG */
            }

#if DEBUG
         if (traceflag >= 2)
            trace_relation(r_head, "Complete Query Relation Trace");
#endif /* DEBUG */

         /* now do something with r_head - like project it */
         if (sub_flag)
            st = r_sub_project(r_head, sel_list, where_cform, from, n);
         else
            st = r_project(selq, sel_list, where_cform, from,
               group_by, having, order_by);

         r_head = NULL;
         return(st);
      }

   /*
    * verify_join_predicate - verify a reduced matrixs join predicate
    *
    *     hueristic: test the final matrix for a matrix where
    *        the only overlap is on the target (project -1 list)
    *        and nowhere else. IE:
    *        s   sp
    *        1   0     1
    *        0   1     2
    *        1   1     -1
    *
    *     since this is essentially nonsensical since there is not
    *     a join predicate, announce this, assume an error, and return.
    */

   static verify_join_predicate(h)
      mat_head *h;

      {
         v_matrix *mlist, *v;
         int i, st;

         mlist = h->m_head;
         st = FB_ERROR;
         for (v = mlist; v != NULL && st == FB_ERROR; v = v->v_next){
            if (v->v_type != V_SINGLE){
               for (i = 0; i < MAXCLAUSES && v->v_list[i] != 0; i++)
                  if (v->v_list[i] != -1){	/* -1 is target marker */
                     st = FB_AOK;
                     break;
                     }
               }
            }
         return(st);
      }
