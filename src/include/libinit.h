/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: libinit.h,v 9.0 2001/01/09 02:56:14 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* for calls to libinit routines */
#define FB_NODB		0		/* for getargs(argc, argv, XXX) */
#define FB_ALLOCDB	1

#define FB_NOINDEX	0		/* opendb(db, mode, YYY, ZZZ) */
#define FB_WITHINDEX	1
#define FB_ALLINDEX	2

#define FB_MUST_INDEX	0		/* ZZZ */
#define FB_OPTIONAL_INDEX	1
#define FB_MAYBE_OPTIONAL_INDEX 2

#if !FB_PROTOTYPES

extern fb_acctlog();
extern fb_addidx();
extern fb_assure_slash();
extern void fb_autoigen();
extern fb_autoindex *fb_ixalloc();
extern fb_bidx *fb_idx_alloc();
extern fb_bseq *fb_seq_alloc();
extern fb_checktotals();
extern fb_closedb();
extern fb_closeidx();
extern void fb_closeix_btree();
extern fb_createidx();
extern fb_database *fb_dballoc();
extern fb_dbargs();
extern fb_describe_bindings();
extern fb_emptyi_dict();
extern fb_ender();
extern fb_field *fb_makefield();
extern fb_free_globals();
extern fb_free_keyboard();
extern fb_get_vdict();
extern fb_getargs();
extern fb_getauto();
extern fb_getd_data();
extern fb_getd_dict();
extern fb_getdef();
extern fb_geti_data();
extern fb_geti_dict();
extern fb_getlink();
extern fb_gets_dict();
extern fb_homefile();
extern void fb_idx_clear();
extern void fb_idx_copy();
extern void fb_idx_free();
extern fb_init_keyboard();
extern fb_initlink();
extern fb_interpret_esig();
extern fb_key_str();
extern fb_magic_environ();
extern fb_make_dbase();
extern fb_nullall();
extern fb_open_auto();
extern fb_opendb();
extern fb_openidx();
extern fb_parse_vdict();
extern fb_parselink();
extern fb_readmrg();
extern fb_s_getlink();
extern void fb_seq_clear();
extern fb_seq_convert();
extern void fb_seq_copy();
extern void fb_seq_free();
extern fb_set_loadfail();
extern fb_setup();
extern fb_setup_argv();
extern fb_setup_exit();
extern fb_subidx();
extern fb_test_help();
extern fb_test_keyboard();
extern fb_trace_keyboard();
extern fb_useidx();
extern fb_vfwd();
extern fb_writeable_homefile();

#else /* FB_PROTOTYPES */
extern fb_acctlog(int type, char *dname, char *iname);
extern fb_addidx(char *iname, fb_database *dp);
extern fb_assure_slash(char *fname);
extern fb_autoigen(fb_database *hdb);
extern fb_autoindex *fb_ixalloc(void);
extern fb_bidx *fb_idx_alloc(char *dindex, int isiz);
extern fb_bseq *fb_seq_alloc(char *dindex, int isiz);
extern fb_checktotals(fb_database *hdb);
extern fb_closedb(fb_database *dp);
extern fb_closeidx(fb_database *dp);
extern fb_closeix_btree(fb_autoindex *ix, fb_database *dp);
extern fb_createidx(char *iname, fb_database *dp);
extern fb_database *fb_dballoc(void);
extern fb_dbargs(char *fname, char *iname, fb_database *dhp);
extern fb_describe_bindings(FILE *fs);
extern fb_emptyi_dict(fb_database *dhp);
extern fb_ender(void);
extern fb_field *fb_makefield(void);
extern fb_free_globals(void);
extern fb_free_keyboard(void);
extern fb_get_vdict(int argc, char **argv);
extern fb_getargs(int argc, char **argv, int dbflag);
extern fb_getauto(fb_database *dp, int);
extern fb_getd_data(int mode, fb_database *dhp);
extern fb_getd_dict(fb_database *hdb);
extern fb_getdef(fb_database *hdb);
extern fb_geti_data(int mode, fb_database *dhp);
extern fb_geti_dict(int mode, fb_database *dhp);
extern fb_getlink(fb_database *dp);
extern fb_gets_dict(int argc, char **argv);
extern fb_homefile(char **addr, char *base);
extern fb_idx_clear(fb_bidx *bi);
extern void fb_idx_copy(fb_bidx *bt, fb_bidx *bf);
extern void fb_idx_free(fb_bidx *bi);
extern fb_init_keyboard(void);
extern fb_initlink(fb_database *dp);
extern fb_interpret_esig(int k);
extern fb_key_str(char *buf, int esig);
extern fb_magic_environ(void);
extern fb_make_dbase(fb_database *dhp);
extern fb_nullall(void);
extern fb_open_auto(fb_database *dp, fb_autoindex *ax, char *seq, int);
extern fb_opendb(fb_database *dp, int mode, int ixflag, int ixoption);
extern fb_openidx(char *iname, fb_database *dp);
extern fb_parse_vdict(char *fname);
extern fb_parselink(char *, char *, char *, char *, char *, long *);
extern fb_readmrg(char *filen);
extern fb_s_getlink(fb_link *ak);
extern fb_seq_clear(fb_bseq *bs);
extern fb_seq_convert(char *buf, char *seq, int len);
extern void fb_seq_copy(fb_bseq *bt, fb_bseq *bf);
extern void fb_seq_free(fb_bseq *bs);
extern fb_set_loadfail(int flag);
extern fb_setup(void);
extern fb_setup_argv(int argc, char **argv);
extern fb_setup_exit(void);
extern fb_subidx(char *iname, fb_database *dp);
extern fb_test_help(void);
extern fb_test_keyboard(char *s, int len);
extern fb_trace_keyboard(void);
extern fb_useidx(int i, fb_database *dp);
extern fb_vfwd(void);
extern fb_writeable_homefile(char **addr, char *base);

#endif /* !FB_PROTOTYPES */
