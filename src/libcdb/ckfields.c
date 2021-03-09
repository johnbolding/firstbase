/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: ckfields.c,v 9.1 2001/02/16 19:00:30 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Checkfields_sid[] = "@(#) $Id: ckfields.c,v 9.1 2001/02/16 19:00:30 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#if FB_PROTOTYPES
static check_autoidx(fb_database *db, int errflag);
#else /* FB_PROTOTYPES */
static check_autoidx();
#endif /* FB_PROTOTYPES */

extern char *cdb_T_AUTOINCR;
extern char *cdb_AUTOMARK;
extern short int cdb_skip_null_auto;
extern short int cdb_regexp;

extern long fb_megasearch(int fd, char *key, int col, long bsbeg, long bsend,
   long bsmax, int recsiz, int backup, char *buf);
extern long fb_btree_search(char *key, fb_bidx *bi, fb_bseq *bs);

/*
 * checkfields - check unique and range for each field that is a macro
 *	return FB_AOK if all checks out, else return FB_ERROR
 */
 
   fb_checkfields(db, errflag)
      fb_database *db;
      int errflag;

      {
	 long bsmax, bsend, urec;
         int st = FB_AOK, i;
         char msg[FB_MAXLINE];
         fb_field *f;
         int save_cdb_regexp;

         /* no regexp for uniq tests */
         save_cdb_regexp = cdb_regexp;
         cdb_regexp = 0;

         for (i = 0; i < db->nfields; i++){
            f = db->kp[i];
            if (f->aid != NULL && f->aid->uniq > 0){
	       if (f->idefault != NULL && equal(f->idefault, cdb_T_AUTOINCR) &&
		     equal(f->fld, cdb_AUTOMARK))
                  continue;
               if (cdb_skip_null_auto && fb_str_is_blanks(f->fld))
                  continue;
               strcpy(cdb_afld, f->fld);
               fb_makess(cdb_afld, f->type, f->size);
               /*
                * dup_fld is now formatted at save time. so must compare
                * against a formatted string (cdb_afld).
                */
               if (equal(cdb_afld, f->aid->dup_fld))
                  continue;

               fb_lock_head(db);
               /* b_tree handled if needed */
               if (db->b_tree)			/* btree index */
                  urec = fb_btree_search(cdb_afld, db->b_idx, db->b_seq);
               else{
                  fb_getxhead(f->aid->hfd, &bsmax, &bsend);
                  urec = fb_megasearch(f->aid->afd, cdb_afld, 0, 1L, bsend,
                     bsmax, f->size + FB_RECORDPTR + 1, 0, db->irec);
                  }
               fb_unlock_head(db);
               if (urec > 0){
                  st = FB_ERROR;
                  if (errflag){
                     sprintf(msg, "`%s' entry `%s' Is in Use. ", f->id,f->fld);
                     fb_fmessage(msg);
                     fb_serror(FB_MESSAGE, "UNIQUE entry Required!", NIL);
                     }
                  break;			/* value already exists */
                  }
               }
            if (f->range != NULL){
               strcpy(cdb_afld, f->fld);
               if (fb_checkrange(f, cdb_afld) == FB_ERROR){
                  st = FB_ERROR;
                  if (errflag){
                     sprintf(msg,
                        "`%s' entry `%s' Is out of range.", f->id, f->fld);
                     fb_fmessage(msg);
                     fb_serror(FB_MESSAGE, NIL, NIL);
                     }
                  break;			/* value out of range */
                  }
               }
            }
         if (st == FB_AOK)
            st = check_autoidx(db, errflag);
         /* restore cdb_regexp */
         cdb_regexp = save_cdb_regexp;
         return(st);
      }

   static check_autoidx(db, errflag)
      fb_database *db;
      int errflag;

      {
         int st = FB_AOK, j, i;
         fb_autoindex *ix;
         long urec;
         char msg[FB_PCOL];
         fb_field *fp;
         
         for (j = 0; j < db->b_maxauto; j++){
            ix = db->b_autoindex[j];
            if (ix->uniq <= 0)
               continue;
            /* generate a key field to compare to dup_fld */
            ix->ix_key_fld[0] = NULL;
            for (i = 0; i < ix->ix_ifields; i++){
               fp = ix->ix_ip[i];
               strcpy(cdb_afld, fp->fld);
               fb_makess(cdb_afld, fp->type, fp->size);
               strcat(ix->ix_key_fld, cdb_afld);
               }
            if (cdb_skip_null_auto && fb_str_is_blanks(ix->ix_key_fld))
               continue;
            if (!equal(ix->ix_key_fld, ix->dup_fld)){
               fb_lock_head(db);
               /* all auto structs are btrees */
               urec = fb_btree_search(ix->ix_key_fld, ix->ix_idx, ix->ix_seq);
               fb_unlock_head(db);
               if (urec > 0){
                  st = FB_ERROR;
                  if (errflag){
                     sprintf(msg, "duplicate entry: index:%s value:`%s'",
                        ix->autoname, ix->ix_key_fld);
                     fb_fmessage(msg);
                     fb_serror(FB_MESSAGE, "UNIQUE entry Required!", NIL);
                     }
                  break;			/* value already exists */
                  }
               }
            }
         return(st);
      }
