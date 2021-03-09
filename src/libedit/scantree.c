/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: scantree.c,v 9.0 2001/01/09 02:56:41 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Scantree_sid[] = "@(#) $Id: scantree.c,v 9.0 2001/01/09 02:56:41 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern long rec;			/* record marker */
extern short pindx;			/* level of index */
extern short cdb_datedisplay;
extern short int cdb_regexp;

#define FORWARD 	CHAR_SLASH	/* meta characters for searches */
#define BACKWARD 	CHAR_BACKSLASH
#define LASTRECORD 	CHAR_DOLLAR
#define FIRSTRECORD	CHAR_PERCENT

static char old_rawkey[FB_MAXLINE] = {""};	/* last raw key supplied */
static char old_formkey[FB_MAXLINE] = {""};/* last raw key, formatted */

/* 
 *  scantree - scan an index looking for com, or handle defaults.
 *            errors propogate through as record 0, which is Never found.
 *
 *            used by both dbedit and dbvedit via fb_scanfor().
 */

   long fb_scantree(key, tdef)
      char *key;
      int   tdef;
      
      {
         
         int offset = 0, len, loopcount = 0;
         char *tkey, buf[FB_MAXLINE];
         fb_bseq *bs;
	 int dir = 1;

	 if (fb_getxhead(cdb_db->ihfd, &(cdb_db->bsmax), &(cdb_db->bsend)) ==
               FB_ERROR)
	    return(0L);
         fb_lock_head(cdb_db);
         fb_gethead(cdb_db);
         fb_unlock_head(cdb_db);
	 if (cdb_db->reccnt <= 0)
	    return(0L);
         bs = cdb_db->b_seq;
         fb_get_seq_head(bs);
	 if (key[0] == LASTRECORD){
            /* set to last record via tail */
            if (fb_seq_getrec(bs->bs_tail, bs) == FB_ERROR)
               return(0L);
            rec = 0;
            if ((rec = fb_key_record(bs->bs_key3, bs->bs_ksize)) > 0)
               bs->bs_curkey = 3;
            else if ((rec = fb_key_record(bs->bs_key2, bs->bs_ksize)) > 0)
               bs->bs_curkey = 2;
            else{
               rec = fb_key_record(bs->bs_key1, bs->bs_ksize);
               bs->bs_curkey = 1;
               }
	    }
	 else if (key[0] == FIRSTRECORD){
            /* set to first record via head */
            if (fb_seq_getrec(bs->bs_head, bs) == FB_ERROR)
               return(0L);
            if ((rec = fb_key_record(bs->bs_key1, bs->bs_ksize)) > 0)
               bs->bs_curkey = 1;
            else if ((rec = fb_key_record(bs->bs_key2, bs->bs_ksize)) > 0)
               bs->bs_curkey = 2;
            else{
               rec = fb_key_record(bs->bs_key3, bs->bs_ksize);
               bs->bs_curkey = 3;
               }
            }
         else if (tdef != 0){
            if (bs->bs_recno == 0){
               if (fb_seq_getrec(bs->bs_head, bs) == FB_ERROR)
                  return(0);
               bs->bs_curkey = 0;
               }
            if (tdef == 1){             /* default --- NEXT RECORD */
               for (loopcount = 0;;){
                  if (bs->bs_curkey >= 3){
                     if (bs->bs_next != 0)
                        fb_seq_getrec(bs->bs_next, bs);
                     else{
                        if (++loopcount > 1)
                           return(0);
                        if (fb_seq_getrec(bs->bs_head, bs) == FB_ERROR)
                           return(0);
                        }
                     bs->bs_curkey = 0;
                     }
                  bs->bs_curkey++;
                  rec = fb_key_eval(bs);
                  if (rec > 0 && rec <= cdb_db->reccnt)
	             break;
                  else if (rec == 0)
                     bs->bs_curkey = 4;
	          }
               }
            else if (tdef == 3){	/* decr one from irec */
               for (loopcount = 0;;){
                  if (bs->bs_curkey <= 1){
                     if (bs->bs_prev != 0)
                        fb_seq_getrec(bs->bs_prev, bs);
                     else{
                        if (++loopcount > 1)
                           return(0);
                        if (fb_seq_getrec(bs->bs_tail, bs) == FB_ERROR)
                           return(0);
                        }
                     bs->bs_curkey = 4;
                     }
                  bs->bs_curkey--;
                  rec = fb_key_eval(bs);
                  if (rec > 0 && rec <= cdb_db->reccnt)
	             break;
	          }
	       }
            else{			/* tdef == 2 - DOT command */
               if (bs->bs_curkey == 0)
                  bs->bs_curkey = 1;
               rec = fb_key_eval(bs);
               }
            /* auto index remnant ??? */
	    if (rec < 0L || rec > cdb_db->reccnt)
               rec = 0L;
            }
         else{
            key = fb_trim(key);
	    if (key[0] == FORWARD || key[0] == BACKWARD){
               if (key[0] == BACKWARD)
                  dir = -1;
	       if (key[1] == NULL){
	          if (old_rawkey[0] == NULL){
		     fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
		     return(-2L);
		     }
                  /*
                   * do a next record without the getrec
                   * - compare key if no match do searchtree
                   *
                   * this should go backwards also, but does not right now.
                   */
                  bs = cdb_db->b_seq;
                  if (!cdb_regexp){
                     for (rec = 0;;){
                        if (bs->bs_curkey >= 3){
                           if (bs->bs_next == 0){
                              rec = 0L;
                              break;
                              }
                           fb_seq_getrec(bs->bs_next, bs);
                           bs->bs_curkey = 0;
                           }
                        bs->bs_curkey++;
                        rec = fb_key_eval(bs);
                        if (rec > 0L && rec <= cdb_db->reccnt)
                           break;
                        else if (rec == 0)
                           bs->bs_curkey = 4;
                        }
                     }
                  if (rec > 0 && !cdb_regexp){
                     tkey = fb_key_ptr(bs);
                     len = strlen(old_formkey);
                     if (strncmp(tkey, old_formkey, len) == 0)
                        return(rec);
                     }
                  strcpy(key, old_rawkey);
		  }
	       else
	          offset = 1;
               }
            /*
             * old_rawkey stores the raw key, no formatting
             *    while old_formkey stores the formatted key
             */
	    strcpy(old_rawkey, key+offset);
            if (FB_OFNUMERIC(cdb_ip[pindx]->type)){
               fb_rjustify(old_formkey, old_rawkey, cdb_ip[pindx]->size,
                  cdb_ip[pindx]->type);
               }
            else
               strcpy(old_formkey, old_rawkey);
            if (cdb_ip[pindx]->type == FB_DATE && strlen(old_formkey) == 6){
               if (cdb_datedisplay == 8)
                  fb_endate(old_formkey);
               else{
                  fb_longdate(buf, old_formkey);
                  strcpy(old_formkey, buf);
                  fb_long_endate(old_formkey);
                  }
               }
	    fb_fmessage(SYSMSG[S_SEARCHING]);
            rec = fb_searchtree(key + offset, cdb_db->b_idx, cdb_db->b_seq);
            }
         if (rec <= 0L || rec > cdb_db->reccnt)
            return(0L);
         return(rec);
      }

