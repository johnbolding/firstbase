/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbmacro.c,v 9.4 2005/01/04 23:31:50 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbmacro_sid[] = "@(#) $Id: dbmacro.c,v 9.4 2005/01/04 23:31:50 john Exp $";
#endif

/*  
 *  dbmacro.c - engine for the dbmacro tool
 */

#include <fb.h>
#include <fb_vars.h>
#include <macro_v.h>
#include <signal.h>
#include <setjmp.h>

extern short int cdb_secure;
extern short int cdb_returnerror;
extern short int cdb_lockmessage;
extern short int cdb_usrlog;
extern short int cdb_limited;
extern short int cdb_locktime;
extern char *cdb_logfile;

static short int cdb_locktime_cgi = 300;

static char *USAGE =
   "usage: dbmacro [-d dbase] [-i index] [-c] [-r] [macrofile | -m script]";
static long n;
short int cdb_write_it = 0;
short int read_only = 0;
short int trace_flag = 0;
int mode;

extern short int cdb_cgi_flag;

#if !FB_PROTOTYPES
static macro();
static initmacro();
static process();
static s_macro();
static writerec();
static void usage();
#else
int main(int, char **);
static int macro(char *, char *);
static int initmacro(char *, char *);
static int process(fb_database *);
static int s_macro(fb_database *);
static int writerec(fb_database *);
static void usage(void);
#endif

static short int withdatabase = 0, withindex = 0;
static fb_mnode *s_exec;
static jmp_buf jmp_env;
static RETSIGTYPE dbmacro_sigalrm(int disp);

/*
 *  dbmacro - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
	 int i;
         char fname[FB_MAXNAME], iname[FB_MAXLINE], *p, var[FB_MAXLINE];
         char word[FB_MAXLINE], *mscript = NULL, path_trans[FB_MAXLINE];
         char path_dir[FB_MAXLINE];

         (void) Dbmacro_sid;

         cdb_batchmode = 1;
         cdb_returnerror = 1;
         fname[0] = NULL;
         iname[0] = NULL;
         fb_basename(fname, argv[0]);
         i = strlen(fname);
         if (equal(fname+i-4, ".cgi"))
            cdb_cgi_flag = 1;
         fname[0] = NULL;
         cdb_db = (fb_database *) NULL;
         p_symtab = symtab = fb_makesymtab();
         g_symtab = fb_makesymtab();
         for (i = 1; i < argc; i++){
            if (argv[i][0] == CHAR_MINUS){
               if (argv[i][2] == NULL){
                  switch (argv[i][1]) {
                     case 'c':
                        cdb_cgi_flag = 1;
                        break;
                     case 'r':
                        read_only = 1;
                        break;
                     case 'd':
                        withdatabase = 1;
                        i++;
                        break;
                     case 'i':
                        withindex = 1;
                        i++;
                        break;
                     case 'x':
                        trace_flag = 1;
                        break;
                     case 'm':
                        i++;
                        mscript = argv[i];
                        break;
                     }
                  }
               }
	    else if ((p = strchr(argv[i], '=')) != 0){
	       *p = NULL;
	       strcpy(var, argv[i]);
	       strcpy(word, p + 1);
	       mf_set_value(var, word);
	       *p = '=';
               }
            else
               strcpy(fname, argv[i]);
            }

         if (cdb_cgi_flag){
            fb_getargs(argc, argv, FB_NODB);
            
            /*
             * this can be used to try and debug why
             * some cgi requests appear to hang.
             *   fb_setup_argv(argc, argv);
             *   fb_writeable_homefile(&cdb_logfile, "USRLOG");
             *   if (cdb_usrlog > 5)
             *      fb_usrlog_begin(argc, argv);
             */

            fb_cgi_init();

            if (fb_cgi_read() != FB_AOK){
               printf("<P>Error reading cgi input\n");
               fflush(stdout);
               exit(-1);
               }
               
            /*
             * use this to test if needed.
             * fb_cgi_echo(); exit();
             */

            /* PATH_TRANSLATED is the cgi fname */
            path_dir[0] = NULL;
            path_trans[0] = NULL;
            p = getenv("PATH_TRANSLATED");
            if (p){
               strcpy(path_trans, p);
               fb_dirname(path_dir, path_trans);
               mf_set_value("PATH_TRANSLATED", path_trans);
               mf_set_value("path_translated", path_trans);
               mf_set_value("PATH_DIR", path_dir);
               mf_set_value("path_dir", path_dir);
               }

            /* fb_getargs(argc, argv, FB_NODB); */

            /* get the database and index field */
            fb_cgi_value(fname, "database");
            fb_cgi_value(iname, "index");
            if (fname[0] != NULL && fname[0] != '/'){
               sprintf(word, "%s%s", path_dir, fname);
               strcpy(fname, word);
               }
            if (iname[0] != NULL && iname[0] != '/'){
               sprintf(word, "%s%s", path_dir, iname);
               strcpy(iname, word);
               }
            if (fname[0] != NULL){
	       cdb_db = fb_dballoc();
               fb_dbargs(fname, iname, cdb_db);
               withdatabase = 1;
               }

            strcpy(fname, path_trans);
            fb_cgi_foreach(mf_set_value);
            fb_cgi_checkbox(mf_set_checkbox);
            }
         else {
            /* normal (non cgi) path */
            if (withdatabase)
               fb_getargs(argc, argv, FB_ALLOCDB);
            else
               fb_getargs(argc, argv, FB_NODB);
            }

         cdb_lockmessage = 0;		/* turn off the lock() err message */
	 fb_allow_int();		/* allows instant piping */
