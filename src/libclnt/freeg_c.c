/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: freeg_c.c,v 9.0 2001/01/09 02:56:34 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Fb_free_globals_sid[] = "@(#) $Id: freeg_c.c,v 9.0 2001/01/09 02:56:34 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

/*
 * fb_free_globals_clnt - r_freeglobals mechanism - rpc version.
 */

   fb_free_globals_clnt()
      {
         static fb_varvec v, *r;
         int st = FB_AOK;

         /*
          * arguments to putrec_svc are:
          *   r_freeglobals
          */
         fb_loadvec(&v, R_FREEGLOBALS, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "freeglobals - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from freeglobals are:
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
