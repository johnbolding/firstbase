/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbrestor.c,v 9.1 2001/02/16 19:47:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbrestor_sid[] = "@(#) $Id: dbrestor.c,v 9.1 2001/02/16 19:47:51 john Exp $";
#endif

/*
 *  dbrestor - is used to recover a fb_database from a KNOWN good database
 *	and KNOWN good fb_database map, using the fb_database log.
 */
 
#include <fb.h>
#include <fb_vars.h>

#if !FB_PROTOTYPES
static void restor();
static void fixdb();
static int loadlog();
static int scanckp();
static int loadrn();
static int nextline();
static int loadfields();
static int nextfield();
static int nextread();
#else /* FB_PROTOTYPES */
extern main(int, char **);
static void restor(char *);
static void fixdb(void);
static int loadlog(void);
static int scanckp(void);
static int loadrn(void);
static int nextline(char *);
static int loadfields(void);
static int nextfield(char *);
static int nextread(char *);
/*static void usage(void);*/
#endif /* FB_PROTOTYPES */

extern char *cdb_DBCLEAN;
#define BLKSIZE		1024
#define SEPARATOR	'|'

fb_database *hp;			/* pointer to current database */
extern short cdb_allow_dirty;
long cpos = 0;				/* current log position */
char buffer[BLKSIZE + 1];		/* for buffered read */
int verbose = 0;

/*
 * main - mainline logic for dbrestor
 */

   main(argc, argv)
   int argc;
   char *argv[];
      {
	 char dname[FB_MAXNAME];
	 int j;

         (void) Dbrestor_sid;

         fb_getargs(argc, argv, FB_NODB);
	 if (fb_testargs(argc, argv, "-v") > 0){
	    verbose = 1;
	    cdb_batchmode = 1;
	    }
	 for (j = 1; j < argc; j++)
	    if (!(argv[j][0] == '-')){
	       fb_rootname(dname, argv[j]);
	       restor(dname);
	       }
         fb_ender();
      }
      
