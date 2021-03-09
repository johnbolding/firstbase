/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: ecommand.c,v 9.0 2001/01/09 02:55:43 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Ecommand_sid[] = "@(#) $Id: ecommand.c,v 9.0 2001/01/09 02:55:43 john Exp $";
#endif

#include <shell.h>
#include <shell_e.h>

static char *FMT1 = ".%s/%s_lck";
static char *SHELLHELP = "shell.hlp";
extern char *cdb_DEFAULT_FIRSTBASE_HOME;
extern char *cdb_CSHELL_BASE;
extern char *cdb_work_dir;
extern char *cdb_pgm;
extern char *cdb_user;
extern char *cdb_e_buf;
extern char *cdb_shell;
extern char *cdb_DCDB;
extern char *cdb_DIDX;
extern short int cdb_edit_input;
extern short int cdb_max_file_name;

/*
 * extended command module 
 */

/*
 * mcommand - implement extended command set for dbmenu
 */
 
   void mcommand()
      {
         char buf[FB_MAXLINE], fname[FB_MAXNAME], pname[FB_MAXNAME];
	 int tst;
	 
	 startcommand();
	 for (;;){
	    if (cdbmenu)
	       fb_initdbd(cdb_db);
	    fb_infoline();
	    fb_fmessage(COMMAND_MSG);
	    tst = fb_input(-cdb_t_lines, 16, 1, 0, FB_ALPHA, buf, FB_ECHO,
               FB_OKEND, FB_NOCONFIRM);
            if (tst == FB_QHELP)
               strcpy(buf, "h");
	    else if (tst != FB_AOK)
	       return;
	    fb_trim(buf);
	    switch(buf[0]){
	       case 'x':
		  sprintf(fname, FMT1, cdb_DEFAULT_FIRSTBASE_HOME,
                     cdb_CSHELL_BASE);
		  if (access(fname, 0) == 0){ 	/* system lock if file */
		     fb_serror(FB_MESSAGE, SYSMSG[S_ACCESSDENIED], NIL);
		     break;
		     }
		  fb_dirname(pname, cdb_db->dbase);
		  sprintf(fname, FMT1, pname, cdb_CSHELL_BASE);
		  if (access(fname, 0) == 0){ 	/* local lock if file */
		     fb_serror(FB_MESSAGE, SYSMSG[S_ACCESSDENIED], NIL);
		     break;
		     }
                  fb_cx_push_env("T", CX_NO_SELECT, NIL);
                  fb_cx_set_toolname("NOTOOL");
                  fb_cx_write(1);	/* since no fb_input() is used */
		  fb_fmessage(NIL);
		  fb_system(cdb_shell, FB_NOROOT);
                  fb_scrhdr(cdb_db, pmenu);
		  startcommand();
                  fb_cx_set_toolname(cdb_pgm);
                  fb_cx_set_workdir(cdb_work_dir);
                  fb_cx_set_menu(cmenu);
                  fb_cx_pop_env();
	          break;
	       case 'u':
		  sprintf(fname, FMT1, cdb_DEFAULT_FIRSTBASE_HOME,
                     cdb_CSHELL_BASE);
		  if (access(fname, 0) == 0){ 	/* system lock if file */
		     fb_serror(FB_MESSAGE, SYSMSG[S_ACCESSDENIED], NIL);
		     break;
		     }
		  fb_dirname(pname, cdb_db->dbase);
		  sprintf(fname, FMT1, pname, cdb_CSHELL_BASE);
		  if (access(fname, 0) == 0){ 	/* local lock if file */
		     fb_serror(FB_MESSAGE, SYSMSG[S_ACCESSDENIED], NIL);
		     break;
		     }
	          if (fb_getfilename(buf, MSG5, NIL) != FB_ERROR){
                     fb_cx_set_toolname("NOTOOL");
                     fb_cx_push_env("T", CX_NO_SELECT, NIL);
                     fb_cx_write(1);	/* since no fb_input() is used */
		     fb_system(buf, FB_NOROOT);
                     fb_scrhdr(cdb_db, pmenu);
		     startcommand();
                     fb_cx_set_toolname(cdb_pgm);
                     fb_cx_set_workdir(cdb_work_dir);
                     fb_cx_set_menu(cmenu);
                     fb_cx_pop_env();
		     }
		  break;
	       case 'p': case 'w':
		  fb_fmessage(fb_getwd(buf));
		  fb_screrr(NIL);
		  break;
	       case 'd':
		  for (;;){
		     fb_getfilename(dname, MSG6, cdb_DCDB);
		     if (strlen(dname) <= cdb_max_file_name)
		        break;
	             fb_serror(FB_MESSAGE, SYSMSG[S_TOO_LONG], dname);
		     }
		  if (strlen(dname) > 0){
		     cdb_db->dbase[0] = NULL;
		     fb_dbargs(dname, iname, cdb_db);
		     }
	          fb_initdbd(cdb_db);
		  break;
	       case 's':
		  fb_getfilename(sname, MSG7, STRING_MINUS);
		  fb_trim(sname);
		  if (sname[0] == NULL)
		     strcpy(sname, SCREENDEFAULT);
		  break;
	       case 'v':
		  fb_getfilename(vname, MSG7V, STRING_MINUS);
		  fb_trim(vname);
		  if (vname[0] == NULL)
		     strcpy(vname, SCREENDEFAULT);
		  break;
	       case 'e':
	          showenv();
		  startcommand();
		  break;
	       case 'l':
	          strcpy(fname, tempfile);
		  fb_move(2, 1); fb_clrtobot(); fb_refresh();
                  close(mkstemp(fname));
		  sprintf(buf, FMT3, DBLS, fname);
		  fb_system(buf, FB_WITHROOT);
		  fb_fhelp(fname);
		  unlink(fname);
		  startcommand();
		  break;
	       case 'i':
		  for (;;){
		     fb_getfilename(iname, MSG8, cdb_DIDX);
		     if (strlen(iname) <= cdb_max_file_name)
		        break;
	             fb_serror(FB_MESSAGE, SYSMSG[S_TOO_LONG], iname);
		     }
		  if (strlen(iname) > 0){
		     cdb_db->dindex[0] = NULL;
		     fb_dbargs(NIL, iname, cdb_db);
		     }
		  if (access(cdb_db->idict, 4) == 0)
	             fb_initdbd(cdb_db);
		  break;
	       case 'c':
                  if (cdb_edit_input)
                     strcpy(cdb_e_buf, fb_getwd(buf));
		  fb_getfilename(buf, MSG9, STRING_DOT);
		  if (chdir(buf) != 0)
		     fb_serror(FB_MESSAGE, buf, SYSMSG[S_NOT_FOUND]);
		  else{
		     fb_mkstr(&cdb_work_dir, fb_getwd(buf));
	             fb_initdbd(cdb_db);
                     fb_cx_set_workdir(cdb_work_dir);
                     fb_getco((fb_database *) NULL);
		     }
		  break;
	       case 'h':
	          fb_move(2, 1), fb_clrtobot();
		  fb_fhelp(SHELLHELP);
		  startcommand();
		  break;
	       }
	    }
      }

