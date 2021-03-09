/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dedate.c,v 9.0 2001/01/09 02:56:19 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dedate_sid[] = "@(#) $Id: dedate.c,v 9.0 2001/01/09 02:56:19 john Exp $";
#endif

#include <fb.h>

/* 
 *  dedate - decode encoded date p: from YYMMDD -> MMDDYY. return p.
 */
 
   char *fb_dedate(p)
      register char *p;
         
      {
         char s[7];
	 
	 if (p == 0 || strlen(p) < 6)
	    return(p);
	 s[0] = p[2]; s[1] = p[3]; /* month */
	 s[2] = p[4]; s[3] = p[5]; /* day */
	 s[4] = p[0]; s[5] = p[1]; /* year */
	 s[6] = 0;
	 strcpy(p, s);
	 return(p);
      }

/* 
 *  long_dedate - decode encoded date p: from CCYYMMDD -> MMDDYYCC. return p.
 */
 
   char *fb_long_dedate(p)
      register char *p;
         
      {
         char s[9];
	 
	 if (p == 0 || strlen(p) < 6)
	    return(p);
	 s[0] = p[4]; s[1] = p[5]; /* month */
	 s[2] = p[6]; s[3] = p[7]; /* day */
	 s[4] = p[0]; s[5] = p[1]; /* century */
	 s[6] = p[2]; s[7] = p[3]; /* year */
	 s[8] = 0;
	 strcpy(p, s);
	 return(p);
      }
