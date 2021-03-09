/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: putrec_c.c,v 9.0 2001/01/09 02:56:36 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Putrec_clnt_sid[] = "@(#) $Id: putrec_c.c,v 9.0 2001/01/09 02:56:36 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

/*
 * fb_putrec_clnt - r_putrec mechanism - rpc version.
 */

   fb_putrec_clnt(n, hp)
      long n;
      register fb_database *hp;

      {
         static fb_varvec v, *r;
         char buf[10], sid[10];
         int st = FB_AOK;

         /*
          * arguments to putrec_svc are:
          *   r_putrec SID recno
          */
         sprintf(sid, "%d", hp->b_sid);
         sprintf(buf, "%ld", n);
         fb_loadvec(&v, R_PUTREC, sid, buf, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "putrec - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from getrec are:
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
