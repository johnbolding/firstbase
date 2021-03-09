/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: errlog.c,v 9.0 2001/01/09 02:56:25 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Errlog_sid[] = "@(#) $Id: errlog.c,v 9.0 2001/01/09 02:56:25 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern char *cdb_errorlog;
extern char *cdb_pgm;
extern char *cdb_user;

/*
 * fb_errorlog - error logging routines.
 *	cdb_errlog - ERRLOG controlled. ON/OFF
 */

   fb_errorlog(e, line)
      int e;
      char *line;

      {
         char buf[500], *ttyname(int filedes), *tty;
         char dt[30], tm[30], ft[30];
         int logfd, n, st = FB_AOK;
         char hname[30], ttname[30], pgm[FB_MAXNAME];

         if (cdb_errorlog == NULL)
            return(st);
         logfd = open(cdb_errorlog, READWRITE);
         if (logfd <= 0){
            close(creat(cdb_errorlog, 0666));
            logfd = open(cdb_errorlog, READWRITE);
            if (logfd <= 0)
               return(FB_ERROR);
            }

#if HAVE_GETHOSTNAME
         gethostname(hname, 30);
#else
         hname[0] = NULL;
#endif
         ttname[0] = NULL;
         tty = ttyname(0);
         if (tty != NULL)
            strcpy(ttname, tty);
         else
            strcpy(ttname, "unknown");

         fb_simpledate(dt, 1);
	 sprintf(ft, FB_FDATE, dt[0], dt[1], dt[2], dt[3], dt[4], dt[5]);
         fb_simpledate(tm, 2);
         buf[0] = NULL;
         sprintf(buf, "%serror #:   %d\n", buf, e);
         if (cdb_pgm != NULL)
            strcpy(pgm, cdb_pgm);
         else
            pgm[0] = NULL;
         sprintf(buf, "%sprogram:   %s\n", buf, pgm);
         sprintf(buf, "%sdate:      %s %s\n", buf, ft, tm);
         sprintf(buf, "%suser:      %s\n", buf, cdb_user);
         sprintf(buf, "%shost:      %s\n", buf, hname);
         sprintf(buf, "%stty:       %s\n", buf, ttname);
         sprintf(buf, "%smessage:   %s\n\n", buf, line);
         n = strlen(buf);
         fb_s_lock(logfd, FB_WAIT, cdb_errorlog);
         if (lseek(logfd, 0L, 2) < 0)
            st = FB_ERROR;
         if (st == FB_AOK)
           if (write(logfd, buf, (unsigned) n) != n)
              st = FB_ERROR;
	 fb_sync_fd(logfd);
         fb_s_unlock(logfd, cdb_errorlog);
         close(logfd);
         if (st == FB_ERROR)
            fb_screrr("USRLOG: could not update log as requested.");
         return(st);
      }
