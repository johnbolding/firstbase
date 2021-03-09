/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_file.c,v 9.3 2002/08/13 19:04:02 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macro_file_sid[] = "@(#) $Id: mac_file.c,v 9.3 2002/08/13 19:04:02 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>

#if !FB_PROTOTYPES
static int fd_slot();
static int fd_init();
#else /* FB_PROTOTYPES */
static int fd_slot(void);
static int fd_init(void);
#endif /* FB_PROTOTYPES */

#define MAXFILES 100
static FILE *fs_array[MAXFILES];
static int init_flag = 0;

   mf_mktemp(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         (void) Macro_file_sid;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         tostring(n);
         strcpy(buf, n->n_pval);
         /*close(mkstemp(buf));*/
         mktemp(buf);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_mkstemp(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXNAME];

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         tostring(n);
         strcpy(buf, n->n_pval);
         close(mkstemp(buf));
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_creat(n, r)
      fb_mnode *n, *r;

      {
         char *fname;
         int mode;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         fname = tostring(n);		n = n->n_next;
         mode = n->n_fval;
         r->n_tval |= T_NUM;
         if (access(fname, 0) != 0)
            r->n_fval = close(creat(fname, (unsigned) mode));
         else
            r->n_fval = chmod(fname, (unsigned) mode);
         return(FB_AOK);
      }

   mf_chmod(n, r)
      fb_mnode *n, *r;

      {
         char *fname;
         int mode;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         fname = tostring(n);		n = n->n_next;
         mode = n->n_fval;
         r->n_tval |= T_NUM;
         r->n_fval = chmod(fname, (unsigned) mode);
         return(FB_AOK);
      }

   mf_fopen(n, r)
      fb_mnode *n, *r;

      {
         int fd;
         char *fname, *s_mode;
         FILE *fs;

         if (!init_flag)
            fd_init();
         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         fname = tostring(n);		n = n->n_next;
         s_mode =  tostring(n);
         fd = fd_slot();
         if (fd < 0 || fd >= MAXFILES)
            return(FB_ERROR);
         fs = fopen(fname, s_mode);
         if (fs == NULL)
            fd = -1;
         else
            fs_array[fd] = fs;
         /*fb_mkstr(&(fs_modes[fd]), s_mode);*/
         r->n_fval = fd;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_fclose(n, r)
      fb_mnode *n, *r;

      {
         int fd;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         fd = n->n_fval;
         if (fd < 0 || fd >= MAXFILES || fs_array[fd] == 0)
            return(FB_ERROR);
         fflush(fs_array[fd]);
         fclose(fs_array[fd]);
         fs_array[fd] = NULL;
         return(FB_AOK);
      }

   mf_popen(n, r)
      fb_mnode *n, *r;

      {
         int fd;
         char *fname, *s_mode;
         FILE *fs;

         if (!init_flag)
            fd_init();
         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         fname = tostring(n);		n = n->n_next;
         s_mode =  tostring(n);
         fd = fd_slot();
         if (fd < 0 || fd >= MAXFILES)
            return(FB_ERROR);
         fs = popen(fname, s_mode);
         if (fs == NULL)
            fd = -1;
         else
            fs_array[fd] = fs;
         r->n_fval = fd;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_pclose(n, r)
      fb_mnode *n, *r;

      {
         int fd;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         fd = n->n_fval;
         if (fd < 0 || fd >= MAXFILES || fs_array[fd] == 0)
            return(FB_ERROR);
         pclose(fs_array[fd]);
         fs_array[fd] = NULL;
         return(FB_AOK);
      }

   mf_fprintf(n, r)
      fb_mnode *n, *r;

      {
         int fd;
         char *buf;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         fd = n->n_fval;		n = n->n_next;
         if (fd < 0 || fd >= MAXFILES || fs_array[fd] == 0)
            return(FB_ERROR);
         mf_sprintf(n, r);
         buf = r->n_nval;
         if (fputs(buf, fs_array[fd]) == EOF)
            return(FB_ERROR);
         return(FB_AOK);
      }

/*
 * mf_fputs - fputs(s, stream)
 */

   mf_fputs(n, r)
      fb_mnode *n, *r;

      {
         int fd;
         char *buf;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         buf = tostring(n);		n = n->n_next;
         fd = n->n_fval;
         if (fd < 0 || fd >= MAXFILES || fs_array[fd] == 0)
            return(FB_ERROR);
         if (fputs(buf, fs_array[fd]) == EOF)
            return(FB_ERROR);
         r->n_fval = FB_AOK;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

/*
 * mf_fgets - fgets(s, n, stream)
 */

   mf_fgets(n, r)
      fb_mnode *n, *r;

      {
         int fd, len, rlen;
         char *buf;
         fb_mnode tn, *ret, *t_ret;

         if (fb_realnodes(n) < 3)
            return(FB_ERROR);
         ret = n;			n = n->n_next;
         len = n->n_fval;		n = n->n_next;
         fd = n->n_fval;
         if (fd < 0 || fd >= MAXFILES || fs_array[fd] == 0)
            return(FB_ERROR);
         if (len <= 0)
            return(FB_ERROR);
         buf = fb_malloc((unsigned) (len + 1));
         if (fgets(buf, len, fs_array[fd]) == NULL)
            rlen = 0;
         else{
            if (ret->n_type != V_ARRAY){
               t_ret = mnode_to_var(ret);
               if (t_ret == NULL){
                  fb_serror(FB_MESSAGE, "Illegal parameter", NIL);
                  return(FB_ERROR);
                  }
               fb_mkstr(&(t_ret->n_nval), buf);
               t_ret->n_tval |= T_STR;
               }
            else{
               fb_clearnode(&tn);
               fb_mkstr(&(tn.n_nval), buf);
               tn.n_tval |= T_STR;
               assign_array(ret, &tn, O_ASSIGN);
               fb_free(tn.n_nval);
               fb_free(tn.n_pval);
               }
            rlen = 1;
            }
         fb_free(buf);
         r->n_fval = rlen;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

/*
 * unlink - implement unlink(2)
 */

   mf_unlink(n, r)
      fb_mnode *n, *r;

      {
         char *fname;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         fname = tostring(n);
         r->n_fval = unlink(fname);
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

/*
 * link - implement link(2)
 */

   mf_link(n, r)
      fb_mnode *n, *r;

      {
         char *oname, *nname;
         fb_mnode *oldpath, *newpath;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         oldpath = n;			n = n->n_next;
         newpath = n;
         oname = tostring(oldpath);
         nname = tostring(newpath);
         r->n_fval = link(oname, nname);
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

/*
 * rename - implement rename(2)
 */

   mf_rename(n, r)
      fb_mnode *n, *r;

      {
         char *oname, *nname;
         fb_mnode *oldpath, *newpath;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         oldpath = n;			n = n->n_next;
         newpath = n;
         oname = tostring(oldpath);
         nname = tostring(newpath);
         r->n_fval = rename(oname, nname);
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

/*
 * symlink - implement symlink(2)
 */

   mf_symlink(n, r)
      fb_mnode *n, *r;

      {
         char *tname, *fname;
         fb_mnode *topath, *frompath;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         frompath = n;			n = n->n_next;
         topath = n;
         fname = tostring(frompath);
         tname = tostring(topath);
         r->n_fval = symlink(fname, tname);
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

/*
 * mf_fseek - fseek(fd, offset, whence)
 */

   mf_fseek(n, r)
      fb_mnode *n, *r;

      {
         long offset;
         int fd, whence;

         fd = n->n_fval;		n = n->n_next;
         if (fd < 0 || fd >= MAXFILES || fs_array[fd] == 0)
            return(FB_ERROR);
         offset = (long) n->n_fval;	n = n->n_next;
         whence = (int) n->n_fval;
         if (whence < 0 || whence > 2)
            return(FB_ERROR);
         r->n_fval = fseek(fs_array[fd], offset, whence);
         return(FB_AOK);
      }

/*
 * mf_fread - fread(addr, len, fd)
 */

   mf_fread(n, r)
      fb_mnode *n, *r;

      {
         int fd, len, rlen;
         char *buf;
         fb_mnode *ret, *t_ret, tn;

         if (fb_realnodes(n) < 3)
            return(FB_ERROR);
         ret = n; 			n = n->n_next;
         len = n->n_fval;		n = n->n_next;
         fd = n->n_fval;
         if (fd < 0 || fd >= MAXFILES || fs_array[fd] == 0)
            return(FB_ERROR);
         if (len <= 0)
            return(FB_ERROR);
         buf = fb_malloc((unsigned) (len + 1));
         if ((rlen = fread(buf, sizeof(char), (unsigned) len,
               fs_array[fd])) > 0){
            buf[rlen] = NULL;
            if (ret->n_type != V_ARRAY){
               t_ret = mnode_to_var(ret);
               if (t_ret == NULL){
                  fb_serror(FB_MESSAGE, "Illegal parameter", NIL);
                  return(FB_ERROR);
                  }
               fb_mkstr(&(t_ret->n_nval), buf);
               t_ret->n_tval |= T_STR;
               }
            else{
               fb_clearnode(&tn);
               fb_mkstr(&(tn.n_nval), buf);
               tn.n_tval |= T_STR;
               assign_array(ret, &tn, O_ASSIGN);
               fb_free(tn.n_nval);
               fb_free(tn.n_pval);
               }
            }
         fb_free(buf);
         r->n_fval = rlen;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

/*
 * mf_fwrite - fwrite(s, len, fd)
 */

   mf_fwrite(n, r)
      fb_mnode *n, *r;

      {
         int fd, len;
         char *buf;

         if (fb_realnodes(n) < 3)
            return(FB_ERROR);
         buf = tostring(n);		n = n->n_next;
         len = n->n_fval;		n = n->n_next;
         fd = n->n_fval;
         if (fd < 0 || fd >= MAXFILES || fs_array[fd] == 0)
            return(FB_ERROR);
         fflush(fs_array[fd]);
         r->n_fval = fwrite(buf, sizeof(char), (unsigned) len, fs_array[fd]);
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_fflush(n, r)
      fb_mnode *n, *r;

      {
         int fd;
         FILE *f_target;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         fd = n->n_fval;

         /* short circuit if its a -1 passed in -- -1 == stdout */
         if (fd == -1)
            f_target = stdout;
         else if (fd < 0 || fd >= MAXFILES || fs_array[fd] == 0)
            return(FB_ERROR);
         else
            f_target = fs_array[fd];
         r->n_fval = fflush(f_target);
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_access(n, r)
      fb_mnode *n, *r;

      {
         char *path;
         int mode;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         path = tostring(n);		n = n->n_next;
         mode = n->n_fval;
         r->n_fval = access(path, mode);
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

/*
 * fd_slot - find the next fd slot
 */

   static fd_slot()

      {
         int i;

         for (i = 0; i < MAXFILES; i++)
            if (fs_array[i] == NULL)
               return(i);
         return(-1);
      }

   static fd_init()
      {
         int i;

         init_flag = 1;
         for (i = 0; i < MAXFILES; i++)
            fs_array[i] = NULL;
         return(-1);
      }
