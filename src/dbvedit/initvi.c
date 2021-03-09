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

static char *MSG_FB_NOINDEX = "warning: no index";
static char *USAGE = "usage: dbvedit/dbvscan [-d dbase] [-i index] [-v view]";
static char *GLOBALADD = "-a";
static char *EDITONE = "-e";
static char *EDITPATTERN = "-ex";
static char *NOWARN = "-w";
char separator = ',';	/* standard separator */
short int fflag = 0;	/* formula fb_field flag -- for custom commands */
short int verbose = 0;	/* for verbose dates/dollars */
short int ignore_case = 0;/* case insensative flag */
short int ignore_newline = 0;/* newline flag for SYS_V newline regexp map */
int add_exactly = 0;	/* counter used to edit -a N records */

int globaledit = 0;	/* for force of a single record edit */
char *globalpat = NULL;

char cdb_macro_b_file[FB_MAXNAME];
char cdb_macro_e_file[FB_MAXNAME];
char cdb_macro_w_file[FB_MAXNAME];
char cdb_macro_comlevel_file[FB_MAXNAME];
char cdb_macro_fldlevel_file[FB_MAXNAME];

extern char *cdb_pgm;
extern char *cdb_VEDIT;

/*
 * initvi - decide permissions, open fb_database, fix underscores.
 */

   initvi(argc, argv)
      int argc;
      char *argv[];

      {
         int i, acc, j;
	 char *p;

         fb_getargs(argc, argv, FB_ALLOCDB);
	 if ((j = fb_testargs(argc, argv, GLOBALADD)) > 0){
	    globaladd = 1;
            if (++j < argc && isdigit(argv[j][0]))
               add_exactly = atoi(argv[j]) + 1;	/* 1 extra for startup */
            }
	 if ((i = fb_testargs(argc, argv, EDITONE)) > 0){
	    globaledit = atoi(argv[i + 1]);
	    globaladd = 0;
	    }
	 else if ((i = fb_testargs(argc, argv, EDITPATTERN)) > 0){
	    fb_mkstr(&(globalpat), argv[i + 1]);
	    globaladd = 0;
	    }
	 if (fb_testargs(argc, argv, "-f") > 0)
	    fflag = 1;
	 if (fb_testargs(argc, argv, "-V") > 0)
	    verbose = 1;
	 if (fb_testargs(argc, argv, "-C") > 0)
	    ignore_case = 1;
	 if (fb_testargs(argc, argv, "-N") > 0)
	    ignore_newline = 1;
	 if ((j = fb_testargs(argc, argv, "-c")) > 0){
	    if (++j >= argc)
	       usage();
	    if (argv[j][1] != NULL)
	       usage();
	    separator = argv[j][0];
	    }
	 hp = cdb_db;				/* tie hp to db */
         scanner = (equal(cdb_pgm, cdb_VEDIT)) ? 0 : 1;
	 if (scanner == 1){
	    acc = READ;
            globaledit = globaladd = 0;
            }
	 else
	    acc = READWRITE;
	 fb_opendb(cdb_db, acc, FB_ALLINDEX, 2); /* FB_MAYBE_OPTIONAL_INDEX; */
	 fb_scrhdr(cdb_db, NIL);
	 fb_gets_dict(argc, argv);
	 if (fb_get_vdict(argc, argv) == FB_ERROR)
	    fb_xerror(FB_BAD_DICT, vdict, "bad view dictionary");
         fb_cx_set_dict(cdb_db->ddict, vdict);
         fb_cx_set_status(CX_EDIT_BOOT);
         fb_cx_write(0);		/* no signal to redraw */
         fb_cx_signal_1();		/* signal_1 for special db work */
         if (scanner == 0){
	    fb_initlog(cdb_db);
            fb_password(hp->dbase);
	    }
	 if (cdb_db->ifd < 0 && cdb_db->b_tree == 0)
	    if (fb_testargs(argc, argv, NOWARN) == 0)
	       fb_serror(FB_MESSAGE, MSG_FB_NOINDEX, NIL);
         for (i = 0; i < hp->nfields; i++){
            fb_nounders(hp->kp[i]);
	    /* remove precision from default strings. */
	    if (hp->kp[i]->type != FB_FORMULA && 
	        hp->kp[i]->idefault != NULL &&
	        hp->kp[i]->dflink == NULL &&
	          (p = (strrchr(hp->kp[i]->idefault, CHAR_COLON))) != 0)
	       *p = NULL;
	    }
	 strcpy(cdb_macro_b_file, cdb_db->ddict);
	 strcpy(cdb_macro_e_file, cdb_db->ddict);
	 strcpy(cdb_macro_w_file, cdb_db->ddict);
	 strcpy(cdb_macro_comlevel_file, cdb_db->ddict);
	 strcpy(cdb_macro_fldlevel_file, cdb_db->ddict);
         if ((p = strrchr(cdb_macro_b_file, '.')) != 0)
            strcpy(p, ".m_begin");
         if ((p = strrchr(cdb_macro_e_file, '.')) != 0)
            strcpy(p, ".m_end");
         if ((p = strrchr(cdb_macro_w_file, '.')) != 0)
            strcpy(p, ".m_endwrite");
         if ((p = strrchr(cdb_macro_comlevel_file, '.')) != 0)
            strcpy(p, ".m_com");
         if ((p = strrchr(cdb_macro_fldlevel_file, '.')) != 0)
            strcpy(p, ".m_fld");

         /* macro init stuff */
         g_symtab = fb_makesymtab();
      }

/*
 * endvi - closing routine for dbvedit.
 */

   endvi()

      {
	 fb_closedb(cdb_db);
         /* garbage collect on the entire macro system */
         fb_gcollect_loadnode();
         fb_gcollect(n_ghead, c_ghead);
         fb_expunge_symtab(g_symtab);
         fb_gcollect_m_pool();
         fb_gcollect_c_pool();
         fb_gcollect_s_pool();
         fb_ender();
      }

/*
 *  usage message 
 */

   usage()
      {
         fb_xerror(FB_MESSAGE, USAGE, NIL);
      }
