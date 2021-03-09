/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: unix_c.c,v 9.0 2001/01/09 02:56:37 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Fb_unix_clnt_sid[] = "@(#) $Id: unix_c.c,v 9.0 2001/01/09 02:56:37 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>
#include <sys/stat.h>
#include <sys/types.h>

extern short int cdb_use_rpc;
extern int errno;

#if !FB_PROTOTYPES
off_t fb_lseek_clnt();
#endif

/*
 * these calls emulate the unix calls of the same name:
 *	fb_open_clnt - open
 *	fb_close_clnt - close
 *	fb_read_clnt - read
 *	fb_write_clnt - write
 *	fb_system_clnt - write
 *	fb_unlink_clnt - unlink
 *	fb_access_clnt - access
 *	fb_getctime - uses stat and gets the ctime of a file
 *	fb_mkdir - mkdir(2) - includes fb_mkdir_clnt
 *	fb_symlink - symlink(2) includes fb_symlink_clnt
 *	fb_umask_clnt
 *	fb_rmdir_clnt
 *	fb_chdir_clnt
 *	fb_getwd_clnt
 *	fb_lockf_clnt
 *	fb_lseek_clnt
 *
 *	fb_errno_clnt
 */

/*
 * this section is the FirstBase style interface for these routines.
 * in each case, when the use_rpc flag is not set, then the standard
 * UNIX call is made instead.
 */

   fb_open(path, flags)
      char *path;
      int flags;

      {
         if (cdb_use_rpc)
            return(fb_open_clnt(path, flags));
         else
            return(open(path, flags));
      }

   
   fb_close(fd)
      int fd;

      {
         if (cdb_use_rpc)
            return(fb_close_clnt(fd));
         else
            return(close(fd));
      }

   fb_read(fd, buf, nbytes)
      int fd;
      char *buf;
      int nbytes;

      {
         if (cdb_use_rpc)
            return(fb_read_clnt(fd, buf, nbytes));
         else
            return(read(fd, buf, (unsigned) nbytes));
      }

   fb_write(fd, buf, nbytes)
      int fd;
      char *buf;
      int nbytes;

      {
         if (cdb_use_rpc)
            return(fb_write_clnt(fd, buf, nbytes));
         else
            return(write(fd, buf, (unsigned) nbytes));
      }

   fb_creat(path, mode)
      char *path;
      int mode;

      {
         if (cdb_use_rpc)
            return(fb_creat_clnt(path, mode));
         else
            return(creat(path, (unsigned) mode));
      }

   fb_unlink(path)
      char *path;

      {
         if (cdb_use_rpc)
            return(fb_unlink_clnt(path));
         else
            return(unlink(path));
      }

   fb_access(path, mode)
      char *path;
      int mode;

      {
         if (cdb_use_rpc)
            return(fb_access_clnt(path, mode));
         else
            return(access(path, mode));
      }

   /* fb_system - done in libcdb/System.c */
   /* fb_getctime - done in libcdb/getctime.c */

   fb_mkdir(path, mode)
      char *path;
      int mode;

      {
         if (cdb_use_rpc)
            return(fb_mkdir_clnt(path, mode));
         else
            return(mkdir(path, (unsigned) mode));
      }

   fb_symlink(name1, name2)
      char *name1, *name2;

      {
         if (cdb_use_rpc)
            return(fb_symlink_clnt(name1, name2));
         else
            return(symlink(name1, name2));
      }

   fb_umask(numask)
      int numask;

      {
         if (cdb_use_rpc)
            return(fb_umask_clnt(numask));
         else
            return(umask((unsigned) numask));
      }

   fb_rmdir(path)
      char *path;

      {
         if (cdb_use_rpc)
            return(fb_rmdir_clnt(path));
         else
            return(rmdir(path));
      }

   fb_chdir(path)
      char *path;

      {
         if (cdb_use_rpc)
            return(fb_chdir_clnt(path));
         else
            return(chdir(path));
      }

   /* fb_getwd is in libcdb/Getwd.c */

   fb_lockf(fd, cmd, size)
      int fd, cmd;
      long size;

      {
         if (cdb_use_rpc)
            return(fb_lockf_clnt(fd, cmd, size));
         else
            return(lockf(fd, cmd, size));
      }
      
   off_t fb_lseek(des, offset, whence)
      int des, whence;
      off_t offset;

      {
         if (cdb_use_rpc)
            return(fb_lseek_clnt(des, offset, whence));
         else
            return(lseek(des, offset, whence));
      }
      
   fb_errno()

      {
         if (cdb_use_rpc)
            return(fb_errno_clnt());
         else
            return(errno);
      }
      
