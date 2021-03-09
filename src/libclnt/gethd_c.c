/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: gethd_c.c,v 9.0 2001/01/09 02:56:35 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Gethead_clnt_sid[] = "@(#) $Id: gethd_c.c,v 9.0 2001/01/09 02:56:35 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

   fb_gethead_clnt(hp)
      register fb_database *hp;

      {
         static fb_varvec v, *r;
         char sid[10];
         int st = FB_AOK;

         /*
          * arguments to gethead_svc are:
          *   r_gethead SID
          */
         sprintf(sid, "%d", hp->b_sid);
         fb_loadvec(&v, R_GETHEAD, sid, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "gethead - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from gethead_svc are:
             *    - nargs
             *    - status
             *    - reccnt
             *    - delcnt
             */
            st = atoi(fb_argvec(r, 1));
            if (st == FB_AOK){
               hp->reccnt = atol(fb_argvec(r, 2));
               hp->delcnt = atol(fb_argvec(r, 3));
               }
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

#endif /* RPC */
