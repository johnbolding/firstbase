/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: trim.c,v 9.0 2001/01/09 02:56:21 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Trim_sid[] = "@(#) $Id: trim.c,v 9.0 2001/01/09 02:56:21 john Exp $";
#endif

#include <fb.h>

/* 
 *  trim - trim trailing white space from p. return p.
 */
 
   char *fb_trim(p)
      register char *p;
      
      {
         register char *q, *r;
         
	 if (!(*p))
	    return(p);
	 for (q = p; *q; q++)
	    ;
         for(r = q - 1; q != p && (*r == ' ' || *r == '\t'); q--, r--)
	    ;
	 *q = 0;
         return(p);
      }
