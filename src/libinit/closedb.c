/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: closedb.c,v 9.0 2001/01/09 02:56:45 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Closedb_sid[] = "@(#) $Id: closedb.c,v 9.0 2001/01/09 02:56:45 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern fb_database *cdb_LastDbase;
extern short int cdb_lockdaemon;
extern short int cdb_use_rpc;
extern short int cdb_secure;

#if FB_PROTOTYPES
static rmlink(fb_database *dp, int n, fb_link *fk);
static void s_fb_closedb(fb_database *dp);
#else
static rmlink();
static void s_fb_closedb();
#endif /* FB_PROTOTYPES */

/*
 *  closedb - close and Free up a cdb database.
 *		search linktop for databases to remove also.
 */

   fb_closedb(dp)
      fb_database *dp;
      
      {
	 fb_link *ak;

#if RPC
         if (cdb_use_rpc){
            /*
             * unlike most other RPC hooks, this one drops through
             * so that the database on the client side is closed also.
             */
            if (fb_closedb_clnt(dp) != FB_AOK)
               return(FB_ERROR);
            }
#endif /* RPC */
         s_fb_closedb(dp);
	 /* for (ak = fb_linktop; ak != NULL; ak = ak->f_next) */
	 /* search for any databases no longer in reference - remove them */
	 for (ak = cdb_linktop; ak != NULL;){
	    if (ak->f_dp->refcnt <= 0){
	       s_fb_closedb(ak->f_dp);
	       ak = cdb_linktop;		/* start at top again */
	       }
	    else
	       ak = ak->f_next;
	    }
         return(FB_AOK);
      }

/*
 *  s_closedb - close and Free up a cdb database. removes tail recursion.
 *
 */

   static void s_fb_closedb(dp)
      register fb_database *dp;
      
      {
         register int i;
         fb_autoindex *ix;
	 
         if (dp == NULL)
	    return;
	 if (dp == cdb_LastDbase)
	    cdb_LastDbase = (fb_database *) NULL;
	 dp->refcnt--;
	 if (dp->refcnt > 0){
	    rmlink(dp, 1, (fb_link *) NULL);
	    return;
	    }
	 for (i = 0; i < dp->nfields; i++){
	    if (dp->kp[i]->dflink != NULL){
	       if (--(dp->kp[i]->dflink->f_dp->refcnt) > 0)
	          rmlink(dp->kp[i]->dflink->f_dp, 1, dp->kp[i]->dflink);
	       }
	    else if (dp->kp[i]->xflink != NULL){
	       if (--(dp->kp[i]->xflink->f_dp->refcnt) > 0)
	          rmlink(dp->kp[i]->xflink->f_dp, 1, dp->kp[i]->xflink);
	       }
	    }
#if RPC
         if (cdb_lockdaemon)
            fb_fcntl_cl_clnt(dp->dmap, 0L, 0L, 0, 0);
#endif
	 rmlink(dp, 0, (fb_link *) NULL);
	 if (dp->ihfd > 0 && dp->ip != NULL && dp->ifields > 0)
	    fb_free((char *) dp->ip[dp->ifields - 1]);
         close(dp->fd);		dp->fd = -1;
         if (dp->ifd >= 0){
	    close(dp->ifd);	dp->ifd = -1;
            }
         if (dp->ihfd >= 0){
	    close(dp->ihfd);	dp->ihfd = -1;
            }
	 close(dp->mfd);	dp->mfd = -1;
         if (dp->logfd >= 0){
	    close(dp->logfd);	dp->logfd = -1;
            }
	 fb_free(dp->dbase);	dp->dbase = NULL;
	 fb_free(dp->dindex);	dp->dindex = NULL;
	 fb_free(dp->ddict);	dp->ddict = NULL;
	 fb_free(dp->idict);	dp->idict = NULL;
	 fb_free(dp->dmap);	dp->dmap = NULL;
	 fb_free(dp->dlog);	dp->dlog = NULL;
	 fb_free(dp->sdict);	dp->sdict = NULL;
	 for (i = 0; dp->kp != NULL && i <= dp->nfields; i++){
	    fb_free(dp->kp[i]->id);
	    fb_free(dp->kp[i]->comment);
	    fb_free(dp->kp[i]->idefault);
	    fb_free(dp->kp[i]->help);
	    fb_free(dp->kp[i]->prev);
	    fb_free(dp->kp[i]->range);
	    fb_free(dp->kp[i]->a_template);
	    fb_free(dp->kp[i]->f_macro);
            if (cdb_secure)
               fb_free(dp->kp[i]->mode);
	    if (dp->kp[i]->aid != NULL){
               ix = dp->kp[i]->aid;
               if (ix != NULL && ix->ix_seq != NULL){
                  close(ix->ix_seq->bs_fd);
                  ix->ix_seq->bs_fd = -1;
                  }
               if (ix != NULL && ix->ix_idx != NULL){
                  close(ix->ix_idx->bi_fd);
                  ix->ix_idx->bi_fd = -1;
                  }
	       fb_free(ix->autoname);
	       fb_free(ix->dup_fld);
               if (ix->ix_tree)
                  fb_closeix_btree(ix, dp);
	       fb_free((char *) (ix));
	       ix = NULL;
	       }
	    fb_free((char *) dp->kp[i]);
	    }
	 fb_free(dp->orec);	  dp->orec = NULL;
	 fb_free(dp->arec);	  dp->arec = NULL;
	 fb_free(dp->irec);	  dp->irec = NULL;
         if (dp->b_autoindex != NULL){
            for (i = 0; i < dp->b_maxauto; i++){
               ix = dp->b_autoindex[i];
               close(ix->hfd);
               if (ix != NULL && ix->ix_seq != NULL){
                  close(ix->ix_seq->bs_fd);
                  ix->ix_seq->bs_fd = -1;
                  }
               if (ix != NULL && ix->ix_idx != NULL){
                  close(ix->ix_idx->bi_fd);
                  ix->ix_idx->bi_fd = -1;
                  }
               fb_free(ix->autoname);
               fb_free(ix->dup_fld);
               fb_closeix_btree(ix, dp);
	       fb_free((char *) (ix));
               dp->b_autoindex[i] = NULL;
               }
            fb_free((char *) dp->b_autoindex);
            dp->b_autoindex = NULL;
            }
         /*
          * this autoindex may have already been disconnected.
          * by fb_closeix_btree, if so dp->b_tree will be unset
          */
         if (dp->b_tree){
            close(dp->b_seq->bs_fd); dp->b_seq->bs_fd = -1;
            close(dp->b_idx->bi_fd); dp->b_idx->bi_fd = -1;
            fb_seq_free(dp->b_seq);
            fb_idx_free(dp->b_idx);
            fb_seq_free(dp->b_seqtmp);
            fb_idx_free(dp->b_idxtmp);
            dp->b_seq = NULL;
            dp->b_idx = NULL;
            dp->b_seqtmp = NULL;
            dp->b_idxtmp = NULL;
            }
	 fb_free((char *) (dp->kp)); dp->kp = NULL;
	 fb_free((char *) (dp->ip)); dp->ip = NULL;
         /* Free up afld and bfld ONLY if not the same as the globals */
         if (cdb_afld != dp->afld){
            fb_free(dp->afld);
            dp->afld = NULL;
            }
         if (cdb_bfld != dp->bfld){
            fb_free(dp->bfld);
            dp->bfld = NULL;
            }
	 fb_free((char *) dp); 				/* whew */
      }

