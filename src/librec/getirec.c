/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getirec.c,v 9.0 2001/01/09 02:56:58 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getirec_sid[] = "@(#) $Id: getirec.c,v 9.0 2001/01/09 02:56:58 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern long fail_irec;
extern short int search_position;
extern short int cdb_use_rpc;
extern short int cdb_secure;
extern long cdb_failrec;
extern short int cdb_loadfail;

extern long fb_megasearch(int fd, char *key, int col, long bsbeg, long bsend,
   long bsmax, int recsiz, int backup, char *buf);
extern long fb_btree_search(char *key, fb_bidx *bi, fb_bseq *bs);
extern long fb_key_eval(fb_bseq *bs);

/*
 *  getirec - given a record number, rec, do a getrec(rec).
 *	then, if the current index is a btree, build the key
 *	   needed for the current index and position the
 *	index pointer so that nextxrec and prevxrec work w.r.t rec.
 */

   fb_getirec(rec, dp)
      long rec;
      fb_database *dp;
      
      {
	 int st = FB_ERROR, i;
         fb_autoindex *ix;
         fb_field *fp;

#if RPC
         if (cdb_use_rpc)
            return(fb_getirec_clnt(rec, dp));
#endif /* RPC */
         dp->rec = 0L;
         st = fb_getrec(rec, dp);
         if (cdb_secure && fb_record_permission(dp, READ) == FB_ERROR)
            st = FB_ERROR;
         if (fb_isdeleted(dp))
            st = FB_ERROR;
         if (st == FB_ERROR)
            return(st);
         dp->bsrec = 0L;
         cdb_failrec = 0L;

         /*
          * if current idx is a btree that has been loaded using fb_useidx,
          * build the key needed and position the index pointer.
          */

         st = FB_ERROR;			/* assume ERROR */
         if (dp->b_curauto >= 0){
            ix = dp->b_autoindex[dp->b_curauto];
            /* generate a key fb_field and copy to dup_fld */
            ix->dup_fld[0] = NULL;
            for (i = 0; i < ix->ix_ifields; i++){
               fp = ix->ix_ip[i];
               strcpy(ix->ix_key_fld, fp->fld);
               fb_makess(ix->ix_key_fld, fp->type, fp->size);
               strcat(ix->dup_fld, ix->ix_key_fld);
               }
            sprintf(ix->dup_fld, "%s%010ld", ix->dup_fld, rec);
            rec = fb_btree_search(ix->dup_fld, dp->b_idx, dp->b_seq);
            if (rec > 0 && rec <= dp->reccnt){
               dp->bsrec = rec;
               st = FB_AOK;
               }
            }
	 return(st);
      }
