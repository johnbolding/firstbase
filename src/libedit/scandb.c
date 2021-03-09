/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: scandb.c,v 9.0 2001/01/09 02:56:41 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Scandb_sid[] = "@(#) $Id: scandb.c,v 9.0 2001/01/09 02:56:41 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <vdict.h>

#include <setjmp.h>
#include <signal.h>

static jmp_buf jmp_env;
static char oldkey[FB_MAXLINE] = {""};	/* for value of the last key */
static char *BADPAT = "bad regular expression: ";
extern short int ignore_case;		/* case insensative flag */
extern short int ignore_newline;	/* newline flag for SYS_V */
extern long rec;			/* current record number */

#define FORWARD 	CHAR_SLASH	/* meta characters for searches */
#define BACKWARD 	CHAR_BACKSLASH
#define LASTRECORD 	CHAR_DOLLAR
#define FIRSTRECORD	CHAR_PERCENT

static long seqsearch(char *key);
static int seqmatch(char *key);
static RETSIGTYPE scandb_onintr(int disp);
static void upper(char *s);
static void nl_to_blank(char *s);
/*static void seqcount(void);*/

extern short int cdb_regexp;

/* 
 *  scandb - scan a fb_database looking for com.
 *           errors propogate through as record 0, which is Never found.
 *           scandb is used when no index is in place.
 *           type defaults: 1 = incr rec, 2 = DOT, 3 = decr rec
 */

   long fb_scandb(key, tdef)
      char *key;
      int tdef;
      
      {
	 long rrec = 0L;
	 char buffer[100];

	 if (key[0] == LASTRECORD)
	    rrec = cdb_db->reccnt;
	 else if (key[0] == FIRSTRECORD)
	    rrec = 1L;
         else if (tdef != 0){		/* provide defaults */
	    rrec = rec;
            if (tdef == 1){             /* default --- add one to rec */
               rrec++;
	       if (rrec > cdb_db->reccnt)
	          rrec = 1L;
	       }
            else if (tdef == 3){	/* default --- decr one from rec */
               rrec--;
	       if (rrec < 1)
	          rrec = cdb_db->reccnt;
	       }
	    }
	 else{				/* search for matching record */
            if (cdb_db->reccnt == 0)
               return(0L);
	    if (rec <= 1 || rec > cdb_db->reccnt)
	       rec = 1;
	    sprintf(buffer, "Working...Use <INTERRUPT> to stop searching...");
	    fb_fmessage(buffer);
	    if (setjmp(jmp_env)){	/* return from interrupt signal */
	       sprintf(buffer, 
	          "Interrupted sequential search at record %ld\n", rec);
	       fb_fmessage(buffer);
	       FB_PAUSE();
	       rrec = -2L;
	       }
	    else{			/* normal path - search for match */
               fb_allow_int();
	       signal(SIGINT, scandb_onintr);
	       rrec = seqsearch(key);
	       }
            fb_catch_int();
	    signal(SIGINT, SIG_IGN);
	    }
         return(rrec);
      }

/* 
 *  scandb_query - scan a fb_database looking for previous pattern.
 *		determine current location among whole group that match.
 *		display this information.
 */

   void fb_scandb_query()
      
      {
	 char buffer[100];

         if (cdb_db->reccnt == 0)
            return;
         if (rec <= 1 || rec > cdb_db->reccnt)
            rec = 1;
         sprintf(buffer, "Working...Use <INTERRUPT> to stop counting...");
         fb_fmessage(buffer);
         if (setjmp(jmp_env)){	/* return from interrupt signal */
            sprintf(buffer, 
               "Interrupted sequential count at record %ld\n", rec);
            fb_fmessage(buffer);
            FB_PAUSE();
            }
         else{			/* normal path - search for match */
            signal(SIGINT, scandb_onintr);
            }
         signal(SIGINT, SIG_IGN);
      }

