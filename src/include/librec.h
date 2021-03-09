/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: librec.h,v 9.0 2001/01/09 02:56:14 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

#if !FB_PROTOTYPES

extern fb_addrec();
extern fb_b_addrec();
extern fb_b_delrec();
extern fb_blockeach();
extern fb_bootmap();
extern fb_btree_delete();
extern fb_btree_insert();
extern fb_bulkrec_begin();
extern fb_bulkrec_end();
extern fb_check_free_idx();
extern fb_check_free_seq();
extern fb_checkauto();
extern fb_checklog();
extern fb_clear_autoindex();
extern fb_currentxrec();
extern fb_delidx();
extern fb_delrec();
extern fb_fetch();
extern fb_ffetch();
extern fb_fgetrec();
extern fb_firstovxrec();
extern fb_firstxrec();
extern fb_foreach();
extern fb_forxeach();
extern fb_fputrec();
extern fb_get_idx_head();
extern fb_get_seq_head();
extern fb_getbxhead();
extern fb_getformula();
extern fb_gethead();
extern fb_getirec();
extern fb_getmap();
extern fb_getrec();
extern fb_getseq();
extern fb_getxhead();
extern fb_getxrec();
extern fb_idx_getrec();
extern fb_idx_putrec();
extern void fb_initlock();
extern void fb_initlog();
extern fb_isdeleted();
extern fb_lastovxrec();
extern fb_lastxrec();
extern fb_lock();
extern fb_lock_head();
extern fb_nextxrec();
extern fb_nobuf();
extern fb_prevxrec();
extern fb_put_autoindex();
extern fb_put_free_bs();
extern fb_put_idx_head();
extern fb_put_seq_head();
extern fb_putbxhead ();
extern fb_putfree();
extern fb_puthead();
extern void fb_putlog();
extern fb_putmap();
extern fb_putmode();
extern fb_putrec();
extern fb_putseq();
extern fb_putxhead();
extern fb_recmode();
extern fb_record_permission();
extern void fb_search_count();
extern fb_seq_getrec();
extern fb_seq_putrec();
extern fb_set_autoindex();
extern fb_setdirty();
extern fb_skipxhead();
extern fb_store();
extern fb_unlock();
extern fb_unlock_head();

extern long fb_btree_search();
extern long fb_getfree();
extern long fb_lastmatch();
extern long fb_lastmatch_tree();
extern long fb_megasearch();
extern long fb_search();
extern long fb_searchtree();

#else /* FB_PROTOTYPES */

extern fb_addrec(fb_database *dp);
extern fb_b_addrec(fb_database *dp);
extern fb_b_delrec(fb_database *dp);
extern fb_blockeach(fb_database *dp, int (f)(fb_database *dp));
extern fb_bootmap(int mfd);
extern fb_btree_delete(char *delkey, long rec, fb_bidx *bi, fb_bseq *bs);
extern fb_btree_insert(fb_autoindex *ix, char *key, fb_database *hp);
extern fb_bulkrec_begin(fb_database *dp, int fwait);
extern fb_bulkrec_end(fb_database *dp);
extern fb_check_free_idx(fb_bidx *bi);
extern fb_check_free_seq(fb_bseq *bs);
extern fb_checkauto(fb_database *hp);
extern fb_checklog(fb_database *hp);
extern fb_clear_autoindex(fb_database *hp);
extern fb_currentxrec(fb_database *dp);
extern fb_delidx(fb_database *hp, long rec);
extern fb_delrec(fb_database *dp);
extern fb_fetch(fb_field *k, char *s, fb_database *dp);
extern fb_ffetch(fb_field *k, char *s, char *buf, fb_database *dp);
extern fb_fgetrec(long n, int fd, int size, char *buf, int header);
extern fb_firstovxrec(fb_database *dp);
extern fb_firstxrec(fb_database *dp);
extern fb_foreach(fb_database *dp, int (f)(fb_database *dp));
extern fb_forxeach(fb_database *dp, int (f)(fb_database *dp));
extern fb_fputrec(long n, int fd, int size, char *buf, int header);
extern fb_get_idx_head(fb_bidx *bi);
extern fb_get_seq_head(fb_bseq *bs);
extern fb_getbxhead(int fd, long *v1, long *v2, long *v3, long *v4);
extern fb_getformula(fb_field *, char *, char *, int, fb_database *);
extern fb_gethead(fb_database *dp);
extern fb_getirec(long rec, fb_database *dp);
extern fb_getmap(int, long, long *, long *, long *, long *rlen);
extern fb_getrec(long n, fb_database *hp);
extern fb_getseq(int fd);
extern fb_getxhead(int fd, long *v1, long *v2);
extern fb_getxrec(char *s, fb_database *dp);
extern fb_idx_getrec(long n, fb_bidx *bi);
extern fb_idx_putrec(long n, fb_bidx *bi);
extern void fb_initlock(int ig, fb_database *db);
extern void fb_initlog(fb_database *hp);
extern fb_isdeleted(fb_database *dp);
extern fb_lastovxrec(fb_database *dp);
extern fb_lastxrec(fb_database *dp);
extern fb_lock(long mrec, fb_database *db, int fwait);
extern fb_lock_head(fb_database *d);
extern fb_nextxrec(fb_database *dp);
extern fb_nobuf(fb_database *db);
extern fb_prevxrec(fb_database *dp);
extern fb_put_autoindex(fb_database *hp);
extern fb_put_free_bs(fb_bseq *bs);
extern fb_put_idx_head(fb_bidx *bi);
extern fb_put_seq_head(fb_bseq *bs);
extern fb_putbxhead (int fd, long v1, long v2, long v3, long v4);
extern fb_putfree(int fd, int mfd, long rpos, long rlen, fb_database *hp);
extern fb_puthead(int fd, long reccnt, long delcnt);
extern void fb_putlog(fb_database *hp);
extern fb_putmap(int mfd, long n, long avail, long rpos, long rlen);
extern fb_putmode(int fd, int uid, int gid, char *mode);
extern fb_putrec(long n, fb_database *hp);
extern fb_putseq(int fd);
extern fb_putxhead(int fd, long v1, long v2);
extern fb_recmode(fb_database *db, int delc, int uid, int gid, char *mode);
extern fb_record_permission(fb_database *db, int type);
extern void fb_search_count(fb_database *, long *, long *, char *, int, long);
extern fb_seq_getrec(long n, fb_bseq *bs);
extern fb_seq_putrec(long n, fb_bseq *bs);
extern fb_set_autoindex(fb_database *hp);
extern fb_setdirty(fb_database *dp, int n);
extern fb_skipxhead(int fd);
extern fb_store(fb_field *k, char *s, fb_database *hp);
extern fb_unlock(long mrec, fb_database *db);
extern fb_unlock_head(fb_database *d);

extern long fb_btree_search(char *key, fb_bidx *bi, fb_bseq *bs);
extern long fb_getfree(int fd, int mfd, long size, long *rlen);
extern long fb_lastmatch(int, char *, long, long, int, char *);
extern long fb_lastmatch_tree(char *key, fb_bseq *bs);
extern long fb_megasearch(int, char *, int, long,long, long, int, int, char *);
extern long fb_search(int, char *, int, long, long, long, int, int, char *);
extern long fb_searchtree(char *key, fb_bidx *bi, fb_bseq *bs);

#endif /* FB_PROTOTYPES */
