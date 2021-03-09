/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: lock.c,v 9.2 2001/02/16 19:34:19 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Lock5_sid[] = "@(#) $Id: lock.c,v 9.2 2001/02/16 19:34:19 john Exp $";
#endif

/*
 *  lock.c - FCNTL module - for use with fcntl() routine.
 *	This locking mechanism uses getitimer()/setitimer() and properly
 *	restores any existing (pending) locks and signal handlers.
 */

#include <fb.h>
#include <fb_ext.h>

#if HAVE_FCNTL && HAVE_GETITIMER

#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <setjmp.h>

static jmp_buf jmp_env;
static char bufrec[FB_MAXLINE];
static long locksize;
static int ignore = 0;
static char *ERRMSG1 = "Error attempting to unlock.";
static char *ERRMSG2 = "Lock timed out/not granted.";
static char *WARNING = "Warning - Locked Record is being skipped: %ld";
static struct flock fk;
extern short int cdb_lockdaemon;
extern short int cdb_locklevel;
extern short int cdb_locktime;
extern short int cdb_lockmessage;
extern short int cdb_use_rpc;

#if FB_PROTOTYPES
static RETSIGTYPE lock5_sigalrm(int disp);
#else
static RETSIGTYPE lock5_sigalrm();
#endif /* FB_PROTOTYPES */

/* 
 *  initlock - initialize lock procedures, ie merely check on ./lock 
 */
 
   void fb_initlock(ig, db)
      int ig;
      fb_database *db;
      
      {
         (void) db;

	 if (ig == 1){	/* ie, if scanner == 1, then silently ignore */
	    ignore = 1;
	    return;
	    }
	 if (cdb_batchmode && cdb_lockmessage == 2)
	    cdb_lockmessage = 1;
	 locksize = FB_MAPREC_SIZE;
      }

/* 
 *  lock - lock a record.  
 *     rec 0 means header, others are physical locations.
 *     if fwait is 1, then wait for clearance.
 *     if fwait is 0, do not wait. period.
 */
 
   fb_lock(mrec, db, fwait)
      long mrec;
      fb_database *db;
      int fwait;
   
      {
	 register int f;
	 int func;

#if RPC
         if (cdb_use_rpc)
            return(fb_lock_clnt(mrec, db, fwait));
#endif /* RPC */
         /* perhaps ignore locking attempts */
	 if (ignore == 1 || mrec < 0L || cdb_locklevel <= 0)
	    return(FB_AOK);
	 if (fwait){
            fb_set_alarm(cdb_locktime, lock5_sigalrm);
            /*
             * setjmp saves the current location to jump back to if needed.
             * a return value of 1 means return from longjmp, meaning
             * the timer expired -- iff the local signal handlers are executed.
             */
            if (setjmp(jmp_env) == 1){
               fb_jump_alarm();
               fb_lerror(FB_ABORT_ERROR, ERRMSG2, NIL);
               return(FB_ERROR);
               }
	    func = F_SETLKW;
	    }
	 else
	    func = F_SETLK;
	 fk.l_type = F_WRLCK;
	 fk.l_whence = 1;
         fk.l_start = mrec * (long) locksize;
         fk.l_len = locksize;
	 if (lseek(db->mfd, fk.l_start, 0) < 0L){
            if (fwait)
               fb_restore_alarm();
	    fb_serror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], "LL");
	    return(FB_ERROR);
	    }
         /*
          * this does not really loop:
          *    when fwait is 1, fcntl hangs
          *    when fwait is 0, it will only loop on user request
          *
          *    well, almost. fb_fcntl_clnt() does loop forever.
          *    its panic out is the alarm/longjmp done here.
          *    OR, it can return 
          */
	 for (;;){
            if (cdb_lockdaemon)
#if RPC
	       f = fb_fcntl_clnt(db->dmap, func, &fk);
#else
               f = -1;
#endif
            else
	       f = fcntl(db->mfd, func, &fk);
	    if (f != -1){
               if (fwait)
                  fb_restore_alarm();
	       return(FB_AOK);
	       }
	    if (fwait == 0){
	       sprintf(bufrec, WARNING, mrec);
	       switch(cdb_lockmessage){
	          case 0:			/* no messages */
		     break;
		  case 1:			/* simple fb_serror */
		     fb_serror(FB_MESSAGE, bufrec, NIL);
		     break;
		  case 2:			/* full fledged response */
		     if (fb_mustbe(CHAR_n, SYSMSG[S_LOCKED_RETRY],
                           cdb_t_lines, 1)== FB_AOK)
			break;
		     fb_fmessage(SYSMSG[S_RETRY]);
		     fflush(stdout);
		     sleep(5);
		     continue;
		     break;
		  }
	       }
            break;
	    }
         if (fwait){
            /* if here and fwait == 1 --- probably an FB_RPC_ERROR */
            fb_restore_alarm();
            }
	 return(FB_ERROR);
      }

