/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: libclnt.h,v 9.0 2001/01/09 02:56:13 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/*
 * prototypes and definitions for client/server rpc calls
 */

#if RPC

#if !FB_PROTOTYPES

/*extern fb_loadvec();*/
extern bool_t fb_xdr_varvec();
extern char **fb_lserver_1();
extern char *fb_argvec();
extern char *fb_getwd_clnt();
extern fb_access();
extern fb_access_clnt();
extern fb_addidx_clnt();
extern fb_addrec_clnt();
extern fb_b_addrec_clnt();
extern fb_b_delrec_clnt();
extern fb_build_svec();
extern fb_bulkrec_begin_clnt();
extern fb_bulkrec_end_clnt();
extern fb_chdir();
extern fb_chdir_clnt();
extern fb_clear_svec();
extern fb_clear_vec();
extern fb_clnt_create();
extern fb_clnt_destroy();
extern fb_clnt_ping();
extern fb_clnt_timeout();
extern fb_close();
extern fb_close_clnt();
extern fb_closedb_clnt();
extern fb_closeidx_clnt();
extern fb_cmd_clnt();
extern fb_concat_svec();
extern fb_creat();
extern fb_creat_clnt();
extern fb_createidx_clnt();
extern fb_delrec_clnt();
extern fb_errno();
extern fb_errno_clnt();
extern fb_fcntl_cl_clnt();
extern fb_fcntl_clnt();
extern fb_fetch_clnt();
extern fb_fetchrec();
extern fb_fetchrec_clnt();
extern fb_fetchxrec();
extern fb_fetchxrec_clnt();
extern fb_firstxrec_clnt();
extern fb_fixfields();
extern fb_free_globals_clnt();
extern fb_free_svec();
extern fb_free_xdr_vec();
extern fb_freevec();
extern fb_gethead_clnt();
extern fb_getirec_clnt();
extern fb_getrec_clnt();
extern fb_getxrec_clnt();
extern fb_lastxrec_clnt();
extern fb_lock_clnt();
extern fb_lockd_clnt_create();
extern fb_lockd_clnt_destroy();
extern fb_lockf();
extern fb_lockf_clnt();
extern fb_mkdir();
extern fb_mkdir_clnt();
extern fb_mkserver_clnt();
extern fb_nextxrec_clnt();
extern fb_open();
extern fb_open_clnt();
extern fb_opendb_clnt();
extern fb_openidx_clnt();
extern fb_prevxrec_clnt();
extern fb_put_autoindex_clnt();
extern fb_putrec_clnt();
extern fb_read();
extern fb_read_clnt();
extern fb_rmdir();
extern fb_rmdir_clnt();
extern fb_s_lock_clnt();
extern fb_s_unlock_clnt();
extern fb_set_autoindex_clnt();
extern fb_status_clnt();
extern fb_stopserver_clnt();
extern fb_store_clnt();
extern fb_subidx_clnt();
extern fb_symlink();
extern fb_symlink_clnt();
extern fb_sync_clnt();
extern fb_system_clnt();
extern fb_tr_lockd_clnt();
extern fb_tracevec();
extern fb_tune_clnt();
extern fb_umask();
extern fb_umask_clnt();
extern fb_unlink();
extern fb_unlink_clnt();
extern fb_unlock_clnt();
extern fb_useidx_clnt();
extern fb_varvec *fb_server_1();
extern fb_varvec *fb_tolockd();
extern fb_varvec *fb_toserver();
extern fb_varvec *fblockd_1();
extern fb_write();
extern fb_write_clnt();
extern off_t fb_lseek();
extern off_t fb_lseek_clnt();
extern time_t fb_getctime_clnt();

#else  /* FB_PROTOTYPES */

/* variable argument prototypes can only be done using stdarg.h, not vararg.h*/
/* extern fb_loadvec(void *v, ...); */

