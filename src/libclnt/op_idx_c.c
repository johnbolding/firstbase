/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: op_idx_c.c,v 9.0 2001/01/09 02:56:36 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char FB_openidx_idx[] = "@(#)op_idx_c.c	1.1 8/19/93 FB";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

   fb_openidx_clnt(iname, hp)
      char *iname;
      fb_database *hp;

      {
         static fb_varvec v, *r;
         char sid[10];
         int st = FB_AOK;

         /*
          * arguments to createidx_svc are:
          *   r_createidx SID iname
          */
         sprintf(sid, "%d", hp->b_sid);
         fb_loadvec(&v, R_OPENIDX, sid, iname, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "openidx_clnt - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from openidx are:
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
