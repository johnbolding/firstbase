/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: cr_view.c,v 9.1 2001/02/16 19:45:35 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Createview_sid[] = "@(#) $Id: cr_view.c,v 9.1 2001/02/16 19:45:35 john Exp $";
#endif

#include "dbsql_e.h"

static move_database();
static change_field_names();
static test_view_files();

/*
 * createview - create a view for dbsql.
 *	Force the creation of a virtual dbase
 *      Move virtual dbase to known one specified
 *	Change names to match those in column list specified
 */

   createview(v)
      node *v;

      {
         char *nname;
         cell *c;
         node *nlist, *n, *pn, *nquery;
         fb_database *tp = NULL;
         int st;

         c = (cell *) v->n_narg[0]->n_obj;
         nname = c->c_sval;
         nlist = v->n_narg[1];
         nquery = v->n_narg[2];

         save_virtual = 1;
         if (interactive && test_view_files(nname) == FB_ERROR){
            fprintf(stdout, "CREATE VIEW not done.\n");
            return(FB_ERROR);
            }
         st = query_select(nquery, 0);
         if (st != FB_AOK){
            fb_serror(FB_MESSAGE, "SELECT part of CREATE VIEW command failed.", NIL);
            return(st);
            }

         /* now locate vp in the vir_dbase list */
         pn = NULL;
         for (n = vir_dbase; n != NULL; n = n->n_virlist){
            tp = (fb_database *) n->n_p1;
            if (tp == vp){
               /* remove from the virtual list */
               if (pn == NULL)
                  vir_dbase = n->n_virlist;
               else
                  pn->n_virlist = n->n_virlist;
               break;
               }
            tp = NULL;
            pn = n;
            }
         if (tp != vp){
            fb_serror(FB_MESSAGE, "Could not find virtual database", vp->dbase);
            return(FB_ERROR);
            }

         fb_closedb(vp);

         /* now move virtual database to requested location
          * also adjust field names using nlist at the same time.
         */
         st = move_database(nname, n->n_nval, nlist);
         if (st != FB_AOK)
            fb_serror(FB_MESSAGE, "CREATE VIEW command failed.", NIL);
            
         save_virtual = 0;
         return(st);
      }

/*
 * move_database - move database oname to nname, change field
 *	names using nlist during the process.
 */

   static move_database(nname, oname, nlist)
      char *nname, *oname;
      node *nlist;

      {
         char n_tname[FB_MAXNAME], o_tname[FB_MAXNAME];

         sprintf(o_tname, SYSMSG[S_FMT_2S], oname, SYSMSG[S_EXT_CDB]);
         sprintf(n_tname, SYSMSG[S_FMT_2S], nname, SYSMSG[S_EXT_CDB]);
         if (link(o_tname, n_tname) < 0){
            if (fb_copyfile(o_tname, n_tname) < 0){
	       fb_serror(FB_MESSAGE, "Could not copy to: ", n_tname);
               return(FB_ERROR);
               }
            }
         unlink(o_tname);

         sprintf(o_tname, SYSMSG[S_FMT_2S], oname, SYSMSG[S_EXT_DDICT]);
         sprintf(n_tname, SYSMSG[S_FMT_2S], nname, SYSMSG[S_EXT_DDICT]);
         if (change_field_names(o_tname, n_tname, nlist) == FB_ERROR){
	    fb_serror(FB_MESSAGE, "Could not create data dictionary: ", n_tname);
            return(FB_ERROR);
            }
         unlink(o_tname);

         sprintf(o_tname, SYSMSG[S_FMT_2S], oname, SYSMSG[S_EXT_MAP]);
         sprintf(n_tname, SYSMSG[S_FMT_2S], nname, SYSMSG[S_EXT_MAP]);
         if (link(o_tname, n_tname) < 0){
            if (fb_copyfile(o_tname, n_tname) < 0){
	       fb_serror(FB_MESSAGE, "Could not copy to: ", n_tname);
               return(FB_ERROR);
               }
            }
         unlink(o_tname);

         sprintf(o_tname, SYSMSG[S_FMT_2S], oname, SYSMSG[S_EXT_LOG]);
         sprintf(n_tname, SYSMSG[S_FMT_2S], nname, SYSMSG[S_EXT_LOG]);
         if (link(o_tname, n_tname) < 0)
            fb_copyfile(o_tname, n_tname);
         unlink(o_tname);
         unlink(oname);		/* last remnant - the temp file marker */
         return(FB_AOK);
      }

/*
 * change_field_names - change the fields names from fname to tname by the
 *	list provided n and its n_list
 */

   static change_field_names(fname, tname, n)
      char *fname, *tname;
      node *n;

      {
         char buf[FB_MAXNAME], line[FB_MAXLINE], word[FB_MAXLINE];
         int ffd, tfd, i, st = FB_AOK;
         cell *c;

         if (n->n_type == S_NULL){
            if (link(fname, tname) < 0)
               if (fb_copyfile(fname, tname) < 0)
                  return(FB_ERROR);
            return(FB_AOK);
            }
         ffd = open(fname, READ);
         if (ffd < 0)
            return(FB_ERROR);
	 close(creat(tname, 0666));
         tfd = open(tname, WRITE);
         while (fb_getlin(line, ffd, FB_MAXLINE) != EOF){
            if (n == NULL){
               st = FB_ERROR;
               fb_serror(FB_MESSAGE, "Column List does not Match Select List", NIL);
               break;
               }
            i = fb_getword(line, 1, word);
            c = (cell *) n->n_obj;
            strcpy(word, c->c_sval);
            word[FB_TITLESIZE] = NULL;
            sprintf(buf, "%s %s", word, line + i);
            write(tfd, buf, strlen(buf));
            n = n->n_list;
            }
         close(ffd);
         close(tfd);
         return(st);
      }

/*
 * test_view_files - test existance of database files, pause if they exist
 */

   static test_view_files(fname)
      char *fname;

      {
         short int file_exists = 0;
         char tname[FB_MAXNAME];

         sprintf(tname, SYSMSG[S_FMT_2S], fname, SYSMSG[S_EXT_CDB]);
         if (access(tname, 0) == 0)
            file_exists = 1;
         sprintf(tname, SYSMSG[S_FMT_2S], fname, SYSMSG[S_EXT_DDICT]);
         if (access(tname, 0) == 0)
            file_exists = 1;
         if (!file_exists)
            return(FB_AOK);
         fprintf(stdout,
            "OVERWRITE database object `%s' ? (y=yes, other=no)? ",
            fname);
         fflush(stdout);
         fgets(tname, 10, stdin);
         fb_rmlead(tname);
         if (tname[0] != CHAR_Y && tname[0] != CHAR_y)
            return(FB_ERROR);
         return(FB_AOK);
      }