/*
 *  restor - simple restore of a fb_database using the dbase log.
 */      

   static void restor(dname)
      char *dname;
      
      {
         int j;
	 char msg[FB_MAXLINE], com[FB_MAXLINE];
	 
	 hp = fb_dballoc();
	 cdb_allow_dirty = 1;
 	 fb_dbargs(dname, NIL, hp);
	 fb_opendb(hp, READWRITE, FB_NOINDEX, 0);
	 fb_initlog(hp);
	 fb_scrhdr(hp, "Check Files"); fb_infoline();
	 if (!cdb_batchmode && !cdb_yesmode){
	    fb_move(4, 1);
	    fb_printw("%s%s!!!\n%s\n%s\n",
	       "Make sure no one else is using ", hp->dbase,
	       "After restoring database, all indexes for",
	       "that database will require regeneration.");
	    sprintf(msg, "Ready to Restore %s (y/<no>) ?", hp->dbase);
	    if (fb_mustbe('y', msg, cdb_t_lines, 1) != FB_AOK)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
	 for (j = 0; j < hp->nfields; j++){
	    if (hp->kp[j]->aid != NULL){	/* abuse msg buffer */
	       sprintf(msg, "%s.idict", hp->kp[j]->aid->autoname);
	       unlink(msg);
	       }
	    }
	 fixdb();
	 fb_setdirty(hp, 0);
	 fb_rootname(msg, cdb_db->dbase);
	 if (!cdb_batchmode){
	    fb_move(4,1), fb_clrtobot();
	    fb_scrstat("Cleaning");
	    sprintf(com, "Cleaning %s", msg);
	    fb_fmessage(com);
	    }
	 fb_closedb(hp);
	 sprintf(com, "%s -b %s", cdb_DBCLEAN, msg);
	 fb_system(com, FB_WITHROOT);
      }

/*
 *  fixdb - fix the open fb_database using the log.
 *            good records to the new fb_database (nfd) and recording
 *            their spots in the fb_database map (mfd).
 */
 
   static void fixdb()
       
      {
         int st;
	 long entry;

	 if (!cdb_batchmode){ 
	    fb_move(4, 1); fb_clrtobot();
	    fb_scrstat("Restoring");
	    fb_infoline();
	    }
	 for (entry = 0; ; entry++){
	    st = loadlog();
	    if (st == FB_ERROR)
	       fb_xerror(FB_MESSAGE, "Error in Loading Log Record:", hp->dlog);
	    else if (st == 0)
	       break;
	    if (!cdb_batchmode || verbose){
	       if (!cdb_batchmode)
	          fb_move(12, 22);
	       fb_printw("Log Entry: %ld     Database Record: %-10ld\n",
	          entry, hp->rec);
	       }
            if (fb_putrec(hp->rec, hp)==FB_ERROR)
	       fb_xerror(FB_WRITE_ERROR, SYSMSG[S_RECORD], hp->dbase);
	    }
	 if (entry == 0)
	    fb_serror(FB_MESSAGE, "Warning: No Complete Log Records Found",NIL);
	 if (!cdb_batchmode || verbose)
	    fb_serror(FB_MESSAGE, "Database Restore Successful:", hp->dbase);
      }

/*
 * loadlog - load a complete log record
 */

   static int loadlog()
      {
         int st;

         if (lseek(hp->logfd, cpos, 0) < 0){
	    if (verbose)
	       fb_serror(FB_SEEK_ERROR, "Log File:", hp->dbase);
	    return(FB_ERROR);
	    }
	 if (scanckp() != FB_AOK)
	    return(0);		/* must be end, without a CK rec */
         if (lseek(hp->logfd, cpos, 0) < 0){
	    if (verbose)
	       fb_serror(FB_SEEK_ERROR, "Log File:", hp->dbase);
	    return(FB_ERROR);
	    }
	 st = loadrn();
	 if (st == 0)
	    return(0);
	 else if (st == FB_ERROR){
	    if (verbose)
	       fb_serror(FB_SEEK_ERROR, "Load Record Number Error:", hp->dbase);
	    return(FB_ERROR);
	    }
#if XDEBUG
         fb_printw("Loaded rec number is %ld\n", hp->rec);
#endif
	 if (loadfields() == FB_ERROR){
	    if (verbose)
	       fb_serror(FB_SEEK_ERROR, "Load Fields Error:", hp->dbase);
	    return(FB_ERROR);
	    }
	 cpos = lseek(hp->logfd, 0L, 1);
	 return(FB_AOK);
      }

/*
 * scanckp - scan forward for a check point record.
 */

   static int scanckp()
      {
         char c;
	 int state = 1;

         for (; nextread(&c) != 0; ){
	    switch(state){
	       case 1:
	          if (c == '@')
		     state = 2;
		  else
		     state = 1;
	          break;
	       case 2:
	          if (c == 'C')
		     state = 3;
		  else
		     state = 1;
	          break;
	       case 3:
	          if (c == FB_NEWLINE)
		     return(FB_AOK);
		  else
		     state = 1;
	          break;
	       }
	    }
	 return(FB_ERROR);
      }

/*
 * loadrn - load the next record number - skip any checkpoint records.
 */

   static int loadrn()
      {
         char recnum[FB_MAXLINE];
         
	 if (nextline(recnum) == 0 || recnum[0] != '@')
	    return(FB_ERROR);
	 if (equal(recnum, "@C")){
	    cpos = lseek(hp->logfd, 0L, 1);
	    if (scanckp() != FB_AOK)
	       return(0);		/* must be end, without a CK rec */
	    if (lseek(hp->logfd, cpos, 0) < 0){
	       if (verbose)
	          fb_serror(FB_SEEK_ERROR, "Log File:", hp->dbase);
	       return(FB_ERROR);
	       }
	    if (nextline(recnum) == 0)
	       return(0);
	    if (recnum[0] != '@')
	       return(FB_ERROR);
	    }
	 hp->rec = atol(recnum + 1);
	 return(FB_AOK);
      }

/*
 * nextline - get a line from fb_input, strip FB_NEWLINE, and store NULL
 */

   static nextline(buf)
      char *buf;

      {
         char c;
	 int i;
         
	 for (i = 1; i < FB_MAXLINE; i++){		/* 1..FB_MAXLINE inclusive */
	    if (nextread(&c) == 0)
	       return(0);
	    if (c == FB_NEWLINE)
	       break;
	    *buf++ = c;
	    }
	 *buf = NULL;
	 return(i);
      }

/*
 * loadfields - load all the fields of the record
 */

   static int loadfields()
      {
         char c;
	 int i;

         for (i = 0; i <= hp->nfields; i++){
	    if (hp->kp[i]->type != FB_FORMULA && hp->kp[i]->dflink == NULL){
	       if (nextfield(cdb_afld) == 0){
	          if (verbose)
		     fb_serror(FB_MESSAGE, "Nextfield Error", NIL);
		  return(FB_ERROR);
		  }
	       if (verbose)
                  fb_printw("\tField %d: %s\n", i, cdb_afld);
	       fb_store(hp->kp[i], cdb_afld, hp);
	       }
	    }
	 if (nextread(&c) == 0 || c != FB_NEWLINE){
	    if (verbose)
	       fb_serror(FB_MESSAGE, "No Newline after record", NIL);
	    return(FB_ERROR);
	    }
	 return(FB_AOK);
      }

/*
 * nextfield - get a fb_field from fb_input, strip PIPE, and store NULL
 */

   static int nextfield(buf)
      char *buf;

      {
         char c, lastc = NULL;
	 int i;
         
	 for (i = 1; i < hp->recsiz; i++){
	    if (nextread(&c) == 0)
	       return(0);
	    while (c == CHAR_BACKSLASH){
	       lastc = c;
	       if (nextread(&c) == 0)
		  return(0);
	       if (c == CHAR_BACKSLASH || (c != CHAR_BACKSLASH && c != SEPARATOR))
	          *buf++ = CHAR_BACKSLASH;
	       }
	    if (c == SEPARATOR && lastc != CHAR_BACKSLASH)
	       break;
	    *buf++ = c;
	    lastc = c;
	    }
	 *buf = NULL;
	 if (c != SEPARATOR){
	    if (verbose)
	       fb_serror(FB_MESSAGE, "No Field Seperator Found", NIL);
	    return(0);
	    }
	 return(i);
      }

/*
 * nextread - read a single character, store in buf.
 */

   static nextread(buf)
      char *buf;
      
      {
         if (read(hp->logfd, buf, 1) != 1)
	    return(0);
	 return(1);
      }
      
/*
 *  usage - for dbrestor
 */
/*
*   static void usage()
*      {
*         fb_xerror(FB_MESSAGE,
*	    "usage: dbrestor [-b] [-y] dbase [dbase ...]", NIL);
*      }
*/
