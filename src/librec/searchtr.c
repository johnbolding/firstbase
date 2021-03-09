/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: searchtr.c,v 9.0 2001/01/09 02:57:02 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Searchtree_sid[] = "@(#) $Id: searchtr.c,v 9.0 2001/01/09 02:57:02 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char oldkey[FB_MAXLINE] = {""};	/* for value of the last key */
static char *BADPAT = "bad regular expression: ";

static long bs_search(long srec, char *key, fb_bseq *bs);
static long bs_regexp(char *key, fb_bseq *bs);
long fb_btree_search(char *key, fb_bidx *bi, fb_bseq *bs);

extern short int cdb_regexp;
extern long cdb_failrec;
extern short int cdb_loadfail;

extern long fb_key_record(char *s, int len);
extern long fb_key_eval(fb_bseq *bs);
extern char *fb_key_ptr(fb_bseq *bs);

/*
 * searchtree - search for a btree+ entry in the sequence set with key.
 *	return the record number of this record.
 */

   long fb_searchtree(key, bi, bs)
      char *key;
      fb_bidx *bi;
      fb_bseq *bs;

      {
         long rec;

         cdb_failrec = 0L;
         if (key[0] != NULL)
            strcpy(oldkey, key);
	 if (cdb_regexp)
	    rec = bs_regexp(oldkey, bs);
         else{
            if (FB_OFNUMERIC(cdb_ip[0]->type) || cdb_ip[0]->type == FB_DATE)
               fb_makess(oldkey, cdb_ip[0]->type, cdb_ip[0]->size);
            rec = fb_btree_search(oldkey, bi, bs);
            }
         return(rec);
      }

/*
 * fb_btree_search - search for sequence set node that has a matching key
 */

   long fb_btree_search(key, bi, bs)
      char *key;
      fb_bidx *bi;
      fb_bseq *bs;

      {
         int klen;
         long rec, irec;
         char typ;

         fb_get_idx_head(bi);
         fb_get_seq_head(bs);
         irec = bi->bi_root;
         klen = strlen(key);
         for (;;){
            /* get the index record irec */
            if (fb_idx_getrec(irec, bi) == FB_ERROR)
               return(FB_ERROR);
            
            if (strncmp(key, bi->bi_key1, klen) <= 0){
               rec = bi->bi_left;
               typ = bi->bi_ltype;
               }
            else{
               if (bi->bi_rtype == FB_BLANK ||
                     strncmp(key, bi->bi_key2, klen) <= 0){
                  rec = bi->bi_middle;
                  typ = bi->bi_mtype;
                  }
               else{
                  rec = bi->bi_right;
                  typ = bi->bi_rtype;
                  }
               }

            /*
             * at this point, rec and typ should be set
             * if typ is 1, recurse (tail recurse removed),
             *    else bottom out in bs_search
             */

            if (typ == CHAR_1)
               irec = rec;
            else{
               if (typ == CHAR_0)
                  rec = bs_search(rec, key, bs);
               else
                  rec = 0L;
               break;
               }
            }
         return(rec);
      }

/*
 * bs_search - search for a value in a sequence node
 */

   static long bs_search(srec, key, bs)
      long srec;
      char *key;
      fb_bseq *bs;

      {
         long rec;
         int klen, st, i;
         char *bkey = NIL;

         /* get the sequence record srec */
         if (fb_seq_getrec(srec, bs) == FB_ERROR)
            return(FB_ERROR);
         klen = strlen(key);

         /*
          * Since deleted keys are not reclaimed, the real key may
          * only be nearby. so, loop here, searching btree sequence set (right)
          * until past the point where this key might be.
          *
          * Also, deleted records that match should not be considered
          * a find since "<key><R21>" can be deleted before "<key><R31>"
          */

         for (st = 1; st > 0; ){
            for (rec = 0L, i = 1; i <= 3; i++){
               bs->bs_curkey = i;
               switch(bs->bs_curkey){
                  case 1: bkey = bs->bs_key1; break;
                  case 2: bkey = bs->bs_key2; break;
                  case 3: bkey = bs->bs_key3; break;
                  }
               st = strncmp(key, bkey, klen);
               /* if search key is <= current bkey, break out */
               if (st <= 0){
                  /* if key is deleted, dont accept status of 0 */
                  if (st == 0 && fb_key_record(bkey, bs->bs_ksize) == -1)
                     st = 1;
                  else
                     break;
                  }
               }
            /* if st <= 0, inner loop found key OR passed its storage point */
            if (st <= 0)
               break;
            srec = bs->bs_next;
            if (srec == 0){
               st = -1;
               break;
               }
            if (fb_seq_getrec(srec, bs) == FB_ERROR)
               return(FB_ERROR);
            bs->bs_curkey = 1;
            }

         /* if the search was successful or app wants to load fail point */
         if (st == 0 || cdb_loadfail){
            rec = fb_key_record(bkey, bs->bs_ksize);
            if (st != 0 && rec > 0){
               cdb_failrec = rec;
               rec = 0;
               }
            }

         if (rec < 0){
            /*
             * a rec of -1 indicates a deleted index marker.
             * here it means a deleted match was found. so skip to
             * the next undeleted record and look at its key.
             */
            for (srec = 0; rec <= 0; ){
               bs->bs_curkey++;
               if (bs->bs_curkey > 3){
                  /* read next node */
                  srec = bs->bs_next;
                  if (srec == 0){
                     srec = -1;
                     break;
                     }
                  if (fb_seq_getrec(srec, bs) == FB_ERROR)
                     return(FB_ERROR);
                  bs->bs_curkey = 1;
                  }
               if (srec < 0)
                  break;

               /*
                * idea here is that a match (or a loadfail request) has
                * occured above, and it must have been a deleted key.
                * so, the very first non-deleted record is the one
                * we are after.
                *
                * however, the status depends on whether the key matches.
                */
               rec = fb_key_eval(bs);
               if (rec > 0)
                  break;
               }
            if (srec >= 0){
               st = strncmp(key, bkey, klen);
               if (st != 0){
                  cdb_failrec = rec;
                  rec = 0;
                  }
               }
            }
         if (rec < 0)
            rec = 0;
         return(rec);
      }

