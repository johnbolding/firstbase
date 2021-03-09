/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: search.c,v 9.0 2001/01/09 02:57:02 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Search_sid[] = "@(#) $Id: search.c,v 9.0 2001/01/09 02:57:02 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char *BADPAT = "bad regular expression: ";
short int search_position = 0;
long fail_irec = 0L;
extern short int cdb_regexp;
extern char cdb_EOREC;

long fb_search(int fd, char *key, int col, long bsbeg, long bsend,
   long bsmax, int recsiz, int backup, char *buf);
   
/*
 * megasearch - pump through search without accepting any ^E tainted index
 *	entries (deleted index entries).
 */

   long fb_megasearch(fd, key, col, bsbeg, bsend, bsmax, recsiz, backup, buf)
      int col, recsiz, backup, fd;
      char *key, *buf;
      long bsend, bsmax, bsbeg;
      
      {
         long rec, nbsmax;

         /* force binary search to begin to be at record 1 */
         bsbeg = 1;
	 nbsmax = bsmax;
	 for (;;){
	    rec = fb_search(fd, key, col, bsbeg++, bsend, nbsmax, recsiz, 
	       backup, buf);
	    /* ignore those with a ^E in index. */
	    if (rec == 0L || buf[recsiz - 2] != cdb_EOREC)
	       break;
	    }
	 return(rec);
      }

/* 
 *  search - do binary search on index for first entry. return record #.
 *     fd = descriptor, 
 *     key = key to search for.
 *     col = column to start key examination (usually 0)
 *     bsbeg = binary search begin; 
 *     bsend = binary search end
 *     bsmax = search max point.
 *     recsiz = record size.
 *     backup = flag indicates 'goto top of equal key' on hits.
 *     buf = space for reads of recsiz in length. db->irec will do.
 *
 *     search sequentially searches overflow area > bsend if needed.
 *     fb_xerror(FB_FATAL_FGETREC, NIL, &rec) used for fatal errors.
 *	(index reads).
 */
 
   long fb_search(fd, key, col, bsbeg, bsend, bsmax, recsiz, backup, buf)
      int col, recsiz, backup, fd;
      char *key, *buf;
      long bsend, bsmax, bsbeg;
      
      {
         long rec, t, b, lastrec, overflow;
         register p, i = 0;
	 int keylen = 0;
         char c;
         
         search_position = 0;
         fail_irec = 0L;
	 if (bsbeg > bsend)
            overflow = bsbeg;
         else
            overflow = bsend + 1;
         b = bsbeg;
         t = bsend + 1L;
	 lastrec = 0L;
	 if (cdb_regexp){
	    if (re_comp(key) != NULL){
	       fb_serror(FB_MESSAGE, BADPAT, key);
	       return(0L);
	       }
	    keylen = strlen(key);
	    overflow = bsbeg;
	    bsbeg = bsend + 1;	/* forces sequential search */
	    }
         for( ; bsbeg <= bsend; ){
	    if (b > t)
	       break;
            rec = (long) (t + b)/ 2L;
	    if (rec == lastrec || rec <= 0L || rec > bsmax){
               if (rec == lastrec)
                  search_position = 1;
	       break;
               }
	    lastrec = rec;
            if (fb_fgetrec(rec, fd, recsiz, buf, 0) == FB_ERROR)
	       fb_xerror(FB_FATAL_FGETREC, NIL, (char *) &rec);
	    p = 0;
	    if (!cdb_regexp){
	       for (i = col; (buf[i] == key[p]) && (key[p] != NULL); p++,i++)
		  ;
	       }
            else{
               c = buf[recsiz - 11];
               buf[recsiz - 11] = NULL;
               if (re_exec(buf)){
                  p = keylen;
                  buf[recsiz - 11] = c;
                  }
               }
            if (key[p] == NULL){
               if (backup == 1){
                  for (rec--; rec >= bsbeg && rec > 0; rec--){
		     if (fb_fgetrec(rec, fd, recsiz, buf, 0) == FB_ERROR)
	                fb_xerror(FB_FATAL_FGETREC, NIL, (char *) &rec);
                     p = 0;
		     if (!cdb_regexp){
			for (i = col;(buf[i]==key[p]) && (key[p]!=NULL);
			      p++,i++)
			   ;
			}
                     else{
                        c = buf[recsiz - 11];
                        buf[recsiz - 11] = NULL;
                        if (re_exec(buf)){
                           p = keylen;
                           buf[recsiz - 11] = c;
                           }
                        }
                     if (key[p] != NULL)
                        break;
                     }
                  if (fb_fgetrec(++rec, fd, recsiz, buf, 0) == FB_ERROR)
		     fb_xerror(FB_FATAL_FGETREC, NIL, (char *) &rec);
                  }
               return(rec);
               }
            else if (rec == b){
               if (buf[i] < key[p])
                  search_position = 1;
               else
                  search_position = -1;
               break;
               }
            else if (buf[i] < key[p])
               b = rec;
            else
               t = rec;
            }
	 if (overflow <= 0)
	    overflow = 1;
         for (rec = overflow; rec <= bsmax; rec++){
            if (fb_fgetrec(rec, fd, recsiz, buf, 0) == FB_ERROR)
	       fb_xerror(FB_FATAL_FGETREC, NIL, (char *) &rec);
	    if (buf[recsiz - 2] == cdb_EOREC)
	       continue;			/* ignore this find */
            p = 0;
	    if (!cdb_regexp){
	       for (i = col; (buf[i] == key[p]) && (key[p] != NULL); p++,i++)
		  ;
	       }
            else{
               c = buf[recsiz - 11];
               buf[recsiz - 11] = NULL;
               if (re_exec(buf)){
                  p = keylen;
                  buf[recsiz - 11] = c;
                  }
               }
            if (key[p] == NULL)
               return(rec);
            }
         fail_irec = lastrec;
         return(0L);
      }