/*
 * scantree_query - handle the ^Q mechanism to show how many matches
 *		   in an interactive manner.
 */

   void fb_scantree_query()

      {
         int len, curkey_bs;
         char key[FB_MAXLINE], buf[FB_MAXLINE], *tkey;
         long tcount = 0, dcount = 0, trec, recno_bs;
         fb_bseq *bs;

         if (old_rawkey[0] == NULL || rec < 1 || rec > cdb_db->reccnt){
            fb_move(cdb_t_lines, 1); fb_clrtoeol();
            fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
            return;
            }
         bs = cdb_db->b_seq;
	 if (fb_getxhead(cdb_db->ihfd, &(cdb_db->bsmax), &(cdb_db->bsend)) ==
               FB_ERROR)
	    return;
	 if (cdb_db->reccnt <= 0)
	    return;
         recno_bs = bs->bs_recno;
         curkey_bs = bs->bs_curkey;
         strcpy(buf, old_rawkey);
         if (FB_OFNUMERIC(cdb_ip[pindx]->type)){
            fb_rjustify(key, buf, cdb_ip[pindx]->size, cdb_ip[pindx]->type);
            }
         else
            strcpy(key, buf);
         trec = fb_searchtree(key, cdb_db->b_idx, cdb_db->b_seq);
         if (trec <= 0)
            return;
         if (trec == rec)
            dcount = 1;
         for (tcount = 1;;){
            if (bs->bs_curkey >= 3){
               if (bs->bs_next == 0){
                  trec = 0L;
                  break;
                  }
               fb_seq_getrec(bs->bs_next, bs);
               bs->bs_curkey = 0;
               }
            bs->bs_curkey++;
            trec = fb_key_eval(bs);
            if (trec > 0L && trec <= cdb_db->reccnt){ /* ignore rec of 0 */
               tkey = fb_key_ptr(bs);
               len = strlen(old_formkey);
               if (strncmp(tkey, old_formkey, len) == 0){
                  tcount++;
                  if (trec == rec)
                     dcount = tcount;
                  }
               else
                  break;
               }
            else if (trec == 0)
               bs->bs_curkey = 4;
            }
         sprintf(buf, "Search String: %s", key);
         fb_fmessage(buf);
         sprintf(buf, "Record is Occurence %ld of %ld Total Matches",
            dcount, tcount);
         fb_serror(FB_MESSAGE, buf, NIL);
         fb_seq_getrec(recno_bs, bs);
         bs->bs_curkey = curkey_bs;
      }
