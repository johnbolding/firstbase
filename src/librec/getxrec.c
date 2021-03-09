/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getxrec.c,v 9.0 2001/01/09 02:56:59 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getxrec_sid[] = "@(#) $Id: getxrec.c,v 9.0 2001/01/09 02:56:59 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern long fail_irec;
extern short int search_position;
extern short int cdb_use_rpc;
extern short int cdb_secure;
extern long cdb_failrec;
extern short int cdb_loadfail;

#if FB_PROTOTYPES
static adjustidx(fb_database *db);
#else
static adjustidx();
#endif /* FB_PROTOTYPES */

extern long fb_megasearch(int fd, char *key, int col, long bsbeg, long bsend,
   long bsmax, int recsiz, int backup, char *buf);
extern long fb_btree_search(char *key, fb_bidx *bi, fb_bseq *bs);
extern long fb_key_eval(fb_bseq *bs);

/*
 *  getxrec - force a record with a particular index entry
 *	to be loaded into dp or return FB_ERROR.
 *	dp->bsrec is set to index record number for standard index
 *	dp->bsrec is set to *actual* fb_database record number for btree+ index
 */

   fb_getxrec(s, dp)
      char *s;
      fb_database *dp;
      
      {
         long rec;
	 int st = FB_ERROR;

#if RPC
         if (cdb_use_rpc)
            return(fb_getxrec_clnt(s, dp));
#endif /* RPC */
         dp->rec = dp->bsrec = 0L;
         cdb_failrec = 0L;
         /* b_tree handled if needed */
         if (dp->b_tree){
            rec = fb_btree_search(s, dp->b_idx, dp->b_seq);
            if (rec > 0 && rec <= dp->reccnt){
               dp->bsrec = rec;
               if (fb_getrec(rec, dp) != FB_ERROR)
                  st = FB_AOK;
               if (cdb_secure && fb_record_permission(dp, READ) == FB_ERROR)
                  st = FB_ERROR;
               }
            else
               dp->bsrec = cdb_failrec;
            }
         /* standard FB index */
         else{
	    rec = fb_megasearch(dp->ifd, s, 0, 1, dp->bsend, dp->bsmax,
               dp->irecsiz, 1, dp->irec);
            if (rec > 0 && rec <= dp->bsmax){
               dp->bsrec = rec;
               rec = atol((char *) 
                  (dp->irec + dp->irecsiz - 11));	/* FB_RECORDPTR + 1 */
               if (rec > 0 && rec <= dp->reccnt)
                  if (fb_getrec(rec, dp) != FB_ERROR)
                     st = FB_AOK;
                  if (cdb_secure && fb_record_permission(dp, READ) == FB_ERROR)
                     st = FB_ERROR;
               }
            else
               dp->bsrec = fail_irec;
	    }
         if (st == FB_ERROR && cdb_loadfail)
            adjustidx(dp);
	 return(st);
      }

/*
 * adjustidx - loads the failure point of the record
 */

   static adjustidx(db)
      fb_database *db;

      {
         int st = FB_ERROR;
         long max;

         if (db->b_tree)
            max = db->reccnt;
         else
            max = db->bsmax;

         if (db->bsrec > 0L && db->bsrec <= max){
            if (!db->b_tree){			/* standad index */
               if (search_position == 1)
                  st = fb_nextxrec(db);
               else
                  st = fb_currentxrec(db);
               }
            else				/* btree index */
               st = fb_getrec(db->bsrec, db);
            }
         if (st == FB_ERROR)
            db->rec = db->bsrec = 0;
      }
