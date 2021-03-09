/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: r_projec.c,v 9.0 2001/01/09 02:55:49 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char R_project_sid[] = "@(#) $Id: r_projec.c,v 9.0 2001/01/09 02:55:49 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * r_project - project the sel_list according to the relation provided.
 */

   r_project(selq, sel_list, where_cform, from, group_by, having,
         order_by)
      node *selq, *sel_list, *where_cform, *from, *group_by;
      node *having, *order_by;

      {
         fb_database *gp, *fp;
         int one_project(), st, blind_project(), o_flag = 0, g_flag = 0;
         int d_flag = 0;
         node *u_vir_dbase(), *sn;
         char iname[FB_MAXNAME], gname[FB_MAXNAME];
         relation *q, *r_distinct();

         if (sel_list == NULL){
            fb_serror(FB_MESSAGE, "R_Project - Null selection list", NIL);
            return(FB_ERROR);
            }
         do_headers(sel_list);
         g_slist = sel_list;
         g_restrict = where_cform;
         g_from = from;
         g_order_by = order_by;
         g_group_by = group_by;
         group_expr = 0;
         st = FB_AOK;
         gp = vp = NULL;

         if (order_by->n_type != S_NULL)
            o_flag = 1;				/* order by flag */
         if (group_by->n_type != S_NULL)
            g_flag = 1;				/* group by flag */
         if (selq->n_type == Q_DISTINCT)
            d_flag = 1;				/* distinct flag */

         if (d_flag && g_flag){
            fb_serror(FB_MESSAGE, "Cannot do Distinct And Group By", NIL);
            d_flag = 0;
            }

         /* in my implementation, groupby implies orderby - disable orderby */
         if (g_flag && o_flag)
            o_flag = 0;

         if (g_flag){
            /* create group by psuedo index for later use, store in gname */
            st = r_order_by(gname, r_head, group_by);
            if (st == FB_ERROR)
               return(FB_ERROR);
            }

         if (d_flag){
            q = r_distinct(r_head, sel_list);
            if (q == NULL)
               st = FB_ERROR;
            else
               q->r_next = r_head->r_next;
            r_head = q;
            }

         /* virtual for orderby, groupby, and for create view command */
         if (o_flag || g_flag || save_virtual)
            create_virtual = 1;
         else
            create_virtual = 0;

         if (create_virtual)
            set_virtual_dbase();

         if (g_flag){
            /*
             * group by
             *    generate a virtual database using the psuedo index
             *    from gname.
             */
            if ((st = openidx_relation(r_head)) != FB_ERROR)
               st = r_groupby(r_head, sel_list, having, from);
            if (st == FB_ERROR)
               fb_serror(FB_MESSAGE, "Group By Failed", NIL);
            }
         else if (d_flag){
            /* distinct */
            if (st != FB_ERROR)
               relateeach(r_head, one_project);
            if (st == FB_ERROR)
               fb_serror(FB_MESSAGE, "Distinct Project - distinct set failed", NIL);
            }
         else
            relateeach(r_head, one_project);

         if (st != FB_ERROR && create_virtual){
            end_virtual_dbase();
            /* save the first virtual database into fp */
            fp = vp;
            sn = vn;
            sn->n_fval = 1;
            if (fb_opendb(fp, READWRITE, FB_ALLINDEX, FB_OPTIONAL_INDEX) == FB_ERROR){
               fb_serror(FB_MESSAGE, "Cannot open virtual database:", fp->dbase);
               st = FB_ERROR;
               }
            if (st != FB_ERROR){
               if (o_flag){
                  st = u_order_by(iname, fp, order_by, (node *) NULL, 1);
                  if (st != FB_ERROR)
                     st = fb_openidx(iname, fp);
                  if (st != FB_ERROR && save_virtual)
                     set_virtual_dbase();
                  if (st != FB_ERROR)
                     fb_forxeach(fp, blind_project);
                  else
                     fb_serror(FB_MESSAGE, "Project - OrderBy failed", NIL);
                  }
               else if (g_flag){
                  if (save_virtual)
                     set_virtual_dbase();
                  fb_foreach(fp, blind_project);
                  }
               }
               if (save_virtual && (o_flag || g_flag)){
                  end_virtual_dbase();
                  fb_closedb(fp);
                  if (fb_opendb(vp, READWRITE,FB_ALLINDEX, FB_OPTIONAL_INDEX) == FB_ERROR){
                     fb_serror(FB_MESSAGE, "Cannot open virtual database:",
                        vp->dbase);
                     st = FB_ERROR;
                     }
                  }
            }

         if (gp != NULL)
            fb_closedb(gp);
         if (vp != NULL && !save_virtual)
            fb_closedb(vp);
         return(st);
      }