/*
 * Before autoconf, this was _M_I386 from an SCO include file, I think.
 * Now its IGNORE_FPE set in configure.in for autoconf to use.
 */
#if IGNORE_FPE
         fb_ignore_fpe();
#endif
         /* set up standard pre constants used to signal */
         mf_set_constant("END", FB_END);
         mf_set_constant("ERROR", FB_ERROR);
         mf_set_constant("ABORT", FB_ABORT);
         mf_set_constant("NEXT", FB_ESIGNAL);
         mf_set_constant("PREV", FB_YSIGNAL);
         mf_set_constant("DEFAULT", FB_DEFAULT);
         mf_set_constant("NO_OP", FB_ERROR);
         mf_set_constant("AOK", 1);
         mf_set_constant("FB_AOK", FB_AOK);
         mf_set_constant("FB_ERROR", FB_ERROR);
         mf_set_constant("FB_END", FB_END);
         mf_set_constant("FB_ABORT", FB_ABORT);
         mf_set_constant("FB_NEXT", FB_ESIGNAL);
         mf_set_constant("FB_PREV", FB_YSIGNAL);
         mf_set_constant("FB_DEFAULT", FB_DEFAULT);
	 macro(fname, mscript);

         /* now do garbage collection */
         fb_gcollect_loadnode();
         fb_gcollect(n_ghead, c_ghead);
         fb_expunge_symtab(p_symtab);
         fb_expunge_symtab(g_symtab);
         fb_gcollect_m_pool();
         fb_gcollect_c_pool();
         fb_gcollect_s_pool();
	 fb_ender();
      }

/*
 *  macro - macro loop for a single dbase.
 */

   static macro(mname, mscript)
      char *mname, *mscript;
      
      {
	 fb_database *hp;
	 int i, tfd, st;

         if (initmacro(mname, mscript) == FB_ERROR)
            usage();
         hp = (fb_database *) NULL;
         if (withdatabase){
            if (cdb_limited){
               fb_serror(FB_MESSAGE, "Could not open database/index", NIL);
               return(FB_ERROR);
               }
            hp = cdb_db;
            tfd = open(cdb_db->dbase, 2);   /* test for write permissions */
            if (tfd > 0)
               mode = READWRITE;
            else
               mode = READ;
            close(tfd);
            st = fb_opendb(hp, mode, FB_ALLINDEX, FB_MAYBE_OPTIONAL_INDEX);
            if (st != FB_AOK){
               fb_serror(FB_MESSAGE, "Could not open database/index", NIL);
               return(FB_ERROR);
               }
            for (i = 0; i < hp->nfields; i++)
               fb_nounders(hp->kp[i]);
            }
         
         process(hp);

         /* no close on fd needed since always stdout */
         if (withdatabase)
	    fb_closedb(hp);
         return(FB_AOK);
      }

   static initmacro(mname, mscript)
      char *mname, *mscript;

      {
         int st;
         char buf[FB_MAXLINE];

         if (cdb_cgi_flag && cdb_usrlog > 5){
            sprintf(buf, "initmacro beginning - %s", mname);
            fb_usrlog_msg(buf);
            }
         if (mscript != NULL)
            st = fb_macroscript(mscript);
         else if (mname != NULL && mname[0] != NULL)
            st = fb_macrotree(mname);
         else
            st = fb_macrostdin();
         if (st == FB_AOK)
            e_winner = winner;
         if (cdb_cgi_flag && cdb_usrlog > 5){
            sprintf(buf, "initmacro ending - %s", mname);
            fb_usrlog_msg(buf);
            }
         return(st);
      }

