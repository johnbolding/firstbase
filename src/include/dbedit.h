/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbedit.h,v 9.0 2001/01/09 02:56:08 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

#include <fb.h>
#include <fb_ext.h>

/*
 * dbedit.h - included by dbedit modules for global definitions
 */
 

#define	REDRAW		FB_AOK	/* to flag redrawing of screen */
#define NOREDRAW	-1	/* to flag no redrawing of screen */

extern char
     com[],			/* command storage area */
     msg[],			/* message for scrhlp and scrstat */
     mode;			/* for autoadd mode */

extern short
     globaladd,			/* set to 1 for globaladd mode */
     scanner,			/* set to 1 if dbscan, else 0 */
     st,			/* status marker */
     pindx,			/* level of index */
     simple_pindx,		/* simple fld pointer when no index used */
     def,			/* type of default ? */
     autodef;			/* for auto default feature */
     
extern long 
     rec,			/* record marker */
     oldrec,			/* last record */
     irec[],			/* to mark index record levels */
     ibase[];			/* to mark index base levels */
     
extern int
     dot;			/* current location of index pointer */

extern fb_database *hp;		/* to tie in the default db */
extern fb_field **cdb_sp;	/* actual screen (displayed) fields */
extern short int cdb_sfields;	/* max screen fields */

#if !FB_PROTOTYPES

extern void db_checkformula();
extern int db_display();
extern fb_local_display();

extern addfield();
extern fb_delete();
extern void dfield();
extern docmd();
extern void edit();
extern edit_add();
extern editor();
extern edit_field();
extern newrec();
extern void fb_password();
extern void fb_percentage();
extern void db_putfield();
extern scanput();
extern db_scanset();
extern showauto();
extern fb_undelete();
extern fb_visual();
extern void fb_xcommand();
extern fb_longinput();
extern void usage();

#else /* FB_PROTOTYPES */
extern void db_checkformula(int, int);
extern int db_display(int);
extern int fb_local_display(int);

extern addfield(int fld, int *top);
extern fb_delete(fb_field *k);
extern void dfield(char *p);
extern docmd(void);
extern void edit(int top);
extern edit_add(int fld, int top);
extern editor(int argc, char **argv);
extern edit_field(int fld, int top);
extern newrec(void);
extern void fb_password(char *dbase);
extern void fb_percentage(int f, int nmax);
extern void db_putfield(int fld, fb_field *k, char *s, int row);
extern scanput(fb_field *tip, char *hlp, int disp_st);
extern db_scanset(fb_field **akp, int top, int maxc);
extern showauto(fb_database *hp);
extern fb_undelete(fb_field *k);
extern fb_visual(char *p, int readonly, int vis);
extern void fb_xcommand(void);
extern fb_longinput(fb_field *f, char *line, int n, int new);
extern void usage(void);
#endif /* FB_PROTOTYPES */