/* 
 *  unlock record 
 */
 
   fb_unlock(mrec, db)
      long mrec;
      fb_database *db;
      
      {
         int st;

#if RPC
         if (cdb_use_rpc)
            return(fb_unlock_clnt(mrec, db));
#endif /* RPC */
         /* ignore locking attempts */
	 if (ignore == 1 || mrec < 0L || cdb_locklevel <= 0)
	    return(FB_AOK);
	 fk.l_type = F_UNLCK;
	 fk.l_whence = 1;
         fk.l_start = mrec * (long) locksize;
         fk.l_len = locksize;
	 if (lseek(db->mfd, fk.l_start, 0) < 0L){
	    fb_serror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], "LU");
            return(FB_ERROR);
	    }
         if (cdb_lockdaemon)
#if RPC
            st = fb_fcntl_clnt(db->dmap, F_SETLK, &fk);
#else
            st = -1;
#endif
         else
            st = fcntl(db->mfd, F_SETLK, &fk);
	 if (st < 0){
	    fb_serror(FB_ABORT_ERROR, ERRMSG1, "(fcntl)");
            return(FB_ERROR);
            }
         return(FB_AOK);
      }

   static RETSIGTYPE lock5_sigalrm(disp)
      int disp;

      {
         (void) disp;
	 longjmp(jmp_env, 1);
      }

#elif HAVE_FCNTL && !GETITIMER

#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <setjmp.h>

static RETSIGTYPE cdb_sigalrm();

static jmp_buf jmp_env;
static char bufrec[FB_MAXLINE];
static long locksize;
static int ignore = 0;
static char *ERRMSG1 = "Error attempting to unlock.";
static char *ERRMSG2 = "Lock timed out/not granted.";
static char *WARNING = "Warning - Locked Record is being skipped: %ld";
static struct flock fk;
extern short int cdb_lockdaemon;
extern short int cdb_use_rpc;
extern short int cdb_lockmessage;
extern short int cdb_locklevel;
extern short int cdb_locktime;

/* 
 *  initlock - initialize lock procedures, ie merely check on ./lock 
 */
 
   fb_initlock(ig, db)
      int ig;
      fb_database *db;
      
      {
	 if (ig == 1){	/* ie, if scanner == 1, then silently ignore */
	    ignore = 1;
	    return;
	    }
	 if (cdb_batchmode && cdb_lockmessage == 2)
	    cdb_lockmessage = 1;
	 locksize = FB_MAPREC_SIZE;
      }

