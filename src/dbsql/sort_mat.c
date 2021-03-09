/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: sort_mat.c,v 9.0 2001/01/09 02:55:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Sort_matrix_sid[] = "@(#) $Id: sort_mat.c,v 9.0 2001/01/09 02:55:51 john Exp $";
#endif

#include "dbsql_e.h"

static matrix_compare();

/*
 * sort_matrix - takes a reduced incidence matrix and sorts it by type/overlap
 *	also, converts the Target list (-1) to a type 4
 */

   sort_matrix(h)
      mat_head *h;

      {
         int found = 0, i, matrix_compare(), n, count;
         v_matrix *v, *q, *vmem, *mlist, *nv, *pv;

         mlist = h->m_head;
         count = countlist(h->m_vars);
         /* also, converts the Target list (-1) to a type V_TARGET (4) */
         for (v = mlist; v != NULL; v = v->v_next){
            for (i = 0; i < MAXCLAUSES && v->v_list[i] != 0; i++)
               if (v->v_list[i] == -1){		/* -1 is target marker */
                  found = 1;
                  break;
                  }
            if (found == 1)
               break;
            }
         if (found){
            if (v->v_type == V_SINGLE){
               found = 0;
               /* find the first type V_DOUBLE (2) that matches */
               for (q = mlist; q != NULL; q = q->v_next)
                  if (q->v_type == V_DOUBLE && is_subset_matrix(v, q, count)){
                     found = 1;
                     break;
                     }
               /* OR find the first type V_NOREDUCE (3) that matches */
               for (q = mlist; !found && q != NULL; q = q->v_next)
                  if (q->v_type == V_NOREDUCE && is_subset_matrix(v,q,count)){
                     found = 1;
                     break;
                     }
               if (found){
                  for (i = 0; i < MAXCLAUSES; i++)
                     if (q->v_list[i] == 0){
                        q->v_list[i] = -1;
                        break;
                        }
                  delete_from_matrix(h, v);
                  v = q;
                  }
               }
            v->v_type = V_TARGET;
            }

         /* convert to a fixed structure for use with qsort */
         n = v_countmatrix(mlist);
         vmem = (v_matrix *) fb_malloc(n * sizeof(v_matrix));
         for (v = mlist, q = vmem; v != NULL; v = v->v_next, q++)
            *q = *v;
         qsort((char *) vmem, n, sizeof(v_matrix), matrix_compare);
         /* now copy back into mlist - in place */
         for (v = mlist, q = vmem, i = 0; i < n; v = v->v_next, q++, i++){
            nv = v->v_next;
            pv = v->v_prev;
            *v = *q;
            v->v_next = nv;
            v->v_prev = pv;
            }
         fb_free((char *) vmem);
      }

/*
 * matrix_compare - the comparison function used by qsort()
 */
 
   static matrix_compare(a, b)
      v_matrix *a, *b;

      {
         if (a->v_type < b->v_type)
            return(-1);
         if (a->v_type > b->v_type)
            return(1);
         /* must be equal - sort by overlap */
         if (a->v_overlap < b->v_overlap)
            return(-1);
         if (a->v_overlap > b->v_overlap)
            return(1);
         /* must be equal - return 0 */
         return(0);
      }

/*
 * is_subset_matrix - is a a subset matrix element of b ?
 *	note: if a is the same as b, refuse to admit its a subelement.
 */

   is_subset_matrix(a, b, count)
      v_matrix *a, *b;
      int count;

      {
         int i;

         if (a == b)
            return(0);
         for (i = 0; i < count; i++)
            if (a->v_array[i] == 1 && b->v_array[i] != 1)
               return(0);
         return(1);
      }
