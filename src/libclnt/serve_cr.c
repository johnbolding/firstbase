/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: serve_cr.c,v 9.0 2001/01/09 02:56:36 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Clnt_create_sid[] = "@(#) $Id: serve_cr.c,v 9.0 2001/01/09 02:56:36 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>
#include <sys/time.h>

extern short int cdb_locktime;
extern char *cdb_server;
extern short int cdb_rpc_retries;
static CLIENT *fb_cl = NULL;

/*
 * clnt_create, toserver, clnt_destroy. clnt_timeout -
 *	these define the lower level of the communications with the server.
 */

   fb_clnt_create()

      {
         int st = FB_AOK, retry;
         u_long pnum;

         if (cdb_server == NULL){
            fb_serror(FB_MESSAGE,
               "FirstBase variable `SERVER' not set.", NIL);
            return(FB_ERROR);
            }
#ifdef STANDALONE_DEBUG
         fb_cl = 1;
         return(FB_AOK);
#endif
         for (retry = 0;; retry++){
            fb_cl = clnt_create(cdb_server, FBSERVER_PROG,
               FBSERVER_VERSION, "tcp");
            if (fb_cl != NULL || retry > cdb_rpc_retries)
               break;
            }
         if (fb_cl == NULL){
            fb_serror(FB_MESSAGE,
               "Could not initialize Server connection", cdb_server);
            return(FB_ERROR);
            }

         /* now request a server to be created for this process */
         st = fb_mkserver_clnt(&pnum);
         sleep(1);
         if (st == FB_AOK){
            /* destroy the link to the fbserver (fbinit) */
            clnt_destroy(fb_cl);
            fb_cl = NULL;
            /* now use pnum to make a new server connection */
            for (retry = 0;; retry++){
               fb_cl = clnt_create(cdb_server, pnum, 1, "tcp");
               if (fb_cl != NULL || retry > cdb_rpc_retries)
                  break;
               sleep(1);
               }
            if (fb_cl == NULL){
               fb_serror(FB_MESSAGE,
                  "Could not create the Client/Server connection", cdb_server);
               return(FB_ERROR);
               }
            else
               fb_clnt_timeout(cdb_locktime + 10);
            }
         return(st);
      }

   fb_varvec *fb_toserver(vp)
      fb_varvec *vp;

      {
         fb_varvec *r;

         if (fb_cl == NULL)
            fb_xerror(FB_ABORT_ERROR,
               "Can't call server() without a client connection", NIL);
         r = fb_server_1(vp, fb_cl);
         if (r == NULL){
            fb_lerror(FB_MESSAGE, "fbserver message failed", vp->v_data);
            clnt_perror(fb_cl, cdb_server);
            return(0);
            }
         return(r);
      }

   fb_clnt_destroy()
      {
         if (fb_cl == NULL)
            return(FB_ERROR);
         fb_stopserver_clnt();
         clnt_destroy(fb_cl);
         fb_cl = NULL;
         return(FB_AOK);
      }

   fb_clnt_timeout(seconds)
      int seconds;

      {
         struct timeval t;

         t.tv_sec = (long) seconds;
         t.tv_usec = 0;
         clnt_control(fb_cl, CLSET_TIMEOUT, (char *) &t);
      }

   fb_clnt_ping()
      {
         if (fb_cl == NULL)
            return(FB_ERROR);
         else
            return(FB_AOK);
      }

#endif /* RPC */
