/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: rmlead.c,v 9.0 2001/01/09 02:56:21 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Rmlead_sid[] = "@(#) $Id: rmlead.c,v 9.0 2001/01/09 02:56:21 john Exp $";
#endif

#include <fb.h>

/* 
 *  rmlead - remove leading blanks from p. return p.
 */
 
   char *fb_rmlead(p)
      register char *p;
      
      {
         register char *q, *r;
         
         for (q = r = p; *r && (*r == ' ' || *r == '\t'); r++)
	    ;
	 for (; *r; *q++ = *r++)
	    ;
	 *q = 0;
         return(p);
      }
