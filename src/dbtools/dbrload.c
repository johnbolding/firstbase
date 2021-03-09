/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbrload.c,v 9.1 2001/02/16 19:47:52 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbrload[] = "@(#)dbrload.c	8.8 04 Aug 1997 FB";
#endif

/* 
 *  dbrload.c - load a file of fb_field values in comma seperated format 
 *     into a cdb fb_database -- but use an index, and look up first field
 *     value. replace that record, or add to end. I.E. dbase must exist.
 */

#include <fb.h>
#include <fb_vars.h>
#include <emit.h>

#if !FB_PROTOTYPES
static int rload();
static void replace();
static void newrec();
static int simplerec();
static int simplefield();
static FILE *finit();
static void usage();
#else /* FB_PROTOTYPES */
extern main(int, char **);
static int rload(FILE *fs);
static void replace(void);
static void newrec(void);
static int simplerec(FILE *);
static int simplefield(FILE *);
static FILE *finit(int, char **);
static void usage(void);
#endif /* FB_PROTOTYPES */

extern short int cdb_usrlog;

static char lastc = NULL;		/* last char seen - pushback kinda */
static char separator = ',';		/* standard separator */
static fb_database *hp;
static char **rp;			/* array of rrec pointers */
static char *rrec;			/* area for replacement record */
static char tbuf[FB_MAXLINE];		/* temporary buffer */
static long line_no = 0;
static int vflag = 0;			/* verbose flag */
static int tflag = 0;			/* trace flag */
static int ival[20];			/* index values - offsets for cdb_kp */
static char *ixrec;			/* index value for lookup */
static int withindex = 1;		/* flag for turning off index feat */
static int nosearch = 0;		/* flag for turning off search */

extern short int cdb_interrupt;

