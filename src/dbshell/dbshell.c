/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbshell.c,v 9.3 2001/02/12 23:25:05 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbshell_sid[] = "@(#) $Id: dbshell.c,v 9.3 2001/02/12 23:25:05 john Exp $";
#endif

/* 
 *  dbshell.c - another fb_database menu tool.
 *     provides a screen full of selections, with $words bound
 *     to series of commands, submitted as a shell process.
 */

#include <shell.h>
#include <shell_v.h>
#include <dbacct.h>
#include <setjmp.h>
#include <signal.h>

extern char *cdb_dbshell_pstring;
extern char *cdb_dbshell_ploc;
extern short int cdb_dbshell_pilength;
extern short int cdb_dbshell_ecommand;
extern short int cdb_locklevel;
extern short int cdb_secure;
extern short int cdb_menupause;
extern char *cdb_menu;
extern short int cdb_edit_input;
extern char *cdb_e_buf;
extern char *cdb_work_dir;
extern short int cdb_functionkeys;
extern short int cdb_dbshell_checkmail;	/* time between checkmail in dbshell */

short int force_exit = 0;
short int exit_value = 0;

char *PSTRING = "Enter Selection: ";
int ploc_row;
int ploc_col;
int screenmax;
static off_t mailsize;
static jmp_buf dbshell_jmp_env;

char menu_rootdir[FB_MAXNAME];

void set_mail();
void test_mail();

#if FB_PROTOTYPES
static void process(void);
static sh_select(void);
static command(char *w);
static rsh_usage(void);
static shell_redraw(void);
static RETSIGTYPE dbshell_sigalrm(int);
#else /* FB_PROTOTYPES */
static void process();
static sh_select();
static command();
static rsh_usage();
static shell_redraw();
static RETSIGTYPE dbshell_sigalrm();
#endif /* FB_PROTOTYPES */

/*
 *  dbshell - main driver 
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
      
      {
         FILE *fb_mustfopen();
	 int i, logshell = 0, rlogshell = 0;
	 char buf[FB_MAXLINE], *p, tname[FB_MAXNAME];
	 
	 cdb_locklevel = -1;
         /* call setup early to determine if cdb_secure */
         fb_setup_argv(argc, argv);
#if FB_CDB_SECURE
         if (cdb_secure){
            fb_basename(tname, argv[0]);
            if (equal(tname, "Fbsh"))
               logshell = 1;
            else if equal(tname, "fbrsh"){
               rlogshell = 1;
               if (fb_rlogin() == FB_ERROR)
                  fb_xerror(FB_MESSAGE, "Remote FirstBase Login failed.", NIL);
               if ((i = fb_testargs(argc, argv, "-x")) > 0){
                  i++;
                  if (i >= argc)
                     rsh_usage();
                  fb_system(argv[i], FB_NOROOT);
                  fb_settty(0);
                  fb_rlogout();
                  exit(0);
                  }
               }
            else
               logshell = 0;
            }
#endif /* FB_CDB_SECURE */
         fb_getargs(argc, argv, FB_ALLOCDB);
	 ploc_row = cdb_t_lines;
	 ploc_col = 1;
         screenmax = cdb_t_lines - 2;
         if (screenmax > SCREENMAX)
            screenmax = SCREENMAX;
	 if (cdb_dbshell_pstring == NULL)
	    fb_mkstr(&cdb_dbshell_pstring, PSTRING);
	 if (cdb_dbshell_ploc != NULL){
	    strcpy(buf, cdb_dbshell_ploc);
	    p = strchr(buf, ',');
	    *p = NULL;
	    ploc_row = atoi(buf);
	    ploc_col = atoi(p + 1);
	    }

         strcpy(cmenu, cdb_menu);
	 strcat(cmenu, "MAIN");
	 fb_rootname(dname, cdb_db->dbase);
	 fb_rootname(iname, cdb_db->dindex);
	 strcpy(sname, SCREENDEFAULT);
	 strcpy(vname, SCREENDEFAULT);
	 for (i = 0; i < SCREENMAX; i++)
	    screen[i] = NULL;
	 head->prev = tail->next = NULL;
	 head->next = tail;
	 tail->prev = head;
         for (i = 1; i < argc; i++)
            if (argv[i][0] == CHAR_MINUS){
               if (i  + 1 < argc){
	          if (argv[i][1] == CHAR_d)
		     fb_rootname(dname, argv[i + 1]);
		  else if (argv[i][1] == CHAR_i)
		     fb_rootname(iname, argv[i + 1]);
		  else if (argv[i][1] == CHAR_s)
		     fb_rootname(sname, argv[i + 1]);
		  else if (argv[i][1] == CHAR_v)
		     fb_rootname(vname, argv[i + 1]);
                  i++;
		  }
               }
            else if (strchr(argv[i], '=') == 0)
               strcpy(cmenu, argv[i]);
         fb_dirname(menu_rootdir, cmenu);
	 fs = fb_mustfopen(cmenu, "r");
	 if (fb_testargs(argc, argv, NON_CDB)){
	    cdb_db->dbase[0] = cdb_db->dindex[0] = NULL;
	    cdbmenu = 0;
	    }
	 else
	    fb_initdbd(cdb_db);
         fb_cx_set_menu(cmenu);
         fb_cx_push_env("EVH", CX_MENU_SELECT, NIL);
         set_mail();
         cdb_functionkeys = 0;
	 for (;;){
	    process();
	    if (force_exit)
	       break;
	    if (cdb_menupause == 0 ||
                  fb_mustbe(CHAR_y, EXITMSG, cdb_t_lines, 1) == FB_AOK)
	       break;
	    }
         fb_cx_pop_env();
	 fclose(fs);
