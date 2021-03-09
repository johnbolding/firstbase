/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getire_c.c,v 9.0 2001/01/09 02:56:35 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getirec_sid[] = "@(#) $Id: getire_c.c,v 9.0 2001/01/09 02:56:35 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

   fb_getirec_clnt(n, hp)
      long n;
      register fb_database *hp;

      {
         static fb_varvec v, *r;
         char sid[10], buf[10];
         int st = FB_AOK;

         /*
          * arguments to getirec_svc are:
          *   r_getirec SID recno
          */
         sprintf(sid, "%d", hp->b_sid);
         sprintf(buf, "%ld", n);
         fb_loadvec(&v, R_GETIREC, sid, buf, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
         hp->rec = 0L;
#if 0
         fb_tracevec(r, "getirec - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from getrec are:
             *    - nargs
             *    - status
             */
            st = atoi(fb_argvec(r, 1));
            if (st == FB_AOK)
               hp->rec = n;
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

#endif /* RPC */