extern bool_t fb_xdr_varvec(XDR *xdrs, fb_varvec *objp);
extern char **fb_lserver_1(char **argp, CLIENT *clnt);
extern char *fb_argvec(fb_varvec *v, int k);
extern char *fb_getwd_clnt(char *p);
extern fb_access(char *path, int mode);
extern fb_access_clnt(char *path, int mode);
extern fb_addidx_clnt(char *iname, fb_database *hp);
extern fb_addrec_clnt(fb_database *hp);
extern fb_b_addrec_clnt(fb_database *hp);
extern fb_b_delrec_clnt(fb_database *hp);
extern fb_build_svec(fb_varvec *v);
extern fb_bulkrec_begin_clnt(fb_database *hp, int fwait);
extern fb_bulkrec_end_clnt(fb_database *hp);
extern fb_chdir(char *path);
extern fb_chdir_clnt(char *path);
extern fb_clear_svec(void);
extern fb_clear_vec(fb_varvec *v);
extern fb_clnt_create(void);
extern fb_clnt_destroy(void);
extern fb_clnt_ping(void);
extern fb_clnt_timeout(int seconds);
extern fb_close(int fd);
extern fb_close_clnt(int fd);
extern fb_closedb_clnt(fb_database *dp);
extern fb_closeidx_clnt(fb_database *hp);
extern fb_cmd_clnt(char *command, fb_database *hp);
extern fb_concat_svec(char *s);
extern fb_creat(char *path, int mode);
extern fb_creat_clnt(char *path, int mode);
extern fb_createidx_clnt(char *iname, fb_database *hp);
extern fb_delrec_clnt(fb_database *hp);
extern fb_errno(void);
extern fb_errno_clnt(void);
extern fb_fcntl_cl_clnt(char *, long, long, int, int);
extern fb_fcntl_clnt(char *fname, int func, struct flock *fk);
extern fb_fetch_clnt(fb_field *k, char *s, fb_database *hp);
extern fb_fetchrec(long n, fb_database *hp);
extern fb_fetchrec_clnt(long n, fb_database *hp);
extern fb_fetchxrec(char *key, fb_database *hp);
extern fb_fetchxrec_clnt(char *key, fb_database *hp);
extern fb_firstxrec_clnt(fb_database *hp);
extern fb_fixfields(fb_database *hp, fb_varvec *rv);
extern fb_free_globals_clnt(void);
extern fb_free_svec(void);
extern fb_free_xdr_vec(fb_varvec *vp);
extern fb_freevec(fb_varvec *v);
extern fb_gethead_clnt(fb_database *hp);
extern fb_getirec_clnt(long n, fb_database *hp);
extern fb_getrec_clnt(long n, fb_database *hp);
extern fb_getxrec_clnt(char *key, fb_database *hp);
extern fb_lastxrec_clnt(fb_database *hp);
extern fb_lock_clnt(long mrec, fb_database *db, int fwait);
extern fb_lockd_clnt_create(void);
extern fb_lockd_clnt_destroy(void);
extern fb_lockf(int fd, int cmd, long size);
extern fb_lockf_clnt(int fd, int cmd, long size);
extern fb_mkdir(char *path, int mode);
extern fb_mkdir_clnt(char *path, int mode);
extern fb_mkserver_clnt(u_long *pnum);
extern fb_nextxrec_clnt(fb_database *hp);
extern fb_open(char *path, int flags);
extern fb_open_clnt(char *path, int flags);
extern fb_opendb_clnt(fb_database *dp, int mode, int ixflag, int ixoption);
extern fb_openidx_clnt(char *iname, fb_database *hp);
extern fb_prevxrec_clnt(fb_database *hp);
extern fb_put_autoindex_clnt(fb_database *hp);
extern fb_putrec_clnt(long n, fb_database *hp);
extern fb_read(int fd, char *buf, int nbytes);
extern fb_read_clnt(int fd, char *buf, int nbytes);
extern fb_rmdir(char *path);
extern fb_rmdir_clnt(char *path);
extern fb_s_lock_clnt(int fd, int fwait, char *fname);
extern fb_s_unlock_clnt(int fd, char *fname);
extern fb_set_autoindex_clnt(fb_database *hp);
extern fb_status_clnt(char *output);
extern fb_stopserver_clnt(void);
extern fb_store_clnt(fb_field *k, char *s, fb_database *hp);
extern fb_subidx_clnt(char *iname, fb_database *hp);
extern fb_symlink(char *name1, char *name2);
extern fb_symlink_clnt(char *name1, char *name2);
extern fb_sync_clnt(fb_database *hp);
extern fb_system_clnt(char *command, int rootperm);
extern fb_tr_lockd_clnt(char *buf, int max);
extern fb_tracevec(fb_varvec *v, char *s);
extern fb_tune_clnt(int loadfail);
extern fb_umask(int numask);
extern fb_umask_clnt(int numask);
extern fb_unlink(char *path);
extern fb_unlink_clnt(char *path);
extern fb_unlock_clnt(long mrec, fb_database *db);
extern fb_useidx_clnt(int n, fb_database *hp);
extern fb_varvec *fb_server_1(fb_varvec *argp, CLIENT *clnt);
extern fb_varvec *fb_tolockd(fb_varvec *vp);
extern fb_varvec *fb_toserver(fb_varvec *vp);
extern fb_varvec *fblockd_1(fb_varvec *argp, CLIENT *clnt);
extern fb_write(int fd, char *buf, int nbytes);
extern fb_write_clnt(int fd, char *buf, int nbytes);
extern off_t fb_lseek(int des, off_t offset, int whence);
extern off_t fb_lseek_clnt(int des, off_t offset, int whence);
extern time_t fb_getctime_clnt(char *path);

#endif /* FB_PROTOTYPES */

#endif