#if FB_CDB_SECURE
         if (cdb_secure){
	    fb_acctlog(FB_AC_ENDTOOL, dname, iname);
            fb_clear(), fb_move(1,78);
            fb_printw(SYSMSG[S_FB]);
            fb_move(cdb_t_lines,1);
            fb_refresh();
            fb_settty(0);
            if (rlogshell)
               fb_rlogout();
            else if (logshell){
               execl("/fbetc/fblogout", "-fblogout", 0);
               printf("No FirstBase Logout Process.\n");
               }
            exit(0);
            }
#endif /* FB_CDB_SECURE */
	 if (force_exit){
	    fb_exit(exit_value);
	    }
	 fb_ender();
      }

/*
 * process loop of dbshell
 */

   static void process()

      {
         int st;
	 
	 initmenu();
         shell_redraw();
	 for (;;){
	    st = sh_select();
	    if (force_exit)
	       return;
	    if (st == FB_END)
	       break;
	    else if (st == FB_ERROR)
	       fb_serror(FB_MESSAGE, "Cannot Find That Command", NIL);
	    else{
	       if (cdbmenu)
	          fb_initdbd(cdb_db);
	       shell_redraw();
	       }
	    }
      }

/*
 * select - selection mechanism of dbshell
 */

   static sh_select()
      {
         char word[FB_MAXLINE], xdir[FB_MAXNAME], *fb_getwd();
	 int st, col;
	 struct an_rmenu *p;
         FILE *fb_mustfopen();
	 
	 for (st = 0;;){
            test_mail();
            /*
             * setjmp is either returning normally or from a signal (ALRM)
             */
	    setjmp(dbshell_jmp_env);
	    if (st != FB_DEFAULT){
	       if (ploc_row > 0){
		  fb_move(cdb_t_lines, 1); fb_clrtoeol();
		  fb_scrhlp(MSG2);
		  fb_move(ploc_row, ploc_col);
		  fb_printw(cdb_dbshell_pstring);
		  }
	       }
	    if (ploc_col > 0)
	       col = ploc_col + strlen(cdb_dbshell_pstring);
	    else
	       col = ploc_col;
            if (cdb_dbshell_checkmail > 0){
               signal(SIGALRM, dbshell_sigalrm);
               alarm(cdb_dbshell_checkmail);
               }
	    st = fb_input(-ploc_row, -col, -cdb_dbshell_pilength, 
	       0, FB_ALPHA, word, FB_ECHO, FB_OKEND, FB_CONFIRM);
            if (cdb_dbshell_checkmail > 0){
               signal(SIGALRM, SIG_IGN);
               alarm(0);
               }
	    if (st == FB_ABORT)
	       return(FB_END);
	    else if (st == FB_END){
	       if (head->next == tail)
	          return(FB_END);
	       strcpy(cmenu, tail->prev->rmenu);
	       strcpy(xdir, tail->prev->rdir);
	       fclose(fs);
	       chdir(xdir);
	       fb_mkstr(&cdb_work_dir, fb_getwd(xdir));
               fb_getco((fb_database *) NULL);
	       fs = fb_mustfopen(cmenu, "r");
               fb_dirname(menu_rootdir, cmenu);
	       p = tail->prev;
	       tail->prev = tail->prev->prev;
	       tail->prev->next = tail;
	       fb_free(p->rmenu);
	       fb_free((char *) p);
               fb_cx_set_workdir(xdir);
               fb_cx_set_menu(cmenu);
               fb_cx_write(1);
	       initmenu();
	       shell_redraw();
	       }
	    else if (st == FB_QHELP){
	       sprintf(word, "%s.help", cmenu);
	       fb_fhelp(word);
	       shell_redraw();
	       }
	    else if (cdb_dbshell_ecommand && 
	          (st == FB_DSIGNAL || st == FB_ESIGNAL || st == FB_YSIGNAL)){
               fb_cx_push_env("EH", CX_ENV_SELECT, NIL);
	       mcommand();
               fb_cx_pop_env();
	       return(FB_AOK);
	       }
	    else if (st == FB_AOK){
	       fb_rmlead(fb_trim(word));
	       if (command(word) == FB_AOK)
	          return(FB_AOK);
	       fseek(fs, seekpoint, 0);
               st = execute(word);
               return(st);
	       }
	    }
      }