/* 
 *  lock - lock a record.  
 *     rec 0 means header, others are physical locations.
 *     if fwait is 1, then wait for clearance.
 *     if fwait is 0, do not wait. period.
 */
 
   fb_lock(mrec, db, fwait)
      long mrec;
      fb_database *db;
      int fwait;
   
      {
	 register int f;
	 int func;

#if RPC
         if (cdb_use_rpc)
            return(fb_lock_clnt(mrec, db, fwait));
#endif /* RPC */
         /* perhaps ignore locking attempts */
	 if (ignore == 1 || mrec < 0L || cdb_locklevel <= 0)
	    return(FB_AOK);
	 if (fwait){
            if (setjmp(jmp_env) == 1){
               /* value of 1 means return from longjmp */
               alarm(0);
               signal(SIGALRM, SIG_DFL);
               fb_lerror(FB_ABORT_ERROR, ERRMSG2, NIL);
               return(FB_ERROR);
               }
            signal(SIGALRM, cdb_sigalrm);
            alarm(cdb_locktime);
	    func = F_SETLKW;
	    }
	 else
	    func = F_SETLK;
	 fk.l_type = F_WRLCK;
	 fk.l_whence = 1;
         fk.l_start = mrec * (long) locksize;
         fk.l_len = locksize;
	 if (lseek(db->mfd, fk.l_start, 0) < 0L){
            if (fwait){
               alarm(0);
               signal(SIGALRM, SIG_DFL);
               }
	    fb_serror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], "LL");
	    return(FB_ERROR);
	    }
         /*
          * this does not really loop:
          *    when fwait is 1, fcntl hangs
          *    when fwait is 0, it will only loop on user request
          *
          *    well, almost. fb_fcntl_clnt() does loop forever.
          *    its panic out is the alarm/longjmp done here.
          *    OR, it can return 
          */
	 for (;;){
            if (cdb_lockdaemon)
#if RPC
	       f = fb_fcntl_clnt(db->dmap, func, &fk);
#else
               f = -1;
#endif
            else
	       f = fcntl(db->mfd, func, &fk);
	    if (f != -1){
               if (fwait){
	          alarm(0);
	          signal(SIGALRM, SIG_DFL);
                  }
	       return(FB_AOK);
	       }
	    if (fwait == 0){
	       sprintf(bufrec, WARNING, mrec);
	       switch(cdb_lockmessage){
	          case 0:			/* no messages */
		     break;
		  case 1:			/* simple fb_serror */
		     fb_serror(FB_MESSAGE, bufrec, NIL);
		     break;
		  case 2:			/* full fledged response */
		     if (fb_mustbe(CHAR_n, SYSMSG[S_LOCKED_RETRY],
                           cdb_t_lines,1) == FB_AOK)
			break;
		     fb_fmessage(SYSMSG[S_RETRY]);
		     fflush(stdout);
		     sleep(5);
		     continue;
		     break;
		  }
	       }
            break;
	    }
         if (fwait){
            /* if here and fwait == 1 --- probably an FB_RPC_ERROR */
            alarm(0);
            signal(SIGALRM, SIG_DFL);
            }
	 return(FB_ERROR);
      }

/* 
 *  unlock record 
 */
 
   fb_unlock(mrec, db)
      long mrec;
      fb_database *db;
      
      {
         int st;

#if RPC
         if (cdb_use_rpc)
            return(fb_unlock_clnt(mrec, db));
#endif /* RPC */
         /* ignore locking attempts */
	 if (ignore == 1 || mrec < 0L || cdb_locklevel <= 0)
	    return(FB_AOK);
	 fk.l_type = F_UNLCK;
	 fk.l_whence = 1;
         fk.l_start = mrec * (long) locksize;
         fk.l_len = locksize;
	 if (lseek(db->mfd, fk.l_start, 0) < 0L){
	    fb_serror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], "LU");
            return(FB_ERROR);
	    }
         if (cdb_lockdaemon)
#if RPC
            st = fb_fcntl_clnt(db->dmap, F_SETLK, &fk);
#else
            st = -1;
#endif
         else
            st = fcntl(db->mfd, F_SETLK, &fk);
	 if (st < 0){
	    fb_serror(FB_ABORT_ERROR, ERRMSG1, "(fcntl)");
            return(FB_ERROR);
            }
         return(FB_AOK);
      }

   static RETSIGTYPE cdb_sigalrm()
      {
	 longjmp(jmp_env, 1);
      }

#elif LOCKF

#include <fb_ext.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>

