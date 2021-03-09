/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: delidx.c,v 9.0 2001/01/09 02:56:56 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Delidx_sid[] = "@(#) $Id: delidx.c,v 9.0 2001/01/09 02:56:56 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#if FB_PROTOTYPES
static autodelete(fb_field *k, long rec);
#else /* FB_PROTOTYPES */
static autodelete();
#endif /* FB_PROTOTYPES */

   fb_delidx(hp, rec)
      fb_database *hp;
      long rec;
      
      {
         register int j, i;
         fb_autoindex *ix;
         int st = FB_AOK, errcnt = 0;
         fb_field *fp;
	 
	 for (j = 0; j < hp->nfields; j++){
            ix = hp->kp[j]->aid; 
	    if (ix != NULL && ix->hfd > 0 && ix->ix_tree)
	       if (autodelete(hp->kp[j], rec) == FB_ERROR)
		  return(FB_ERROR);
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
            st = fb_btree_delete(ix->ix_key_fld, rec, ix->ix_idx, ix->ix_seq);
            if (st == FB_ERROR)
               errcnt++;
            }
         if (errcnt > 0)
            st = FB_ERROR;
	 return(st);
      }

   static autodelete(k, rec)
      fb_field *k;
      long rec;

      {
         fb_bseq *bs;
         fb_bidx *bi;
         fb_autoindex *ix;
         int st = FB_AOK;

         /* build a index record using fb_makess() - store in afld */
         strcpy(cdb_afld, k->fld);
         fb_makess(cdb_afld, k->type, k->size);
         ix = k->aid;
         bs = ix->ix_seq;
         bi = ix->ix_idx;
         st = fb_btree_delete(cdb_afld, rec, bi, bs);
         return(st);
      }
