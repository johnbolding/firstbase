/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: allsync.c,v 9.1 2001/01/12 22:51:58 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Allsync_sid[] = "@(#) $Id: allsync.c,v 9.1 2001/01/12 22:51:58 john Exp $";
#endif

#include <fb.h>

#if FB_PROTOTYPES
static sync_ix(fb_autoindex *ix);
#else /* FB_PROTOTYPES */
static sync_ix();
#endif /* FB_PROTOTYPES */
extern short int cdb_use_rpc;

/*
 *  allsync.c - sync up all files, indexes, headers, etc.
 */

   fb_allsync(db)
      fb_database *db;
      
      {
	 int i;
         fb_autoindex *ix;

	 fb_sync_fd(db->mfd);
	 fb_sync_fd(db->fd);
	 fb_sync_fd(db->ifd);
	 fb_sync_fd(db->ihfd);
         if (db->b_tree){
            if (db->b_seq != NULL)
               fb_sync_fd(db->b_seq->bs_fd);
            if (db->b_idx != NULL)
               fb_sync_fd(db->b_idx->bi_fd);
            }
         for (i = 0; i < db->nfields; i++){
            if (db->kp[i]->aid != NULL && db->kp[i]->aid->autoname != NULL &&
                   db->kp[i]->aid->autoname[0] != 0){
               ix = db->kp[i]->aid;
               sync_ix(ix);
	       }
            }

         /* now do the same for the dbase.auto structure */
         for (i = 0; i < db->b_maxauto; i++){
            ix = db->b_autoindex[i];
            sync_ix(ix);
            }
         return(FB_AOK);
      }

   static sync_ix(ix)
      fb_autoindex *ix;

      {
         fb_sync_fd(ix->afd);
         fb_sync_fd(ix->hfd);
         if (ix->ix_tree){
            if (ix->ix_seq != NULL)
               fb_sync_fd(ix->ix_seq->bs_fd);
            if (ix->ix_idx != NULL)
               fb_sync_fd(ix->ix_idx->bi_fd);
            }
         return(FB_AOK);
      }

   fb_sync(db)
      fb_database *db;

      {
#if RPC
         if (cdb_use_rpc)
            return(fb_sync_clnt(db));
#endif /* RPC */
         return(fb_allsync(db));
      }

   fb_sync_fd(fd)
      int fd;

      {
	 struct stat sbuf;
         int st = FB_ERROR;

         if (fd > 0){
#if HAVE_FSYNC
	    fsync(fd);
#endif /* HAVE_FSYNC */
	    fstat(fd, &sbuf);
            st = FB_AOK;
            }
         return(st);
      }