/*
 * this section is the real RPC version of these UNIX calls.
 */

   fb_open_clnt(path, flags)
      char *path;
      int flags;

      {
         static fb_varvec v, *r;
         char buf[10];
         int fd = -1;

         /*
          * arguments to open_clnt are:
          *   r_open path flags
          */
         sprintf(buf, "%d", flags);
         fb_loadvec(&v, R_OPEN, path, buf, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "open - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from open are:
             *    - nargs
             *    - file descriptor
             */
            fd = atoi(fb_argvec(r, 1));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(fd);
      }

   fb_close_clnt(fd)
      int fd;

      {
         static fb_varvec v, *r;
         char buf[10];
         int st = -1;

         /*
          * arguments to close_clnt are:
          *   r_close fd
          */
         sprintf(buf, "%d", fd);
         fb_loadvec(&v, R_CLOSE, buf, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "close - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from close are:
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

   fb_read_clnt(fd, buf, nbytes)
      int fd;
      char *buf;
      int nbytes;

      {
         static fb_varvec v, *r;
         char fbuf[10], nbuf[10];

         /*
          * arguments to read_clnt are:
          *   r_read fd nbytes
          */
         sprintf(fbuf, "%d", fd);
         sprintf(nbuf, "%d", nbytes);
         fb_loadvec(&v, R_READ, fbuf, nbuf, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "read - results from toserver:");
#endif
         nbytes = -1;
         if (r != NULL){
            /*
             * results back from read are:
             *    - nargs
             *    - nbytes
             *    - read string
             */
            nbytes = atoi(fb_argvec(r, 1));
            if (nbytes > 0)
               memcpy(buf, fb_argvec(r, 2), (unsigned) nbytes);
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(nbytes);
      }

   fb_write_clnt(fd, buf, nbytes)
      int fd;
      char *buf;
      int nbytes;

      {
         static fb_varvec v, *r;
         char fbuf[10], nbuf[10], *cbuf;

         /*
          * arguments to write_clnt are:
          *   r_write fd buf nbytes
          */
         sprintf(fbuf, "%d", fd);
         sprintf(nbuf, "%d", nbytes);
         cbuf = fb_malloc((unsigned) (nbytes + 1));
         memcpy(cbuf, buf, (unsigned) nbytes);
         cbuf[nbytes] = NULL;
         fb_loadvec(&v, R_WRITE, fbuf, cbuf, nbuf, 0);
         fb_free(cbuf);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "write - results from toserver:");
#endif
         nbytes = -1;
         if (r != NULL){
            /*
             * results back from write are:
             *    - nargs
             *    - nbytes
             */
            nbytes = atoi(fb_argvec(r, 1));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(nbytes);
      }

   fb_creat_clnt(path, mode)
      char *path;
      int mode;

      {
         static fb_varvec v, *r;
         char buf[10];
         int fd = -1;

         /*
          * arguments to creat_clnt are:
          *   r_creat path mode
          */
         sprintf(buf, "%d", mode);
         fb_loadvec(&v, R_CREAT, path, buf, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "creat - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from creat are:
             *    - nargs
             *    - file descriptor
             */
            fd = atoi(fb_argvec(r, 1));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(fd);
      }

   fb_system_clnt(command, rootperm)
      char *command;
      int rootperm;

      {
         static fb_varvec v, *r;
         char buf[10];
         int st = -1;

         /*
          * arguments to system_clnt are:
          *   r_system command rootperm
          */
         sprintf(buf, "%d", rootperm);
         fb_loadvec(&v, R_SYSTEM, command, buf, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "r_system - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from r_system are:
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

   fb_unlink_clnt(path)
      char *path;

      {
         static fb_varvec v, *r;
         int st = -1;

         /*
          * arguments to unlink_clnt are:
          *   r_unlink path
          */
         fb_loadvec(&v, R_UNLINK, path, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "unlink - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from unlink are:
             *    - nargs
             *    - status return from UNIX unlink(2) command
             */
            st = atoi(fb_argvec(r, 1));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

/*
 * fb_getctime_clnt - returns the time_t of the file path
 */

   time_t fb_getctime_clnt(path)
      char *path;

      {
         static fb_varvec v, *r;
         time_t tval = -1;

         /*
          * arguments to getctime_clnt are:
          *   r_unlink path
          */
         fb_loadvec(&v, R_GETCTIME, path, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "getctime - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from getctime are:
             *    - nargs
             *    - time_t (long) tval as a buffer
             */
            tval = atol(fb_argvec(r, 1));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(tval);
      }

   fb_access_clnt(path, mode)
      char *path;
      int mode;

      {
         static fb_varvec v, *r;
         char buf[10];
         int st = -1;

         /*
          * arguments to access_clnt are:
          *   r_access path mode
          */
         sprintf(buf, "%d", mode);
         fb_loadvec(&v, R_ACCESS, path, buf, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "access - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from access are:
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

   fb_mkdir_clnt(path, mode)
      char *path;
      int mode;

      {
         static fb_varvec v, *r;
         char buf[10];
         int st = -1;

         /*
          * arguments to mkdir_clnt are:
          *   r_mkdir path mode
          */
         sprintf(buf, "%d", mode);
         fb_loadvec(&v, R_MKDIR, path, buf, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "mkdir - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from access are:
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

   fb_symlink_clnt(name1, name2)
      char *name1, *name2;

      {
         static fb_varvec v, *r;
         int st = -1;

         /*
          * arguments to system_clnt are:
          *   r_mkdir path mode
          */
         fb_loadvec(&v, R_SYMLINK, name1, name2, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "symlink - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from access are:
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

   fb_umask_clnt(numask)
      int numask;

      {
         static fb_varvec v, *r;
         int st = -1;
         char buf[10];

         /*
          * arguments to umask_clnt are:
          *   r_umask numask
          */
         sprintf(buf, "%d", numask);
         fb_loadvec(&v, R_UMASK, buf, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "umask - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from umask are:
             *    - nargs
             *    - status return from UNIX umask(2) command
             */
            st = atoi(fb_argvec(r, 1));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

   fb_rmdir_clnt(path)
      char *path;

      {
         static fb_varvec v, *r;
         int st = -1;

         /*
          * arguments to rmdir_clnt are:
          *   r_rmdir path
          */
         fb_loadvec(&v, R_RMDIR, path, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "rmdir - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from rmdir are:
             *    - nargs
             *    - status return from UNIX rmdir(2) command
             */
            st = atoi(fb_argvec(r, 1));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

   fb_chdir_clnt(path)
      char *path;

      {
         static fb_varvec v, *r;
         int st = -1;

         /*
          * arguments to chdir_clnt are:
          *   r_chdir path
          */
         fb_loadvec(&v, R_CHDIR, path, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "chdir - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from creat are:
             *    - nargs
             *    - status return from UNIX chdir(2) command
             */
            st = atoi(fb_argvec(r, 1));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

   char *fb_getwd_clnt(p)
      char *p;

      {
         static fb_varvec v, *r;

         /*
          * arguments to link_clnt are:
          *   r_getwd
          */
         fb_loadvec(&v, R_GETWD, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "getwd - results from toserver:");
#endif
         *p = NULL;
         if (r != NULL){
            /*
             * results back from getwd are:
             *    - nargs
             *    - string form of working directory
             */
            strcpy(p, fb_argvec(r, 1));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(p);
      }

   off_t fb_lseek_clnt(des, offset, whence)
      int des, whence;
      off_t offset;

      {
         static fb_varvec v, *r;
         char dbuf[10], wbuf[10], obuf[20];
         int st = 0;

         /*
          * arguments to lseek_clnt are:
          *   r_lseek des offset whence
          */
         sprintf(dbuf, "%d", des);
         sprintf(obuf, "%ld", offset);
         sprintf(wbuf, "%d", whence);
         fb_loadvec(&v, R_LSEEK, dbuf, obuf, wbuf, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "lseek - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from lseek are:
             *    - status
             */
            st = atol(fb_argvec(r, 1));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

   fb_lockf_clnt(fd, cmd, size)
      int fd, cmd;
      long size;

      {
         static fb_varvec v, *r;
         char fbuf[10], cbuf[10], sbuf[20];
         int st = 0;

         /*
          * arguments to lockf_clnt are:
          *   r_lockf fd cmd size
          */
         sprintf(fbuf, "%d", fd);
         sprintf(cbuf, "%d", cmd);
         sprintf(sbuf, "%ld", size);
         fb_loadvec(&v, R_LOCKF, fbuf, cbuf, sbuf, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "lockf - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from lockf are:
             *    - status
             */
            st = atoi(fb_argvec(r, 1));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

   fb_errno_clnt()

      {
         static fb_varvec v, *r;
         int st = -1;

         /*
          * arguments to errno_clnt are:
          *   r_errno
          */
         fb_loadvec(&v, R_ERRNO, 0);
         r = fb_toserver(&v);
#if 0
         fb_tracevec(r, "errno - results from toserver:");
#endif
         if (r != NULL){
            /*
             * results back from errno are:
             *    - errno
             */
            st = atoi(fb_argvec(r, 1));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

#endif /* RPC */
