/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: libedit.h,v 9.0 2001/01/09 02:56:14 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

#if !FB_PROTOTYPES

extern fb_cnode *fb_makecnode();
extern int fb_cgi_read();
extern int fb_check_abslink();
extern int fb_check_valuelink();
extern int fb_choiceinput();
extern int fb_delete();
extern int fb_longinput();
extern int fb_put();
extern int fb_test_tree();
extern int fb_undelete();
extern int fb_vedit();
extern int fb_visual();
extern int showauto();
extern long fb_scandb();
extern long fb_scanfor();
extern long fb_scantree();
extern void fb_addcnode();
extern void fb_addcomment();
extern void fb_cgi_checkbox();
extern void fb_cgi_echo();
extern void fb_cgi_foreach();
extern void fb_cgi_init();
extern void fb_cgi_value();
extern void fb_checkformula();
extern void fb_clrpage();
extern void fb_cshell();
extern void fb_d_dfield();
extern void fb_dfield();
extern void fb_display();
extern void fb_freeccom();
extern void fb_freecnode();
extern void fb_ghostfield();
extern void fb_initcnode();
extern void fb_initcomment();
extern int fb_macroinput();
extern void fb_password();
extern void fb_percentage();
extern void fb_putfield();
extern void fb_scandb_query();
extern void fb_scanfor_query();
extern void fb_scanset();
extern void fb_scantree_query();
extern void fb_showcnode();
extern void fb_showcomment();
extern void fb_xcommand();
extern void winsize();

#else  /* FB_PROTOTYPES */

extern fb_cnode *fb_makecnode(void);
extern int fb_cgi_read(void);
extern int fb_check_abslink(void);
extern int fb_check_valuelink(void);
extern int fb_choiceinput(fb_field *, char *, int, int, int, int, int);
extern int fb_delete(fb_field *k);
extern int fb_longinput(fb_field *f, char *line, int n, int new);
extern int fb_put(char *p);
extern int fb_test_tree(fb_database *db);
extern int fb_undelete(fb_field *k);
extern int fb_vedit(int readonly, int vis, fb_field *f);
extern int fb_visual(char *p, int readonly, int vis);
extern int showauto(fb_database *hp);
extern long fb_scandb(char *key, int tdef);
extern long fb_scanfor(char *key, int tdef);
extern long fb_scantree(char *key, int tdef);
extern void fb_addcnode(char *val);
extern void fb_addcomment(char *val);
extern void fb_cgi_checkbox(int (*f)(char *name, char *val));
extern void fb_cgi_echo(void);
extern void fb_cgi_foreach(int (*)(char *name, char *val));
extern void fb_cgi_init(void);
extern void fb_cgi_value(char *value, char *fn);
extern void fb_checkformula(int fld);
extern void fb_clrpage(void);
extern void fb_cshell(char *com);
extern void fb_d_dfield(fb_field *f);
extern void fb_dfield(char *p);
extern void fb_display(int pflag);
extern void fb_freeccom(void);
extern void fb_freecnode(void);
extern void fb_ghostfield(fb_node *n, int revflag);
extern void fb_initcnode(void);
extern void fb_initcomment(FILE *cfs, char *helpfile);
extern int fb_macroinput(fb_field *, fb_node *, int, int, int, int, int);
extern void fb_password(char *dbase);
extern void fb_percentage(int f, int nmax);
extern void fb_putfield(fb_node *n, fb_field *k, char *s);
extern void fb_scandb_query(void);
extern void fb_scanfor_query(void);
extern void fb_scanset(void);
extern void fb_scantree_query(void);
extern void fb_showcnode(int sug_col);
extern void fb_showcomment(void);
extern void fb_xcommand(void);
extern void winsize(void);

#endif /* FB_PROTOTYPES */