/* 
 *  lastmatch - return record # of last match in file.
 *     fd = descriptor, 
 *     key = key to search for.
 *     rec = record number to start with -- assumed done by fb_search()
 *     recsiz = record size.
 *     buf = space for reads of recsiz in length. db->irec will do.
 */
 
   long fb_lastmatch(fd, key, rec, bsend, recsiz, buf)
      int fd, recsiz;
      char *key, *buf;
      long rec, bsend;
      
      {
         register int p, i;
	 int keylen = 0;
         char c;
         
	 if (cdb_regexp){
	    if (re_comp(key) != NULL){
	       fb_serror(FB_MESSAGE, BADPAT, key);
	       return(0L);
	       }
	    keylen = strlen(key);
	    }
         for (rec++; rec <= bsend; rec++){
            if (fb_fgetrec(rec, fd, recsiz, buf, 0) == FB_ERROR)
	       fb_xerror(FB_FATAL_FGETREC, NIL, (char *) &rec);
	    if (buf[recsiz - 2] == cdb_EOREC)
	       continue;			/* ignore this find */
            p = 0;
	    if (!cdb_regexp){
	       for (i = 0; (buf[i] == key[p]) && (key[p] != NULL); p++,i++)
		  ;
	       }
            else{
               c = buf[recsiz - 11];
               buf[recsiz - 11] = NULL;
               if (re_exec(buf)){
                  p = keylen;
                  buf[recsiz - 11] = c;
                  }
               }
            if (key[p] != NULL)
               break;
            }
         return(--rec);
      }

/* 
 *  search_count - count all records in the hp idx that match key
 *     key = key to search for.
 *     col = column to start key examination (usually 0)
 *     cur_rec = the outside current record - used to set dcount
 *
 *     return long values in dcount (dot count) and tcount (total count)
 */
 
   void fb_search_count(hp, tcount, dcount, key, col, cur_rec)
      int col;
      char *key;
      fb_database *hp;
      long *tcount, *dcount, cur_rec;
      
      {
         long rec, bsmax, overflow, tc, dc, bsend, bsbeg;
         register p, i;
	 int keylen = 0, recsiz, fd;
         char *buf, c;

         *tcount = 0;
         *dcount = 0;
         tc = dc = 0;
         recsiz = hp->irecsiz;
         buf = hp->irec;
         fd = hp->ifd;
         bsbeg = 1;
         bsend = hp->bsend;
         bsmax = hp->bsmax;
	 overflow = (long) ((bsbeg > bsend) ? bsbeg : bsend + 1);
	 if (cdb_regexp){
	    if (re_comp(key) != NULL){
	       fb_serror(FB_MESSAGE, BADPAT, key);
	       return;
	       }
	    keylen = strlen(key);
	    overflow = bsbeg;
	    }

         if (!cdb_regexp){
            /* do a binary search to get going */
            rec = fb_search(hp->ifd, key, col, bsbeg, bsend, 
                  bsmax, hp->irecsiz, 1, hp->irec);
            if (rec == 0L)
               return;
            }
         else
            rec = bsend + 1;		/* regexp, so force overflow only */

         /* now count the indexed records that match - in line in index*/
         for (; rec <= bsend; rec++){
            if (fb_fgetrec(rec, fd, recsiz, buf, 0) == FB_ERROR)
	       fb_xerror(FB_FATAL_FGETREC, NIL, (char *) &rec);
	    if (buf[recsiz - 2] == cdb_EOREC)
	       continue;			/* ignore this find */
            p = 0;
	    if (!cdb_regexp){
	       for (i = col; (buf[i] == key[p]) && (key[p] != NULL); p++,i++)
		  ;
	       }
            else{
               c = buf[recsiz - 11];
               buf[recsiz - 11] = NULL;
               if (re_exec(buf)){
                  p = keylen;
                  buf[recsiz - 11] = c;
                  }
               }
            if (key[p] == NULL){
               tc++;
               if (rec == cur_rec)
                  dc = tc;
               }
            else
               break;			/* on first failure, get out */
            }

         /* now count the indexed records that match - overflow area only */
         for (rec = overflow; rec <= hp->bsmax; rec++){
            if (fb_fgetrec(rec, fd, recsiz, buf, 0) == FB_ERROR)
	       fb_xerror(FB_FATAL_FGETREC, NIL, (char *) &rec);
	    if (buf[recsiz - 2] == cdb_EOREC)
	       continue;			/* ignore this find */
            p = 0;
	    if (!cdb_regexp){
	       for (i = col; (buf[i] == key[p]) && (key[p] != NULL); p++,i++)
		  ;
	       }
            else{
               c = buf[recsiz - 11];
               buf[recsiz - 11] = NULL;
               if (re_exec(buf)){
                  p = keylen;
                  buf[recsiz - 11] = c;
                  }
               }
            if (key[p] == NULL){
               tc++;
               if (rec == cur_rec)
                  dc = tc;
               }
            }
         *tcount = tc;
         *dcount = dc;
         return;
      }
