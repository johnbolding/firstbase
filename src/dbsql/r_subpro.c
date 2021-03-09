/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: r_subpro.c,v 9.0 2001/01/09 02:55:49 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char R_project_sid[] = "@(#) $Id: r_subpro.c,v 9.0 2001/01/09 02:55:49 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * r_sub_project - project the sel_list according to the canonical res tree
 *	the assumption at this point is that the relation passed in is
 *	the final result, DISTINCT and ALL.
 *
 *	r_sub_project builds a sel_list for use with outside layers.
 *	this list is based in the current node n.
 */

   r_sub_project(r, sel_list, where_cform, from, cur_n)
      node *sel_list, *where_cform, *from, *cur_n;
      relation *r;

      {
         fb_database *dp;
         int one_sub_project(), st;
         node *dn;
         node *x_restrict, *x_slist, *x_from;

         if (sel_list == NULL){
            fb_serror(FB_MESSAGE, "Project - Null selection list", NIL);
            return(FB_ERROR);
            }
         dn = from;
         if (dn == NULL){
            fb_serror(FB_MESSAGE, "Project - Null database node", NIL);
            return(FB_ERROR);
            }
         dp = (fb_database *) dn->n_p1;
         if (dp == NULL){
            fb_serror(FB_MESSAGE, "Project - Null database pointer", NIL);
            return(FB_ERROR);
            }
         /* save the globals needed for outer layers */
         x_slist = g_slist;
         x_restrict = g_restrict;
         x_from = g_from;

         g_slist = sel_list;
         g_restrict = where_cform;
         g_from = from;
         st = FB_AOK;
         /* set_where was already done in layer above */
         sub_list = sp = NULL;

         st = relateeach(r, one_sub_project);
         cur_n->n_list = sub_list;
         /* restore the globals needed for outer layers */
         g_slist = x_slist;
         g_restrict = x_restrict;
         g_from = x_from;
         return(st);
      }
