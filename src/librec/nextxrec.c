/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: nextxrec.c,v 9.0 2001/01/09 02:57:00 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Nextxrec_sid[] = "@(#) $Id: nextxrec.c,v 9.0 2001/01/09 02:57:00 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_use_rpc;
extern short int cdb_secure;
extern long fb_key_eval(fb_bseq *bs);

/*
 *  nextxrec - force the next indexed record to be loaded.
 */

   fb_nextxrec(dp)
      fb_database *dp;
      
      {
         long rec, save_recno;
	 int st = FB_ERROR, save_curkey;
         fb_bseq *bs;
	 
#if RPC
         if (cdb_use_rpc)
            return(fb_nextxrec_clnt(dp));
#endif /* RPC */
         if (dp->b_tree){				/* btree path */
            bs = dp->b_seq;
            if (bs->bs_recno == 0){
               bs->bs_next = 0;
               bs->bs_curkey = 0;
               fb_get_seq_head(bs);
               if (fb_seq_getrec(bs->bs_head, bs) == FB_ERROR)
                  return(FB_ERROR);
               }
            save_recno = bs->bs_recno;
            save_curkey = bs->bs_curkey;
            /*
             * the current node has to be re-read in case a key was deleted
             * by some other process.
             */
            if (bs->bs_curkey >= 1 && bs->bs_curkey <= 3)
               fb_seq_getrec(bs->bs_recno, bs);
            for (;;){
               if (bs->bs_curkey >= 3){
                  if (bs->bs_next == 0)
                     break;
                  if (fb_seq_getrec(bs->bs_next, bs) != FB_AOK){
                     st = FB_ERROR;
                     break;
                     }
                  bs->bs_curkey = 0;
                  }
               bs->bs_curkey++;
               rec = fb_key_eval(bs);
               if (rec > 0L && rec <= dp->reccnt){	/* ignore rec of 0 */
                  if (fb_getrec(rec, dp) == FB_ERROR)
                     fb_xerror(FB_FATAL_GETREC, dp->dbase, (char *) &rec);
                  if (cdb_secure && fb_record_permission(dp, READ) == FB_ERROR)
                     continue;
                  st = FB_AOK;
                  dp->bsrec = rec;
                  break;
                  }
               else if (rec == 0)
                  bs->bs_curkey = 4;
               else if (rec < -1){
                  st = FB_ERROR;
                  break;
                  }
               }
            /* if not found, restore saved location */
            if (st == FB_ERROR){
               bs->bs_recno = save_recno;
               bs->bs_curkey = save_curkey;
               }
            }
         else {						/* standard FB index */
            while (read(dp->ifd, dp->irec, (unsigned) dp->irecsiz) ==
                  dp->irecsiz){
               dp->bsrec += 1L;
               rec = atol((char *) (dp->irec + dp->irecsiz - 11));
               if (rec > 0L && rec <= dp->reccnt){	/* ignore rec of 0 */
                  if (fb_getrec(rec, dp) == FB_ERROR)
                     fb_xerror(FB_FATAL_GETREC, dp->dbase, (char *) &rec);
                  st = FB_AOK;
                  if (cdb_secure && fb_record_permission(dp, READ) == FB_ERROR)
                     st = FB_ERROR;
                  break;
                  }
               }
            }
	  return(st);
      }