static jmp_buf jmp_env;
static char bufrec[FB_MAXLINE];
static long locksize;
static int ignore = 0;
static char *ERRMSG = "Process timed out attempting blocking Lock.";
static char *ERRMSG1 = "Lock error attempting to unset blocking Lock.";
static char *ERRMSG2 = "Lock request timed out/not granted.";
static char *WARNING = "Warning - Locked Record is being skipped: %ld";

extern short int cdb_lockmessage;
extern short int cdb_locklevel;
extern short int cdb_locktime;

#if FB_PROTOTYPES
static void RETSIGTYPE lock3_sigalrm(int disp);
#else
static void RETSIGTYPE lock3_sigalrm();
#endif /* FB_PROTOTYPES */

/* 
 *  initlock - initialize lock procedures, ie merely check on ./lock 
 */
 
   fb_initlock(ig, db)
      int ig;
      fb_database *db;
      
      {
	 if (ig == 1){	/* ie, if scanner == 1, then silently ignore */
	    ignore = 1;
	    return;
	    }
	 if (cdb_batchmode && cdb_lockmessage == 2)
	    cdb_lockmessage = 1;
	 locksize = FB_MAPREC_SIZE;
       }

/* 
 *  lock - lock a record.  
 *     rec 0 means header, others are physical locations.
 *     if fwait is 1, then wait for clearance.
 *     if fwait is 0, do not wait. period.
 */
 
   fb_lock(mrec, db, fwait)
      long mrec;
      fb_database *db;
      int fwait;
   
      {
	 register int f;
	 long pos;
	 int func;
   
         /* perhaps ignore locking attempts */
	 if (ignore == 1 || mrec < 0L || cdb_locklevel <= 0)
	    return(FB_AOK);
	 if (fwait){
	    if (setjmp(jmp_env) == 1){
               /* value of 1 means return from longjmp */
	       alarm(0);
	       signal(SIGALRM, SIG_DFL);
               fb_lerror(FB_ABORT_ERROR, ERRMSG2, NIL);
	       return(FB_ERROR);
               }
	    signal(SIGALRM, lock3_sigalrm);
	    alarm(cdb_locktime);
	    func = F_LOCK;
	    }
	 else
	    func = F_TLOCK;
         pos = mrec * (long) locksize;
	 if (lseek(db->mfd, pos, 0) < 0L){
	    alarm(0);
	    signal(SIGALRM, SIG_DFL);
	    fb_serror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], "LL");
	    return(FB_ERROR);
	    }
	 for (;;) {
	    f = lockf(db->mfd, func, locksize);
	    if (f == 0){
	       alarm(0);
	       signal(SIGALRM, SIG_DFL);
	       return(FB_AOK);
	       }
	    if (fwait == 0){
	       sprintf(bufrec, WARNING, mrec);
	       switch(cdb_lockmessage){
	          case 0:			/* no messages */
		     break;
		  case 1:			/* simple fb_serror */
		     fb_serror(FB_MESSAGE, bufrec, NIL);
		     break;
		  case 2:			/* full fledged response */
		     if (fb_mustbe(CHAR_n, SYSMSG[S_LOCKED_RETRY],
                           cdb_t_lines,1) == FB_AOK)
			break;
		     fb_fmessage(SYSMSG[S_RETRY]);
		     sleep(5);
		     continue;
		     break;
		  }
	       return(FB_ERROR);
	       }
	    }
      }

/* 
 *  unlock record 
 */
 
   fb_unlock(mrec, db)
      long mrec;
      fb_database *db;
      
      {
	 long pos;

	 if (ignore == 1 || mrec < 0L)	/* ignore locking attempts */
	    return(0);
         pos = mrec * (long) locksize;
	 if (lseek(db->mfd, pos, 0) < 0L){
	    fb_xerror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], "LU");
	    return(FB_ERROR);
	    }
	 if (lockf(db->mfd, F_ULOCK, locksize) < 0)
	    fb_xerror(FB_MESSAGE, ERRMSG1, NIL);
         return(FB_AOK);
      }

   static RETSIGTYPE lock3_sigalrm(disp)
      int disp;

      {
         (void) disp;
	 longjmp(jmp_env, 1);
      }

