/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: autoindx.c,v 9.0 2001/01/09 02:56:55 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Autoindex_sid[] = "@(#) $Id: autoindx.c,v 9.0 2001/01/09 02:56:55 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#if FB_PROTOTYPES
static autoupdate(fb_field *k, fb_database *hp, long rec);
#else
static autoupdate();
#endif /* FB_PROTOTYPES */

static char *FMT1 = 	"%010ld";
static char *ZEROS = 	"000000000";
extern short int cdb_use_rpc;
extern short int cdb_usrlog;
extern short int cdb_error;
extern short int cdb_skip_null_auto;

extern char *cdb_S_EOREC;

/*
 * autoindex - update all needed indexs. idea is to add to end if
 *     the record is new or that fb_field has been updated.
 *     if updated, the old values record pointer is zeroed out.
 *     if an index is past the threshold setting of overflow records,
 *     the index is sorted.
 *     mutual exclusion here is guaranteed since dbedit has
 *     record 0 locked at the moment. so, these autofiles
 *     should not be changing (due to FirstBase).
 */

   fb_put_autoindex(hp)
      fb_database *hp;
      
      {
         register int j, i;
         fb_field *fp;
         char lbuf[40];
         fb_autoindex *ix;
         int st = FB_AOK;
         long rec;
	 
         (void) Autoindex_sid;

#if RPC
         if (cdb_use_rpc)
            return(fb_put_autoindex_clnt(hp));
#endif /* RPC */
	 for (j = 0; j < hp->nfields; j++)
	    if (hp->kp[j]->aid != NULL && hp->kp[j]->aid->hfd > 0){
               /*
                * if (cdb_skip_null_auto && fb_str_is_blanks(hp->kp[j]->fld))
                *   break;
                */
               if (autoupdate(hp->kp[j], hp, hp->rec) == FB_ERROR)
                  return(FB_ERROR);
               /*
                * this has been moved to fb_set_autoindex(), and is called
                * from those that want to do multiple writes:
                *
                * dbvedit and dbedit - edit.c.
                *
                * strcpy(hp->kp[j]->aid->dup_fld, FB_FLD(j,hp));
                */
               }

         /* now check the dbase.auto structure */
         for (j = 0; j < hp->b_maxauto; j++){
            ix = hp->b_autoindex[j];
            /* generate a key fb_field to compare to dup_fld */
            ix->ix_key_fld[0] = NULL;
            for (i = 0; i < ix->ix_ifields; i++){
               fp = ix->ix_ip[i];
               strcpy(cdb_afld, fp->fld);
               fb_makess(cdb_afld, fp->type, fp->size);
               strcat(ix->ix_key_fld, cdb_afld);
               }
            /* this removes null entries from auto indexes if desired */
            /*
             * if (cdb_skip_null_auto && fb_str_is_blanks(ix->ix_key_fld))
             *    continue;
             */
            if (!equal(ix->ix_key_fld, ix->dup_fld)){
               sprintf(lbuf, FMT1, hp->rec);
               strcat(ix->ix_key_fld, lbuf);
               st = fb_btree_insert(ix, ix->ix_key_fld, hp);
               if (st == FB_ERROR){
                  fb_lerror(FB_MESSAGE,
                     "Error inserting into btree:", ix->autoname);
                  cdb_error = FB_BAD_INDEX;
                  break;
                  }
               /* iff current index, sync that index */
               if (j == hp->b_curauto){
                  rec = fb_btree_search(ix->ix_key_fld, hp->b_idx, hp->b_seq);
                  if (rec > 0 && rec <= hp->reccnt)
                     hp->bsrec = rec;
                  }
               }
            }
	 return(FB_AOK);
      }

