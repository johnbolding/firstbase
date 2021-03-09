/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: onintr.c,v 9.1 2001/01/22 17:36:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Onintr_sid[] = "@(#) $Id: onintr.c,v 9.1 2001/01/22 17:36:57 john Exp $";
#endif

static char *MSG_INT = "\nInterrupt\n";

#include <fb.h>
#include <fb_ext.h>
#include <sys/types.h>
#include <signal.h>

extern short int cdb_lockdaemon;
extern short int cdb_use_rpc;
extern short int cdb_usrlog;

/*
 * interrupt exit. general use.
 */

   RETSIGTYPE fb_onintr(disp)
      int disp;

      {
#if RPC
         int pid;
#endif /* RPC */

         /* first things first, ignore new interrupts */
         signal(SIGINT, SIG_IGN);
         fb_refresh();
         fb_settty(0);
	 fprintf(stderr, MSG_INT);
	 fflush(stderr);
         if (cdb_usrlog > 0)
            fb_usrlog_end();
#if RPC
         if (cdb_lockdaemon){
            pid = getpid();
            fb_fcntl_cl_clnt(NIL, 0L, 0L, pid, 0);
            }
         if (cdb_use_rpc)
            fb_clnt_destroy();
#endif /* RPC */
	 exit(2);
      }