#elif FLOCK

/*
 *  lock.c - FLOCK module - for use with flock() routine.
 *	also - since flock is really untested, use flock only
 *	when the locklevel is 2. otherwise, use standard file lock.
 */
 
#include <fb_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/file.h>

static char curlock[FB_MAXLINE];		/* Last used name of lock */
static char lockdir[FB_MAXLINE];		/* for lock directory */
static char bufrec[FB_MAXLINE];
static int ignore = 0;
static char *ERRMSG = "Process timed out attempting blocking Lock.";
static char *ERRMSG0 = "Lock error attempting to set blocking Lock.";
static char *ERRMSG1 = "Lock error attempting to unset blocking Lock.";
static char *WARNING = "Warning - Locked Record is being skipped: %ld";

static RETSIGTYPE sigalrm()	{ fb_xerror(FB_MESSAGE, ERRMSG, NIL); }

/* 
 *  initlock - initialize lock procedures, ie merely check on ./lock 
 */
 
   fb_initlock(ig, db)
      int ig;
      fb_database *db;
      
      {
         char fname[FB_MAXLINE], dname[FB_MAXNAME];
	 
	 if (ig == 1){	/* ie, if scanner == 1, then silently ignore */
	    ignore = 1;
	    return;
	    }
         fb_dirname(dname, db->dbase);	/* leaves trailing slash, or empty */
	 sprintf(lockdir, SYSMSG[S_LOCK_FMT1], dname);
         if (access(lockdir, 0) != 0){
	    fb_serror(FB_CANT_OPEN, lockdir, NIL);
	    ignore = 1;				/* ignore lock attempts */
	    }
	 else{
	    ignore = 0;				/* no ignore lock attempts */
            fb_basename(fname, db->dbase);		/* for later lock calls */
	    strcat(lockdir, fname);
	    }
	 if (cdb_batchmode)
	    cdb_lockmessage = 1;
       }

/* 
 *  cdb_lock - lock a record.  
 *     rec 0 means header, others are physical locations.
 *     if fwait is 1, then wait for clearance.
 *     if fwait is 0, do not wait. period.
 */
 
   fb_lock(mrec, db, fwait)
      long mrec;
      fb_database *db;
      int fwait;
   
      {
	 register int f;
   
	 if (ignore == 1 || mrec < 0L)	/* ignore locking attempts */
	    return(0);
	 if (cdb_locklevel == 2 && mrec == 0L){
	    if (fwait){
	       signal(SIGALRM, sigalrm);
	       alarm(cdb_locktime);
	       }
	    if (flock(db->mfd, LOCK_EX) < 0){
	       alarm(0);
	       signal(SIGALRM, SIG_DFL);
	       fb_xerror(FB_MESSAGE, ERRMSG0, NIL);
	       }
	    alarm(0);
	    signal(SIGALRM, SIG_DFL);
	    return(0);
	    }
	 sprintf(curlock, SYSMSG[S_LOCK_FMT2], lockdir, mrec);
	 if (fwait){
	    signal(SIGALRM, sigalrm);
	    alarm(cdb_locktime);
	    }
	 for (;;) {
	    f = lock1();
	    if (f == 0){
	       alarm(0);
	       signal(SIGALRM, SIG_DFL);
	       return(0);
	       }
	    if (fwait == 0){
	       sprintf(bufrec, WARNING, mrec);
	       switch(cdb_lockmessage){
	          case 0:			/* no messages */
		     break;
		  case 1:			/* simple fb_serror */
		     fb_serror(FB_MESSAGE, bufrec, NIL);
		     break;
		  case 2:			/* full fledged response */
		     if (fb_mustbe(CHAR_n,SYSMSG[S_LOCKED_RETRY],fb_t_lines,1)==FB_AOK)
			break;
		     fb_fmessage(SYSMSG[S_RETRY]);
		     sleep(5);
		     continue;
		     break;
		  }
	       return(FB_ERROR);
	       }
	    }
      }

