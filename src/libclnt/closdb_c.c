/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: closdb_c.c,v 9.0 2001/01/09 02:56:32 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char fb_closedb_clnt_sid[] = "@(#) $Id: closdb_c.c,v 9.0 2001/01/09 02:56:32 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

/*
 *  fb_closedb_clnt - close a fb_database opened via fbserver
 */

   fb_closedb_clnt(dp)
      fb_database *dp;
      
      {
         static fb_varvec v, *r;
         int st = FB_AOK;
         char buf[30];

         /*
          * arguments to closedb_svc are:
          *    closedb SID
          */
         sprintf(buf, "%d", dp->b_sid);
         fb_loadvec(&v, R_CLOSEDB, buf, 0);
         r = fb_toserver(&v);
         /* interpret the results */
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "closedb_clnt - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from closedb_svc are:
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

