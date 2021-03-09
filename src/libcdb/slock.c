/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: slock.c,v 9.1 2001/02/16 19:09:07 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Cdb_slock_sid[] = "@(#) $Id: slock.c,v 9.1 2001/02/16 19:09:07 john Exp $";
#endif

/*
 * slock.c has all the different styles of locking in it
 * autoconf detects the best method to use, first fit style.
 */

#include <fb.h>

#if HAVE_FCNTL && HAVE_GETITIMER

#include <fb_ext.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <setjmp.h>

static jmp_buf jmp_env;

static struct flock fk;

extern short int cdb_lockdaemon;
extern short int cdb_locklevel;
extern short int cdb_locktime;
extern short int cdb_lockmessage;
extern short int cdb_use_rpc;
static RETSIGTYPE slock5_sigalrm();

/* 
 *  fb_s_lock - simple lock a complete file.
 */
 
   fb_s_lock(fd, fwait, fname)
      int fd, fwait;
      char *fname;
   
      {
	 register int f;
	 int func, st = FB_AOK;

         /* ignore locking attempts if locklevel is not set */
	 if (cdb_locklevel <= 0)
	    return(FB_AOK);
	 if (fwait){
            fb_set_alarm(cdb_locktime, slock5_sigalrm);
            /*
             * setjmp saves the current location to jump back to if needed.
             * a return value of 1 means return from longjmp, meaning
             * the timer expired -- iff the local signal handlers are executed.
             */
	    if (setjmp(jmp_env) == 1){
               fb_jump_alarm();
	       return(FB_ERROR);
               }
	    func = F_SETLKW;
	    }
	 else
	    func = F_SETLK;
	 fk.l_type = F_WRLCK;
	 fk.l_whence = fk.l_start = fk.l_len = 0;
	 if (lseek(fd, fk.l_start, 0) < 0L){
            if (fwait)
               fb_restore_alarm();
	    return(FB_ERROR);
	    }
         if (cdb_lockdaemon)
#if RPC
            f = fb_fcntl_clnt(fname, func, &fk);
#else
            f = -1;
#endif
         else
            f = fcntl(fd, func, &fk);
         if (f == -1)
            st = FB_ERROR;
         if (fwait)
            fb_restore_alarm();
	 return(st);
      }

/* 
 *  s_unlock record 
 */
 
   fb_s_unlock(fd, fname)
      int fd;
      char *fname;
      
      {
         int st;

         /* ignore locking attempts if locklevel is not set */
	 if (cdb_locklevel <= 0)
	    return(FB_AOK);
	 fk.l_type = F_UNLCK;
	 fk.l_whence = fk.l_start = fk.l_len = 0;
	 if (lseek(fd, fk.l_start, 0) < 0L){
	    /*fb_serror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], "UN4");*/
            return(FB_ERROR);
	    }
         if (cdb_lockdaemon){
#if RPC
	    st = fb_fcntl_clnt(fname, F_SETLK, &fk);
            if (st == 0)
               fb_fcntl_cl_clnt(fname, 0L, 0L, 0, 0);
#else
            st = -1;
#endif /* RPC */
            }
	 else
            st = fcntl(fd, F_SETLK, &fk);
         if (st < 0)
            return(FB_ERROR);
         return(FB_AOK);
      }

   static RETSIGTYPE slock5_sigalrm()
      {
	 longjmp(jmp_env, 1);
      }


#elif HAVE_FCNTL && !HAVE_GETITIMER

#include <fb_ext.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>

static jmp_buf jmp_env;

static struct flock fk;

extern short int cdb_lockdaemon;
extern short int cdb_use_rpc;
extern short int cdb_locktime;

#if FB_PROTOTYPES
static RETSIGTYPE cdb_sigalrm(int disp);
#else /* FB_PROTOTYPES */
static RETSIGTYPE cdb_sigalrm();
#endif /* FB_PROTOTYPES */

/* 
 *  fb_s_lock - simple lock a complete file.
 */
 
   fb_s_lock(fd, fwait, fname)
      int fd;			
      int fwait;
      char *fname;
   
      {
	 register int f;
	 int func;
   
#if RPC
         if (cdb_use_rpc)
            return(fb_s_lock_clnt(fd, fwait, fname));
#endif /* RPC */
	 if (fwait){
	    if (setjmp(jmp_env) == 1){
               /* value of 1 means return from longjmp */
	       alarm((unsigned) 0);
	       signal(SIGALRM, SIG_DFL);
	       return(FB_ERROR);
               }
            signal(SIGALRM, cdb_sigalrm);
            alarm((unsigned) cdb_locktime);
	    func = F_SETLKW;
	    }
	 else
	    func = F_SETLK;
	 fk.l_type = F_WRLCK;
	 fk.l_whence = fk.l_start = fk.l_len = 0;
	 if (lseek(fd, fk.l_start, 0) < 0L){
            if (fwait){
	       alarm(0);
	       signal(SIGALRM, SIG_DFL);
               }
	    return(FB_ERROR);
	    }
         if (cdb_lockdaemon)
#if RPC
            f = fb_fcntl_clnt(fname, func, &fk);
#else
            f = -1;
#endif
         else
            f = fcntl(fd, func, &fk);
         if (f != -1){
            if (fwait){
               alarm(0);
               signal(SIGALRM, SIG_DFL);
               }
            return(FB_AOK);
            }
         if (fwait){
            alarm(0);
            signal(SIGALRM, SIG_DFL);
            }
	 return(FB_ERROR);
      }

