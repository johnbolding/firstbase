/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: openidx.c,v 9.0 2001/01/09 02:56:49 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Openidx_sid[] = "@(#) $Id: openidx.c,v 9.0 2001/01/09 02:56:49 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_use_rpc;

/*
 *  openidx - open a fb_database index
 */

   fb_openidx(iname, dp)
      fb_database *dp;
      char *iname;
      
      {
#if RPC
         if (cdb_use_rpc)
            return(fb_openidx_clnt(iname, dp));
#endif /* RPC */
         fb_dbargs(NIL, iname, dp);
         if (fb_geti_dict(READWRITE, dp) == FB_ERROR)
            return(FB_ERROR);
         if (fb_geti_data(READWRITE, dp) == FB_ERROR)
            return(FB_ERROR);
	 return(FB_AOK);
      }

/*
 *  closeidx - close a fb_database index
 */

   fb_closeidx(dp)
      fb_database *dp;

      {
#if RPC
         if (cdb_use_rpc)
            return(fb_closeidx_clnt(dp));
#endif /* RPC */
         if (dp->b_tree){
            close(dp->b_seq->bs_fd);
            close(dp->b_idx->bi_fd);
            fb_free((char *) dp->b_seq);
            fb_free((char *) dp->b_idx);
            fb_free((char *) dp->b_seqtmp);
            fb_free((char *) dp->b_idxtmp);
            dp->b_tree = 0;
            }
         else{
            close(dp->ifd);
            close(dp->ihfd);
            }
         dp->ifd = -1;
         dp->ihfd = -1;
         dp->bsend = 0;
         dp->bsmax = 0;
         dp->bsrec = 0;
         dp->irecsiz = 0;
         dp->b_seq = NULL;
         dp->b_idx = NULL;
         dp->b_seqtmp = NULL;
         dp->b_idxtmp = NULL;
         return(FB_AOK);
      }

/*
 * fb_useidx - switch an auto btree index into the fb_database structure.
 */

   fb_useidx(i, dp)
      int i;
      fb_database *dp;

      {
         fb_autoindex *ix;

#if RPC
         if (cdb_use_rpc)
            return(fb_useidx_clnt(i, dp));
#endif /* RPC */
         if (dp->b_maxauto <= 0 || i < 0 || i >= dp->b_maxauto)
            return(FB_ERROR);
         ix = dp->b_autoindex[i];
         dp->b_curauto = i;
         dp->b_seq = ix->ix_seq;
         dp->b_idx = ix->ix_idx;
         dp->b_seqtmp = ix->ix_seqtmp;
         dp->b_idxtmp = ix->ix_idxtmp;
         dp->b_tree = 1;
         return(FB_AOK);
      }
