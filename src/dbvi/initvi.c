/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: initvi.c,v 9.1 2001/01/16 02:46:49 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Initvi_sid[] = "@(#) $Id: initvi.c,v 9.1 2001/01/16 02:46:49 john Exp $";
#endif

/* initilization and shutdown routines for dbvi */

#include <dbvi_ext.h>
#include <sys/types.h>
#include <signal.h>

static char *MSG_FB_NOINDEX = "warning: no index";
static char *USAGE = "usage: dbvi/dbview [-d dbase] [-v view] [-c cols]";
char ddname[FB_MAXNAME];

extern short cdb_InfoLineToggle;
extern short int cdb_error_row_offset;
extern char *cdb_pgm;

static dump();

/*
 * initvi - decide permissions, open fb_database, fix underscores.
 */

   initvi(argc, argv)
      int argc;
      char *argv[];

      {
         int i, acc;
	 char *p;

         cdb_error_row_offset = 0;		/* set for screrr */
         fb_getargs(argc, argv, FB_ALLOCDB);
	 if ((i = fb_testargs(argc, argv, "-dd")) > 0)
	    strcpy(ddname, argv[i + 1]);
	 else
	    ddname[0] = NULL;
         cdb_InfoLineToggle = -1;
	 hp = cdb_db;				/* tie hp to db */
         scanner = (equal(cdb_pgm, "dbvi")) ? 0 : 1;
	 if (scanner == 1)
	    acc = READ;
	 else
	    acc = READWRITE;
	 fb_opendb(cdb_db, acc, FB_ALLINDEX, 2); /* FB_MAYBE_OPTIONAL_INDEX; */
	 fb_scrhdr(cdb_db, NIL);
	 if (fb_get_vdict(argc, argv) == FB_ERROR)
	    fb_xerror(FB_BAD_DICT, vdict, NIL);
	 pcur = phead;
	 if (get_cdict(argc, argv) == FB_ERROR)
	    fb_xerror(FB_BAD_DICT, cdict, NIL);
	 fb_scrstat("Reading ...");
	 fb_refresh();
	 initcolumn();
         if (cdb_db->reccnt > 0)
	    readcolumn();
	 else
	    emptyclist();
         for (i = 0; i < hp->nfields; i++){
            fb_nounders(hp->kp[i]);
	    /* remove precision from default strings. */
	    if (hp->kp[i]->type != FB_FORMULA && 
	        hp->kp[i]->idefault != NULL &&
	        hp->kp[i]->dflink == NULL &&
	          (p = (strrchr(hp->kp[i]->idefault, CHAR_COLON))) != 0)
	       *p = NULL;
	    }
      }

/*
 * endvi - closing routine for dbvi.
 */

   endvi()

      {
         char command[FB_MAXLINE], filen[FB_MAXLINE];
	 int cleanit = 0;

	 if (access(cdb_db->dmap, 0) != 0){
	    cleanit = 1;
	    fb_scrstat("Cleaning...");
	    fb_basename(filen, cdb_db->dbase);
	    sprintf(command, "Cleaning database %s before exiting...", filen);
	    fb_fmessage(command);
	    if (ddname[0] == NULL)
	       sprintf(command, "dbclean -b %s", cdb_db->dbase);
	    else
	       sprintf(command, "dbclean -dd %s -b %s", ddname, cdb_db->dbase);
	    }
	 fb_closedb(cdb_db);
	 if (cleanit)
	    fb_system(command, FB_NOROOT);
         fb_ender();
      }

/*
 *  usage message 
 */

   usage()
      {
         fb_xerror(FB_MESSAGE, USAGE, NIL);
      }

   dbvi_sigill() 	{ dump(SIGILL); }
   dbvi_sigfpe() 	{ dump(SIGFPE); }
   dbvi_sigbus()	{ dump(SIGBUS); }
   dbvi_sigsegv()	{ dump(SIGSEGV); }
#if SIGSYS
   dbvi_sigsys()	{ dump(SIGSYS); }
#endif
   dbvi_sigterm()	{ dump(0); }
   dbvi_sighup()	{ dump(0); }
#if SIGLOST
   dbvi_siglost()	{ dump(SIGLOST); }
#endif

   static dump(type)
      int type;

      {
         fb_move(1,1); fb_clear(); fb_refresh();
	 printf("\n\nDbvi has encountered a fatal error of type %d.\n", type);
	 printf("An attempt is being made to write your database\n");
	 printf("to a file named \"dbvi_save\".\n");
	 printf("\n\nDo Not ATTEMPT Any More work on this Database!!\n");
	 printf("\n\nPlease report this FATAL error ** ASAP **.\n");
	 savefile(1);
	 fb_exit(type);
      }
