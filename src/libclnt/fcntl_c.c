/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: fcntl_c.c,v 9.0 2001/01/09 02:56:34 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Fb_fcntl_sid[] = "@(#) $Id: fcntl_c.c,v 9.0 2001/01/09 02:56:34 john Exp $";
#endif

#include <fb.h>

#if !RPC
   fb_fcntl_clnt(a,b,c)
      int a,b,c;
   {
      return(FB_ERROR);
   }
#endif /* !RPC */

#if RPC

#include <fb_ext.h>
#include <fbserver.h>

static char hostname[FB_MAXNAME];
int cdb_locking_pid = 0;

extern short int cdb_locklevel;
extern char *cdb_work_dir;

/*
 * fb_fcntl_clnt - provide locking sort of like the fcntl() call ...
 *	this is the client side of the mechanism.
 *
 *	gather up the arguments and pass to lockd
 */

   fb_fcntl_clnt(fname, func, fk)
      struct flock *fk;
      int func;
      char *fname;

      {
         int st = 1;
         static fb_varvec v, *r;
         char b_name[FB_MAXNAME];
         char b_type[30], b_whence[30], b_start[30], b_len[30], b_pid[30];
         char b_func[30];

         /*
          * arguments to fb_fcntl_svc are:
          *   r_fcntl fname func fk->l_type fk->l_whence fk->l_start fk->l_len
          *	hostname pid
          *
          * note: here, too, we have to change func and fk->l_type to
          *   canonical constant values as they seem to change for different OS
          */

         switch(func){
            case F_SETLK:  func = FB_F_SETLK; break;
            case F_SETLKW: func = FB_F_SETLKW; break;
            }
         switch(fk->l_type){
            case F_WRLCK: fk->l_type = FB_F_WRLCK; break;
            case F_UNLCK: fk->l_type = FB_F_UNLCK; break;
            }
#if 0
         fprintf(stderr, "fb_fcntl_clnt - fname=%s, func=%d\n", fname, func);
         fprintf(stderr,
            "   l_type=%d l_whence=%d l_start=%ld l_len=%ld l_pid=%d\n",
            fk->l_type, fk->l_whence, fk->l_start, fk->l_len, fk->l_pid);
#endif
         if (cdb_locking_pid == 0){
            if (fb_lockd_clnt_create() != FB_AOK){
               cdb_locklevel = 0;
               fb_serror(FB_RPC_ERROR, "WARNING: fblockd connection failed.", NIL);
               return(0);
               }
            cdb_locking_pid = getpid();
            gethostname(hostname, FB_MAXNAME);
            }

         if (fname[0] != '/'){
            strcpy(b_name, cdb_work_dir);
            fb_assure_slash(b_name);
            strcat(b_name, fname);
            }
         else
            strcpy(b_name, fname);

         sprintf(b_func, "%d", func);
         sprintf(b_type, "%d", fk->l_type);
         sprintf(b_whence, "%d", fk->l_whence);
         sprintf(b_start, "%ld", fk->l_start);
         sprintf(b_len, "%ld", fk->l_len);
         sprintf(b_pid, "%d", cdb_locking_pid);

         fb_loadvec(&v, R_FCNTL, b_name, b_func, b_type, b_whence,
            b_start, b_len, hostname, b_pid, 0);
         /*
          * this beast loops forever.
          * its outs are:
          *	- granting of the lock
          *	- an alarm set/caught
          */
         for (;;){
            st = 1;
            r = fb_tolockd(&v);
            if (r == (fb_varvec *) NULL){
               st = FB_ERROR;
               break;
               }
#if 0
            fb_tracevec(r, "fcntl - results from tolockd:");
#endif
            if (st == 1){
               /*
                * results back from fb_fcntl_svc are:
                *    - nargs
                *    - status
                */
               st = atoi(fb_argvec(r, 1));
               }
            /* results are freed by the caller the *next* time */
            if (func != FB_F_SETLKW || st == 0)
               break;
            }
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

/*
 * fb_fcntl_cl_clnt - clear the lockd name indicated.
 */

   fb_fcntl_cl_clnt(fname, start, len, procid, override)
      char *fname;
      long start, len;
      int procid, override;

      {
         static fb_varvec v, *r;
         int st = FB_AOK;
         char buf[FB_MAXLINE], buf1[10], b_start[30], b_len[30], b_pid[30];

         /*
          * arguments to fb_fcntl_clear_clnt are:
          *   r_fcntl_clear fname start len pid hostname override
          */

         if (hostname[0] == NULL)
            gethostname(hostname, FB_MAXNAME);

         if (fname[0] != '/' && fname[0] != NULL){
            strcpy(buf, cdb_work_dir);
            fb_assure_slash(buf);
            strcat(buf, fname);
            }
         else
            strcpy(buf, fname);
         sprintf(b_start, "%ld", start);
         sprintf(b_len, "%ld", len);
         sprintf(b_pid, "%d", procid);
         sprintf(buf1, "%d", override);
         fb_loadvec(&v, R_FCNTL_CLR, buf, b_start, b_len, b_pid, hostname,
            buf1, 0);
#if 0
         fb_tracevec(v, "fb_fcntl_cl_clnt - sending to tolockd:");
#endif
         r = fb_tolockd(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fb_tracevec(r, "fb_fcntl_cl_clnt - results from tolockd:");
#endif
         if (st == FB_AOK){
            /*
             * results back from fcntl_clr are:
             *    - nargs
             *    - status
             */
            st = atoi(fb_argvec(r, 1));
            }
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         /* results are freed in the fcntl_svc */
         return(st);
      }

#endif /* RPC */
