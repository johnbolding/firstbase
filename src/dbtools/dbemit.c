/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbemit.c,v 9.2 2002/03/09 00:07:35 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbemit_sid[] = "@(#) $Id: dbemit.c,v 9.2 2002/03/09 00:07:35 john Exp $";
#endif

/*  
 *  dbemit.c - emit all the fb_field values of an entire database
 *    in comma seperated format .
 */

#include <fb.h>
#include <fb_vars.h>
#include <emit.h>

#if !FB_PROTOTYPES
static void demit();
static int emit();
static int semit();
static void finit();
static void begintable();
static void endtable();
void usage();
#else /* FB_PROTOTYPES */
extern main(int, char **);
static void demit(int, char **);
static int emit(fb_database *);
static int semit(fb_database *);
static void finit(char *);
static void begintable(void);
static void endtable(void);
void usage(void);
#endif /* FB_PROTOTYPES */

static fflag = 0;				/* 'force-formula-flag' */
static Bflag = 0;				/* Blocking flag */
static pflag = 0;				/* piping flag */
static stdflag = 0;				/* stdout flag */
static tableflag = 0;				/* for tbl output */
static quoteflag = 0;				/* quote flag for fields */
static verbose = 0;				/* for verbose dates */
static rflag = 0;				/* record flag */

static int fd;
static long n;
static char separator = ',';			/* standard separator */
fb_field **cdb_sp;				/* screen pointer */
short int cdb_sfields;				/* number of screen fields */

static char *S_QUOTE = "\"";
static char *S_BACKSLASH = "\\";

static char *USAGE = 
   "usage: dbemit [options] [-d dbase] [-i index] [-s screen]";

extern char *cdb_work_dir;
extern char *cdb_coname;

/*
 *  dbemit - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
	 int i;

         (void) Dbemit_sid;

         for (i = 1; i < argc; i++){
            if (argv[i][0] == CHAR_MINUS)
               if (argv[i][1] == NULL){
                  stdflag = 1;
                  cdb_batchmode = 1;
                  }
               else if (argv[i][2] == NULL){
                  switch (argv[i][1]) {
                     case 'v':
                        verbose = 1;
                        break;
                     case 't':
                        tableflag = 1;
                        separator = ':';
                        break;
                     case 'q':
                        quoteflag = 1;
                        break;
                     case 'c':
                        if (++i >= argc)
                           usage();
                        if (argv[i][1] != NULL)
                           usage();
                        separator = argv[i][0];
                        break;
                     case 'f':
                        fflag = 1;
                        break;
                     case 'B':
                        Bflag = 1;
                        break;
                     case 'p':
                        pflag = 1;
                        break;
                     case 'r':
                        rflag = 1;
                        break;
                     }
                  }
            }
         fb_getargs(argc, argv, FB_ALLOCDB);
	 
	 fb_allow_int();		/* allows instant piping */	
	 if (cdb_yesmode)
	    sleep(1);				/* some are too fast */
	 demit(argc, argv);
	 fb_ender();
      }

