/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: whereeac.c,v 9.0 2001/01/09 02:55:52 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Whereeach_sid[] = "@(#) $Id: whereeac.c,v 9.0 2001/01/09 02:55:52 john Exp $";
#endif

#include "dbsql_e.h"

extern short int cdb_secure;

static int where_clause = 0;
static char *BADPAT = "bad regular expression: ";

static pre_restrict();

/*
 * whereeach - the routines to walk through where clauses feeding
 *	projection or other mechanims with proper records.
 *	db is used for cdb_secure code to test permissions.
 */

   whereeach(dp, f, res)
      fb_database *dp;
      int (*f)();
      node *res;

      {
         long rec, save_rec;
         short int first_eval = 1;

         save_rec = dp->rec;
	 for (rec = 1; rec <= dp->reccnt; rec++){
	    if (u_getrec(rec, dp) == FB_ERROR)
	       fb_xerror(FB_FATAL_GETREC, dp->dbase, (char *) &rec);
	    if (!(FB_ISDELETED(dp)) && test_restrict(res, dp) == FB_AOK){
               if (first_eval){
                  eval_functions = 1;
                  first_eval = 0;
                  }
               else
                  eval_functions = 0;
	       if (((*f)(dp)) == FB_ERROR)
		  return(FB_ERROR);
               }
	    }
         if (save_rec > 0)
            u_getrec(save_rec, dp);
         eval_functions = 1;
         return(FB_AOK);
      }

/*
 * wherexeach - foreach record in dp's index, get record and call f(dp)
 *	as long as f(dp) returns non FB_ERROR code or until eof of idx.
 */

   wherexeach(dp, f, res)
      fb_database *dp;
      int (*f)();
      node *res;
      
      {
         long rec, save_rec;
         int max, first_eval = 1;
         char crec[FB_RECORDPTR+1];
	 
         save_rec = dp->rec;
	 lseek(dp->ifd, 0L, 0);
         fb_r_init(dp->ifd);
         max = dp->irecsiz + 13;
         while (fb_nextline(dp->irec, max) != 0){
            fb_ffetch(dp->ip[((dp->ifields) - 1)], crec, dp->irec, dp);
            rec = atol(crec);
            if (rec > 0L && rec <= dp->reccnt){	/* ignore rec of 0 */
	       if (u_getrec(rec, dp) == FB_ERROR)
	          fb_xerror(FB_FATAL_GETREC, dp->dbase, (char *) &rec);
	       if (!(FB_ISDELETED(dp)) && test_restrict(res, dp) == FB_AOK){
                  if (first_eval){
                     eval_functions = 1;
                     first_eval = 0;
                     }
                  else
                     eval_functions = 0;
	          if (((*f)(dp)) == FB_ERROR)
	             return(FB_ERROR);
                  }
	       }
	    }
         if (save_rec > 0)
            u_getrec(save_rec, dp);
         eval_functions = 1;
         return(FB_AOK);
      }

/*
 * set_where - set the where restrictions, evalute some if possible.
 */

   set_where(res)
      node *res;

      {
         node *n, *t;
         int g_where = 0;

         where_clause = 0;
         for (n = res->n_narg[1]; n != NULL; n = n->n_narg[1]){
            t = n->n_narg[0];
            if (t == NULL || t->n_type == S_NULL)
               break;
            if (pre_restrict(t) == FB_ERROR)
               return(FB_ERROR);
            g_where = 1;
            }
         where_clause = g_where;	/* set after pre_restrict */
#if DEBUG
         if (traceflag > 7){
            fprintf(stderr, "\n\n after pre_where() \n");
            trace_canon(res);
            }
#endif /* DEBUG */
         return(FB_AOK);
      }

/*
 * test_restrict - test the canonical tree, res, against the loaded record
 *	cdb_secure uses the fb_database dp to test permissions.
 */

   test_restrict(res, dp)
      node *res;
      fb_database *dp;

      {
         node *n, *t;
         float expr();
         int st = FB_AOK;

         if (cdb_secure && dp != (fb_database *) NULL &&
               fb_record_permission(dp, READ) == FB_ERROR)
            return(FB_ERROR);
         if (where_clause == 0)
            return(FB_AOK);
         eval_functions = 0;	/* done already */
         for (n = res->n_narg[1]; n != NULL; n = n->n_narg[1]){
            t = n->n_narg[0];
            if (t == NULL || t->n_type == S_NULL)
               break;
            if (expr(t) == 0){
               st = FB_ERROR;
               break;
               }
            }
         eval_functions = 1;
         return(st);
      }

/*
 * pre_restrict - pre-process n
 *	- replace any F_ (function) calls with constant equivalents
 *	- locate any LIKE predicates, and set up the compile string
 *		store this as a char * into c_p1;
 *	- do any sub_query processing
 */

   static pre_restrict(n)
      node *n;

      {
         cell *c, *sinstall(), *cn;
         float function();
         char sval[FB_MAXLINE], *p, *q;
         node *nn, *x_restrict, *x_slist, *x_from;

         c = (cell *) n->n_obj;
         if (n->n_type == P_LIKE || n->n_type == P_NOT_LIKE){
            npatterns++;
            pre_restrict(n->n_narg[0]);
            /* narg[1] is the pattern specifier */
            nn = n->n_narg[1];
            cn = (cell *) nn->n_obj;
            p = nn->n_nval;
            q = sval;
            *q++ = '^';
            for (; *p; p++){
               if (*p == CHAR_PERCENT){
                  *q++ = CHAR_DOT;
                  *q++ = CHAR_STAR;
                  }
               else if (*p == FB_UNDERSCORE)
                  *q++ = CHAR_DOT;
               else
                  *q++ = *p;
               }
            *q = NULL;
            if (re_comp(sval) != NULL){
               fb_serror(FB_MESSAGE, BADPAT, cn->c_sval);
               return(FB_ERROR);
               }
#if SYS_V
            nn->n_p1 = (int) cdb_Re_pat;
#endif /* SYS_V */
            /* now install the new pattern and save it for multiple LIKEs */
            c = sinstall(sval);
            fb_mkstr(&(nn->n_nval), c->c_sval);
            }
         else if (n->n_type == S_SUBQ && n->n_list == NULL){
            /* save the globals needed for outer layers */
            x_slist = g_slist;
            x_restrict = g_restrict;
            x_from = g_from;
            query_select(n, 1);
            /* restore the globals needed for outer layers */
            g_slist = x_slist;
            g_restrict = x_restrict;
            g_from = x_from;
            }
         else if (istype_bop(n)){
            pre_restrict(n->n_narg[0]);
            pre_restrict(n->n_narg[1]);
            }
         else if (istype_uop(n)){
            pre_restrict(n->n_narg[0]);
            }
         else if (istype_top(n)){
            pre_restrict(n->n_narg[0]);
            pre_restrict(n->n_narg[1]);
            pre_restrict(n->n_narg[2]);
            }
         else if (istype_lop(n)){
            pre_restrict(n->n_narg[0]);
            for (nn = n->n_narg[1]; nn != NULL; nn = nn->n_list)
               pre_restrict(nn);
            }
         else if (istype_fcn(n))
            function(n);
         return(FB_AOK);
      }
