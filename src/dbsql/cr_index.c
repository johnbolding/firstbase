/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: cr_index.c,v 9.1 2001/02/16 19:45:35 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Createindex_sid[] = "@(#) $Id: cr_index.c,v 9.1 2001/02/16 19:45:35 john Exp $";
#endif

#include "dbsql_e.h"

static query_index();
static move_index();
static test_index_files();

/*
 * createindex - create an index for dbsql.
 *	Force the creation of a virtual index
 *      Move virtual index to known one specified.
 *	Works only for single variable fb_database requests.
 */

   createindex(v)
      node *v;

      {
         char *nname, iname[FB_MAXNAME];
         cell *c;
         int st;

         c = (cell *) v->n_narg[0]->n_obj;
         nname = c->c_sval;
         if (interactive && test_index_files(nname) == FB_ERROR){
            fprintf(stdout, "CREATE INDEX not done.\n");
            return(FB_ERROR);
            }
         st = query_index(v, iname);
         if (st != FB_AOK)
            fb_serror(FB_MESSAGE, "QUERY part of CREATE INDEX command failed.", NIL);
         else
            st = move_index(nname, iname);
        return(st);
     }

/*
 * query_index - generate the actual index
 */

   static query_index(n, iname)
      node *n;
      char *iname;

      {
         int st = FB_AOK;
         fb_database *dp;
         node *from, *order_by, *where_cform, *make_canon(), *where;

         from =      n->n_narg[1];
         order_by =  n->n_narg[2];
         where =     n->n_narg[3];
         where_cform = make_canon(where);
         relation_function = 0;

         if (st != FB_ERROR)
            st = u_opentables(from);
         if (st != FB_ERROR)
            st = u_verify_select(order_by, from);
         if (st != FB_ERROR)
            st = u_verify_tables(where_cform->n_list, from);
         if (st != FB_ERROR)
            st = u_verify_cform(where_cform, from);
         if (st != FB_ERROR)
            st = set_where(where_cform);
         if (st == FB_ERROR)
            return(FB_ERROR);

#if DEBUG
         if (traceflag > 5)
            trace_canon(where_cform);
#endif /* DEBUG */

         if (where_cform->n_tvarc > 1){
            fb_serror(FB_MESSAGE, "Cannot make an index on multiple databases.",NIL);
            return(FB_ERROR);
            }

         g_restrict = where_cform;
         g_from = from;
         g_order_by = order_by;
         group_expr = 0;

         dp = (fb_database *) from->n_p1;
         st = u_order_by(iname, dp, order_by, where_cform, 1);
         return(st);
      }

/*
 * move_index - move index oname to nname.
 */

   static move_index(nname, oname)
      char *nname, *oname;

      {
         char n_tname[FB_MAXNAME], o_tname[FB_MAXNAME];

	 sprintf(o_tname, SYSMSG[S_FMT_2S], oname, SYSMSG[S_EXT_IDX]);
	 sprintf(n_tname, SYSMSG[S_FMT_2S], nname, SYSMSG[S_EXT_IDX]);
         if (link(o_tname, n_tname) < 0){
            if (fb_copyfile(o_tname, n_tname) < 0){
	       fb_serror(FB_MESSAGE, "Could not copy to: ", n_tname);
               return(FB_ERROR);
               }
            }
         unlink(o_tname);

	 sprintf(o_tname, SYSMSG[S_FMT_2S], oname, SYSMSG[S_EXT_IDICT]);
	 sprintf(n_tname, SYSMSG[S_FMT_2S], nname, SYSMSG[S_EXT_IDICT]);
         if (link(o_tname, n_tname) < 0){
            if (fb_copyfile(o_tname, n_tname) < 0){
	       fb_serror(FB_MESSAGE, "Could not copy to: ", n_tname);
               return(FB_ERROR);
               }
            }
         unlink(o_tname);

         return(FB_AOK);
      }

/*
 * test_index_files - test existance of index files, pause if they exist
 */

   static test_index_files(iname)
      char *iname;

      {
         short int file_exists = 0;
         char tname[FB_MAXNAME];

         sprintf(tname, SYSMSG[S_FMT_2S], iname, SYSMSG[S_EXT_IDX]);
         if (access(tname, 0) == 0)
            file_exists = 1;
         sprintf(tname, SYSMSG[S_FMT_2S], iname, SYSMSG[S_EXT_IDICT]);
         if (access(tname, 0) == 0)
            file_exists = 1;
         if (!file_exists)
            return(FB_AOK);
         fprintf(stdout,
            "OVERWRITE index object `%s' ? (y=yes, other=no)? ",
            iname);
         fflush(stdout);
         fgets(tname, 10, stdin);
         fb_rmlead(tname);
         if (tname[0] != CHAR_Y && tname[0] != CHAR_y)
            return(FB_ERROR);
         return(FB_AOK);
      }
