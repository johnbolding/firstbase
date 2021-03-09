/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: lock_c.c,v 9.0 2001/01/09 02:56:35 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Lock_clnt_sid[] = "@(#) $Id: lock_c.c,v 9.0 2001/01/09 02:56:35 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

/*
 * fb_lock_clnt - r_lockrec mechanism - rpc version.
 */

   fb_lock_clnt(mrec, db, fwait)
      long mrec;
      fb_database *db;
      int fwait;

      {
         static fb_varvec v, *r;
         char buf[10], sid[10], wbuf[10];
         int st = FB_AOK;

         /*
          * arguments to lock_svc are:
          *   r_lockrec SID mrec fwait
          */
         sprintf(sid, "%d", db->b_sid);
         sprintf(buf, "%ld", mrec);
         sprintf(wbuf, "%d", fwait);
         fb_loadvec(&v, R_LOCKREC, sid, buf, wbuf, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "lockrec - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from getrec are:
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

/*
 * fb_unlock_clnt - r_unlockrec mechanism - rpc version.
 */

   fb_unlock_clnt(mrec, db)
      long mrec;
      fb_database *db;

      {
         static fb_varvec v, *r;
         char buf[10], sid[10];
         int st = FB_AOK;

         /*
          * arguments to lock_svc are:
          *   r_lockrec SID mrec fwait
          */
         sprintf(sid, "%d", db->b_sid);
         sprintf(buf, "%ld", mrec);
         fb_loadvec(&v, R_UNLOCKREC, sid, buf, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "unlockrec - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from getrec are:
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

/*
 * fb_s_lock_clnt - r_s_lock mechanism - rpc version.
 */

   fb_s_lock_clnt(fd, fwait, fname)
      int fd;			
      int fwait;
      char *fname;

      {
         static fb_varvec v, *r;
         char fbuf[10], wbuf[10];
         int st = FB_AOK;

         /*
          * arguments to s_lock_svc are:
          *   r_s_lock fd fwait fname
          */
         sprintf(fbuf, "%d", fd);
         sprintf(wbuf, "%d", fwait);
         fb_loadvec(&v, R_S_LOCK, fbuf, wbuf, fname, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "s_lock - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from getrec are:
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

/*
 * fb_s_unlock_clnt - r_s_unlock mechanism - rpc version.
 */

   fb_s_unlock_clnt(fd, fname)
      int fd;			
      char *fname;

      {
         static fb_varvec v, *r;
         char fbuf[10];
         int st = FB_AOK;

         /*
          * arguments to s_lock_svc are:
          *   r_s_lock fd fwait fname
          */
         sprintf(fbuf, "%d", fd);
         fb_loadvec(&v, R_S_UNLOCK, fbuf, fname, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "s_unlock - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from getrec are:
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
