/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: noroot.c,v 9.0 2001/01/09 02:56:20 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Noroot_sid[] = "@(#) $Id: noroot.c,v 9.0 2001/01/09 02:56:20 john Exp $";
#endif

#include <fb.h>
#include <pwd.h>

/*
 * noroot - turn off root permissions.
 */

   fb_noroot()
      
      {
         struct passwd *syspw;

	 syspw = getpwuid(getuid());
	 if (setuid(syspw->pw_uid) < 0){
	    fprintf(stderr, "cannot reset uid\n");
	    exit(0);
	    }
	 if (setgid(syspw->pw_gid) < 0){
	    fprintf(stderr, "cannot reset gid\n");
	    exit(0);
	    }
      }
