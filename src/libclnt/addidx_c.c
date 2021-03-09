/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: addidx_c.c,v 9.0 2001/01/09 02:56:32 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char FB_addidx_idx[] = "@(#)addidx_c.c	1.1 8/19/93 FB";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

/*
 * fb_addidx_clnt - client version of fb_addidx
 */

   fb_addidx_clnt(iname, hp)
      char *iname;
      fb_database *hp;

      {
         static fb_varvec v, *r;
         char sid[10];
         int st = FB_AOK;

         /*
          * arguments to createidx_svc are:
          *   r_addidx SID iname
          */
         sprintf(sid, "%d", hp->b_sid);
         fb_loadvec(&v, R_ADDIDX, sid, iname, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "addidx_clnt - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from addidx are:
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

/*
 * fb_subidx_clnt - client version of fb_addidx
 */

   fb_subidx_clnt(iname, hp)
      char *iname;
      fb_database *hp;

      {
         static fb_varvec v, *r;
         char sid[10];
         int st = FB_AOK;

         /*
          * arguments to createidx_svc are:
          *   r_subidx SID iname
          */
         sprintf(sid, "%d", hp->b_sid);
         fb_loadvec(&v, R_SUBIDX, sid, iname, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "subidx_clnt - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from subidx are:
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
