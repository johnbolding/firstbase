/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getlogin.c,v 9.0 2001/01/09 02:57:06 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Db_getlogin_sid[] = "@(#) $Id: getlogin.c,v 9.0 2001/01/09 02:57:06 john Exp $";
#endif

#include <fb.h>
#include <dbutmp.h>

char dbname[8];		/* used by fb_getlogin() */

   char *fb_getlogin()	/* get cdb login name */
      {
         int t, f, r;
	 fb_utmp usave;

         t = TTYSLOT();
         if (t > 0 && (f = open(UTMP, 0)) >= 0) {
            if (lseek(f, (long)(t * sizeof(usave)), 0) < 0)
	       return((char *) NULL);
            r = read(f, (char *) &usave, sizeof(usave));
            close(f);
            if (r != sizeof(usave))
	       return((char *) NULL);
	    if (usave.ut_dbname[0] == NULL)
	       return((char *) NULL);
	    strcpy(dbname, usave.ut_dbname);
	    return(dbname);
	    }
	 return((char *) NULL);
      }
