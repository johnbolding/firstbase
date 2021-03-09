/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: scanfor.c,v 9.0 2001/01/09 02:56:41 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Scanfor_sid[] = "@(#) $Id: scanfor.c,v 9.0 2001/01/09 02:56:41 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char oldkey[FB_MAXLINE] = {""};	/* for value of the last key */
extern short cdb_datedisplay;
static long last_irec = 0;

extern long 
     rec,			/* record marker */
     irec[];			/* to mark index record levels */
extern short pindx;		/* level of index */

#define FORWARD 	CHAR_SLASH	/* meta characters for searches */
#define BACKWARD 	CHAR_BACKSLASH
#define LASTRECORD 	CHAR_DOLLAR
#define FIRSTRECORD	CHAR_PERCENT

extern short int cdb_wrapscan;
extern char cdb_EOREC;

/* 
 *  scanfor - scan an index looking for com, or handle defaults.
 *            errors propogate through as record 0, which is Never found.
 *
 *            used by both dbedit and dbvedit.
 */

   long fb_scanfor(key, tdef)
      char *key;
      int   tdef;
      
      {
         
         int col, i, backup, offset;
         long nbsmax, obsbeg, bsbeg, bsend;
         char crec[FB_RECORDPTR+1], buf[FB_MAXLINE];

         if (cdb_db->b_tree && cdb_db->b_seq->bs_fd > 0 &&
               cdb_db->b_idx->bi_fd > 0)
            return(fb_scantree(key, tdef));
	 if (cdb_db->ifd < 0)
	    return(fb_scandb(key, tdef));
	 if (fb_getxhead(cdb_db->ihfd, &(cdb_db->bsmax), &(cdb_db->bsend)) ==
               FB_ERROR)
	    return(0L);
	 if (cdb_db->bsmax <= 0)
	    return(0L);
	 if (key[0] == LASTRECORD){
	    rec = cdb_db->bsend;
	    if (cdb_db->bsend > 0L)
               if (fb_fgetrec(rec, cdb_db->ifd, cdb_db->irecsiz,
                     cdb_db->irec, 0) == FB_ERROR)
	          fb_xerror(FB_FATAL_FGETREC, cdb_db->dbase, (char *) &rec);
	    }
	 else if (key[0] == FIRSTRECORD){
	    rec = 1L;
	    if (cdb_db->bsend > 0L)
               if (fb_fgetrec(rec, cdb_db->ifd, cdb_db->irecsiz,
                     cdb_db->irec, 0) == FB_ERROR)
	          fb_xerror(FB_FATAL_FGETREC, cdb_db->dbase, (char *) &rec);
	    }
         else if (tdef != 0){
            if (tdef == 1){             /* default --- add one to irec */
	       if (irec[pindx] > 0L)
	          irec[pindx]++;
               if (irec[pindx] <= 0 || irec[pindx] > cdb_db->bsmax)
	          irec[pindx] = 1;
	        }
            else if (tdef == 3){	/* default --- decr one from irec */
	       if (irec[pindx] > 0L)
	          irec[pindx]--;
               if (irec[pindx] <= 0 || irec[pindx] > cdb_db->bsmax)
	          irec[pindx] = cdb_db->bsmax;
	        }
            rec = irec[pindx];          /* always deref irec */
            for (; rec > 0L && rec <= cdb_db->bsmax; rec++ ){
               if (fb_fgetrec(rec, cdb_db->ifd, cdb_db->irecsiz,
                     cdb_db->irec, 0) == FB_ERROR)
	          fb_xerror(FB_FATAL_FGETREC, cdb_db->dbase, (char *) &rec);
               if (cdb_db->irec[cdb_db->irecsiz - 2] == cdb_EOREC)
	          continue;		/* ignore: its an autoindex remnant */
	       else
	          break;
	       }
	    if (rec < 0L || rec > cdb_db->bsmax)
               rec = 0L;
	    if (irec[pindx] > cdb_db->bsmax)
	       irec[pindx] = cdb_db->bsmax;
            }
         else{
            if (pindx == 0){
               bsend = cdb_db->bsend;
               backup = 1;
               }
            else{
               bsend = irec[pindx];
               backup = 0;
               }
            key = fb_trim(key);
	    offset = 0;
	    if (key[0] == FORWARD || key[0] == BACKWARD){
	       if (key[1] == NULL){
	          if (oldkey[0] == NULL){
		     fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
		     return(-2L);
		     }
	          strcpy(key, oldkey);
		  }
	       else
	          offset = 1;
	       if (key[0] == BACKWARD){
	          backup = 1;
	          if (pindx == 0)
		     bsbeg = 1L;
		  else
		     bsbeg = irec[pindx - 1];
		  }
	       else
		  bsbeg = irec[pindx] + 1L;
	       }
	    else if (equal(key, oldkey))
	       bsbeg = irec[pindx] + 1L;
	    else
	       bsbeg = irec[pindx];
            for (col = 0, i = 0; i < pindx; i++)
               col += cdb_ip[i]->size;
	    if (FB_OFNUMERIC(cdb_ip[pindx]->type)){
	       fb_rjustify(oldkey, key+offset,
                  cdb_ip[pindx]->size, cdb_ip[pindx]->type);
	       strcpy(key, oldkey);
	       offset = 0;
	       }
	    else
	       strcpy(oldkey, key+offset);
	    fb_fmessage(SYSMSG[S_SEARCHING]);
	    if (cdb_ip[pindx]->type == FB_DATE && strlen(key) == 6){
	       if (cdb_datedisplay == 8)
	          fb_endate(key);
	       else{
	          fb_longdate(buf, key);
		  strcpy(key, buf);
		  fb_long_endate(key);
	          }
	       }
	    obsbeg = bsbeg;
	    nbsmax = cdb_db->bsmax;
	    for (;;){
               rec = fb_search(cdb_db->ifd, key+offset, col, bsbeg++, bsend, 
	             nbsmax, cdb_db->irecsiz, backup, cdb_db->irec);
	       if (rec != 0L)
		  if (cdb_db->irec[cdb_db->irecsiz - 2] == cdb_EOREC)
		     continue;			/* ignore this find */
	       if (rec == 0L && obsbeg != 1 && cdb_wrapscan){
                  /* fb_prints("Search wrapped...");
                   * fb_refresh();
                   */
		  nbsmax = obsbeg;
		  obsbeg = bsbeg = 1;
		  if (bsend > nbsmax)
		     bsend = nbsmax;
		  continue;
		  }
	       else
	          break;
	       }
            }
         if (rec <= 0L || rec > cdb_db->bsmax)
            return(0L);
         last_irec = rec;
         irec[pindx] = rec;
         fb_ffetch(cdb_ip[(cdb_db->ifields - 1)], crec, cdb_db->irec, cdb_db);
	 cdb_db->irec[cdb_db->irecsiz] = NULL;
         return(atol(crec));
      }

