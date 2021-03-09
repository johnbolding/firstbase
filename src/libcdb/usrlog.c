/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: usrlog.c,v 9.1 2001/02/16 19:09:07 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Usrlog_sid[] = "@(#) $Id: usrlog.c,v 9.1 2001/02/16 19:09:07 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern char *cdb_logfile;
extern char *cdb_pgm;
extern char *cdb_user;
extern short int cdb_usrlog;

static char hname[30], *ttname, s_pid[30];
static int logfd = -1;
#if FB_PROTOTYPES
static void usrlog_stamp(char *buf, char *s);
static int usrlog_update(char *s);
#else /* FB_PROTOTYPES */
static void usrlog_stamp();
static int usrlog_update();
#endif /* FB_PROTOTYPES */

/*
 * usrlog - user logging routines.
 *	cdb_usrlog - USRLOG controlled. 1-10 real values.
 *	set to 99 for loads of values.
 */

   void fb_usrlog_begin(argc, argv)
      int argc;
      char *argv[];

      {
         char buf[500], *ttyname(int fildes);
         int i;

         if (cdb_usrlog <= 0 || logfd >= 0)
            return;
         logfd = open(cdb_logfile, READWRITE);
         if (logfd <= 0){
            close(creat(cdb_logfile, 0666));
            logfd = open(cdb_logfile, READWRITE);
            if (logfd <= 0)
               return;
            }
#if HAVE_GETHOSTNAME
         gethostname(hname, 30);
#else /* HAVE_GETHOSTNAME */
         hname[0] = NULL;
#endif
         ttname = ttyname(0);
         if (ttname == NULL){
            sprintf(s_pid, "PID=%d", getpid());
            ttname = s_pid;
            }
         usrlog_stamp(buf, "BT");
         if (cdb_usrlog >= 2){
            /* show arguments too */
            strcat(buf, "   [");
            for (i = 1; i < argc; i++)
               sprintf(buf, "%s %s", buf, argv[i]);
            strcat(buf, " ]\n");
            }
         usrlog_update(buf);
      }

   void fb_usrlog_end()
      {
         char buf[500];

         if (cdb_usrlog <= 0 || logfd <= 0)
            return;
         usrlog_stamp(buf, "ET");
         usrlog_update(buf);
         close(logfd);
         logfd = -1;
      }

   void fb_usrlog_msg(s)
      char *s;

      {
         char buf[500];

         if (cdb_usrlog <= 0)
            return;
         usrlog_stamp(buf, "MS");
         sprintf(buf, "%s   [ %s ]\n", buf, s);
         usrlog_update(buf);
      }

   static void usrlog_stamp(buf, s)
      char *buf, *s;

      {
         char dt[30], tm[30], ft[30];

         fb_simpledate(dt, 1);
	 sprintf(ft, FB_FDATE, dt[0], dt[1], dt[2], dt[3], dt[4], dt[5]);
         fb_simpledate(tm, 2);
         sprintf(buf, "%s: %-10s %s %s %-10s %-10s %s\n", s, cdb_pgm, ft, tm,
            cdb_user, hname, ttname);
      }

/*
 * usrlog_update - lock file, write to file, unlock file
 */

   static int usrlog_update(s)
      char *s;

      {
         int n, st = FB_AOK;

         if (cdb_usrlog <= 0)
            return(st);
         n = strlen(s);
         fb_s_lock(logfd, FB_WAIT, cdb_logfile);
         if (lseek(logfd, 0L, 2) < 0)
            st = FB_ERROR;
         if (st == FB_AOK)
           if (write(logfd, s, (unsigned) n) != n)
              st = FB_ERROR;
         fb_s_unlock(logfd, cdb_logfile);
	 fb_sync_fd(logfd);
         if (st == FB_ERROR)
            fb_serror(FB_MESSAGE, "could not update usrlog as requested.",NIL);
         return(st);
      }
