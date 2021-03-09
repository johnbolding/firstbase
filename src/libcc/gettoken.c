/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: gettoken.c,v 9.0 2001/01/09 02:56:19 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Gettoken_sid[] = "@(#) $Id: gettoken.c,v 9.0 2001/01/09 02:56:19 john Exp $";
#endif

#include <fb.h>
#if FB_PROTOTYPES
static int inrange(int c, int xc);
#else /* FB_PROTOTYPES */
static int inrange();
#endif /* FB_PROTOTYPES */

/* 
 *  gettoken - get the next token from line i...this is not
 *     the same as getword as this returns white space as well.
 *     c is also a skippable character for words, allowing embedded
 *     underscores for instance.
 *     Also needs a 1 to get it started, ala getword()
 */
 
   int fb_gettoken(in, i, out, c)
      register char *in, *out, c;
      int i;
   
      {
	 in = in + i - 1; 
	 if (*in == ' ' || *in == '\n' || *in == '\t'){
	    while (*in == ' ' || *in == '\n' || *in == '\t'){
	       *out++ = *in++;
	       i++;
	       }
	    }
	 else if (*in){
	    *out++ = *in;
	    i++;
	    if (inrange(*in, c)){
	       in++;
	       while (inrange(*in, c)){
		  *out++ = *in++;
		  i++;
		  }
	       }
	    }
	 else
	    i = 0;
	 *out = 0;
	 return(i);
      }

/* 
 *  inrange - check if c is inrange with valid character string, or == xc.
 */
 
   static int inrange(c, xc)
      int c, xc;
      
      {
	 if (c &&
	       ((c >= 'A' && c <= 'Z') ||
	       (c >= 'a' && c <= 'z') ||
	       (c >= '0' && c <= '9') ||
	       (c == xc)))
	    return(1);
	 else
	    return(0);
      }
