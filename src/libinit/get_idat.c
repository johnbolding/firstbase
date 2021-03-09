/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: get_idat.c,v 9.0 2001/01/09 02:56:47 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Geti_data_sid[] = "@(#) $Id: get_idat.c,v 9.0 2001/01/09 02:56:47 john Exp $";
#endif

#include <fb.h>

/* 
 *  geti_data - initialize the index data file using mode.
 */
 
   fb_geti_data(mode, dhp)
      int mode;
      fb_database *dhp;
   
      {
	 int i, st = FB_AOK;
	 char autofile[FB_MAXNAME];
         fb_bseq *bs;
         fb_bidx *bi;
         fb_autoindex *ix;

         /* search the ddict auto indexes for this index */
	 for (i = 0; i < dhp->nfields; i++){
	    if (dhp->kp[i]->aid != NULL && 
	           dhp->kp[i]->aid->autoname != NULL &&
	           dhp->kp[i]->aid->autoname[0] != NULL){
               ix = dhp->kp[i]->aid;
	       fb_dirname(autofile, dhp->dbase);
	       strcat(autofile, ix->autoname);
	       strcat(autofile, SYSMSG[S_EXT_IDX]);
	       if (equal(autofile, dhp->dindex) && ix->hfd > 0){
                  if (ix->ix_tree == 1){
	             dhp->b_tree = 1;
                     /* copy the structures file descriptors */
                     dhp->b_seq->bs_fd = ix->ix_seq->bs_fd;
                     dhp->b_idx->bi_fd = ix->ix_idx->bi_fd;
                     }
                  else
	             dhp->ifd = ix->afd;
		  return(FB_AOK);
		  }
	       }
	    }

         /* search the dbase.auto indexes for this index */
         for (i = 0; i < dhp->b_maxauto; i++){
            ix = dhp->b_autoindex[i];
            fb_dirname(autofile, dhp->dbase);
            strcat(autofile, ix->autoname);
	    strcat(autofile, SYSMSG[S_EXT_IDX]);
            if (equal(autofile, dhp->dindex) && ix->hfd > 0){
               if (ix->ix_tree == 1){
                  dhp->b_tree = 1;
                  /* copy the structures file descriptors */
                  dhp->b_seq->bs_fd = ix->ix_seq->bs_fd;
                  dhp->b_idx->bi_fd = ix->ix_idx->bi_fd;
                  }
               else
                  dhp->ifd = ix->afd;
               return(FB_AOK);
               }
            }

         /* if its a b_tree index, open seq and index files, else dindex */
         if (dhp->b_tree){

            dhp->b_seq->bs_fd = open(dhp->b_seq->bs_name, mode);
            dhp->b_idx->bi_fd = open(dhp->b_idx->bi_name, mode);

            if (dhp->b_seq->bs_fd < 0 || dhp->b_idx->bi_fd < 0)
               st = FB_ERROR;
            bi = dhp->b_idx;
            fb_getbxhead(bi->bi_fd, &(bi->bi_root), &(bi->bi_height),
               &(bi->bi_reccnt), &(bi->bi_free));
            bs = dhp->b_seq;
            fb_getbxhead(bs->bs_fd, &(bs->bs_head), &(bs->bs_tail),
               &(bs->bs_reccnt), &(bs->bs_free));
            }
         else if ((dhp->ifd = open(dhp->dindex, mode)) < 0)
            st = FB_ERROR;

         if (st == FB_ERROR)
	    fb_emptyi_dict(dhp);		/* force empty if no data */
         return(st);
      }
