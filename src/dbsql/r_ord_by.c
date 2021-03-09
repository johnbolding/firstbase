/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: r_ord_by.c,v 9.0 2001/01/09 02:55:49 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char R_order_by_sid[] = "@(#) $Id: r_ord_by.c,v 9.0 2001/01/09 02:55:49 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * r_order_by - generate an order by index and store its name in iname.
 * 	Relation version. Generate an index on a relation!
 */

   r_order_by(iname, r, ob_list)
      char *iname;
      relation *r;
      node *ob_list;

      {
         node *qn, *u_vir_index(), *n;
         int vir_enter(), i, enter_rel_index();
         fb_field *fb_findfield();
         cell *c;
         char com[FB_MAXLINE];

         qn = u_vir_index();
         strcpy(iname, qn->n_nval);
         fb_mkstr(&(r->r_ibase), iname);
         if (init_rel_index(qn, r) == FB_ERROR)
            return(FB_ERROR);
         /* now set up the order by/group by fields */
         for (n = ob_list, i=0; n != NULL && i < FB_MAXBY; n = n->n_list, i++){
            c = (cell *) n->n_obj;
            if (n->n_type == V_ID)
               vir_by[i] = (fb_field *) n->n_p2;
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
         /* now generate the index on the relation */
         relateeach(r, enter_rel_index);
         end_rel_index(r);
	 sprintf(com, "sort -o %s %s", r->r_index, r->r_index);
	 fb_system(com, FB_WITHROOT);
         return(FB_AOK);
      }