/*
 * seqsearch - sequential search mechanism
 */

   static long seqsearch(key)
      char *key;

      {
         long rrec = 0L, last_rec;
	 int dir = 1;

         fb_trim(key);
	 if (key[0] == FORWARD || key[0] == BACKWARD){
	    if (key[0] == BACKWARD)
	       dir = -1;
	    if (key[1] == NULL){	/* use remembered pattern */
	       if (oldkey[0] == NULL){
	          fb_move(cdb_t_lines, 1); fb_clrtoeol();
                  signal(SIGINT, SIG_IGN);
		  fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
		  return(-2L);
		  }
	       strcpy(key, oldkey);
	       }
	    else{
	       strcpy(cdb_bfld, key);
	       strcpy(key, cdb_bfld + 1);
	       }
	    rec += dir;		/* bump towards dir to get off cur rec */
	    if (rec > cdb_db->reccnt)
	       rec = 1;
	    else if (rec <= 0)
	       rec = cdb_db->reccnt;
	    }
	 if (cdb_regexp){
            if (ignore_case)
               upper(key);
	    if (re_comp(key) != NULL){
	       fb_move(cdb_t_lines, 1);
               fb_clrtoeol();
               signal(SIGINT, SIG_IGN);
	       fb_serror(FB_MESSAGE, BADPAT, key);
	       return(-2L);
	       }
	    }
	 last_rec = rec - dir;
	 if (last_rec <= 0)
	    last_rec = cdb_db->reccnt;
	 else if (last_rec > cdb_db->reccnt)
	    last_rec = 1;
	 strcpy(oldkey, key);
	 for (; ; rec += dir){
	    if (rec <= 0)
	       rec = cdb_db->reccnt;
	    else if (rec > cdb_db->reccnt){
               fb_bell(); fb_refresh();
               /* fb_fmessage("Search wrapped..."); */
	       rec = 1;
               }
	    if (fb_getrec(rec, cdb_db) == FB_ERROR)
	       fb_xerror(FB_FATAL_GETREC, cdb_db->dbase, (char *) &rec);
	    if (!(FB_ISDELETED(cdb_db)))
	       if (seqmatch(key) == FB_AOK){
		  rrec = rec;
		  break;
		  }
	    if (rec == last_rec)
	       break;
	    }
	 return(rrec);
      }


#if THIS_CODE_IS_UNUSED

/*
 * seqcount - sequential count mechanism - from record level only.
 */

   static void seqcount()

      {
         long rrec = 0L, tcount, dcount;
         char key[FB_MAXLINE], buf[FB_MAXLINE];

         if (oldkey[0] == NULL || rec < 1 || rec > cdb_db->reccnt){
            fb_move(cdb_t_lines, 1);
            fb_clrtoeol();
            signal(SIGINT, SIG_IGN);
            fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
            return;
            }
         strcpy(key, oldkey);
	 if (cdb_regexp){
            if (ignore_case)
               upper(key);
	    if (re_comp(key) != NULL){
	       fb_move(cdb_t_lines, 1);
               fb_clrtoeol();
               signal(SIGINT, SIG_IGN);
	       fb_serror(FB_MESSAGE, BADPAT, key);
	       return;
	       }
	    }
         tcount = dcount = 0;
	 for (rrec = 1; rrec <= cdb_db->reccnt; rrec++){
	    if (fb_getrec(rrec, cdb_db) == FB_ERROR)
	       fb_xerror(FB_FATAL_GETREC, cdb_db->dbase, (char *) &rrec);
	    if (!(FB_ISDELETED(cdb_db)))
	       if (seqmatch(key) == FB_AOK){
		  tcount++;
                  if (rec == rrec)
                     dcount = tcount;
		  }
	    }
         sprintf(buf, "Search String: %s", key);
         fb_fmessage(buf);
         sprintf(buf, "Record is Occurrence %ld of %ld Total Matches",
            dcount, tcount);
         fb_serror(FB_MESSAGE, buf, NIL);
      }

#endif /* THIS CODE IS UNUSED */

/*
 * seqmatch - sequential match routine. tries to sequentially locate key
 *            in the first referenced fb_field of the fb_database.
 */

   static int seqmatch(key)
      char *key;

      {
         char *k, *f;
	 
	 if (cdb_regexp){
	    if (re_comp(key) != NULL)
	       return(FB_ERROR);
            strcpy(cdb_afld, cdb_ip[0]->fld);
            if (ignore_case)
               upper(cdb_afld);		/* does nflag also if needed */
            else if (ignore_newline)
               nl_to_blank(cdb_afld);	/* for SYS_V, change NL to FB_BLANK */
	    if (re_exec(cdb_afld))
	       return(FB_AOK);
	    return(FB_ERROR);
	    }
	 for (k = key, f = cdb_ip[0]->fld; *k && *f && *k == *f; k++, f++)
	    ;
	 if (*k == NULL)
	    return(FB_AOK);
	 return(FB_ERROR);
      }

   static RETSIGTYPE scandb_onintr(disp)		/* set up jump point */
      int disp;

      {
         longjmp(jmp_env, 1);
      }

   static void upper(s)	/* convert to upper for C.I, and/or do nflag */
      char *s;
      
      {
         char *p;
	 
	 for (p = s; *p; p++){
	    if (islower(*p))
	       *p = toupper(*p);
	    else if (ignore_newline && *p == FB_NEWLINE)
	       *p = FB_BLANK;
	    }
      }

   static void nl_to_blank(s)
      char *s;
      
      {
         char *p;
	 
	 for (p = s; *p; p++)
	    if (*p == FB_NEWLINE)
	       *p = FB_BLANK;
      }
