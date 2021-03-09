/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbexit.c,v 9.0 2001/01/09 02:56:25 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbexit_sid[] = "@(#) $Id: dbexit.c,v 9.0 2001/01/09 02:56:25 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern char *cdb_lserver;
extern char *cdb_pgm;
extern short int cdb_lockdaemon;
extern short int cdb_use_rpc;
extern short int cdb_usrlog;
extern short int cdb_license;
extern short int cdb_free_all_memory;

/* 
 *  dbexit - exit after correcting terminal mode 
 */
 
   fb_exit(status)
      int status;
      
      {
         int pid;

         fb_settty(0);	/* FB_ENDMODE */
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
         if (cdb_free_all_memory){
            fb_setup_exit();
            fb_free_globals(); 
            }
         exit(status);
      }
