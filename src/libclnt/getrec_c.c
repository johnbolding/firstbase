/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getrec_c.c,v 9.0 2001/01/09 02:56:35 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getrec_clnt_sid[] = "@(#) $Id: getrec_c.c,v 9.0 2001/01/09 02:56:35 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

   fb_getrec_clnt(n, hp)
      long n;
      register fb_database *hp;

      {
         static fb_varvec v, *r;
         char sid[10], buf[10];
         int st = FB_AOK;

         /*
          * arguments to getrec_svc are:
          *   r_getrec SID recno
          */
         sprintf(sid, "%d", hp->b_sid);
         sprintf(buf, "%ld", n);
         fb_loadvec(&v, R_GETREC, sid, buf, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "getrec - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from getrec are:
             *    - nargs
             *    - status
             *    - recno
             */
            st = atoi(fb_argvec(r, 1));
            if (st == FB_AOK)
               hp->rec = atol(fb_argvec(r, 2));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

#endif /* RPC */
