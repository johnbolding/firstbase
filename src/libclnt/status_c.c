/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: status_c.c,v 9.0 2001/01/09 02:56:36 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Status_clnt_sid[] = "@(#) $Id: status_c.c,v 9.0 2001/01/09 02:56:36 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

/*
 * fb_status_clnt - used for a single command that returns a single status,
 *	where no side affects are neeeded.
 */

   fb_status_clnt(output)
      char *output;

      {
         static fb_varvec v, *r;
         char *p, *q;
         int st = FB_AOK, nargs, i;

         /*
          * arguments for status_svc
          *   R_STATUS
          */
         fb_loadvec(&v, R_STATUS, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "status_clnt - results from toserver:");
#endif
         output[0] = NULL;
         if (st == FB_AOK){
            /*
             * results back from status:
             *    - nargs
             *    - status
             *    - return comment line 1 ...
             *    - status comment line 1 ...
             */
            nargs = atoi(fb_argvec(r, 0));
            st = atoi(fb_argvec(r, 1));
            for (p = output, i = 2; i < nargs; i++){
               q = fb_argvec(r, i);
               strcpy(p, q);
               p += strlen(q);
               *p++ = FB_NEWLINE;
               }
            *p = NULL;
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

#endif /* RPC */
