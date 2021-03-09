/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: xrec.c,v 9.0 2001/01/09 02:57:02 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Xrec_sid[] = "@(#) $Id: xrec.c,v 9.0 2001/01/09 02:57:02 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/* various library functions for index access to records */

extern short int cdb_use_rpc;
extern short int cdb_secure;
extern long fb_key_eval(fb_bseq *bs);

/*
 *  prevxrec - force the previous indexed record to be loaded.
 */

   fb_prevxrec(dp)
      fb_database *dp;
      
      {
         long rec, save_recno;
	 int st = FB_ERROR, save_curkey;
         fb_bseq *bs;
	 
#if RPC
         if (cdb_use_rpc)
            return(fb_prevxrec_clnt(dp));
#endif /* RPC */
         if (dp->b_tree){				/* btree path */
            bs = dp->b_seq;
            save_recno = bs->bs_recno;
            save_curkey = bs->bs_curkey;
            /*
             * the current node has to be re-read in case a key was deleted
             * by some other process.
             */
            if (bs->bs_curkey >= 1 && bs->bs_curkey <= 3)
               fb_seq_getrec(bs->bs_recno, bs);
            for (;;){
               if (bs->bs_curkey <= 1){
                  if (bs->bs_prev == 0)
                     break;
                  fb_seq_getrec(bs->bs_prev, bs);
                  bs->bs_curkey = 4;			/* seed curkey */
                  }
               bs->bs_curkey--;
               rec = fb_key_eval(bs);
               if (rec > 0L && rec <= dp->reccnt){	/* ignore rec of 0 */
                  if (fb_getrec(rec, dp) == FB_ERROR)
                     fb_xerror(FB_FATAL_GETREC, dp->dbase, (char *) &rec);
                  if (cdb_secure && fb_record_permission(dp, READ) == FB_ERROR)
                     continue;
                  st = FB_AOK;
                  break;
                  }
               }
            /* if not found, restore saved location */
            if (st == FB_ERROR){
               bs->bs_recno = save_recno;
               bs->bs_curkey = save_curkey;
               }
            }
         else{						/* standard FB index */
            /* back up over current indexed record */
            if (lseek(dp->ifd, -(dp->irecsiz), 1) < 0L){
               lseek(dp->ifd, 0, 0);	/* return to known state */
               dp->bsrec = 0L;
               return(st);
               }
            for (;;){
               /* now get to previous record */
               if (lseek(dp->ifd, -(dp->irecsiz), 1) < 0L){
                  lseek(dp->ifd, 0, 0);	/* return to known state */
                  dp->bsrec = 0L;
                  break;
                  }
               dp->bsrec -= 1L;
               if (read(dp->ifd, dp->irec, (unsigned) dp->irecsiz) !=
                     dp->irecsiz)
                  break;
               rec = atol((char *) (dp->irec + dp->irecsiz - 11));
               if (rec > 0L && rec <= dp->reccnt){	/* ignore rec of 0 */
                  if (fb_getrec(rec, dp) == FB_ERROR)
                     fb_xerror(FB_FATAL_GETREC, dp->dbase, (char *) &rec);
                  if (cdb_secure && fb_record_permission(dp, READ) == FB_ERROR)
                     continue;
                  st = FB_AOK;
                  break;
                  }
               /* back up over current indexed record */
               if (lseek(dp->ifd, -(dp->irecsiz), 1) < 0L){
                  lseek(dp->ifd, 0, 0);	/* return to known state */
                  break;
                  }
               }
            }
	  return(st);
      }

/*
 *  firstxrec - force the first indexed record to be loaded.
 */

   fb_firstxrec(dp)
      fb_database *dp;
      
      {
         fb_bseq *bs;

#if RPC
         if (cdb_use_rpc)
            return(fb_firstxrec_clnt(dp));
#endif /* RPC */
         if (dp->b_tree){
            bs = dp->b_seq;
            bs->bs_next = 0;
            bs->bs_curkey = 0;
            fb_get_seq_head(bs);
            if (fb_seq_getrec(bs->bs_head, bs) == FB_ERROR)
               return(FB_ERROR);
            }
         else{
            /* get first indexed records */
            lseek(dp->ifd, 0, 0);	/* return to known state */
            dp->bsrec = 0L;
            }
         return(fb_nextxrec(dp));
      }

/*
 *  lastxrec - force the last indexed record to be loaded.
 */

   fb_lastxrec(dp)
      fb_database *dp;
      
      {
         long rec;
         fb_bseq *bs;

#if RPC
         if (cdb_use_rpc)
            return(fb_lastxrec_clnt(dp));
#endif /* RPC */
         if (dp->b_tree){
            bs = dp->b_seq;
            bs->bs_prev = 0;
            bs->bs_curkey = 4;
            fb_get_seq_head(bs);
            if (fb_seq_getrec(bs->bs_tail, bs) == FB_ERROR)
               return(FB_ERROR);
            return(fb_prevxrec(dp));
            }
         else{
            rec = dp->bsend;
            if (rec <= 0L)
               return(FB_ERROR);
            if (fb_fgetrec(rec, dp->ifd, dp->irecsiz, dp->irec, 0) == FB_ERROR)
               return(FB_ERROR);
            dp->bsrec = rec;
            rec = atol((char *) (dp->irec + dp->irecsiz - 11));
            if (rec > 0L && rec <= dp->reccnt){	/* ignore rec of 0 */
               if (fb_getrec(rec, dp) == FB_ERROR)
                  return(FB_ERROR);
               return(FB_AOK);
               }
            else
               return(fb_prevxrec(dp));
            }
      }

/*
 *  firstovxrec - force the first overflow indexed record to be loaded.
 */

   fb_firstovxrec(dp)
      fb_database *dp;
      
      {
	 if (fb_lastxrec(dp) == FB_AOK)
	    return(fb_nextxrec(dp));
	 return(FB_ERROR);
      }

/*
 *  lastovxrec - force the last overflow indexed record to be loaded.
 */

   fb_lastovxrec(dp)
      fb_database *dp;
      
      {
         long rec;
	 
	 rec = dp->bsmax;
	 if (rec <= 0L)
	    return(FB_ERROR);
	 if (fb_fgetrec(rec, dp->ifd, dp->irecsiz, dp->irec, 0) == FB_ERROR)
	    return(FB_ERROR);
         dp->bsrec = rec;
	 rec = atol((char *) (dp->irec + dp->irecsiz - 11));
	 if (rec > 0L && rec <= dp->reccnt){	/* ignore rec of 0 */
	    if (fb_getrec(rec, dp) == FB_ERROR)
	       return(FB_ERROR);
	    return(FB_AOK);
	    }
	 else
	    return(fb_prevxrec(dp));
      }

/*
 *  currentxrec - force the loading of the current indexed record to be loaded.
 */

   fb_currentxrec(dp)
      fb_database *dp;
      
      {
         long rec;
	 
         if (dp->b_tree)
            rec = fb_key_eval(dp->b_seq);
         else
            rec = atol((char *) (dp->irec + dp->irecsiz - 11));
         if (rec > 0L && rec <= dp->reccnt)	/* ignore rec of 0 */
            if (fb_getrec(rec, dp) != FB_ERROR)
               return(FB_AOK);
	 return(FB_ERROR);
      }
