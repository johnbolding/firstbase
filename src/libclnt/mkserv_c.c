/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mkserv_c.c,v 9.0 2001/01/09 02:56:36 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Mkserver_clnt_sid[] = "@(#) $Id: mkserv_c.c,v 9.0 2001/01/09 02:56:36 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#if RPC

#include <fbserver.h>

/*
 * fb_mkserver_clnt - make a server on the other machine just for this pid
 */

   fb_mkserver_clnt(pnum)
      u_long *pnum;

      {
         static fb_varvec v, *r;
         char pid[10], *buf, hostname[FB_MAXNAME];
         int st = FB_AOK;

         /*
          * arguments to addrec_svc are:
          *   r_mkserver hostname PID
          */
         gethostname(hostname, FB_MAXNAME);
         sprintf(pid, "%d", (int) getpid());
         fb_loadvec(&v, R_MAKESERVER, hostname, pid, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "mkserver - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from mkserver_svc are:
             *    - nargs
             *    - status
             *    - tmp_prog - an unsigned long for the server
             */
            st = atoi(fb_argvec(r, 1));
            buf = fb_argvec(r, 2);
            sscanf(buf, "%lu", pnum);
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

   fb_stopserver_clnt()
      {
         static fb_varvec v, *r;
         int st = FB_AOK;

         /*
          * arguments to stopserver_clnt are:
          *   R_STOPSERVER
          */
         fb_loadvec(&v, R_STOPSERVER, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "stopserver - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from stopserver_svc are:
             *    - nargs
             *    - status
             */
            st = atoi(fb_argvec(r, 1));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

#endif /* RPC */