/* 
 *  cdb_unlock record 
 */
 
   fb_unlock(mrec, db)
      long mrec;
      fb_database *db;
      
      {
	 if (ignore == 1 || mrec < 0L)	/* ignore locking attempts */
	    return;
	 if (cdb_locklevel == 2 && mrec == 0L){
	    if (flock(db->mfd, LOCK_UN) < 0)
	       fb_xerror(FB_MESSAGE, ERRMSG1, NIL);
	    return;
	    }
         sprintf(curlock, SYSMSG[S_LOCK_FMT2], lockdir, mrec);
	 unlink(curlock);
      }

/* 
 *  lock1 - attempt to set a record lock via temp file. fail=-1, sucess=0 
 */
 
   static lock1()

      {
	 register int fd;
   
	 fd = open(curlock, O_CREAT | O_EXCL, 0);
	 if (fd < 0)
	    return(FB_ERROR);
	 close(fd);
	 return(0);
      }
      
#else /* if FILELOCK */

/*
 *  lock1.c - FILELOCK module -- for use when no FLOCK or LOCK is available.
 */

static char curlock[FB_MAXLINE];		/* Last used name of lock */
static char lockdir[FB_MAXLINE];		/* for lock directory */
static int ignore = 0;
static char *ERRMSG = "Process timed out attempting blocking lock.";

/* 
 *  initlock - initialize lock procedures, ie merely check on ./lock 
 */
 
   fb_initlock(ig, dbase)
      int ig;
      char *dbase;
      
      {
         char fname[FB_MAXLINE], dname[FB_MAXNAME];
	 
	 if (ig == 1){	/* ie, if scanner == 1, then silently ignore */
	    ignore = 1;
	    return;
	    }
         fb_dirname(dname, dbase);		/* trailing slash, or empty */
	 sprintf(lockdir, SYSMSG[S_LOCK_FMT1], dname);
         if (access(lockdir, 0) != 0){
	    fb_serror(FB_CANT_OPEN, lockdir, NIL);
	    ignore = 1;				/* ignore lock attempts */
	    }
	 else{
	    ignore = 0;				/* no ignore lock attempts */
            fb_basename(fname, dbase);		/* for later lock calls */
	    strcat(lockdir, fname);
	    }
       }

/* 
 *  cdb_lock - lock a record.  
 *     rec 0 means header, others are physical locations.
 *     if fwait is 1, then wait for clearance.
 *     if fwait is 0, do not wait. period.
 */
 
   fb_lock(mrec, db, fwait)
      long mrec;
      fb_database *db;
      int fwait;
   
      {
	 register int f;
	 long tries = 0;
   
	 if (ignore == 1 || mrec < 0L)	/* ignore locking attempts */
	    return(0);
	 sprintf(curlock, SYSMSG[S_LOCK_FMT2], lockdir, mrec);
	 for (;;) {
	    f = lock1();
	    if (f == 0)
	       return(0);
	    if (fwait == 0){
	       if (fb_mustbe(CHAR_n, SYSMSG[S_LOCKED_RETRY],
                     cdb_t_lines, 1) == FB_AOK)
		  return(FB_ERROR);
	       fb_fmessage(SYSMSG[S_RETRY]);
	       sleep(5);
	       continue;
	       }
	    }
      }

/* 
 *  unlock record 
 */
 
   fb_unlock(mrec, db)
      long mrec;
      fb_database *db;
      
      {
	 if (ignore == 1 || mrec < 0L)	/* ignore locking attempts */
	    return;
         sprintf(curlock, SYSMSG[S_LOCK_FMT2], lockdir, mrec);
	 unlink(curlock);
      }

/* 
 *  lock1 - attempt to set a record lock via temp file. fail=-1, sucess=0 
 */
 
   static lock1()

      {
	 register int fd;
   
	 fd = open(curlock, O_CREAT | O_EXCL, 0);
	 if (fd < 0)
	    return(FB_ERROR);
	 close(fd);
	 return(0);
      }

#endif /* FILELOCK */
