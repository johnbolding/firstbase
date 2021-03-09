/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbedit.c,v 9.1 2001/01/16 02:46:27 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)dbedit.c	8.2 04 Aug 1997 FB";
#endif

/*
 *  dbedit.c - this is the heart of the cdb system.
 *             as such, it was developed incrementally.
 *             it shows.
 */

#include <fb.h>
#include <fb_vars.h>

char
     com[FB_MAXLINE] = { NULL },	/* command storage area */
     msg[FB_MAXLINE] = { NULL },	/* message for scrhlp and scrstat */
     mode = NULL;		/* for autoadd mode */
     
short
     globaladd = 0,		/* set to 1 for globaladd mode */
     scanner = 0,		/* set to 1 if dbscan, else 0 */
     st = 0,			/* status marker */
     pindx = 0,			/* level of index */
     simple_pindx = 0,		/* simple fld pointer when no index used */
     def = 0,			/* type of default ? */
     autodef = 0,		/* for auto default feature */
     ignore_case = 0,		/* case insensative flag */
     ignore_newline = 0;	/* newline flag for SYS_V newline regexp map */

long 
     rec = 0L,			/* record marker */
     oldrec = 0L,		/* last record */
     irec[FB_MAXIDXS] = { 0L },	/* to mark index record levels */
     ibase[FB_MAXIDXS] = { 0L };	/* to mark index base levels */

int dot = 0;			/* current fb_field location */

fb_database *hp;		/* to point to the global fb_database */

static char *MSG_NOINDEX = "warning: no index";
static char *NOWARN = "-w";
static char *USAGE = "usage: dbscan/dbedit [-d dbase] [-i index] [-s screen]";

extern char *cdb_pgm;
extern char *cdb_EDIT;

#if !FB_PROTOTYPES
int main();
void usage();
#else
int main(int, char **);
void usage(void);
#endif

/*
 *  dbedit - main driver
 */ 
     
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         int i, acc;
	 char *p;

         fb_getargs(argc, argv, FB_ALLOCDB);
	 hp = cdb_db;				/* tie hp to db */
         scanner = (equal(cdb_pgm, cdb_EDIT)) ? 0 : 1;
	 if (scanner == 1)
	    acc = READ;
	 else
	    acc = READWRITE;
	 fb_opendb(hp, acc, FB_ALLINDEX, 2); /* FB_MAYBE_OPTIONAL_INDEX */
	 fb_scrhdr(hp, NIL);
	 if (hp->ifd < 0 && hp->b_tree == 0)
            if (fb_testargs(argc, argv, NOWARN) == 0)
	       fb_serror(FB_MESSAGE, MSG_NOINDEX, NIL);
         if (scanner == 0){
	    fb_initlog(hp);
            fb_password(hp->dbase);
	    }
         for (i = 0; i < hp->nfields; i++){
            fb_nounders(hp->kp[i]);
	    /* remove precision from default strings. should be elsewhere. */
	    if (hp->kp[i]->type != FB_FORMULA && 
	        hp->kp[i]->idefault != NULL &&
	        hp->kp[i]->dflink == NULL &&
	          (p = (strrchr(hp->kp[i]->idefault, CHAR_COLON))) != 0)
	       *p = NULL;
	    }
	 if (fb_testargs(argc, argv, "-a") > 0)
	    globaladd = 1;
         if (fb_testargs(argc, argv, "-C") > 0)
            ignore_case = 1;
         if (fb_testargs(argc, argv, "-N") > 0)
            ignore_newline = 1;
         editor(argc, argv);
	 fb_closedb(hp);
         fb_ender();
      }

/*
 *  usage message 
 */

   void usage()
      {
         fb_xerror(FB_MESSAGE, USAGE, NIL);
      }
