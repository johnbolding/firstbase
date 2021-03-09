/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: libcdb.h,v 9.1 2004/12/31 04:35:22 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

#if !FB_PROTOTYPES
extern FILE *fb_mustfopen();
extern char *fb_basename();
extern char *fb_dirname();
extern char *fb_formdollar();
extern char *fb_formfield();
extern char *fb_getwd();
extern char *fb_key_ptr();
extern char *fb_longdate();
extern char *fb_malloc();
extern char *fb_mkstr();
extern char *fb_pathname();
extern char *fb_rjustify();
extern char *getwd();
extern fb_aline *fb_copyalist();
extern fb_aline *fb_makeline();
extern void fb_allow_int();
extern int fb_allsync();
extern int fb_catch_int();
extern int fb_catch_signal();
extern int fb_checkfields();
extern int fb_checkrange();
extern int fb_copyfile();
extern int fb_copytlist();
extern int fb_delete_line();
extern int fb_delete_token();
extern int fb_errorlog();
extern int fb_exit();
extern int fb_fhelp();
extern fb_field *fb_findfield();
extern int fb_findnfield();
extern int fb_free();
extern int fb_freealist();
extern int fb_freeline();
extern int fb_freepage();
extern int fb_freetokens();
extern int fb_freevnode();
extern int fb_gethostid();
extern void fb_help();
extern void fb_ignore_fpe();
extern int fb_initstack();
extern int fb_insert_line();
extern int fb_insert_page();
extern int fb_insert_token();
extern int fb_install();
extern int fb_jump_alarm();
extern int fb_largest_key();
extern int fb_lerror();
extern int fb_lxerror();
extern int fb_makenedit();
extern int fb_makess();
extern fb_mpage *fb_make_mpage();
extern int fb_mustopen();
extern int fb_mustwrite();
extern int fb_nodecimal();
extern int fb_nounders();
extern fb_page *fb_makepage();
extern int fb_push();
extern void fb_putdecimal();
extern int fb_release_signal();
extern int fb_restore_alarm();
extern int fb_rightmost_seq();
extern int fb_s_lock();
extern int fb_s_lock();
extern int fb_s_lock();
extern int fb_s_lock();
extern int fb_s_unlock();
extern int fb_s_unlock();
extern int fb_s_unlock();
extern int fb_s_unlock();
extern void fb_screenprint();
extern int fb_screrr();
extern int fb_sdict();
extern int fb_serror();
extern int fb_set_alarm();
extern int fb_settty();
extern int fb_sfatal();
extern int fb_sync();
extern int fb_sync_fd();
extern int fb_system();
extern int fb_testargs();
extern fb_token *fb_maketoken();
extern int fb_unders();
extern int fb_xerror();
extern long fb_key_eval();
extern long fb_key_record();
extern long fb_locate_rightmost_seq();
extern long fb_pop();
extern time_t fb_getctime();
extern void fb_onintr();
extern void fb_sigbus();
extern void fb_sigfpe();
extern void fb_sighup();
extern void fb_sigill();
extern void fb_siglost();
extern void fb_sigsegv();
extern void fb_sigsys();
extern void fb_sigterm();
extern void fb_usrlog_begin();
extern void fb_usrlog_end();
extern void fb_usrlog_msg();

/*extern void fb_sprintf();*/
extern void fb_sprintf(char *, char *, ...);

#else /* FB_PROTOTYPES */

