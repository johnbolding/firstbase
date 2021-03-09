/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: System.c,v 9.4 2001/03/18 16:51:33 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char System_sid[] = "@(#) $Id: System.c,v 9.4 2001/03/18 16:51:33 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif /* HAVE_SYS_WAIT_H */

#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

#include <signal.h>

#if HAVE_SETJMP_H
#include <setjmp.h>
static jmp_buf jmp_env;
#if FB_PROTOTYPES
static RETSIGTYPE ontstp(int);
static RETSIGTYPE stopjob(int);
#else /* FB_PROTOTYPES */
static RETSIGTYPE ontstp();
static RETSIGTYPE stopjob();
#endif /* FB_PROTOTYPES */
#endif /* HAVE_SETJMP_H */

#if HAVE_VFORK_H
#include <vfork.h>
#endif /* HAVE_VFORK_H */

#define MAXARGS 15
#define MAXWORD 100

extern short int cdb_secure;
extern short int cdb_okstop;
extern short int cdb_use_rpc;

/* the global errno for detecting error status from execvp */
extern int errno;

/*
 *  System - simulate the unix system(3) call using fork and exec.
 *	- this saves one process call.
 */

   fb_system(s, rootperm)
      register char *s;
      int rootperm;
   
      {
	 char *argv[MAXARGS], **p, name[MAXWORD], word[MAXWORD];
	 short j, i;
         int rval = 0, pid, t_pid, st = 0;
         int status;

#if RPC
         if (cdb_use_rpc)
            return(fb_system_clnt(s, rootperm));
#endif /* RPC */
         errno = 0;
         status = 0;
         if (strchr(s, ';') != 0 || strchr(s, '>') != 0 ||
	     strchr(s, '|') != 0 || strchr(s, '\"') != 0 ||
	     strchr(s, '<') != 0){	/* compound commands go to shell */
            /*
             * this will use either vfork or fork depending on the system
             */
            pid = vfork();
	    if (pid == 0){
               if (cdb_secure)
                  if (rootperm == 0)
                     fb_noroot();
               if (!cdb_batchmode)
	          fb_settty(FB_ENDMODE);
               /*
                * this used to call system() but its flaky on linux, so...
                */
               argv[0] = "sh";
               argv[1] = "-c";
               argv[2] = s;
               argv[3] = 0;
               execvp("/bin/sh", argv);
               /* execvp should not return */
               exit(127);
	       }
	    else{
#if HAVE_SETJMP_H
	       if (cdb_okstop){
		  if (setjmp(jmp_env)){ /* return from TTYSTOP sig handle*/
		     stopjob();
		     sleep(1);
		     }
		  signal(SIGTSTP, ontstp);
		  }
	       else
		  signal(SIGTSTP, SIG_IGN);
#endif /* HAVE_SETJMP_H */
               /*
                * wait as needed for return.
                */

               for (;;){
	          t_pid = waitpid(pid, &status, 0);
                  if (t_pid < 0 || t_pid == pid)
                     break;
                  }
               if (WIFEXITED(status))
                  rval = WEXITSTATUS(status);
               else
                  rval = -1;
	       }
            if (!cdb_batchmode)
	       fb_settty(FB_EDITMODE);
	    return(rval);
	    }
	 j = fb_getword(s, 1, name);
	 argv[0] = 0;
	 p = argv;
	 fb_mkstr(p, name);
	 for (i = 1; i < MAXARGS && (j = fb_getword(s, j, word)) > 0; i++){
	    *++p = 0;
	    fb_mkstr(p, word);
	    }
	 *++p = 0;
         /*
          * this will use either vfork or fork depending on the system
          */
         pid = vfork();
	 if (pid == 0){
            if (cdb_secure)
	       if (rootperm == 0)
	          fb_noroot();
            if (!cdb_batchmode)
	       fb_settty(FB_ENDMODE);
	    execvp(name, argv);
            /* execvp should not return */
            exit(127);
	    }
	 else{
#if HAVE_SETJMP_H
	    if (cdb_okstop){
	       if (setjmp(jmp_env)){	/* return from TTYSTOP sig handler */
		  stopjob();
		  sleep(1);
		  }
	       signal(SIGTSTP, ontstp);
	       }
	    else
	       signal(SIGTSTP, SIG_IGN);
#endif /* HAVE_SETJMP_H */
            /*
             * #if !VFORK && !LINUX
	     * sleep(3);
             */
            for (;;){
               t_pid = waitpid(pid, &status, 0);
               if (t_pid < 0 || t_pid == pid)
                  break;
               }
            if (WIFEXITED(status))
               rval = WEXITSTATUS(status);
            else
               rval = -1;
            if (!cdb_batchmode)
	       fb_settty(FB_EDITMODE);
            for (p = argv; *p != 0; p++)
               fb_free(*p);
	    }
         return(rval);
      }

#if HAVE_SETJMP_H
   static RETSIGTYPE ontstp(disp)		/* set up jump point */
      int disp;

      {
         (void) disp;

	 longjmp(jmp_env, 1);
      }

   static RETSIGTYPE stopjob(disp)		/* stop this job */
      int disp;

      {
         (void) disp;

         fflush(stdout);
	 fb_settty(FB_ENDMODE);
	 signal(SIGTSTP, SIG_DFL);
	 kill(getpid(), SIGTSTP);
	 signal(SIGTSTP, SIG_IGN);
	 fb_settty(FB_EDITMODE);
      }
#endif /* HAVE_SETJMP_H */
