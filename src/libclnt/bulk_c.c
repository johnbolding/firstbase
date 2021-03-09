/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: bulk_c.c,v 9.0 2001/01/09 02:56:32 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Bulk_c_sid[] = "@(#) $Id: bulk_c.c,v 9.0 2001/01/09 02:56:32 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

extern short int cdb_error;

/*
 * fb_bulkrec_begin_clnt - implement bulkrec_begin for RPCs
 *    fb_bulkrec_end_clnt is in the cmd_clnt.c area
 */

   fb_bulkrec_begin_clnt(hp, fwait)
      register fb_database *hp;
      int fwait;

      {
         static fb_varvec v, *r;
         char sid[10], buf[10];
         int st = FB_AOK;

         /*
          * arguments to bulkrec_begin_svc are:
          *   r_bulkrec_begin SID fwait
          */
         sprintf(sid, "%d", hp->b_sid);
         sprintf(buf, "%d", fwait);
         fb_loadvec(&v, R_BULKREC_BEGIN, sid, buf, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "bulkrec_begin - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from getrec are:
             *    - nargs
             *    - status
             *    - cdb_error
             */
            st = atoi(fb_argvec(r, 1));
            cdb_error = atol(fb_argvec(r, 2));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

#endif /* RPC */
