/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getgid.c,v 9.0 2001/01/09 02:57:06 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Db_getgid_sid[] = "@(#) $Id: getgid.c,v 9.0 2001/01/09 02:57:06 john Exp $";
#endif

#include <fb.h>
#include <dbpwd.h>
#include <dbutmp.h>

extern short int cdb_no_acct;

   fb_getgid()		/* return cdb group id of logged in slot */
      {
         int t, f;
	 fb_utmp usave;
         fb_passwd *pw;

         if (cdb_no_acct)
            return(0);
         t = TTYSLOT();
         if (t > 0 && (f = open(UTMP, 0)) >= 0) {
            lseek(f, (long)(t * sizeof(usave)), 0);
            read(f, (char *) &usave, sizeof(usave));
            close(f);
	    if ((pw = fb_getpwnam(usave.ut_dbname)) == NULL)
	       return(-1);
	    return(pw->dbpw_gid);
	    }
	 return(-1);
      }
