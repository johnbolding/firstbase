/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: u_ord_by.c,v 9.0 2001/01/09 02:55:52 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char U_order_by[] = "@(#)u_ord_by.c	8.1 8/19/93 FB";
#endif

#include "dbsql_e.h"

/*
 * u_order_by - generate an order by index and store its name in iname.
 * 	list_type controls a standard list, 0, or an order by list, 1.
 */

   u_order_by(iname, dp, ob_list, res, list_type)
      char *iname;
      fb_database *dp;
      node *ob_list, *res;
      int list_type;

      {
         node *qn, *u_vir_index(), *n, *tn;
         int vir_enter(), i, num;
         fb_field *fb_findfield();
         cell *c;
         char com[FB_MAXLINE], rev[10];

         qn = u_vir_index();
         strcpy(iname, qn->n_nval);
         if (init_vir_index(qn, dp) == FB_ERROR)
            return(FB_ERROR);
         vir_by[0] = NULL;
         for (n = ob_list, i=0; n != NULL && i < FB_MAXBY; n = n->n_list, i++){
            /* list_type == 1 means an order by clause,
             * list_type == 0 means a flat list, like group by
             */
            if (list_type == 1)
               tn = n->n_narg[0];
            else
               tn = n;
            c = (cell *) tn->n_obj;
            if (tn->n_type == V_ID){
               vir_by[i] = fb_findfield(c->c_sval, dp);
               }
            else{
               num = atoi(c->c_sval) - 1;
               if (num < dp->nfields && num > 0)
                  vir_by[i] = dp->kp[num];
               }
            if (vir_by[i] == NULL){
               fb_serror(FB_MESSAGE,
                  "Could not set virtual orderby/groupby field:", c->c_sval);
               return(FB_ERROR);
               }
            else if (vir_by[i]->size > 300){
               fb_serror(FB_MESSAGE,
                  "Cannot Use fields with NEWLINES for index/orderby/groupby:",
                  c->c_sval);
               return(FB_ERROR);
               }
            }
         virtual_count = 0;
         if (res == (node *) NULL)
            fb_foreach(dp, vir_enter);
         else
            whereeach(dp, vir_enter, res);
         end_vir_index(dp);
         rev[0] = NULL;
         n = ob_list->n_narg[1];
         if (n != NULL && n->n_type == Q_DESC)
            strcpy(rev, "-r");
	 sprintf(com, "sort %s -o %s %s", rev, dp->dindex, dp->dindex);
	 fb_system(com, FB_WITHROOT);
         return(FB_AOK);
      }
