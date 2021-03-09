/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: u_qsel.c,v 9.0 2001/01/09 02:55:52 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char U_qselect_sid[] = "@(#) $Id: u_qsel.c,v 9.0 2001/01/09 02:55:52 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * u_qselect - provide a qselect utility for single variable requests
 */

   u_qselect(selq, sel_list, from, where_cform, group_by, having,
         sub_flag, n, order_by)
      node *selq, *sel_list, *from, *where_cform, *group_by, *having, *n;
      node *order_by;
      int sub_flag;

      {
         int st = FB_AOK;

         relation_function = 0;
         if (st != FB_ERROR)
            st = u_opentables(from);
         if (st != FB_ERROR)
            st = u_verify_select(sel_list, from);
         if (st != FB_ERROR)
            st = u_verify_tables(where_cform->n_list, from);
         if (st != FB_ERROR)
            st = u_verify_cform(where_cform, from);
         if (st != FB_ERROR)
            st = set_where(where_cform);
         if (st == FB_ERROR)
            return(FB_ERROR);

         /* check for an invalid non-join query, i.e. 2+Q without where */
         if (st != FB_ERROR && countlist(from) > 1){
            fb_serror(FB_MESSAGE, "Invalid Query: No Join predicate specified.",NIL);
            st = FB_ERROR;
            }

         if (st != FB_ERROR){
            if (sub_flag)
               st = u_sub_project(selq, sel_list, where_cform, from, n);
            else
               st = u_project(selq, sel_list, where_cform, from, group_by,
                  having, order_by);
            }
         return(st);
      }