/*
 * command()
 */

   static command(w)
      char *w;
   
      {
         char dir[FB_MAXLINE], *fb_getwd();
         int st;

         if (equal(w, "cd")){
            fb_cx_push_env("XT", CX_KEY_SELECT, NIL);
            fb_move(cdb_t_lines, 1); fb_clrtoeol();
            fb_printw("Change to What Directory? ");
            if (cdb_edit_input)
               strcpy(cdb_e_buf, fb_getwd(dir));
            st = fb_input(cdb_t_lines, 30, 50, 0, FB_ALPHA, dir, FB_ECHO,
               FB_OKEND, FB_CONFIRM);
            if (st == FB_AOK){
               fb_trim(dir);
               if (chdir(dir) != 0)
                  fb_serror(FB_MESSAGE, "Cannot change to directory:", dir);
               else{
                  fb_mkstr(&cdb_work_dir, fb_getwd(dir));
                  fb_initdbd(cdb_db);
                  fb_cx_set_workdir(dir);
                  fb_getco((fb_database *) NULL);
                  }
               }
            fb_cx_pop_env();
            return(FB_AOK);
            }
         else if (equal(w, "?b")){
            fb_help(w, cdb_db);
            shell_redraw();
            return(FB_AOK);
            }
         return(FB_ERROR);
      }

/*
 * shell_redraw - redraw the screen
 */

   static shell_redraw()
      {
         char token[FB_MAXLINE], *p;
	 int j, k, row;

	 fb_clear();
	 if ((p = strrchr(cmenu, '/')) == 0)
	    strcpy(pmenu, cmenu);
	 else
	    strcpy(pmenu, p+1);
	 fb_scrhdr(cdb_db, pmenu);
	 for (row = 2, k = 0; k < SCREENMAX; k++, row++){
	    p = screen[k];
	    if (p != NULL && *p){
	       fb_move(row, 1);
	       for (j = 1; (j = fb_gettoken(p, j, token, '_')) > 0; ){
		  if (token[0] != CHAR_DOLLAR)
		     fb_printw("%s", token);
		  else{
		     j = fb_gettoken(p, j, token, '_');
		     if (j <= 0)
			break;
		     fb_underscore(token, 0);
		     fb_reverse(token);
		     }
		  }
	       }
	    }
	 fb_scrtime(cdb_db);
      }

/*
 * initmenu - gather in the current screen into screen array.
 */

   initmenu()
      {
         char buffer[FB_MAXLINE];
	 int k = 0, j;
	 
	 for (j = 0; j < SCREENMAX; j++)
	    if (screen[j] != NULL){
	       fb_free(screen[j]);
	       screen[j] = NULL;
	       }
         fseek(fs, 0L, 0);
	 seekpoint = 0L;
	 while (fgets(buffer, FB_MAXLINE, fs) != NULL){
	    if (buffer[0] == CHAR_PERCENT){
	       seekpoint = ftell(fs);
	       break;
	       }
	    if (k >= SCREENMAX)
	       break;
	    fb_mkstr(&(screen[k++]), buffer);
	    }
	 if (seekpoint == 0L){
	    while (fgets(buffer, FB_MAXLINE, fs) != NULL){
	       if (buffer[0] == CHAR_PERCENT){
		  seekpoint = ftell(fs);
		  break;
		  }
	       }
	    }
	 if (seekpoint == 0L)
            fb_xerror(FB_MESSAGE, "dbshell MENU [database options] [-n]", NIL);
      }

/* 
 *  usage - usage for dbmenu 
 *
 * usage()
 *    {
 *       fb_xerror(FB_MESSAGE, "dbshell MENU [database options] [-n]", NIL);
 *    }
 */

/*
 * rsh_usage - usage for dbshell
 */

   static rsh_usage()
      {
         fb_xerror(FB_MESSAGE, 
            "fbrsh [-n] [-x command] [database options] menufile", NIL);
      }

/*
 * set_mail - init the email size. clear the email line.
 */

   void set_mail()
      {
         char mailfile[FB_MAXLINE], *p;
         struct stat ms;

         mailsize = 0;
         p = getenv("MAIL");
         if (p == NULL)
            return;
         strcpy(mailfile, p);
         if (stat(mailfile, &ms) == 0){
            mailsize = ms.st_size;
            }
      }

/*
 * test_mail -
 *    if no change in size, return
 *    size > old size, show You have new mail.
 *    size < old size, clear message
 */
 
   void test_mail()
      {
         char mailfile[FB_MAXLINE], *p;
         struct stat ms;
         off_t new_mailsize;

         p = getenv("MAIL");
         if (p == NULL)
            return;
         strcpy(mailfile, p);
         if (stat(mailfile, &ms) == 0)
            new_mailsize = ms.st_size;
         else
            new_mailsize = 0;
         if (mailsize < new_mailsize){
            fb_clear();
            fb_move(12,35);
            fb_reverse("You have new mail.");
            fb_refresh();
            cdb_e_buf[0] = NULL;
            FB_PAUSE();
            shell_redraw();
            if (stat(mailfile, &ms) == 0)
               new_mailsize = ms.st_size;
            else
               new_mailsize = 0;
            }
         mailsize = new_mailsize;
         fb_refresh();
      }

   static RETSIGTYPE dbshell_sigalrm(d)
      int d;

      {
         test_mail();
         fb_infoline();
	 longjmp(dbshell_jmp_env, 1);
      }
