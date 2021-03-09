/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dropidx.c,v 9.1 2001/02/16 19:45:36 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dropindex_sid[] = "@(#) $Id: dropidx.c,v 9.1 2001/02/16 19:45:36 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * dropindex - fb_delete a index for dbsql.
 *	If in interactive mode, ask before deleting, else just do it.
 */

   dropindex(v)
      node *v;

      {
         char *fname, tname[FB_MAXNAME];
         cell *c;

         c = (cell *) v->n_narg[0]->n_obj;
         fname = c->c_sval;
         if (interactive){
            fprintf(stdout,
               "Really DELETE index object `%s' ? (y=yes, other=no)? ",
               fname);
            fflush(stdout);
            fgets(tname, 10, stdin);
            fb_rmlead(tname);
            if (tname[0] != CHAR_Y && tname[0] != CHAR_y){
               fprintf(stdout, "DROP INDEX not done.\n");
               return(FB_ERROR);
               }
            }
         sprintf(tname, SYSMSG[S_FMT_2S], fname, SYSMSG[S_EXT_IDX]);
         unlink(tname);
         sprintf(tname, SYSMSG[S_FMT_2S], fname, SYSMSG[S_EXT_IDICT]);
         unlink(tname);
         return(FB_AOK);
      }
