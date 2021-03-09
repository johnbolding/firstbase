/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: checkaut.c,v 9.0 2001/01/09 02:56:56 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Checkauto_sid[] = "@(#) $Id: checkaut.c,v 9.0 2001/01/09 02:56:56 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#if FB_PROTOTYPES
static autoincr(fb_field *k, fb_database *hp);
static btree_autoincr(fb_autoindex *ix, fb_database *hp);
#else /* FB_PROTOTYPES */
static autoincr();
static btree_autoincr();
#endif /* FB_PROTOTYPES */

extern char *cdb_T_AUTOINCR;
extern char *cdb_AUTOMARK;
extern short int cdb_usrlog;

/* checkauto - force an autoincrement.
*	NOTE: mutual exclusion here is guaranteed since dbedit has
*	record 0 locked at the moment. so, these autofiles
*	should not be changing (due to Cdb).
*	Return the number of auto fields changed, or FB_ERROR.
*/

   fb_checkauto(hp)
      fb_database *hp;

      {
         register int j, n = 0;
         fb_autoindex *ix;
         int st = FB_AOK;
         fb_field *f;
	 
	 for (j = 0; j < hp->nfields; j++){
            ix = hp->kp[j]->aid; 
	    if (ix != NULL && ix->hfd > 0)
	       if (equal(hp->kp[j]->idefault, cdb_T_AUTOINCR) &&
	              equal(hp->kp[j]->fld, cdb_AUTOMARK)){
                  if (ix->ix_tree)
                     st = btree_autoincr(ix, hp);
                  else
	             st = autoincr(hp->kp[j], hp);
                  if (st == FB_ERROR)
		     return(FB_ERROR);
		  n++;
		  }
            }
         for (j = 0; j < hp->b_maxauto; j++){
            ix = hp->b_autoindex[j];
            f = ix->ix_ip[0];
	    if (equal(f->idefault, cdb_T_AUTOINCR) &&
                  equal(f->fld, cdb_AUTOMARK)){
	       if (btree_autoincr(ix, hp) == FB_ERROR)
                  return(FB_ERROR);
               n++;
               }
            }
	 return(n);
      }

/*
 *  autoincr - search index and autoincrement this fb_field value.
 */
 
   static autoincr(k, hp)
      fb_field *k;
      fb_database *hp;
      
      {
         long bsmax, bsend;
         char *buf, ubuf[FB_MAXLINE];
	 int rlength, st = FB_AOK;
	 long lastval = 0, rec, val;

	 rlength = k->size + FB_RECORDPTR + 1;
	 buf = hp->irec;

	 /* get bsmax,bsend from index header file */
         fb_getxhead(k->aid->hfd, &bsmax, &bsend);
	 if (bsend > 0){
	    rec = bsend;
	    if (fb_fgetrec(rec, k->aid->afd, rlength, buf, 0) == FB_ERROR)
	       fb_xerror(FB_FATAL_FGETREC, k->aid->autoname, (char *) &bsend);
	    buf[strlen(buf) - 11] = NULL;
	    lastval = atol(buf);
	    }
         /*
          * this used to loop through all new records, but
          * by definition, since locked, the last number should
          * be at the last record in the index, unless someone
          * has changed a key ... assume locked so as to get speed.
          */

         rec = bsmax;
         if (rec > 0){
            if (fb_fgetrec(rec, k->aid->afd, rlength, buf, 0) == FB_ERROR)
               fb_xerror(FB_FATAL_FGETREC, k->aid->autoname, (char *) &bsend);
            buf[strlen(buf) - 11] = NULL;
            val = atol(buf);
            if (val > lastval)
               lastval = val;
            }

	 sprintf(buf, FB_FDIGITS, ++lastval);
	 if ((int) strlen(buf) > k->size){
	    sprintf(buf, FB_FDIGITS, 0);
	    st = FB_ERROR;
	    }
	 fb_store(k, buf, hp);
         if (cdb_usrlog > 20){
            sprintf(ubuf, "CS-autoincr: %s (%s)(%c)(%ld)",
               buf, k->id, k->lock, bsmax);
            fb_usrlog_msg(ubuf);
            }
	 return(st);
      }

/*
 *  btree_autoincr - search index and autoincrement this fb_field value.
 *	for the first fb_field in the ix array, locate the last,
 *	incr it, store it.
 */
 
   static btree_autoincr(ix, hp)
      fb_autoindex *ix;
      fb_database *hp;
      
      {
         fb_bseq *bs;
         int st = FB_AOK;
         fb_field *f;
         char *p, ubuf[FB_MAXLINE];
         long val = 0L;

         bs = ix->ix_seq;
         fb_get_seq_head(bs);
         f = ix->ix_ip[0];
         p = bs->bs_key1;
         if (p == NULL || bs->bs_ksize < f->size)
            return(FB_ERROR);
         /* set to last record via tail */
         if (bs->bs_tail > 0){
            if (fb_seq_getrec(bs->bs_tail, bs) == FB_ERROR)
               return(FB_ERROR);
            /* even if the key is deleted, its value is still there. use it */
            if (fb_key_record(bs->bs_key3, bs->bs_ksize) != 0)
               p = bs->bs_key3;
            else if (fb_key_record(bs->bs_key2, bs->bs_ksize) != 0)
               p = bs->bs_key2;
            else
               p = bs->bs_key1;
            if (p == NULL || bs->bs_ksize < f->size)
               return(FB_ERROR);
            p[f->size] = NULL;
            val = atol(p);
            }
         else
            val = 0L;
	 sprintf(p, FB_FDIGITS, ++val);
	 if ((int) strlen(p) > f->size){
	    sprintf(p, FB_FDIGITS, 0);
	    st = FB_ERROR;
	    }
	 fb_store(f, p, hp);
         if (cdb_usrlog > 20){
            sprintf(ubuf, "CS-autoincr: %s (%s)(%c)(%ld)",
               p, f->id, f->lock, bs->bs_reccnt);
            fb_usrlog_msg(ubuf);
            }
         return(st);
      }
