/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: acct.c,v 9.1 2001/01/16 02:46:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Db_acct_sid[] = "@(#) $Id: acct.c,v 9.1 2001/01/16 02:46:51 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <pwd.h>
#include <dbacct.h>

static char FB_NODEV[] = {"/dev/tty??"};
extern char *cdb_pgm;

#if FB_PROTOTYPES
static storeacct(fb_acct *pa, int type, char *logname, char *dname,
   char *iname);
extern struct passwd *getpwuid(uid_t);
extern char *fb_getlogin(void);
extern char *ttyname(int filedes);
#else /* FB_PROTOTYPES */
static storeacct();
extern struct passwd *getpwuid();
extern char *fb_getlogin();
extern char *ttyname();
#endif /* FB_PROTOTYPES */

/*
 * provide acct trail hook. if not logged into cdb, hang up.
 */

   fb_acctlog(type, dname, iname)
      int type;
      char *dname, *iname;
      
      {
         char *logname;
	 fb_acct a;
	 int f;

         logname = fb_getlogin();
         if (logname == (char *) 0){
	    fprintf(stderr,
               "You must be logged into FirstBase to use this tool.\n");
	    fb_exit(0);
	    }
	 if (access(FB_ACCT, 0) >= 0)
	    storeacct(&a, type, logname, dname, iname);
	 if ((f = open(FB_ACCT, 1)) >= 0) {
	    fb_s_lock(f, FB_WAIT, FB_ACCT);
	    lseek(f, 0L, 2);
	    write(f, (char *) &a, sizeof(a));
	    fb_s_unlock(f, FB_ACCT);
	    close(f);
	    }
      }

   static storeacct(pa, type, logname, dname, iname)
      fb_acct *pa;
      int type;
      char *logname, *dname, *iname;

      {

         char *ttyn, *p, buf[FB_MAXNAME];
         struct passwd *syspw;

	 strncpy(pa->ac_dbname, logname, 7);
	 pa->ac_dbname[7] = NULL;
	 time(&(pa->ac_time));
	 ttyn = ttyname(0);
	 if (ttyn == 0)
	    ttyn = FB_NODEV;
	 p = strchr(ttyn+1, '/') + 1;
	 strncpy(pa->ac_line, p, 7);
	 pa->ac_line[7] = NULL;
	 syspw = getpwuid(getuid());
	 strncpy(pa->ac_uname, syspw->pw_name, 7);
	 pa->ac_uname[7] = NULL;
	 pa->ac_type = (short) type;
	 strncpy(pa->ac_toolname, cdb_pgm, 7);
	 pa->ac_toolname[7] = NULL;
	 fb_basename(buf, dname);
	 strncpy(pa->ac_dbase, buf, 11);
	 pa->ac_dbase[11] = NULL;
	 fb_basename(buf, iname);
	 strncpy(pa->ac_index, buf, 11);
	 pa->ac_index[11] = NULL;
      }