/* 
 *  rmlink - remove links of this dp in the linktop list
 *	if n is == 0, remove all lines, else remove just ONE occurence.
 */

   static rmlink(dp, n, fk)
      fb_database *dp;
      int n;		/* n == 0 means all links, n == 1 means 1 link */
      fb_link *fk;

      {
         fb_link *ak, *pk, *dk;
	 
	 /* 
	 * search for fb_database with matching dps and unlink those
	 */
	 for (pk = NULL, ak = cdb_linktop; ak != NULL; ){
	    if (ak->f_dp == dp &&
                  (fk == (fb_link *) NULL || fk == ak)){
	       if (pk == NULL){
	          cdb_linktop = ak->f_next;
	          }
	       else{
	          pk->f_next = ak->f_next;
	          }
	       dk = ak;
	       ak = dk->f_next;
               dk->f_dp = NULL;
	       fb_free(dk->f_fld);
	       fb_free(dk->f_xfld);
	       fb_free((char *) dk);
	       if (n >= 1)
	          break;
	       }
	    else{
	       pk = ak;
	       ak = ak->f_next;
	       }
	    }
      }

   void fb_closeix_btree(ix, dp)
      fb_autoindex *ix;
      fb_database *dp;

      {
         if (ix == NULL)
            return;
         /*
          * if this index is the same as the default index, short circuit
          * out of the free portion so that its not freed up twice.
          */

         /*
          * this catches uses of the useidx, which copies pointers
          */
         if (dp->b_seq == ix->ix_seq){
            dp->b_seq = NULL;
            dp->b_idx = NULL;
            dp->b_seqtmp = NULL;
            dp->b_idxtmp = NULL;
            dp->b_tree = 0;
            }
         /*
          * this catches diff structs allocated at opendb time.
          */
         if (dp->b_seq != NULL &&
               equal(dp->b_seq->bs_name, ix->ix_seq->bs_name)){

            /*
             * these can now be freed normally since just fds are copied
             * now, not the entire struct as used to be done.
             */

            fb_seq_free(dp->b_seq);
            fb_seq_free(dp->b_seqtmp);
            fb_idx_free(dp->b_idx);
            fb_idx_free(dp->b_idxtmp);
            dp->b_seq = NULL;
            dp->b_idx = NULL;
            dp->b_seqtmp = NULL;
            dp->b_idxtmp = NULL;
            dp->b_tree = 0;
            }

         fb_seq_free(ix->ix_seq);
         fb_seq_free(ix->ix_seqtmp);
         fb_idx_free(ix->ix_idx);
         fb_idx_free(ix->ix_idxtmp);
         if (ix->ix_ip != NULL && ix->ix_ifields > 0)
            fb_free((char *) ix->ix_ip[ix->ix_ifields]);
         if (ix->ix_ip == dp->ip)
            dp->ip = NULL;
         if (ix->ix_ip != NULL)
            fb_free((char *) ix->ix_ip);
         fb_free(ix->ix_key_fld);
      }

#if FB_CLOSEDB_DEBUG
   tracelink(s)
      char *s;

      {
         fb_link *ak;
	 int j;
	 
	 fprintf(stderr, "*** START TRACE *** %s \n", s);
	 fflush(stderr);
	 for (j = 0, ak = fb_linktop; ak != NULL; ak = ak->f_next){
	    fprintf(stderr, "...database[%d@%x]=%s/%s [ref:%d]\n", ++j, 
	       ak->f_dp, ak->f_dp->dbase, ak->f_dp->dindex, ak->f_dp->refcnt);
	    fflush(stderr);
	    fprintf(stderr, "......kp[0] %x %s   ip[0] %x %s\n",
	       ak->f_dp->kp[0], ak->f_dp->kp[0]->id,
	       ak->f_dp->ip[0], ak->f_dp->ip[0]->id);
	    fflush(stderr);
	    }
	 fprintf(stderr, "*** END TRACE ***\n");
	 fflush(stderr);
      }
#endif /* DEBUG */