/*
 *  demit - emit loop for a single dbase.
 */

   static void demit(argc, argv)
      int argc;
      char *argv[];
      
      {
         char fname[FB_MAXNAME], msg[FB_MAXLINE];
	 fb_database *hp;
	 int i;
	 
	 hp = cdb_db;
	 if (fb_testargs(argc, argv, "-i") > 0)
	    fb_opendb(hp, READ, FB_WITHINDEX, FB_MUST_INDEX);
         else
	    fb_opendb(hp, READ, FB_NOINDEX, FB_MUST_INDEX);
         for (i = 0; i < hp->nfields; i++)
            fb_nounders(hp->kp[i]);
	 fb_gets_dict(argc, argv);
	 fb_scrhdr(hp, "Check Files"); fb_infoline();
	 fb_basename(fname, hp->dbase);
	 strcat(fname, EMITEXTENSION);
	 if (stdflag == 1)
	    fd = 1;	/* standard output */
	 else
            finit(fname);		/* global fd allows foreach */
         fb_w_init(1, fd, -1);
	 if (!cdb_batchmode && !cdb_yesmode){
	    sprintf(msg, "Ok to emit from Database %s (y/n)? ",
	       hp->dbase);
	    if (fb_mustbe('y', msg, cdb_t_lines, 1) == FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
	 if (tableflag)
	    begintable();
         if (emit(hp) == FB_ERROR)
            fb_serror(FB_MESSAGE, SYSMSG[S_NOT_DONE], NIL);
	 if (tableflag)
	    endtable();
	 fb_wflush(1);
	 if (fd != 1)
	    close(fd);
	 fb_closedb(hp);
      }

/* 
 *  emit - emit all fields of the fb_database 
 */
 
   static emit(hp)
      fb_database *hp;
      
      {
         n = 0L;
	 if (!cdb_batchmode){
	    FB_XWAIT();
	    fb_gcounter(n);
	    }
	 if (hp->dindex == NULL || hp->dindex[0] == NULL ||
               (hp->ifd < 0 && !hp->b_tree)){
            if (Bflag == 0)
	       return(fb_foreach(hp, semit));
            else
	       return(fb_blockeach(hp, semit));
            }
	 else
	    return(fb_forxeach(hp, semit));
      }

/* 
 *  semit - emit all the fields of a buffer 
 */

   static int semit(hp)
      fb_database *hp;
      
      {
         int i;
	 char *p, *df, rbuf[10];;
	 
	 n++;
	 if (!cdb_batchmode)
	    fb_gcounter(n);
         if (rflag){
            sprintf(rbuf, "%ld", hp->rec);
	    fb_nextwrite(0, rbuf);
	    fb_w_write(0, &separator);	/* output separator (,:) */
            }
         for (i = 0; i < cdb_sfields; i++)
	    if (fflag == 1 || 
	          (cdb_sp[i]->type != FB_FORMULA && cdb_sp[i]->dflink==NULL)){
	       if (i != 0)
		  fb_w_write(0, &separator);	/* output separator (,:) */
	       if (cdb_sp[i]->type == FB_FORMULA || cdb_sp[i]->dflink != NULL){
	          fb_fetch(cdb_sp[i], cdb_afld, hp);
		  df = cdb_afld;
		  }
	       else
	          df = cdb_sp[i]->fld;
               if (cdb_sp[i]->type == FB_BINARY)
                  ; /* do nothing */
	       else if (!(FB_OFNUMERIC(cdb_sp[i]->type)) && cdb_sp[i]->type != FB_FORMULA){
	          if (quoteflag)
	             fb_w_write(0, S_QUOTE);
		  if (cdb_sp[i]->type == FB_DATE && verbose){
		     strcpy(cdb_afld, df);
		     p = fb_formfield(cdb_bfld, cdb_afld, cdb_sp[i]->type, 8);
		     }
		  else
		     p = df;
		  for ( ; *p; p++){ /* escape emb quotes/backg */
		     if (*p == FB_NEWLINE)
		        fb_nextwrite(0, "\\n");
		     else {
		        if (*p == CHAR_QUOTE || *p == CHAR_BACKSLASH)
		           fb_w_write(0, S_BACKSLASH);
		        fb_w_write(0, p);
			}
		     }
	          if (quoteflag)
	             fb_w_write(0, S_QUOTE);
		  }
	       else {
	          if (cdb_sp[i]->type == FB_DOLLARS && verbose){
		     strcpy(cdb_afld, df);
		     fb_formdollar(cdb_bfld, cdb_afld, cdb_sp[i]->size);
                     fb_trim(cdb_bfld);
                     df = cdb_bfld;
		     }
		  fb_nextwrite(0, df);
		  }
	       }
	 fb_w_write(0, SYSMSG[S_STRING_NEWLINE]);
         if (pflag)
            fb_wflush(1);
	 return(FB_AOK);
      }

/* 
 *  finit - initialize --- merely check for existance. ask about overwrite 
 */
 
   static void finit(fname)
      char *fname;
      
      {
         char str[FB_MAXLINE];
	 
	 if (access(fname, 0) != -1 && !cdb_batchmode && !cdb_yesmode){
	    sprintf(str, 
	       "Permission to OVERWRITE %s (y = yes, <cr> = no)? ", fname);
	    if (fb_mustbe('y', str, 15, 10) == FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
	 sprintf(str, "to %s", fname);
	 if (!cdb_batchmode)
	    fb_scrstat(str);
         close(creat(fname, 0666));	/* create fname */
	 fd = fb_mustopen(fname, WRITE);
      }

/*
 * begintable - emit pre tbl output
 */

   static void begintable()
      {
         int i, j;
	 char name[FB_MAXNAME], buffer[FB_MAXLINE];
	 
	 sprintf (buffer, ".he '%s''Page %%'\n", cdb_coname);
	 fb_nextwrite(0, buffer);
	 sprintf (buffer, "Directory: %s\n.br\n", cdb_work_dir);
	 fb_nextwrite(0, buffer);
	 sprintf (buffer, "Database: %s\n.br\n",
            fb_basename(name, cdb_db->dbase));
	 fb_nextwrite(0, buffer);
	 if (cdb_db->ifd > 0){
	    sprintf (buffer, "Index: %s\n.br\n",
               fb_basename(name, cdb_db->dindex));
	    fb_nextwrite(0, buffer);
	    }
	 if (cdb_db->sdict != NIL){
	    sprintf (buffer, "Screen: %s\n.br\n",
               fb_basename(name, cdb_db->sdict));
	    fb_nextwrite(0, buffer);
	    }
	 sprintf(buffer, ".sp 2\n");
	 fb_nextwrite(0, buffer);
         sprintf(buffer, ".TS H\ntab (%c);\n", separator);
	 fb_nextwrite(0, buffer);
	 buffer[0] = NULL;
	 for (j = 1; j <= 2; j++){
	    for (i = 0; i < cdb_sfields; i++)
	       strcat(buffer, "c ");
	    strcat(buffer, "\n");
	    fb_nextwrite(0, buffer);
	    buffer[0] = NULL;
	    }
	 buffer[0] = NULL;
	 for (i = 0; i < cdb_sfields; i++)
	    switch (cdb_sp[i]->type){
	       case '$':
	       case 'f':
	       case 'F':
	       case 'n':
	       case 'N':
	       case 'c':
	          strcat(buffer, "n1 "); break;
	       default:
	          strcat(buffer, "l1 "); break;
	       }
	 strcat(buffer, ".\n");
	 fb_nextwrite(0, buffer);
	 for (i = 0; i < cdb_sfields; i++){
	    sprintf(buffer, "%s%c", cdb_sp[i]->id, separator);
	    fb_nextwrite(0, buffer);
	    }
	 fb_nextwrite(0, "\n");
	 for (i = 0; i < cdb_sfields; i++){
	    sprintf(buffer, "\\_%c", separator);
	    fb_nextwrite(0, buffer);
	    }
	 sprintf(buffer, "\n\n.TH\n");
	 fb_nextwrite(0, buffer);
      }

/*
 * endtable - emit end tbl output
 */

   static void endtable()
      {
         char buffer[FB_MAXLINE];
	 
         sprintf(buffer, ".TE\n");
	 fb_nextwrite(0, buffer);
      }

/* 
 * usage message 
 */

   void usage()
      {
         fb_xerror(FB_MESSAGE, USAGE, NIL);
      }
