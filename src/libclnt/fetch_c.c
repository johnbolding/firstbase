/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: fetch_c.c,v 9.0 2001/01/09 02:56:34 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Fetch_clnt_sid[] = "@(#) $Id: fetch_c.c,v 9.0 2001/01/09 02:56:34 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

/*
 * fb_fetch_clnt - r_fetch mechanism - rpc version of fetch.
 */

   fb_fetch_clnt(k, s, hp)
      fb_field *k;
      register char *s;
      fb_database *hp;
   
      {
         static fb_varvec v, *r;
         char sid[10];
         int st = FB_AOK;

         /*
          * arguments to fb_fetch_svc are:
          *   r_fetch SID fieldname
          */
         sprintf(sid, "%d", hp->b_sid);
         fb_loadvec(&v, R_FETCH, sid, k->id, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "fetch - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from fb_fetch_svc are:
             *    - nargs
             *    - status
             *    - field_data
             */
            st = atoi(fb_argvec(r, 1));
            if (st == FB_AOK)
               strcpy(s, fb_argvec(r, 2));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

#endif /* RPC */
