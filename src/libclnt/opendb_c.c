/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: opendb_c.c,v 9.0 2001/01/09 02:56:36 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Opendb_clnt_sid[] = "@(#) $Id: opendb_c.c,v 9.0 2001/01/09 02:56:36 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

extern short int cdb_fixedwidth;
extern short int cdb_initscreen_done;
extern short int cdb_datedisplay;
extern short int cdb_returnerror;
extern short int cdb_error;

/*
 *  fb_opendb_clnt - open a fb_database using a given mode.
 *
 *	- if ixflag == FB_NOINDEX, open index also.
 *	- if ixflag == FB_WITHINDEX, open index also.
 *	- if ixflag == FB_ALLINDEX, open index and autoindexes.
 *
 *      - if ixoption == FB_OPTIONAL_INDEX, return
 *	- if ixoption == FB_MAYBE_OPTIONAL_INDEX, fb_serror out
 *	- if ixoption == FB_MUST_INDEX, fb_xerror out
 */

   fb_opendb_clnt(dp, mode, ixflag, ixoption)
      fb_database *dp;
      int mode, ixflag, ixoption;
      
      {
         static fb_varvec v, *r;
         int st = FB_AOK, i, l_fieldlength = 0, firstline, lastline, linep, loc;
         int isiz, p, nline, ifields;
         char dname[FB_MAXNAME], iname[FB_MAXNAME], s_mode[5];
         char *fb_argvec(fb_varvec *v, int k);
         char s_ixflag[5], s_ixoption[5], line[FB_MAXLINE], *nameline;
         char buf[FB_MAXLINE], aname[FB_MAXLINE];
         fb_field *f, *fb_makefield(void);
         fb_field *fb_findfield(char *s, fb_database *hdb);
         fb_autoindex *fb_ixalloc(void), *ax;

         if (cdb_t_lines == 0 || cdb_initscreen_done == 0)
	    cdb_batchmode = 1;
         if (dp == NULL){
	    if (cdb_returnerror){
	       cdb_error = FB_BAD_DATA;
	       return(FB_ERROR);
	       }
	    fb_xerror(FB_BAD_DATA, SYSMSG[S_NO_INDEX], NIL);
	    }
	 fb_setup();		/* process /usr/lib/.cdbrc setup file */
         dp->fixedwidth = cdb_fixedwidth;
         /*
          * arguments to opendb_svc are:
          *    opendb dbase index mode ixflag ixoption
          */
         dname[0] = iname[0] = NULL;
         if (dp->dbase != NULL){
            fb_basename(buf, dp->dbase);
            fb_dirname(dname, dp->dbase);
            strcat(dname, buf);
            }
         if (dp->dindex != NULL){
            fb_basename(buf, dp->dindex);
            fb_dirname(iname, dp->dindex);
            strcat(iname, buf);
            }
         sprintf(s_mode, "%d", mode);
         sprintf(s_ixflag, "%d", ixflag);
         sprintf(s_ixoption, "%d", ixoption);
         fb_loadvec(&v, R_OPENDB, dname, iname, s_mode, s_ixflag,
            s_ixoption, 0);
         r = fb_toserver(&v);
         /* interpret the results */
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "opendb_clnt - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from opendb_svc are:
             *    - nargs
             *    - status
             *    - SID - fb_database server id
             *    - fb_field names (name,type,size|...)
             *    - recsiz
             *    - nfields
             *    - irecsiz
             *    - ifields
             *    - reccnt
             *    - delcnt
             *    - b_maxauto
             *    - ix->autoname,ix->ix_ifields,fb_field0, ... ,fb_field N-1
             *    -  ... ... ...
             */
            st = atoi(fb_argvec(r, 1));
            dp->b_sid = atoi(fb_argvec(r, 2));
            /* fields are in arg 3 - dealt with below */
            dp->recsiz = atoi(fb_argvec(r, 4));
            dp->nfields = atoi(fb_argvec(r, 5));
            dp->irecsiz = atoi(fb_argvec(r, 6));
            dp->ifields = atoi(fb_argvec(r, 7));
            dp->reccnt = atol(fb_argvec(r, 8));
            dp->delcnt = atol(fb_argvec(r, 9));
            dp->b_maxauto = atoi(fb_argvec(r, 10));
            /* autoindex fields in 11 .. N are handled below */

            /* now simulate the getd_dict allocation noise */
            cdb_keymap = (fb_field **)
              (fb_malloc((unsigned) ((dp->nfields+1) * (sizeof(fb_field *)))));
            /* set up the pointer to the fb_field pointers */
            cdb_kp = dp->kp = cdb_keymap;
            nameline = fb_argvec(r, 3);
            for (i = 0, l_fieldlength = 0; i <= dp->nfields; i++){
               f = fb_makefield();
               dp->kp[i] = f;
               fb_subline(line, nameline, i + 1, '|');
               fb_subline(buf, line, 1, ',');
               fb_mkstr(&(f->id), buf);
               fb_subline(buf, line, 2, ',');
               f->type = buf[0];
               fb_subline(buf, line, 3, ',');
               f->size = atoi(buf);
               l_fieldlength = MAX(l_fieldlength, f->size);
               }
            /* and the general buffers */
            cdb_afld = (char *) fb_malloc((unsigned) (l_fieldlength + 11));
            cdb_bfld = (char *) fb_malloc((unsigned) (l_fieldlength + 11));
	    dp->afld = cdb_afld;
	    dp->bfld = cdb_bfld;
	    dp->irec = (char *) fb_malloc((unsigned) (dp->irecsiz + 13));
	    dp->orec = (char *) fb_malloc((unsigned) (dp->recsiz + 10));
	    dp->arec = (char *) fb_malloc((unsigned) (dp->recsiz + 10));
	    dp->orec[0] = dp->arec[0] = cdb_afld[0] = cdb_bfld[0] = NULL;

            /* allocate/parse the autoindex stuff
             *    - ix->autoname,ix->ix_ifields,fb_field0, ... ,fb_field N-1
             *    -  ... ... ...
             *    ... b_maxauto of them
             */
            firstline = 11; /* fixed in count due to args above */
            lastline = firstline + dp->b_maxauto - 1;
            /* allocate the auto index array */
            dp->b_autoindex = (fb_autoindex **)
               fb_malloc((unsigned)
                  ((dp->b_maxauto + 1) * sizeof(fb_autoindex)));
            for (i = 0, nline = firstline; nline <= lastline; nline++, i++){
               nameline = fb_argvec(r, nline);
               fb_subline(aname, nameline, 1, CHAR_COMMA);
               fb_subline(buf, nameline, 2, CHAR_COMMA);
               ifields = atoi(buf);
               ax = dp->b_autoindex[i] = fb_ixalloc();
               fb_mkstr(&(ax->autoname), aname);
               ax->ix_ifields = ifields;
               ax->ix_tree = 1;
               /*
                * now make ax->ix_ip and then loop over 1 ... ifields of
                *    comma seperated from nameline - get fb_field ptr and store.
                */
               ax->ix_ip = (fb_field **)
                  fb_malloc((unsigned) ((ifields+1) * (sizeof(fb_field *))));
               for (linep = 3, loc = 0, p = 0; p < ifields; p++, linep++){
                  fb_subline(buf, nameline, linep, CHAR_COMMA);
                  if ((ax->ix_ip[p] = fb_findfield(buf, dp)) != NULL){
                     loc += (ax->ix_ip[p]->size);
                     if (ax->ix_ip[p]->type == FB_DATE && cdb_datedisplay == 10)
                        loc += 2;		/* century space */
                     }
                  else{
                     if (cdb_returnerror)
                        fb_lerror(FB_BAD_DICT, aname, buf);
                     else
                        fb_serror(FB_BAD_DICT, aname, buf);
                     return(FB_ERROR);
                     }
                  }
               /* record pointer 'fb_field' - FB_RECORDPTR */
               ax->ix_ip[p] = (fb_field *)
                  fb_malloc((unsigned) sizeof(fb_field));
               ax->ix_ip[p]->size = 10;
               ax->ix_ip[p]->type = CHAR_n;
               ax->ix_ip[p]->loc = loc;
               ax->ix_ip[p]->id = NIL;
               /* isiz has index size (key size) */
               isiz = loc + ax->ix_ip[p]->size;
               ax->dup_fld = (char *) fb_malloc((unsigned) (isiz + 13));
                                                                  /*13=fudge*/
               ax->dup_fld[0] = NULL;
               ax->ix_key_fld = (char *) fb_malloc((unsigned) (isiz + 13));
               ax->ix_key_fld[0] = NULL;
               }
            }

         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

#endif /* RPC */