/*
 * bs_regexp - used to do sequential search on an index. usually regexp=ON
 */

   static long bs_regexp(key, bs)
      char *key;
      fb_bseq *bs;

      {
         long rec, first_bs_recno;
         int first_bs_curkey, firstloop = 1, siz, st;
         char *buf, c;

         if (re_comp(key) != NULL){
            fb_serror(FB_MESSAGE, BADPAT, key);
            return(0L);
            }
         fb_getbxhead(bs->bs_fd, &(bs->bs_head), &(bs->bs_tail),
            &(bs->bs_reccnt), &(bs->bs_free));
         if (bs->bs_recno == 0){
            if (fb_seq_getrec(bs->bs_head, bs) == FB_ERROR)
               return(0);
            bs->bs_curkey = 0;
            }
         first_bs_recno = bs->bs_recno;
         first_bs_curkey = bs->bs_curkey;
         for (;;){
            rec = 0;
            if (bs->bs_curkey >= 3){
               if (bs->bs_next != 0)
                  fb_seq_getrec(bs->bs_next, bs);
               else if (fb_seq_getrec(bs->bs_head, bs) == FB_ERROR)
                  return(0);
               bs->bs_curkey = 0;
               firstloop = 0;
               }
            bs->bs_curkey++;
            rec = fb_key_eval(bs);
            if (rec > 0 && rec <= cdb_db->reccnt){
               /* if there was a key, now check regexp pattern */
               buf = fb_key_ptr(bs);
               siz = bs->bs_ksize - (FB_RECORDPTR + 1);
               c = buf[siz];
               buf[siz] = NULL;
               st = re_exec(buf);
               buf[siz] = c;
               if (st)
                  break;
               rec = 0;
               }
            else if (rec == 0)
               bs->bs_curkey = 4;
            if (bs->bs_recno == first_bs_recno &&
                  bs->bs_curkey >= first_bs_curkey && !firstloop)
               break;
            }
         return(rec);
      }

/*
 * fb_lastmatch_tree - fine the lastmatch for a given key, return its rec
 *	assume bs already points to the matching record.
 */

   long fb_lastmatch_tree(key, bs)
      char *key;
      fb_bseq *bs;

      {
         long rec = 0, srec = 0, match_srec = 0, match_curkey = 0;
         char *bkey;
         int klen, st;

         /*
          * first assert that the current record does match key.
          */

         klen = strlen(key);
         bkey = fb_key_ptr(bs);
         srec = bs->bs_recno;
         st = strncmp(key, bkey, klen);
         if (st != 0)
            return(FB_ERROR);
         match_srec = srec;
         match_curkey = bs->bs_curkey;
         for (;;){
            bs->bs_curkey++;
            if (bs->bs_curkey > 3){
               /* read next node */
               srec = bs->bs_next;
               if (srec == 0){
                  srec = -1;
                  break;
                  }
               if (fb_seq_getrec(srec, bs) == FB_ERROR)
                  return(FB_ERROR);
               bs->bs_curkey = 1;
               }
            bkey = fb_key_ptr(bs);
            st = strncmp(key, bkey, klen);
            /* if search key is not an exact match, break out */
            if (st != 0)
               break;
            match_srec = srec;
            match_curkey = bs->bs_curkey;
            }

         /* assert: match_srec must be > 0 at this point */
         /* load the last match found so upon return the bs node is set */
         if (fb_seq_getrec(match_srec, bs) == FB_ERROR)
            return(FB_ERROR);
         bs->bs_curkey = match_curkey;
         rec = fb_key_eval(bs);
         return(rec);
      }