/* 
 *  process - loop boot strap point using foreach ...
 */
 
   static process(hp)
      fb_database *hp;
      
      {
         int st = FB_ERROR;
	 
         n = 0L;

         /* if running with a database, locate and execute BEGIN section */
         if (withdatabase){
            s_exec = locate_section(S_BEGIN);
            if (s_exec != (fb_mnode *) NULL){
               mf_init_stack(main_sv);
               mf_make_frame(main_sv);
               m_verify_sub(s_exec);
               macro_statement(s_exec, main_sv);
               /* leave this sym table for other use */
               }
            }

         /* locate and execute BODY section */
         mf_init_stack(main_sv);
         mf_make_frame(main_sv);
         s_exec = locate_section(S_BODY);
         if (s_exec != (fb_mnode *) NULL){
            if (trace_flag)
               fb_macrotrace(s_exec, NULL);
            m_verify_sub(s_exec);
            if (withdatabase){
               if (hp->dindex == NULL || hp->dindex[0] == NULL || hp->ihfd < 0)
                  st = fb_foreach(hp, s_macro);
               else
                  st = fb_forxeach(hp, s_macro);
               }
            else
               s_macro((fb_database *) NULL);
            mf_destroy_frame(main_sv);
            m_destroy_call_lists(s_exec);
            }

         /* if running with a database, locate and execute END section */
         if (withdatabase){
            s_exec = locate_section(S_END);
            if (s_exec != (fb_mnode *) NULL){
               mf_init_stack(main_sv);
               mf_make_frame(main_sv);
               m_verify_sub(s_exec);
               macro_statement(s_exec, main_sv);
               }
            }

         return(st);
      }

/* 
 *  s_macro - called by foreach type mechanism
 */

   static s_macro(hp)
      fb_database *hp;
      
      {
	 n++;
         if (withdatabase)
            fb_set_autoindex(hp);
         if (s_exec != NULL){
            /*
             * if (cdb_cgi_flag && cdb_usrlog > 5)
             *   fb_usrlog_msg("process (7A)");
             */
            if (cdb_cgi_flag){
               if (setjmp(jmp_env) == 1){
                  /* value of 1 means return from longjmp */
                  alarm(0);
                  signal(SIGALRM, SIG_DFL);
                  fb_usrlog_msg("longjmp out of dbmacro execution");
                  return(FB_ERROR);
                  }
               signal(SIGALRM, dbmacro_sigalrm);
               alarm(cdb_locktime_cgi);
               }
            macro_statement(s_exec, main_sv);
            if (cdb_cgi_flag){
               alarm(0);
               signal(SIGALRM, SIG_DFL);
               }
            }
         if (main_sv->break_flag)
            fb_xerror(FB_MESSAGE, "Illegal `break'", NIL);
         if (main_sv->continue_flag)
            fb_xerror(FB_MESSAGE, "Illegal `continue'", NIL);
         if (cdb_write_it && mode == READWRITE && withdatabase){
            if (cdb_secure && !fb_record_permission(cdb_db, WRITE))
               cdb_write_it = 0;
            if (cdb_write_it && !read_only)
               writerec(hp);
            }
         if (main_sv->return_flag)
            return(FB_ERROR);
	 return(FB_AOK);
      }

/*
 * writerec - actual code to write/ceate the record.
 */

   static writerec(hp)
      fb_database *hp;

      {
         long rec;
         FILE *fs;

         rec = hp->rec;
         /* get lock of record first */
         if (fb_lock(rec, hp, FB_NOWAIT) != FB_AOK){ /* dont wait if locked */
            if (cdb_cgi_flag)
               fs = stdout;
            else
               fs = stderr;
            fprintf(fs, "Warning - record not written to disk: %ld\n",rec);
            return(0);
            }

         /* critical section - do not let this be interrupted */
	 signal(SIGINT, SIG_IGN);

	 fb_lock_head(hp);
	 fb_setdirty(hp, 1);
	 fb_allsync(hp);

	 /* if a putrec causes fb_xerror, rec 0 is still locked
	  * this will keep others out of the dbase until the
	  * problem can be fixed. this is a feature, not a bug.
	  */

	 /* fb_putlog(hp); */

	 if (fb_putrec(hp->rec, hp)==FB_ERROR)
	    fb_xerror(FB_WRITE_ERROR, SYSMSG[S_RECORD], hp->dbase);

	 /* fb_checklog(hp); */
	 fb_setdirty(hp, 0);
	 if (fb_put_autoindex(hp) == FB_ERROR)
	    fb_serror(FB_BAD_INDEX, hp->dbase, NIL);
	 fb_allsync(hp);
	 fb_unlock_head(hp);
	 fb_unlock(rec, hp);
	 cdb_write_it = 0;

         /* end critical section */
         signal(SIGINT, fb_onintr);
         return(1);
      }

/* 
 * usage message 
 */

   static void usage()
      {
         fb_xerror(FB_MESSAGE, USAGE, NIL);
      }

   static RETSIGTYPE dbmacro_sigalrm(disp)
      int disp;

      {
         (void) disp;
	 longjmp(jmp_env, 1);
      }