/*
 * scanfor_query - handle the ^Q mechanism to show how many matches
 *		   in an interactive manner.
 */

   void fb_scanfor_query()

      {
         int col, offset, i;
         char key[FB_MAXLINE], buf[FB_MAXLINE];
         long tcount, dcount;

         if (cdb_db->b_tree && cdb_db->b_seq->bs_fd > 0 &&
               cdb_db->b_idx->bi_fd > 0){
	    fb_scantree_query();
	    return;
            }
	 else if (cdb_db->ifd < 0){
	    fb_scandb_query();
	    return;
            }
         if (oldkey[0] == NULL || rec < 1 || rec > cdb_db->reccnt){
            fb_move(cdb_t_lines, 1); fb_clrtoeol();
            fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
            return;
            }
	 if (fb_getxhead(cdb_db->ihfd, &(cdb_db->bsmax), &(cdb_db->bsend)) ==
               FB_ERROR)
	    return;
	 if (cdb_db->bsmax <= 0)
	    return;
         if (oldkey[0] == FORWARD || oldkey[0] == BACKWARD)
            offset = 1;
         else
            offset = 0;
         for (col = 0, i = 0; i < pindx; i++)
            col += cdb_ip[i]->size;
         if (FB_OFNUMERIC(cdb_ip[pindx]->type)){
            fb_rjustify(key, oldkey+offset, cdb_ip[pindx]->size,
               cdb_ip[pindx]->type);
            offset = 0;
            }
         else
            strcpy(key, oldkey+offset);
         if (cdb_ip[pindx]->type == FB_DATE && strlen(key) == 6){
            if (cdb_datedisplay == 8)
               fb_endate(key);
            else{
               fb_longdate(buf, key);
               strcpy(key, buf);
               fb_long_endate(key);
               }
            }
         fb_search_count(cdb_db, &tcount, &dcount, key+offset, col, last_irec);
         sprintf(buf, "Search String: %s", key);
         fb_fmessage(buf);
         sprintf(buf, "Record is Occurence %ld of %ld Total Matches",
            dcount, tcount);
         fb_serror(FB_MESSAGE, buf, NIL);
      }