/*
 *  autoupdate - updates a single fb_field that is either new or has changed.
 */
 
   static autoupdate(k, hp, rec)
      fb_field *k;
      fb_database *hp;
      long rec;
      
      {
         long bsmax, bsend, srec, drec;
         char lbuf[40], ubuf[FB_MAXLINE];
	 int slen, rlength, in_overflow = 0, found = 0;

	 rlength = k->size + FB_RECORDPTR + 1;

         /* build a index record using fb_makess() - store in afld */
         strcpy(cdb_afld, k->fld);
         fb_makess(cdb_afld, k->type, k->size);
         /*
          * dup_fld is now formatted at save time. so must compare
          * against a formatted string (cdb_afld).
          */
         if (equal(cdb_afld, k->aid->dup_fld))
            return(FB_AOK);

         sprintf(lbuf, FMT1, rec);
         strcat(cdb_afld, lbuf);
         if (k->aid->ix_tree == 1){
            return(fb_btree_insert(k->aid, cdb_afld, hp));
            }
         strcat(cdb_afld, SYSMSG[S_STRING_NEWLINE]);

	 /* get bsmax,bsend from index header file */
         fb_getxhead(k->aid->hfd, &bsmax, &bsend);

	 /* seek and write auto index entry */
         if (lseek(k->aid->afd, bsmax * (long) rlength, 0) < 0L){
	    fb_lerror(FB_SEEK_ERROR, hp->dbase, SYSMSG[S_BAD_INDEX]);
	    return(FB_ERROR);
	    }
	 if (write(k->aid->afd, cdb_afld, (unsigned) rlength) != rlength){
	    fb_lerror(FB_WRITE_ERROR, hp->dbase, SYSMSG[S_BAD_INDEX]);
	    return(FB_ERROR);
	    }

	 /* redo header -- note: only bsmax changed */
	 fb_putxhead(k->aid->hfd, ++bsmax, bsend);

         if (cdb_usrlog > 20){
            sprintf(ubuf, "CS-autoindex: %s (%s)(%c)(%ld)",
               k->fld, k->id, k->lock, bsmax);
            fb_usrlog_msg(ubuf);
            }

	 /* now find old spot and zero out and flag its record portion */
	 if (k->aid->dup_fld[0] != NULL){
            /* make a search key for old spot if applicable */
            fb_makess(k->aid->dup_fld, k->type, k->size);
	    srec = fb_megasearch(k->aid->afd, k->aid->dup_fld, 0, 
	             1, bsend, bsmax, rlength, 1, hp->irec);
            if (srec == 0){
               fb_lerror(FB_MESSAGE, k->id, SYSMSG[S_NOT_FOUND]);
               return(FB_ERROR);
               }
            slen = strlen(k->aid->dup_fld);
            if (srec > bsend)
               in_overflow = 1;
            for (;;){
               /* test the record for an absolute record number match */
               drec = atol((char *) (hp->irec + rlength - FB_RECORDPTR));
               if (drec == rec){
                  /* seek backwards FB_RECORDPTR+1 bytes since already there */
                  if (lseek(k->aid->afd, -(FB_RECORDPTR+1), 1) < 0L){
                     fb_lerror(FB_SEEK_ERROR, hp->dbase, SYSMSG[S_BAD_INDEX]);
                     return(FB_ERROR);
                     }
                  /* write 9 ZEROS and a ^E marker - cdb_S_EOREC */
                  strcpy(lbuf, ZEROS);
                  strcat(lbuf, cdb_S_EOREC);
                  if (write(k->aid->afd, lbuf, FB_RECORDPTR) != FB_RECORDPTR){
                     fb_lerror(FB_WRITE_ERROR, hp->dbase, SYSMSG[S_BAD_INDEX]);
                     return(FB_ERROR);
                     }
                  break;
                  }
               /* if not a record number match, then loop until key match */
               found = 0;
               for (;;){
                  if (read(k->aid->afd, hp->irec, (unsigned)rlength)!= rlength)
                     break;
                  /* now test the string for same key match */
                  if (strncmp(k->aid->dup_fld, hp->irec, slen) == 0){
                     found = 1;
                     break;
                     }
                  if (!in_overflow){
                     if (bsend <= 0)
                        break;
	            if (fb_fgetrec(bsend, k->aid->afd, rlength, hp->irec,
                          0) == FB_ERROR)
	                break;
                     in_overflow = 1;
                     }
	          }
               if (found == 0)
                  break;
               }
            }
	 return(FB_AOK);
      }

/*
 * fb_set_autoindex - save fields into dup_fld.
 *	for the formal autoindex structure, save it in key format (formatted)
 */

   fb_set_autoindex(hp)
      fb_database *hp;

      {
         int j, i;
         fb_autoindex *ix;
         fb_field *fp;

#if RPC
         if (cdb_use_rpc)
            return(fb_set_autoindex_clnt(hp));
#endif /* RPC */
         for (j = 0; j < hp->nfields; j++){
            fp = hp->kp[j];
            ix = fp->aid;
            if (ix != NULL){
               strcpy(ix->dup_fld, fp->fld);
               fb_makess(ix->dup_fld, fp->type, fp->size);
               }
            }
         
         for (j = 0; j < hp->b_maxauto; j++){
            ix = hp->b_autoindex[j];
            /* generate a key fb_field and copy to dup_fld */
            ix->dup_fld[0] = NULL;
            for (i = 0; i < ix->ix_ifields; i++){
               fp = ix->ix_ip[i];
               strcpy(ix->ix_key_fld, fp->fld);
               fb_makess(ix->ix_key_fld, fp->type, fp->size);
               strcat(ix->dup_fld, ix->ix_key_fld);
               }
            }
         return(FB_AOK);
      }

/*
 * fb_clear_autoindex - clear all of the dup_fld areas.
 */

   fb_clear_autoindex(hp)
      fb_database *hp;

      {
         int j;
         fb_autoindex *ix;

         for (j = 0; j < hp->nfields; j++)
            if (hp->kp[j]->aid != NULL)
               hp->kp[j]->aid->dup_fld[0] = NULL;
         for (j = 0; j < hp->b_maxauto; j++){
            ix = hp->b_autoindex[j];
            ix->dup_fld[0] = NULL;
            ix->ix_key_fld[0] = NULL;
            }
      }
