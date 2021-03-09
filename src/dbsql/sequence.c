/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: sequence.c,v 9.0 2001/01/09 02:55:50 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Sequence_sid[] = "@(#) $Id: sequence.c,v 9.0 2001/01/09 02:55:50 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * sequence - tear apart h_from, building h_into with the pieces.
 *	take the first TWO variable component of h_from,
 *	along with ALL one variable components in the same variables,
 *	and generate a new matrix list into h_subq
 */

   sequence(h_subq, h_from)
      mat_head *h_subq, *h_from;

      {
         v_matrix *v, *q, *nq;
         int count;

         clear_mat_head(h_subq);
         count = countlist(h_from->m_vars);
         h_subq->m_vars = h_from->m_vars;
         for (v = h_from->m_head; v != NULL; v = v->v_next)
            if (v->v_type == V_DOUBLE || v->v_type == V_NOREDUCE ||
                  v->v_type == V_TARGET)
               break;
         if (v == NULL){
            /* must be only singles left -- locate first one */
            for (v = h_from->m_head; v != NULL; v = v->v_next)
               if (v->v_type == V_SINGLE)
                  break;
            }

         /* unlink/capture all V_SINGLEs which are a true subset of v */
         for (q = h_from->m_head; q != NULL; q = nq){
            nq = q->v_next;
            if (q->v_type != V_SINGLE)
               break;
            /* add it to the subq */
            if (is_subset_matrix(q, v, count)){
               unlink_from_matrix(h_from, q);
               add_to_matrix(h_subq, q);
               }
            }

         /* finally, unlink and add v itself */
         unlink_from_matrix(h_from, v);
         add_to_matrix(h_subq, v);
      }
