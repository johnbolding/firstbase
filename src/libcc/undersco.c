/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: undersco.c,v 9.0 2001/01/09 02:56:22 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Underscore_sid[] = "@(#) $Id: undersco.c,v 9.0 2001/01/09 02:56:22 john Exp $";
#endif

#include <fb.h>

/* 
 *  underscore - replace p's underscores/blanks with blanks/underscores
 *     if rep_blank == 1, replace blank with underscore.
 *     if rep_blank == 0, replace unerscore with blank.
 *     return p.
 */

   char *fb_underscore(p, rep_blank)
      char *p;
      int rep_blank;
      
      {
         register char *f, *t;
         char c, r;
	 
	 c = (rep_blank == 0) ? '_' : ' ';
	 r = (rep_blank == 0) ? ' ' : '_';
	 for (f = t = p; *f; f++, t++){
	    if (*f == '\\' && *(f+1) == c)
	       f++;
	    else if (*f == c)
	       *f = r;
	    *t = *f;
	    }
	 *t = '\0';
	 return(p);
      }
