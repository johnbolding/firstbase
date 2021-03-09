/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: trlckd_c.c,v 9.0 2001/01/09 02:56:37 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Fb_tracelockd_sid[] = "@(#) $Id: trlckd_c.c,v 9.0 2001/01/09 02:56:37 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fbserver.h>

/*
 * fb_tracelockd_svc - client side: trace the lockd structures
 */

   fb_tr_lockd_clnt(buf, max)
      char *buf;
      int max;

      {
         static fb_varvec v, *r;
         int st = FB_AOK, nargs, i, total = 0;
         char *p, *fb_argvec(fb_varvec *v, int k);

         /*
          * arguments to fb_tracelockd_svc
          *   r_tracelockd 
          */
         fb_loadvec(&v, R_TRACELOCKD, 0);
         r = fb_tolockd(&v);
         if (r == NULL)
            st = FB_ERROR;
         else{
            buf[0] = NULL;
            nargs = atoi(fb_argvec(r, 0)); 
            for (i = 1; i < nargs; i++){
               p = fb_argvec(r, i);
               total += strlen(p);
               if (total < max){
                  strcat(buf, p);
                  strcat(buf, "\n");
                  }
               }
            }
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

#endif /* RPC */
