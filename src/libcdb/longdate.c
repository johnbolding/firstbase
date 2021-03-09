/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: longdate.c,v 9.0 2001/01/09 02:56:26 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Longdate_sid[] = "@(#) $Id: longdate.c,v 9.0 2001/01/09 02:56:26 john Exp $";
#endif

extern short cdb_centurymark;
extern char *cdb_centurybase;
extern char *cdb_centurynext;

#include <fb.h>
#include <fb_ext.h>

/*
 * longdate - generate a long date from standard Cdb date t into s. return s.
 *	MMDDYY -->> MMDDCCYY
 */

   char *fb_longdate(s, t)
      char *s, *t;
      
      {
         int i;
	 char *p;

	 if (t == 0 || strlen(t) < 6){
	    strcpy(s, t);
	    return(s);
	    }
         p = s;
	 for (i = 1; i <= 4; i++)
	    *p++ = *t++;
	 if (atoi(t) > cdb_centurymark)
	    strcpy(p, cdb_centurybase);
	 else
	    strcpy(p, cdb_centurynext);
	 p++; p++;
	 for (i = 1; i <= 2; i++)
	    *p++ = *t++;
	 *p = 0;  
	 return(s);
      }
