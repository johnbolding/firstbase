/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: initmrg.c,v 9.0 2001/01/09 02:55:41 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Initvi_sid[] = "@(#) $Id: initmrg.c,v 9.0 2001/01/09 02:55:41 john Exp $";
#endif

/* initilization and shutdown routines for dbdmrg */

#include <dbdmrg_e.h>
#include <sys/types.h>
#include <signal.h>

static char *USAGE = "usage: dbdmrg [-d dbase] [-s screen] mergefile";

extern short fb_InfoLineToggle;
extern short error_row_offset;
extern char iname[];		/* index name - used in mface.c */

char *ebuf;			/* buffer to edit in */
char filen[FB_MAXNAME];		/* file name of buffer */

static dump();

/*
 * initmrg - decide permissions, open fb_database, fix underscores.
 */

   initmrg(argc, argv)
      int argc;
      char *argv[];

      {
         int i, acc;

         fb_getargs(argc, argv, FB_ALLOCDB);
	 hp = cdb_db;				/* tie hp to db */
	 acc = READ;
	 fb_opendb(cdb_db, acc, FB_WITHINDEX, FB_OPTIONAL_INDEX);
	 fb_scrhdr(cdb_db, NIL);
	 fb_gets_dict(argc, argv);
	 fb_scrstat("Reading ...");
	 fb_refresh();
	 filen[0] = NULL;
	 iname[0] = NULL;
	 for (i = 1; i < argc; i++){
	    if (argv[i][0] == '-' && argv[i][1] == 'w'){
               i++;
               linewidth = atoi(argv[i]);
               }
	    else if (argv[i][0] == '-' && argv[i][1] == 'i'){
               i++;
               strcpy(iname, argv[i]);
               }
	    else if (argv[i][0] == '-' && argv[i][1] != NULL){
	       if (argv[i][1] != 'b' && argv[i][1] != 'y')
	          i++;
	       }
	    else if (filen[0] == NULL)
	       strcpy(filen, argv[i]);
	    }
	 if (strlen(filen) == 0)
	    usage();
         if (linewidth == 0)
            linewidth = 80;
         fb_readmrg(filen);
	 mpcur = mphead;
         mpcur->mp_acur = mpcur->mp_ahead;
         mpcur->mp_atop = mpcur->mp_ahead;
         base_right = cdb_t_cols;
         base_length = base_right - base_left;
         base_bottom = cdb_t_lines - 2;
         set_screen();
         ebuf = fb_malloc(linewidth + 10);
      }

/*
 * endmrg - closing routine for dbdmrg.
 */

   endmrg()

      {
	 fb_closedb(cdb_db);
         fb_ender();
      }

/*
 *  usage message 
 */

   usage()
      {
         fb_xerror(FB_MESSAGE, USAGE, NIL);
      }

   void mrg_sigill() 	{ dump(SIGILL); }
   void mrg_sigfpe() 	{ dump(SIGFPE); }
   void mrg_sigbus()	{ dump(SIGBUS); }
   void mrg_sigsegv()	{ dump(SIGSEGV); }
#ifdef SIGSYS
   void mrg_sigsys()	{ dump(SIGSYS); }
#endif
   void mrg_sigterm()	{ dump(0); }
   void mrg_sighup()	{ dump(0); }
#ifdef SIGLOST
   void mrg_siglost()	{ dump(SIGLOST); }
#endif

   static dump(type)
      int type;

      {
	 fb_exit(type);
      }