extern FILE *fb_mustfopen(char *file, char *mode);
extern char *fb_basename(char *s, char *t);
extern char *fb_dirname(char *s, char *t);
extern char *fb_formdollar(char *s, char *t, int size);
extern char *fb_formfield(char *s, char *t, int f_type, int size);
extern char *fb_getwd(char *p);
extern char *fb_key_ptr(fb_bseq *bs);
extern char *fb_longdate(char *s, char *t);
extern char *fb_malloc(unsigned s);
extern char *fb_mkstr(char **p, char *s);
extern char *fb_pathname(char *fp, char *f);
extern char *fb_rjustify(char *s, char *t, int size, int type);
extern char *getwd(char *p);
extern fb_aline *fb_copyalist(fb_aline *a, int count, fb_mpage *p);
extern fb_aline *fb_makeline(void);
extern void fb_allow_int(void);
extern int fb_allsync(fb_database *db);
extern int fb_catch_int(void);
extern int fb_catch_signal(void);
extern int fb_checkfields(fb_database *db, int errflag);
extern int fb_checkrange(fb_field *fp, char *inp);
extern int fb_copyfile(char *fname, char *tname);
extern int fb_copytlist(fb_aline *a, fb_token *t);
extern int fb_delete_line(fb_aline *a, fb_mpage *p);
extern int fb_delete_token(fb_token *t, fb_aline *a);
extern int fb_errorlog(int e, char *line);
extern int fb_exit(int status);
extern int fb_fhelp(char *f);
extern fb_field *fb_findfield(char *s, fb_database *hdb);
extern int fb_findnfield(char *s, fb_database *hdb);
extern int fb_free(char *p);
extern int fb_freealist(fb_aline *a);
extern int fb_freeline(fb_aline *a);
extern int fb_freepage(fb_mpage *p);
extern int fb_freetokens(fb_token *t);
extern int fb_freevnode(void);
extern int fb_gethostid(char *buf);
extern int fb_help(char *s, fb_database *hp);
extern void fb_ignore_fpe(void);
extern int fb_initstack(void);
extern int fb_insert_line(fb_aline *a, fb_aline *aa, fb_mpage *p);
extern int fb_insert_page(fb_mpage *p, fb_mpage *ap);
extern int fb_insert_token(fb_token *t, fb_token *at, fb_aline *a);
extern int fb_install(int, int, char *, int, int, int, int, int);
extern int fb_jump_alarm(void);
extern int fb_largest_key(char *, int, long, int, fb_bidx *,fb_bseq *);
extern int fb_lerror(int e, char *p, char *q);
extern int fb_lxerror(int e, char *p, char *q);
extern int fb_makenedit(void);
extern int fb_makess(char *buf, int type, int size);
extern fb_mpage *fb_make_mpage(void);
extern int fb_mustopen(char *file, int mode);
extern int fb_mustwrite(int fd, char *s);
extern int fb_nodecimal(char *buf);
extern int fb_nounders(fb_field *k);
extern fb_page *fb_makepage(void);
extern int fb_push(long v);
extern int fb_putdecimal(char *buf);
extern int fb_release_signal(void);
extern int fb_restore_alarm(void);
extern int fb_rightmost_seq(char *p, int ksize, fb_bseq *bs);
extern int fb_s_lock(int fd, int fwait, char *fname);
extern int fb_s_lock(int fd, int fwait, char *fname);
extern int fb_s_lock(int fd, int fwait, char *fname);
extern int fb_s_lock(int fd, int fwait, char *fname);
extern int fb_s_unlock(int fd, char *fname);
extern int fb_s_unlock(int fd, char *fname);
extern int fb_s_unlock(int fd, char *fname);
extern int fb_s_unlock(int fd, char *fname);
extern int fb_screenprint(char *f);
extern int fb_screrr(char *s);
extern int fb_sdict(fb_field *fp, FILE *fs, int doauto);
extern int fb_serror(int e, char *p, char *q);
extern int fb_set_alarm(long locktime, void (*nhandler)(int disp));
extern int fb_settty(int m);
extern int fb_sfatal(int e, char *s);
extern int fb_sync(fb_database *db);
extern int fb_sync_fd(int fd);
extern int fb_system(char *s, int rootperm);
extern int fb_testargs(int argc, char **argv, char *s);
extern fb_token *fb_maketoken(void);
extern int fb_unders(fb_field *k);
extern int fb_xerror(int e, char *p, char *q);
extern long fb_key_eval(fb_bseq *bs);
extern long fb_key_record(char *s, int len);
extern long fb_locate_rightmost_seq(long, int, fb_bidx *, fb_bseq *);
extern long fb_pop(void);
extern time_t fb_getctime(char *path);
extern void fb_onintr(int disp);
extern void fb_sigbus(int disp);
extern void fb_sigfpe(int disp);
extern void fb_sighup(int disp);
extern void fb_sigill(int disp);
extern void fb_siglost(int disp);
extern void fb_sigsegv(int disp);
extern void fb_sigsys(int disp);
extern void fb_sigterm(int disp);
extern void fb_usrlog_begin(int argc, char **argv);
extern void fb_usrlog_end(void);
extern void fb_usrlog_msg(char *s);

/* prototypes for these only work with stdargs.h, not with varargs.h */
/* extern void fb_sprintf(char *s, ...); */

#endif /* FB_PROTOTYPES */