/*
 *  showenv - display the current environment
 */

   showenv()
      {
         char **p, buf[FB_MAXLINE];
	 int row;
	 
         fb_move(2, 1); fb_clrtobot();
	 fb_move(3, 10); fb_stand(MSG10);
	 row = 5;
	 fb_move(row, 10); fb_printw(USER);
	 fb_move(row++, 25); fb_stand(cdb_user);
	 fb_move(row, 10); fb_printw(DIRECTORY);
	 fb_move(row++, 25); fb_stand(fb_getwd(buf));
	 fb_move(row, 10); fb_printw(DATABASE);
	 fb_move(row, 25); fb_stand(dname);
	 if (strlen(cdb_db->dbase) == 0){
	    fb_stand(MSG12);
	    }
	 else if (access(cdb_db->dbase, 0) != 0){
	    fb_stand(MSG13);
	    }
	 row++;
	 fb_move(row, 10); fb_printw(INDEX);
	 fb_move(row, 25); fb_stand(iname);
         if (access(cdb_db->idict, 0) == 0){
            if (fb_test_tree(cdb_db))
               fb_prints(" (Btree)");
            else
               fb_prints(" (Flat)");
            }
	 if (strlen(cdb_db->dindex) == 0)
	    fb_stand(MSG12);
	 else if (access(cdb_db->idict, 0) != 0)
	    fb_stand(MSG13);
	 row++;

	 fb_move(row, 10); fb_printw(SCREEN);
	 fb_move(row++, 25);
	 if (!equal(sname, STRING_MINUS)){
	    fb_stand(sname);
	    strcpy(buf, sname);
	    strcat(buf, SCREEN_EXT);
	    if (access(buf, 0) != 0)
	       fb_stand(MSG13);
	    }
	 else
	    fb_stand(MSG12);

	 fb_move(row, 10); fb_printw(VIEW);
	 fb_move(row++, 25);
	 if (!equal(vname, STRING_MINUS)){
	    fb_stand(vname);
	    strcpy(buf, vname);
	    strcat(buf, VIEW_EXT);
	    if (access(buf, 0) != 0)
	       fb_stand(MSG13);
	    }
	 else
	    fb_stand(MSG12);

	 for (p = environ; *p; p++)
	    if (strncmp(*p, FIRSTBASE, 3) == 0){
	       fb_move(++row, 25);
	       fb_stand(*p);
	       }
	 fb_screrr(NIL);
      }

/*
 *  startcommand - set up command screen
 */

   startcommand()
      {
         int row;
	 
	 fb_move(2, 1), fb_clrtobot(); fb_refresh();
	 fb_scrstat(MSG14);
	 fb_move(3, 10); fb_stand(MSG15);
	 for (row = 5; row <= 21; row++){
	    if (!(COMHELP[row-5]))
	       break;
	    fb_move(row, 10);
	    fb_printw(FB_FSTRING, COMHELP[row-5]);
	    }
      }
