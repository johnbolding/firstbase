/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: u_subpro.c,v 9.0 2001/01/09 02:55:52 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char U_project_sid[] = "@(#) $Id: u_subpro.c,v 9.0 2001/01/09 02:55:52 john Exp $";
#endif

#include "dbsql_e.h"

static void do_sub_project();

/*
 * u_sub_project - project the sel_list according to the canonical res tree
 *	the assumption at this point is that all requests are in a
 *	single variable state, i.e., only one dbase.
 *
 *	u_sub_project builds a sel_list for use with outside layers.
 *	this list is based in the current node n.
 */

   u_sub_project(selq, sel_list, where_cform, from, cur_n)
      node *selq, *sel_list, *where_cform, *from, *cur_n;

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
         if (selq->n_type != Q_DISTINCT)
            whereeach(dp, one_sub_project, where_cform);
         else if ((st = u_distinct(selq, sel_list, dp, where_cform)) != FB_ERROR){
            if ((st = fb_openidx(selq->n_virlist->n_nval, dp)) != FB_ERROR)
               fb_forxeach(dp, one_sub_project);
            }
         if (st == FB_ERROR)
            fb_serror(FB_MESSAGE, "Distinct Project - distinct set failed", NIL);
         else
            cur_n->n_list = sub_list;
         /* restore the globals needed for outer layers */
         g_slist = x_slist;
         g_restrict = x_restrict;
         g_from = x_from;
         return(st);
      }

/*
 * one_sub_project - first layer of one sub projection, from foreach/whereach
 */

   one_sub_project(dp)
      fb_database *dp;

      {
         do_sub_project(g_slist);
         if (!full_height(g_slist))
            return(FB_ERROR);
         else
            return(FB_AOK);
      }

/*
 * do_sub_project - do only the first in the slist by definition of SUBQ
 */

   static void do_sub_project(slist)
      node *slist;

      {
         node *n, *vnode(), *nn;
         cell *nc, *sinstall();
         char aline[FB_MAXLINE], *p;
         int ntype, size;
         float fval = 0;
         fb_field *f;
         fb_database *d;
         
         if ((n = slist) == NULL)
            return;
         expr(n);
         if (is_null(n))
            return;
         if (istype_fld(n)){
            d = (fb_database *) n->n_p1;
            f = (fb_field *) n->n_p2;
            fb_fetch(f, cdb_bfld, d);
            if (n->n_width != 0)
               size = n->n_width;
            else
               size = f->size;
            fb_formfield(cdb_afld, cdb_bfld, f->type, size);
            if (FB_OFNUMERIC(f->type))
               ntype = V_FCON;
            else
               ntype = V_SCON;
            p = cdb_afld;
            fval = n->n_fval;
            }
         else if (istype_str(n)){
            sprintf(aline, "%s", n->n_nval);
            ntype = V_SCON;
            p = aline;
            }
         else{
            sprintf(aline, "%.*f", n->n_scale, n->n_fval);
            ntype = V_FCON;
            p = aline;
            fval = n->n_fval;
            }
         /* make the list element */
         nc = sinstall(p);
         nn = vnode(ntype, (int) nc);
         nn->n_tval |= T_CON;
         if (ntype == V_SCON){
            nn->n_tval |= T_STR;
            fb_mkstr(&(nn->n_nval), nc->c_sval);
            }
         nn->n_fval = fval;
         /* link element in place */
         if (sub_list == NULL)
            sub_list = sp = nn;
         else{
            sp->n_list = nn;
            sp = nn;
            }
      }
