/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: store_c.c,v 9.0 2001/01/09 02:56:37 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Store_clnt_sid[] = "@(#) $Id: store_c.c,v 9.0 2001/01/09 02:56:37 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

/*
 * fb_store_clnt - r_store mechanism - rpc version of store.
 */

   fb_store_clnt(k, s, hp)
      fb_field *k;
      register char *s;
      fb_database *hp;
   
      {
         static fb_varvec v, *r;
         char sid[10];
         int st = FB_AOK;

         /*
          * arguments to fb_store_svc are:
          *   r_store SID fieldname string
          */
         sprintf(sid, "%d", hp->b_sid);
         fb_loadvec(&v, R_STORE, sid, k->id, s, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "store - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from fb_store_svc are:
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
