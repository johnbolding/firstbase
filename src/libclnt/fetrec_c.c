/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: fetrec_c.c,v 9.0 2001/01/09 02:56:34 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Fetchrec_clnt_sid[] = "@(#) $Id: fetrec_c.c,v 9.0 2001/01/09 02:56:34 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

extern short int cdb_loadfail;
extern short int cdb_use_rpc;
extern long cdb_failrec;

/*
 * fb_fetchrec_clnt - fetchrec is getrec but with implicit fetches
 *	for ALL fields.
 */

   fb_fetchrec_clnt(n, hp)
      long n;
      register fb_database *hp;

      {
         static fb_varvec v, *r;
         char sid[10], buf[10];
         int st = FB_AOK;

         /*
          * arguments to getrec_svc are:
          *   r_getrec SID recno
          */
         sprintf(sid, "%d", hp->b_sid);
         sprintf(buf, "%ld", n);
         fb_loadvec(&v, R_FETCHREC, sid, buf, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "fetchrec - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from getrec are:
             *    - nargs
             *    - status
             *    - recno
             *    - fb_field1
             *    - fb_field2
             *    - ...
             *    - fieldN
             */
            st = atoi(fb_argvec(r, 1));
            if (st == FB_AOK)
               fb_fixfields(hp, r);
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

/*
 * fb_fetchxrec_clnt - fetchxrec is getxrec but with implicit fetches
 *	for ALL fields.
 */

   fb_fetchxrec_clnt(key, hp)
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
         fb_loadvec(&v, R_FETCHXREC, sid, key, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "fetchxrec - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from getxrec are:
             *    - nargs
             *    - status
             *    - recno
             *    - fb_field1
             *    - fb_field2
             *    - ...
             *    - fieldN
             */
            st = atoi(fb_argvec(r, 1));
            if (st == FB_AOK || cdb_loadfail == 1)
               fb_fixfields(hp, r);
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

/*
 * fb_fetchrec - fetchrec is getrec but with implicit fetches
 *	for ALL fields.
 */

   fb_fetchrec(n, hp)
      long n;
      register fb_database *hp;

      {
         if (cdb_use_rpc)
            return(fb_fetchrec_clnt(n, hp));
         return(fb_getrec(n, hp));
      }

/*
 * fb_fetchxrec - fetchxrec is getxrec but with implicit fetches
 *	for ALL fields.
 */

   fb_fetchxrec(key, hp)
      char *key;
      register fb_database *hp;

      {
         if (cdb_use_rpc)
            return(fb_fetchxrec_clnt(key, hp));
         return(fb_getxrec(key, hp));
      }

#endif /* RPC */