/*
 *  dbrload - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         char str[FB_MAXLINE];
         FILE *fs;
	 int i, j;

         (void) Dbrload;

         fb_getargs(argc, argv, FB_ALLOCDB);
	 hp = cdb_db;
	 if (cdb_interrupt)
            fb_allow_int();
	 fb_opendb(cdb_db, READWRITE, FB_ALLINDEX, FB_MAYBE_OPTIONAL_INDEX);
	 if (cdb_db->ihfd > 0)
	    withindex = 1;
	 else
	    withindex = 0;
	 rp = (char **) fb_malloc(cdb_db->nfields * sizeof(char *));
	 rrec = (char *) fb_malloc((unsigned) (cdb_db->recsiz+400));
	 ixrec = (char *) fb_malloc((unsigned) (cdb_db->irecsiz + 13));
	 /* find all the indexed fields and save offsets in ival[] */
	 for (j = 0; j < cdb_db->nfields; j++)
	    rp[j] = NIL;
	 for (i = 0; i < cdb_db->ifields; i++){
	    for (j = 0; j < cdb_db->nfields; j++){
	       if (cdb_db->kp[j] == cdb_ip[i])
		  ival[i] = j;
	       }
	    }
         fb_scrhdr(cdb_db, "Open Files"); fb_infoline();
	 if ((j = fb_testargs(argc, argv, "-v")) > 0)
	    vflag = 1;
	 if ((j = fb_testargs(argc, argv, "-t")) > 0)
	    tflag = 1;
	 if ((j = fb_testargs(argc, argv, "-n")) > 0)
	    nosearch = 1;
	 if ((j = fb_testargs(argc, argv, "-c")) > 0){
	    if (++j >= argc)
	       usage();
	    if (argv[j][1] != NULL)
	       usage();
	    separator = argv[j][0];
	    }
	 fs = finit(argc, argv);
	 if (!cdb_batchmode && !cdb_yesmode){
	    sprintf(str,
               "Ok to Replace/Load using FirstBase Database %s (y/n) ?", 
	       cdb_db->dbase);
	    if (fb_mustbe('y', str, cdb_t_lines, 1) == FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
         if (rload(fs) == FB_ERROR)
	    fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	 fclose(fs);
	 fb_closedb(cdb_db);
	 fb_ender();
      }

/* 
 *  rload - rload all lines from a load file (fs) into a FirstBase fb_database 
 */
 
   static int rload(fs)
      FILE *fs;
      
      {
         long n;
	 int i;

	 n = 0L;
         if (!cdb_batchmode){
	    FB_XWAIT();
	    fb_gcounter(n);
	    }
	 for (;;){
	    if (simplerec(fs) == EOF)	/* get load rec into rrec */
	       break;
	    n++;
	    if (!cdb_batchmode)
	       fb_gcounter(n);
            if (tflag && cdb_batchmode)
	       for (i = 0; i < cdb_db->nfields; i++)
		  printf("%ld tracing: field %d:[%s] - ...%s...\n",
		     n, i, cdb_db->kp[i]->id, rp[i]);
	    ixrec[0] = NULL;
	    for (i = 0; withindex && i < cdb_db->ifields - 1; i++){
	       strcpy(cdb_afld, rp[ival[i]]);
	       fb_makess(cdb_afld, cdb_db->ip[i]->type, cdb_db->ip[i]->size);
	       strcat(ixrec, cdb_afld);
	       }
	    if (tflag && cdb_batchmode)
	       printf("   ixrec is ---%s---\n", ixrec);
	    if (withindex)
               fb_getxhead(cdb_db->ihfd, &(cdb_db->bsmax), &(cdb_db->bsend));
	    if (withindex && !nosearch && fb_getxrec(ixrec, cdb_db) !=FB_ERROR)
	       replace();
	    else{
	       newrec();
               /*
                * with the advent of btree's, this function is moot
                *
	        * if (withindex)
		*   addix();
                */
	       }
	    }
         return(FB_AOK);
      }

/* 
 *  replace - replace needed fields from rp into db.
 */

   static void replace()
      {
         int i, j, ixfield;
	 char f[FB_MAXLINE];

	 if (fb_lock(cdb_db->rec, cdb_db, FB_NOWAIT) != FB_AOK)
	    return;    			/* dont wait if locked */
	 if (vflag)
            fb_serror(FB_MESSAGE, "Replacing record with index value:", ixrec);
         fb_set_autoindex(cdb_db);
	 for (i = 0; i < cdb_db->nfields; i++){
	    /* test for an index fb_field -- if so, do not replace it. */
	    ixfield = 0;
	    for (j = 0; j < cdb_db->ifields; j++){
	       if (cdb_db->ip[j] == cdb_db->kp[i]){
	          ixfield = 1;
		  break;
		  }
	       }
	    if (!ixfield && rp[i] != NULL && rp[i][0] != NULL){
	       cdb_bfld[0] = NULL;
	       if (equal(rp[i], "$ERASE"))
	          rp[i][0] = NULL;
	       else if (rp[i][0] == '$' && rp[i][1] == 'F' && rp[i][2]==':'){
	          strcpy(f, rp[i] + 3);
	          if (fb_getformula(cdb_db->kp[i], f, cdb_bfld, 0, cdb_db) ==
                        FB_ERROR)
	             fb_serror(FB_MESSAGE, SYSMSG[S_BAD_FORMULA], f);
	          }
	       else{
	          strcpy(cdb_bfld, rp[i]);
		  }
	       fb_store(cdb_db->kp[i], cdb_bfld, cdb_db);
	       }
	    }
	 if (tflag)
	    for (i = 0; i < cdb_db->nfields; i++)
	       printf("replace tracing: field %d:[%s] - ...%s...\n",
		  i, cdb_db->kp[i]->id, cdb_db->kp[i]->fld);
	 fb_lock_head(cdb_db);
         if (cdb_usrlog > 10)
            fb_usrlog_msg("CS-begin dbrload-replace");
	 fb_setdirty(cdb_db, 1);
	 if (fb_putrec(cdb_db->rec, cdb_db) == FB_ERROR)
	    fb_xerror(FB_FATAL_PUTREC, cdb_db->dbase, (char *) &(cdb_db->rec));
	 fb_setdirty(cdb_db, 0);
         if (fb_put_autoindex(cdb_db) == FB_ERROR)
            fb_serror(FB_BAD_INDEX, cdb_db->dbase, NIL);
         if (cdb_usrlog > 10)
            fb_usrlog_msg("CS-end dbrload-replace");
	 fb_unlock_head(cdb_db);
	 fb_unlock(cdb_db->rec, cdb_db);
      }

/* 
 *  newrec - create a new record using the new index value.
 */

   static void newrec()
      {
         int i;
	 
         if (vflag)
            fb_serror(FB_MESSAGE, "Adding record with index value:", ixrec);
	 for (i = 0; i < cdb_db->nfields; i++){
            if (strlen(rp[i]) > cdb_db->kp[i]->size)
	       fb_xerror(FB_MESSAGE, "string too long for field",
                  cdb_db->kp[i]->id);
	    fb_store(cdb_db->kp[i], rp[i], cdb_db);
            }
	 if (fb_addrec(cdb_db) == FB_ERROR){
	    cdb_db->rec = -1;
	    fb_xerror(FB_FATAL_PUTREC, cdb_db->dbase, (char *) &(cdb_db->rec));
	    }
      }

/* 
 *  simplerec - get a single simple record from file fs (comma sep format)
 *     and store complete record in rrec. 
 *     pointers are in rp[0]..rp[cdb_db->nfields - 1].
 */
 
   static simplerec(fs)
      FILE *fs;
      
      {
	 int i, st = FB_END;
	 char *p, *q;

         p = rrec;
	 line_no++;
	 for (i = 0; i < cdb_db->nfields; i++)
            rp[i] = NIL;
	 for (i = 0; i < cdb_db->nfields; i++){
	    if (cdb_db->kp[i]->type != FB_FORMULA && cdb_db->kp[i]->dflink ==
                  NULL && cdb_db->kp[i]->type != FB_LINK){
	       st = simplefield(fs);
	       if (st == EOF || st == FB_END)	/* eof or eorec */
	          break;
	       rp[i] = p;			/* save fb_field pointer */
	       if (cdb_bfld[0] != '$' && cdb_bfld[1] != 'F' &&
                     cdb_bfld[2]!=':' &&
	             strlen(cdb_bfld) > cdb_db->kp[i]->size){
	          sprintf(tbuf, "Truncating Field %d of line %ld",
		     i + 1, line_no);
	          fb_serror(FB_MESSAGE, tbuf, NIL);
		  cdb_bfld[cdb_db->kp[i]->size] = NULL;
		  }
	       for (q = cdb_bfld; *q; )		/* copy in the fb_field */
	          *p++ = *q++;
	       *p++ = '\000';
	       }
	    }
	 if (st != FB_END){
	    for (i = 0;;){
	       st = simplefield(fs);
	       if (st == EOF || st == FB_END)	/* eof or eorec */
	          break;
	       if (i == 0){
		  sprintf(tbuf, "Ignoring extra field(s) at end of line %ld",
		     line_no);
		  fb_serror(FB_MESSAGE, tbuf, NIL);
		  i = 1;	/* set flag to not repeat errmsg */
		  }
	       }
	    }
	 return(st);
      }

/* 
 *  simplefield - get a single simple fb_field from file fs (comma sep format)
 *     return value in cdb_bfld.
 */
 
   static simplefield(fs)
      FILE *fs;
      
         {
	    char *p;
	    int c, st, qlev, count, emsg = 0;

	    if (lastc == FB_NEWLINE){
	       lastc = NULL;
	       return(FB_END);
	       }
	    qlev = count = 0; 
	    p = cdb_bfld;
	    *p = NULL;
	    for (st = 0; ;){			/* use cdb_bfld to gather */
	       if ((c = fgetc(fs)) == EOF)
	          return(EOF);
	       if (c != '\n' && c != '\t' && c != ' ' && !isprint(c))
		  c = CHAR_STAR;
	       if ((char) c == CHAR_BACKSLASH){	/* covers \\ and \" .. */
		  c = fgetc(fs);
		  if (c == 'n')
		     c = FB_NEWLINE;
		  }
	       else if ((char) c == separator){
		  if (qlev == 0 || qlev == 2)  
		     st = FB_AOK;		/* must be end of fb_field */
		  }
	       else if ((char) c == FB_NEWLINE){
	          if (p == cdb_bfld)
		     st = FB_END;		/* end of record */
		  else{
		     st = FB_AOK;
		     lastc = FB_NEWLINE;
		     }
		  }
	       else if ((char) c == CHAR_QUOTE){
		  qlev++;
		  continue;			/* jump out */
		  }
	       if (st == FB_AOK || st == FB_END)
	          break;
	       if (++count >= cdb_fieldlength){
	          if (emsg == 0){
	             sprintf(tbuf, "Truncation within line %ld", line_no);
	             fb_serror(FB_MESSAGE, tbuf, NIL);
		     }
		  emsg = 1;
		  }
	       else
	          *p++ = c;			/* store character */
	       }
	    *p = NULL;				/* return field in cdb_bfld */
	    return(st);
	 }

/* 
 *  finit - intiialize --- merely check for existance. ask about overwrite 
 */
 
   static FILE *finit(argc, argv)
      int argc;
      char *argv[];
      
      {
         char str[FB_MAXLINE], fname[FB_MAXNAME];
	 int i;
	 
	 fname[0] = NULL;
	 for (i = 1; i < argc; i++)
	    if (argv[i][0] == '-'){
	       if (argv[i][1] == 'd' || argv[i][1] == 'i' ||
		   argv[i][1] == 's' || argv[i][1] == 'c')
	          i++;
	       }
	    else{
	       strcpy(fname, argv[i]);
	       break;
	       }
	 if (fname[0] == NULL){		/* requires full path name */
	    fb_basename(fname, cdb_db->dbase);	/* calc default file name */
	    sprintf(str, "%s%s", fname, EMITEXTENSION);
	    if (fb_getfilename(fname, "Load File Name: ", str) == FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
	 sprintf(str, "from %s", fname);
	 fb_scrstat(str);
	 return(fb_mustfopen(fname, "r"));
      }

/* 
 *  usage message 
 */
 
   static void usage()
      {
         fb_xerror(FB_MESSAGE,
	    "usage: dbrload [-b] [-c sep] [-d dbase] [-i index] [LOADFILE ]",
	     NIL);
      }
