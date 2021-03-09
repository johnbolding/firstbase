/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: ex_limit.c,v 9.0 2001/01/09 02:55:59 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Ex_select_sid[] = "@(#) $Id: ex_limit.c,v 9.0 2001/01/09 02:55:59 john Exp $";
#endif

#include <dbve_ext.h>
#include <sys/types.h>
#include <sys/stat.h>

extern short int cdb_ex_fullkey;
static ex_setlimit_tree();

/*
 * ex_setlimit - set the limits (scope) of this requested exchoice.
 *	merely adjust the ex_first and ex_last fields.
 *	remember: these fields represent INDEX rec numbers.
 *	this mechanism is for flat or normal FirstBase indexes.
 */

   ex_setlimit(e)
      fb_exchoice *e;
      
      {
	 long rec, bsbeg, bsend, bsmax, fb_search(), fb_lastmatch();
	 int fd, recsiz, i;
	 char *buf, *fld;
	 fb_field *f;
	 char key[FB_MAXLINE];

         if (e->ex_filters[0] == NULL)
	    return(FB_AOK);
	 f = e->ex_filters[0];
	 if (!cdb_ex_fullkey && strlen(f->fld) == 0)
	    return(FB_AOK);
         if (e->ex_tree)
            return(ex_setlimit_tree(e));
	 fd = e->ex_db->ifd;
	 recsiz = e->ex_db->irecsiz;
	 buf = e->ex_db->irec;
	 bsbeg = 1L;
	 bsend = e->ex_db->bsend;
	 bsmax = e->ex_db->bsmax;
	 fld = cdb_bfld;
	 key[0] = NULL;
	 for (i = 0; i < NFILTERS; i++){
	    f = e->ex_filters[i];
	    if (f == NULL)
	       break;
	    if (!cdb_ex_fullkey && strlen(f->fld) == 0)
	       break;
	    strcpy(fld, f->fld);
	    fb_makess(fld, f->type, f->size);
	    if (cdb_ex_fullkey)
	       fb_underscore(fld, 1);
	    strcat(key, fld);
	    }
	 fb_trim(key);
	 if (cdb_ex_fullkey)
	    fb_underscore(key, 0);
	 if (key[0] == NULL)
	    return(FB_AOK);
	 rec = fb_search(fd, key, 0, bsbeg, bsend, bsmax, recsiz, 1, buf);
	 if (rec >= e->ex_first && rec <= e->ex_last){
	    e->ex_first = e->ex_ptop = rec;
	    e->ex_last = fb_lastmatch(fd, key, rec, e->ex_last, recsiz, buf);
	    return(FB_AOK);
	    }
	 return(FB_ERROR);
      }

/*
 * ex_setlimit_tree - set the limits (scope) of this requested exchoice.
 *	adjust the ex_first and ex_last fields.
 *	remember: these fields represent BTREE SEQ NODE NUMBERS
 *	along with the current key for each one.
 *	this mechanism is for BTree FirstBase indexes.
 */

   static ex_setlimit_tree(e)
      fb_exchoice *e;
      
      {
	 long rec, fb_btree_search(), fb_lastmatch_tree();
	 int i;
	 char *fld;
	 fb_field *f;
	 char key[FB_MAXLINE];
         fb_bidx *bi;
         fb_bseq *bs;

         /* make the key */
	 f = e->ex_filters[0];
	 fld = cdb_bfld;
	 key[0] = NULL;
	 for (i = 0; i < NFILTERS; i++){
	    f = e->ex_filters[i];
	    if (f == NULL)
	       break;
	    if (!cdb_ex_fullkey && strlen(f->fld) == 0)
	       break;
	    strcpy(fld, f->fld);
	    fb_makess(fld, f->type, f->size);
	    if (cdb_ex_fullkey)
	       fb_underscore(fld, 1);
	    strcat(key, fld);
	    }
	 fb_trim(key);
	 if (cdb_ex_fullkey)
	    fb_underscore(key, 0);
	 if (key[0] == NULL)
	    return(FB_AOK);
         bi = e->ex_db->b_idx;
         bs = e->ex_db->b_seq;
         /* search the btree for this key -- and save its curkey also */
	 rec = fb_btree_search(key, bi, bs);
         if (rec > 0){
	    e->ex_first = e->ex_ptop = bs->bs_recno;
	    e->ex_first_curkey = e->ex_ptop_curkey = bs->bs_curkey;
	    if ((rec = fb_lastmatch_tree(key, bs)) > 0){
               e->ex_last = bs->bs_recno;
               e->ex_last_curkey = bs->bs_curkey;
	       return(FB_AOK);
               }
	    }
	 return(FB_ERROR);
      }