/* 
 *  s_unlock record 
 */
 
   fb_s_unlock(fd, fname)
      int fd;
      char *fname;
      
      {
         int st;

#if RPC
         if (cdb_use_rpc)
            return(fb_s_unlock_clnt(fd, fname));
#endif /* RPC */
	 fk.l_type = F_UNLCK;
	 fk.l_whence = fk.l_start = fk.l_len = 0;
	 if (lseek(fd, fk.l_start, 0) < 0L){
	    /*fb_serror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], "UN4");*/
            return(FB_ERROR);
	    }
         if (cdb_lockdaemon){
#if RPC
	    st = fb_fcntl_clnt(fname, F_SETLK, &fk);
            if (st == 0)
               fb_fcntl_cl_clnt(fname, 0L, 0L, 0, 0);
#else
            st = -1;
#endif /* RPC */
            }
	 else
            st = fcntl(fd, F_SETLK, &fk);
         if (st < 0)
            return(FB_ERROR);
         return(FB_AOK);
      }

   static RETSIGTYPE cdb_sigalrm(disp)
      int disp;

      {
	 longjmp(jmp_env, 1);
      }

#elif HAVE_LOCKF

#include <fb_ext.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>

static jmp_buf jmp_env;
static char *ERRMSG = "Process timed out attempting blocking Lock.";
static char *ERRMSG0 = "Lock error attempting to set blocking Lock.";
static char *ERRMSG1 = "Lock error attempting to unset blocking Lock.";
static char *ERRMSG2 = "Lock request timed out/not granted.";
extern short int cdb_locktime;

static RETSIGTYPE slock3_sigalrm(int disp);

/* 
 *  fb_s_lock - simple lock of a complete file.
 */
 
   fb_s_lock(fd, fwait, fname)
      int fd;
      int fwait;
      char *fname;
   
      {
	 register int f;
	 int func;
   
	 if (fwait){
	    if (setjmp(jmp_env) == 1){
               /* value of 1 means return from longjmp */
	       alarm(0);
	       signal(SIGALRM, SIG_DFL);
               fb_lerror(FB_ABORT_ERROR, ERRMSG2, NIL);
	       return(FB_ERROR);
               }
	    signal(SIGALRM, slock3_sigalrm);
	    alarm(cdb_locktime);
	    func = F_LOCK;
	    }
	 else
	    func = F_TLOCK;
	 if (lseek(fd, 0L, 0) < 0L){
	    signal(SIGALRM, SIG_DFL);
	    alarm(0);
	    fb_serror(FB_SEEK_ERROR, "SL3", NIL);
	    return(FB_ERROR);
	    }
	 for (;;) {
	    f = lockf(fd, func, 0L);
	    if (f == 0){
	       signal(SIGALRM, SIG_DFL);
	       alarm(0);
	       return(FB_AOK);
	       }
	    if (fwait == 0)
	       break;
	    }
	 signal(SIGALRM, SIG_DFL);
	 alarm(0);
	 return(FB_ERROR);
      }

/* 
 *  s_unlock - simple unlock of a file.
 */
 
   fb_s_unlock(fd, fname)
      int fd;
      char *fname;
      
      {
	 long pos;

	 if (lseek(fd, 0L, 0) < 0L){
	    fb_serror(FB_SEEK_ERROR, "LU3", NIL);
	    return(FB_ERROR);
	    }
	 if (lockf(fd, F_ULOCK, 0L) < 0)
	    fb_serror(FB_MESSAGE, ERRMSG1, NIL);
         return(FB_AOK);
      }


   static RETSIGTYPE slock3_sigalrm(disp)
      int disp;

      {
         (void) disp;
	 longjmp(jmp_env, 1);
      }

#else /* if HAVE_FLOCK */

#include <fb_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#if HEADER_SYS_FILE_H
#include <sys/file.h>
#endif /* HEADER_SYS_FILE_H */

static char *ERRMSG = "Process timed out attempting blocking Lock.";
static char *ERRMSG0 = "Lock error attempting to set blocking Lock.";
static char *ERRMSG1 = "Lock error attempting to unset blocking Lock.";

static RETSIGTYPE sigalrm()	{ fb_xerror(FB_MESSAGE, ERRMSG, NIL); }

/* 
 *  fb_s_lock - simple lock of a whole file.
 */
 
   fb_s_lock(fd, fwait, fname)
      int fd, fwait;
      char *fname;

      {
	 if (fwait){
	    signal(SIGALRM, sigalrm);
	    alarm(cdb_locktime);
	    }
	 if (flock(fd, LOCK_EX) < 0){
	    alarm(0);
	    fb_xerror(FB_MESSAGE, ERRMSG0, NIL);
	    }
	 signal(SIGALRM, SIG_DFL);
	 alarm(0);
	 return(0);
      }

/* 
 *  fb_s_unlock - simple unlock of a file.
 */
 
   fb_s_unlock(fd, fname)
      int fd;
      char *fname;
      
      {
	 if (flock(fd, LOCK_UN) < 0){
	    fb_xerror(FB_MESSAGE, ERRMSG1, NIL);
	    return;
	    }
      }

#endif /* end of LOCK tests to see which way to build locking */

/* else no locking. cannot build. */
