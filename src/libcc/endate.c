/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: endate.c,v 9.0 2001/01/09 02:56:19 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Endate_sid[] = "@(#) $Id: endate.c,v 9.0 2001/01/09 02:56:19 john Exp $";
#endif

#include <fb.h>

/* 
 *  endate - format date p into encoded version YYMMDD. return p.
 */
 
   char *fb_endate(p)
      register char *p;
         
      {
         char s[7];

	 if (p == 0 || strlen(p) < 6)
	    return(p);
	 s[0] = p[4]; s[1] = p[5]; /* year */
	 s[2] = p[0]; s[3] = p[1]; /* month */
	 s[4] = p[2]; s[5] = p[3]; /* day */
	 s[6] = 0;
	 return(strcpy(p, s));
      }

/* 
 *  long_endate - format date p into encoded version CCYYMMDD. return p.
 */
 
   char *fb_long_endate(p)
      register char *p;
         
      {
         char s[9];

	 if (p == 0 || strlen(p) < 8)
	    return(p);
	 s[0] = p[4]; s[1] = p[5]; /* century */
	 s[2] = p[6]; s[3] = p[7]; /* year */
	 s[4] = p[0]; s[5] = p[1]; /* month */
	 s[6] = p[2]; s[7] = p[3]; /* day */
	 s[8] = 0;
	 return(strcpy(p, s));
      }

/*
 * fb_ameri_date - called if date is entered european and needs to be american
 */

   void fb_ameri_date(s)
      char *s;

      {
         char p, q;

         /* EUROPEAN is DDMMYY or DDMMYYYY - swap to MMDDYY or MMDDYYYY */
         p = s[0]; q = s[1];
         s[0] = s[2];
         s[1] = s[3];
         s[2] = p;
         s[3] = q;
      }
