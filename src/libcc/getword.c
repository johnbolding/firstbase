/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getword.c,v 9.0 2001/01/09 02:56:19 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getword_sid[] = "@(#) $Id: getword.c,v 9.0 2001/01/09 02:56:19 john Exp $";
#endif

#include <fb.h>

/* 
 *  getword - get the next non-white word from line at i. 
 *     due to my Y'ness at the time, i made this procedure
 *     compensate for the zero-based arrays of C.
 *     So pass this silly thing a 1 to get it started.
 */
 
   fb_getword(in, i, out)
      register char *in, *out;
      register int i;
   
      {
	 in = in + i - 1; 
	 while (*in == ' ' || *in == '\n' || *in == '\t'){
	    in++;
	    i++;
	 }
	 if (*in)
	    while (*in && *in != ' ' && *in != '\n' && *in != '\t'){
	       *out++ = *in++;
	       i++;
	       }
	 else
	    i = 0;
	 *out = 0;
	 return(i);
      }
