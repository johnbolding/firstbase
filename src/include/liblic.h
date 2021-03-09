/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: liblic.h,v 9.0 2001/01/09 02:56:14 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

#if !FB_PROTOTYPES

extern double fb_xlm();
extern double fb_xpm();
extern double fb_xsm();
extern double fb_xtm();
extern fb_addlink_queue();
extern fb_addto_queue();
extern fb_cf();
extern fb_check_queue();
extern fb_compact_queue();
extern fb_count_queue();
extern fb_create_queue();
extern fb_ef();
extern fb_endfw();
extern fb_endlwent();
extern fb_find_queue();
extern fb_fixfw();
extern fb_freelwd();
extern fb_fwclear();
extern fb_fwlog();
extern fb_fwnrecs();
extern fb_getfwavail();
extern fb_getfwforq();
extern fb_getfwtrylic();
extern fb_getfwvfy();
extern fb_getqhead();
extern fb_init_random();
extern fb_ishex();
extern fb_list_queue();
extern fb_loadfwd();
extern fb_loadlwd();
extern fb_lock_queue();
/*extern fb_lwd *fb_getlw_hostid();*/
/*extern fb_lwd *fb_getlwent();*/
extern fb_make_edate();
extern fb_next_queue();
extern fb_pinghost();
extern fb_putlic();
/*extern fb_qrec *fb_alloc_qnode();*/
extern fb_remove_queue();
extern fb_rmfrom_queue();
extern fb_setlic();
extern fb_setlwent();
extern fb_shortdate();
extern fb_single_fixfw();
extern fb_stringfw();
extern fb_testfw();
extern fb_unlock_queue();
extern long fb_host_to_long();
extern unsigned int fb_fgh();
extern void fb_lw_tostring();
extern void fb_mmg();
extern void fb_rotate();
extern void fb_tonum();

#else /* FB_PROTOTYPES */

extern double fb_xlm(void);
extern double fb_xpm(void);
extern double fb_xsm(void);
extern double fb_xtm(void);
extern fb_addlink_queue(fb_qrec *q);
extern fb_addto_queue(int uid, int gid, char *user, char *host, int lockflag);
extern fb_cf(char *v, char *q, int n);
extern fb_check_queue(int uid, int gid, char *user, char *host);
extern fb_compact_queue(void);
extern fb_count_queue(void);
extern fb_create_queue(void);
extern fb_ef(char *q, char *v, int n);
extern fb_endfw(void);
extern fb_endlwent(void);
extern fb_find_queue(fb_qrec *q, int uid, int gid);
extern fb_fixfw(void);
extern fb_freelwd(fb_lwd *p);
extern fb_fwclear(void);
extern fb_fwlog(int action, char *user);
extern fb_fwnrecs(int fd);
extern fb_getfwavail(char *res, int uid, int gid);
extern fb_getfwforq(void);
extern fb_getfwtrylic(char *res, char *user, char *host);
extern fb_getfwvfy(char *res, char *user);
extern fb_getqhead(int *nrecs, int *top, int *bottom);
extern fb_init_random(void);
extern fb_ishex(char *s);
extern fb_list_queue(char *s, int qlim);
extern fb_loadfwd(fb_floatwd *fp, char *buf);
extern fb_loadlwd(char *buf, fb_lwd *p);
extern fb_lock_queue(void);
extern fb_lwd *fb_getlw_hostid(char *id);
extern fb_lwd *fb_getlwent(void);
extern fb_make_edate(char *d);
extern fb_next_queue(int i);
extern fb_pinghost(char *s);
extern fb_putlic(char *buf);
extern fb_qrec *fb_alloc_qnode(void);
extern fb_remove_queue(void);
extern fb_rmfrom_queue(int uid, int gid, char *user, char *host);
extern fb_setlic(char *buf);
extern fb_setlwent(void);
extern fb_shortdate(char *d);
extern fb_single_fixfw(int n_fid);
extern fb_stringfw(char *s, fb_floatwd *fp);
extern fb_testfw(void);
extern fb_unlock_queue(void);
extern long fb_host_to_long(char *hostid);
extern unsigned int fb_fgh(void);
extern void fb_lw_tostring(char *s, double lwd, int m, int d, int y);
extern void fb_mmg(double *, char *, char *, char *, char *);
extern void fb_rotate(char *s, int d);
extern void fb_tonum(char *pd, int *pfm, int *pfd, int *pfy);

#endif /* FB_PROTOTYPES */
