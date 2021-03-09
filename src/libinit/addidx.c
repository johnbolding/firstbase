/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: addidx.c,v 9.0 2001/01/09 02:56:45 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Addidx_sid[] = "@(#) $Id: addidx.c,v 9.0 2001/01/09 02:56:45 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_use_rpc;

extern fb_autoindex *fb_ixalloc(void);

/*
 *  addidx - add an index name to the list of auto indexes being kept
 */

   fb_addidx(iname, dp)
      fb_database *dp;
      char *iname;
      
      {
         fb_autoindex *ax, **new_autoindex;
         char seq[FB_SEQSIZE + 1];
         int st, i;

#if RPC
         if (cdb_use_rpc)
            return(fb_addidx_clnt(iname, dp));
#endif /* RPC */

         /* gen the ax structure */
         ax = fb_ixalloc();
	 fb_mkstr(&(ax->autoname), iname);
         sprintf(seq, SYSMSG[S_FMT_04D], fb_getseq(dp->fd));

         /* open the ax structure */
         st = fb_open_auto(dp, ax, seq, READWRITE);

         /* add the ax structure to current b_autoindex */
         if (st == FB_AOK){
            new_autoindex = (fb_autoindex **)
               fb_malloc((dp->b_maxauto + 2) * sizeof(fb_autoindex *));
            for (i = 0; i < dp->b_maxauto; i++){
               new_autoindex[i] = dp->b_autoindex[i];
               dp->b_autoindex[i] = NULL;
               }
            new_autoindex[i] = ax;
            fb_free((char *) dp->b_autoindex);
            dp->b_autoindex = new_autoindex;
            dp->b_maxauto++;
            }
         return(st);
      }

/*
 *  subidx - subtract an index name from the list of auto indexes being kept
 */

   fb_subidx(iname, dp)
      fb_database *dp;
      char *iname;
      
      {
         fb_autoindex *ix = NULL;
         int i;

#if RPC
         if (cdb_use_rpc)
            return(fb_subidx_clnt(iname, dp));
#endif /* RPC */

         /*
          *  no restructuring here ---
          *    find the one to remove
          *    free up the ax found (close down the idx)
          *    slide all others up
          *    decrement b_maxauto
          */

         for (i = 0; i < dp->b_maxauto; i++){
            ix = dp->b_autoindex[i];
            if (equal(ix->autoname, iname))
               break;
            }
         if (i >= dp->b_maxauto || ix == NULL)
            return(FB_ERROR);

         /* free up ix and the index there */
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

         for (; i < dp->b_maxauto; i++)
            dp->b_autoindex[i] = dp->b_autoindex[i + 1];
         dp->b_autoindex[i] = NULL;
         dp->b_maxauto--;

         return(FB_AOK);
      }
