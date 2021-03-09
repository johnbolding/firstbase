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

/* initilization and shutdown routines for dbvedit */

#include <dbve_ext.h>
#include <macro_e.h>

extern short int cdb_locklevel;

#if PRINTMOTD
static char *MOTD[] = {
   "FirstBase is the proprietary RDBMS",
   "produced and distributed by FirstBase Software.",
   "",
   "This tool, dbvemit(1), is a small part of FirstBase. It has been",
   "licensed from FirstBase Software on a site by site basis.",
   "",
   "No further distribution of dbvemit(1) is allowed without the consent",
   "of FirstBase Software.",
   "", "", "", "",
   "FirstBase Software",
   "7090 N. Oracle, Suite 165",
   "Tucson, AZ  85704",
   "520/742-7897",
   0 };
#endif /* PRINTMOTD */

static char *USAGE = 
  "usage: dbvemit [-d dbase] [-i index] [-v view] [-q] [-c sep] [-a N] [file]";
static char *VEMITEXTENSION = ".vemit";
extern int quoteflag;
extern char separator;
extern char *begtok, *endtok;

#define ADD_EXACTLY	"-a"
int add_exactly = 0;			/* counter used to edit -a N records */

char cdb_macro_b_file[FB_MAXNAME];
char cdb_macro_e_file[FB_MAXNAME];
char cdb_macro_w_file[FB_MAXNAME];

/*
 * initvi - decide permissions, open fb_database, fix underscores.
 */

   initvi(argc, argv)
      int argc;
      char *argv[];

      {
         int i;
	 char *p, fname[FB_MAXNAME];

         cdb_locklevel = 0;
	 fname[0] = NULL;
	 for (i = 1; i < argc; i++){
	    if (argv[i][0] == '-'){
	       if (argv[i][1] == 'd' || argv[i][1] == 'i' ||
		   argv[i][1] == 'b' || argv[i][1] == 'e' ||
		   argv[i][1] == 'a' || 
		   argv[i][1] == 'v' || argv[i][1] == 'c')
	          i++;
	       }
	    else{
	       strcpy(fname, argv[i]);
	       break;
	       }
	    }
	 if (fb_testargs(argc, argv, "-q") > 0)
	    quoteflag = 1;
	 if ((i = fb_testargs(argc, argv, "-c")) > 0){
	    if (++i >= argc)
	       usage();
	    if (argv[i][1] != NULL)
	       usage();
	    separator = argv[i][0];
	    }
	 begtok = endtok = NULL;
	 if ((i = fb_testargs(argc, argv, "-bt")) > 0){
	    if (++i >= argc)
	       usage();
	    begtok = argv[i];
	    }
	 if ((i = fb_testargs(argc, argv, "-et")) > 0){
	    if (++i >= argc)
	       usage();
	    endtok = argv[i];
	    }
	 if ((i = fb_testargs(argc, argv, ADD_EXACTLY)) > 0){
	    if (++i >= argc)
	       usage();
	    add_exactly = atoi(argv[i]) + 1;	/* 1 extra for startup loop */
            }
         fb_getargs(argc, argv, FB_ALLOCDB);
	 if (fname[0] == NULL){		/* requires full path name */
	    fb_basename(fname, cdb_db->dbase);	/* calc default file name */
	    strcat(fname, VEMITEXTENSION);
	    }
	 globaladd = 1;
	 hp = cdb_db;				/* tie hp to db */
         scanner = 0;
         if (fb_getd_dict(hp) == FB_ERROR){
	    fb_clear();
	    fb_xerror(FB_BAD_DICT, hp->ddict, NIL);
	    }
	 fb_emptyi_dict(hp);
	 fb_scrhdr(cdb_db, NIL);
	 if (fb_get_vdict(argc, argv) == FB_ERROR)
	    fb_xerror(FB_BAD_DICT, vdict, NIL);
         fb_cx_set_dict(cdb_db->ddict, vdict);
         fb_cx_set_status(CX_EDIT_BOOT);
         fb_cx_write(0);		/* no signal to redraw */
         fb_cx_signal_1();		/* signal_1 for special db work */
/*
*        if (scanner == 0){
* 	    fb_initlog(cdb_db);
*           fb_password(hp->dbase);
*	    }
*/

/*
*	 if (cdb_db->ifd < 0)
*	    fb_serror(FB_MESSAGE, MSG_FB_NOINDEX, NIL);
*/
         for (i = 0; i < hp->nfields; i++){
            fb_nounders(hp->kp[i]);
	    /* remove precision from default strings. */
	    if (hp->kp[i]->type != FB_FORMULA && 
	        hp->kp[i]->idefault != NULL &&
	        hp->kp[i]->dflink == NULL &&
	          (p = (strrchr(hp->kp[i]->idefault, CHAR_COLON))) != 0)
	       *p = NULL;
	    }
	 outinit(fname);
#if PRINTMOTD
	 motd();
#endif /* PRINTMOTD */
	 strcpy(cdb_macro_b_file, cdb_db->ddict);
	 strcpy(cdb_macro_e_file, cdb_db->ddict);
	 strcpy(cdb_macro_w_file, cdb_db->ddict);
         if ((p = strrchr(cdb_macro_b_file, '.')) != 0)
            strcpy(p, ".m_begin");
         if ((p = strrchr(cdb_macro_e_file, '.')) != 0)
            strcpy(p, ".m_end");
         if ((p = strrchr(cdb_macro_w_file, '.')) != 0)
            strcpy(p, ".m_endwrite");

         /* macro init stuff */
         g_symtab = fb_makesymtab();
      }

/*
 * endvi - closing routine for dbvedit.
 */

   endvi()

      {
	 /* fb_closedb(cdb_db); */
	 outend();
         fb_ender();
      }

/*
 *  usage message 
 */

   usage()
      {
         fb_xerror(FB_MESSAGE, USAGE, NIL);
      }

#if PRINTMOTD
   static motd()
      {
         int p, row;

         for (p = 0, row = 5; MOTD[p]; p++, row++){
	    fb_move(row, 5);
	    fb_printw("%s\n", MOTD[p]);
	    }
	 FB_PAUSE();
      }
#endif /* PRINTMOTD */
