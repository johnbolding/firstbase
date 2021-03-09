/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getargs.c,v 9.2 2001/02/05 18:25:15 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getargs_sid[] = "@(#) $Id: getargs.c,v 9.2 2001/02/05 18:25:15 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <dbacct.h>

static char *M_LI =
   "Warning: Interactive FirstBase Window should be >= 24 Lines.";
static char *M_CO =
   "Warning: Interactive FirstBase Window should be >= 80 Columns.";

extern int cdb_umask;

extern short int cdb_no_acct;
extern short int cdb_debugmode;
extern short int cdb_yesmode;
extern short int cdb_secure;
extern short int cdb_usrlog;
extern char *cdb_dbase;
extern char *cdb_index;
extern char *cdb_DCDB;
extern char *cdb_DIDX;
extern char *cdb_dbuser;
extern char *cdb_DBVEMIT;
extern char *cdb_VEDIT;
extern char *cdb_VSCAN;
extern char *cdb_DBVI;
extern char *cdb_DBVIEW;
extern char *cdb_DBSHELL;
extern char *cdb_pgm;

extern int fb_magic_environ(void);

/* 
 *  getargs - gather all arguments into proper places.
 *	if dbflag = ALLOCDB, global db is allocated and name loaded.
 *	if dbflag = NODB, no allocation will be done.
 *	screen is always initialized, and then cleared if not cdb_batchmode.
 */
 
   fb_getargs(argc, argv, dbflag)
      register argc;
      int dbflag;
      char *argv[];
      
      {
         register int i;
         char fname[FB_MAXNAME], iname[FB_MAXNAME], ddname[FB_MAXNAME];
         char tname[FB_MAXNAME];

	 fb_setup_argv(argc, argv);		/* process setup rc file */
         /* set umask only when cdb_umask is > 0, else default mask */
         if (cdb_umask >= 0)
	    umask((unsigned)(cdb_umask));
         if (cdb_usrlog > 0)
            fb_usrlog_begin(argc, argv);
         if (cdb_dbase != NULL)
            strcpy(fname, cdb_dbase);
	 else
            strcpy(fname, cdb_DCDB);
         if (cdb_index != NULL)
            strcpy(iname, cdb_index);
	 else
            strcpy(iname, cdb_DIDX);
	 if (fb_testargs(argc, argv, "-debug") > 0)
	    cdb_debugmode = 1;
	 if (fb_testargs(argc, argv, "-version") > 0 ||
             fb_testargs(argc, argv, "--version") > 0 ||
             fb_testargs(argc, argv, "-V") > 0){
            printf("FirstBase %s\n", VERSION);
            exit(0);
            }
	 if ((i = fb_testargs(argc, argv, "-dd")) > 0)
	    strcpy(ddname, argv[i + 1]);
	 else
	    ddname[0] = NULL;
	 if (fb_testargs(argc, argv, "-b") == 0 && !cdb_batchmode){
	    fb_initscreen();
	    if (cdb_t_lines < 24){
	       cdb_t_lines = 0;
	       if (!cdb_debugmode)
		  fb_xerror(FB_MESSAGE, M_LI, NIL);
	       cdb_t_lines = 24;
	       }
            if (cdb_t_cols < 80){
               cdb_t_lines = 0;
               if (!cdb_debugmode)
                  fb_xerror(FB_MESSAGE, M_CO, NIL);
               cdb_t_lines = 24;
               }
	    }
         for (i = 1; i < argc; i++)
            if (argv[i][0] == CHAR_MINUS && argv[i][2] == NULL)
	       switch (argv[i][1]) {
		  case 'd':
		     fname[0] = NULL;
                     if (i + 1 >= argc)
                        fb_xerror(FB_MESSAGE,
                           "Ill formed dbase (-d) argument", NIL);
		     if (argv[i + 1] != NULL)
		        fb_rootname(fname, argv[++i]);
		     break;
		  case 'i':
		     iname[0] = NULL;
                     if (i + 1 >= argc)
                        fb_xerror(FB_MESSAGE,
                           "Ill formed index (-i) argument", NIL);
		     if (argv[i + 1] != NULL)
		        fb_rootname(iname, argv[++i]);
		     break;
		  case 'b': 
		     cdb_batchmode = 1;
		     break;
		  case 'y': cdb_yesmode = 1; break;
		  }
#if FB_CDB_SECURE
         if (cdb_secure){
            if (!equal(cdb_pgm, cdb_DBVEMIT) && !cdb_no_acct &&
                  !fb_magic_environ()){
               fb_acctlog(FB_AC_BEGINTOOL, fname, iname);
               fb_mkstr(&cdb_dbuser, fb_getlogin());
               }
            if (cdb_no_acct)
               fb_mkstr(&cdb_dbuser, "fbroot");
            }
#endif /* FB_CDB_SECURE */
         fb_cx_boot();
	 fb_settty(FB_EDITMODE);
         if (dbflag == FB_ALLOCDB){
	    cdb_db = fb_dballoc();
            fb_dbargs(fname, iname, cdb_db);
	    if (ddname[0] != NULL){
	       sprintf(tname, SYSMSG[S_FMT_2S], ddname, SYSMSG[S_EXT_DDICT]);
	       fb_mkstr(&(cdb_db->ddict), tname);
	       }
	    }
      }

   int fb_magic_environ()
      {
         char *p;

         p = getenv("firstbase_magic");
         if (p != NULL)
            if (atoi(p) == 16)
               return(1);
         return(0);
      }
