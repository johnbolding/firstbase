/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: tune_c.c,v 9.0 2001/01/09 02:56:37 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Tune_clnt_sid[] = "@(#) $Id: tune_c.c,v 9.0 2001/01/09 02:56:37 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

/*
 * fb_tune_clnt - used to tune the server with a global variable change.
 *	globals passed in are:
 *		cdb_loadfail
 */

   fb_tune_clnt(loadfail)
      int loadfail;

      {
         static fb_varvec v, *r;
         int st = FB_AOK;
         char buf[10];

         /*
          * arguments for status_svc
          *   R_TUNE loadfail
          */
         sprintf(buf, "%d", loadfail);
         fb_loadvec(&v, R_TUNE, buf, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "tune_clnt - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from tune:
             *    - status
             */
            st = atoi(fb_argvec(r, 0));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

#endif /* RPC */
