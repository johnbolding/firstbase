/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: lockd_cr.c,v 9.0 2001/01/09 02:56:35 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Lockd_create_sid[] = "@(#) $Id: lockd_cr.c,v 9.0 2001/01/09 02:56:35 john Exp $";
#endif

#include <fb.h>

#if RPC

#include <fb_ext.h>
#include <fbserver.h>

extern short int cdb_locklevel;
extern char *cdb_server;
extern int cdb_locking_pid;
static CLIENT *cdb_cl_lockd = NULL;

/*
 * fb_lockd_clnt_create - create the firstbase lockdaemon - NFS RPC calls
 */

   fb_lockd_clnt_create()

      {
         if (cdb_server == NULL){
            fb_serror(FB_MESSAGE,
               "FirstBase variable `FIRSTBASESERVER' not set.", NIL);
            return(FB_ERROR);
            }
         if (cdb_cl_lockd == NULL)
            cdb_cl_lockd =
               clnt_create(cdb_server, FBLOCKD_PROG, FBLOCKD_VERSION, "tcp");
         if (cdb_cl_lockd == NULL){
            cdb_locklevel = 0;
            fb_serror(FB_RPC_ERROR, "can't connect to fblockd:", cdb_server);
            return(FB_ERROR);
            }
         return(FB_AOK);
      }

/*
 * fb_tolockd - communicates with the lock daemon server
 */

   fb_varvec *fb_tolockd(vp)
      fb_varvec *vp;

      {
         fb_varvec *r;
         char buf[FB_MAXLINE], *p;
         int rpc_errors = 0;

         if (cdb_cl_lockd == NULL)
            return((fb_varvec *) NULL);
         for (;;){
            r = fblockd_1(vp, cdb_cl_lockd);
            if (r == NULL){
               p = clnt_sperror(cdb_cl_lockd, buf);
               fprintf(stderr, "\n%s\n", p);
               if (!cdb_batchmode){
                  fb_settty(FB_ENDMODE);
                  fprintf(stderr, "hit <RETURN> to retry lock daemon: ");
                  fflush(stderr);
                  fgets(buf, FB_MAXLINE-1, stdin);
                  fb_settty(FB_EDITMODE);
                  }
               fprintf(stderr, "fblockd: retrying connection ...\n");
               for (;;){
                  if (++rpc_errors > 3){
                     cdb_cl_lockd = NULL;
                     cdb_locklevel = -1;
                     fprintf(stderr,
                        "fblockd: connection failed! WARNING: No Locking.\n");
                     if (!cdb_batchmode){
                        fb_settty(FB_ENDMODE);
                        fprintf(stderr, "hit <RETURN> to continue: ");
                        fflush(stderr);
                        fgets(buf, FB_MAXLINE-1, stdin);
                        fb_settty(FB_EDITMODE);
                        }
                     return((fb_varvec *) NULL);
                     }
                  cdb_cl_lockd =
                     clnt_create(cdb_server, FBLOCKD_PROG,
                        FBLOCKD_VERSION, "tcp");
                  if (cdb_cl_lockd != NULL){
                     fprintf(stderr, "fblockd: reconnected!\n");
                     break;
                     }
                  }
               continue;
               }
            break;
            }
         return(r);
      }

   fb_lockd_clnt_destroy()
      {
         clnt_destroy(cdb_cl_lockd);
         cdb_cl_lockd = NULL;
         cdb_locking_pid = 0;
      }

#endif /* RPC */
