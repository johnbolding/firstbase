/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getxre_c.c,v 9.0 2001/01/09 02:56:35 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getxrec_sid_clnt[] = "@(#)gtxrec_c.c	8.2 9/6/93 FB";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

   fb_getxrec_clnt(key, hp)
      char *key;
      register fb_database *hp;

      {
         static fb_varvec v, *r;
         char sid[10];
         int st = FB_AOK;

         /*
          * arguments to getxrec_svc are:
          *    r_getxrec SID key
          */
         sprintf(sid, "%d", hp->b_sid);
         fb_loadvec(&v, R_GETXREC, sid, key, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "getxrec - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from getxrec are:
             *    - nargs
             *    - status
             *    - recno
             */
            st = atoi(fb_argvec(r, 1));
            hp->rec = atol(fb_argvec(r, 2));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

#endif /* RPC */
