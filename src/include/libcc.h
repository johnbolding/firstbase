/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: libcc.h,v 9.0 2001/01/09 02:56:13 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

#if !FB_PROTOTYPES
extern char *fb_dedate();
extern char *fb_endate();
extern char *fb_long_dedate();
extern char *fb_long_endate();
extern char *fb_makpwd();
extern char *fb_pad();
extern char *fb_rmlead();
extern char *fb_rootname();
extern char *fb_simpledate();
extern char *fb_tdate();
extern char *fb_trim();
extern char *fb_underscore();

extern void fb_ameri_date();
extern fb_countlines();
extern void fb_gethostname();
extern fb_getlin();
extern fb_gettoken();
extern fb_getword();
extern fb_julian();
extern fb_ln_end();
extern fb_ln_free();
extern fb_ln_get();
extern fb_ln_init();
extern fb_ln_load();
extern fb_nextline();
extern fb_nextread();
extern fb_nextwrite();
extern fb_noroot();
extern fb_r_end();
extern fb_r_init();
extern fb_r_rewind();
extern fb_str_is_blanks();
extern fb_strcmp();
extern fb_subline();
extern fb_w_end();
extern fb_w_init();
extern fb_w_write();
extern fb_w_writen();
extern void fb_wflush();
extern fb_x_end();
extern fb_x_init();
extern fb_x_nextline();
extern fb_x_nextread();
extern char *RE_COMP();
extern int RE_EXEC();

extern double ATOF();
#else /* FB_PROTOTYPES */

extern char *fb_dedate(char *p);
extern char *fb_endate(char *p);
extern char *fb_long_dedate(char *p);
extern char *fb_long_endate(char *p);
extern char *fb_makpwd(char *s, char *key, int siz);
extern char *fb_pad(char *s, char *t, int size);
extern char *fb_rmlead(char *p);
extern char *fb_rootname(char *r, char *p);
extern char *fb_simpledate(char *p, int m);
extern char *fb_tdate(char *p);
extern char *fb_trim(char *p);
extern char *fb_underscore(char *p, int rep_blank);

extern fb_ameri_date(char *s);
extern fb_countlines(char *s, int attr);
extern void fb_gethostname(char *name, int namelen);
extern fb_getlin(char *line, int fd, int maxline);
extern fb_gettoken(char *in, int i, char *out, int c);
extern fb_getword(char *in, int i, char *out);
extern fb_julian(int m, int d, int y);
extern fb_ln_end(void);
extern fb_ln_free(void);
extern fb_ln_get(int i, char *buf, int max);
extern fb_ln_init(int fd);
extern fb_ln_load(char *buf, int max);
extern fb_nextline(char *buf, int max);
extern fb_nextread(char *buf);
extern fb_nextwrite(int ch, char *buf);
extern fb_noroot(void);
extern fb_r_end(void);
extern fb_r_init(int fd);
extern fb_r_rewind(void);
extern fb_str_is_blanks(char *s);
extern fb_strcmp(char *a, char *b);
extern fb_subline(char *line, char *s, int snum, int attr);
extern fb_w_end(int n);
extern fb_w_init(int n, int a, int b);
extern fb_w_write(int ch, char *buf);
extern fb_w_writen(int ch, char *buf, int n);
extern fb_wflush(int n);
extern fb_x_end(int n);
extern fb_x_init(int n, int a, int b);
extern fb_x_nextline(int ch, char *buf, int max);
extern fb_x_nextread(int ch, char *buf);
extern char *RE_COMP(char *pat);
extern int RE_EXEC(char *s);
extern double ATOF(char *);
#endif /* FB_PROTOTYPES */
