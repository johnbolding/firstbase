/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: useidx_c.c,v 9.0 2001/01/09 02:56:37 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char FB_useidx_sid[] = "@(#) $Id: useidx_c.c,v 9.0 2001/01/09 02:56:37 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

   fb_useidx_clnt(n, hp)
      int n;
      register fb_database *hp;

      {
         static fb_varvec v, *r;
         char sid[10], buf[10];
         int st = FB_AOK;

         hp->b_curauto = -1;
         /*
          * arguments to useidx_svc are:
          *   r_useidx SID n
          */
         sprintf(sid, "%d", hp->b_sid);
         sprintf(buf, "%d", n);
         fb_loadvec(&v, R_USEIDX, sid, buf, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "useidx_clnt - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from useidx are:
             *    - nargs
             *    - status
             */
            st = atoi(fb_argvec(r, 1));
            if (st == FB_AOK){
               hp->b_tree = 1;
               hp->b_curauto = n;
               }
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

#endif /* RPC */
