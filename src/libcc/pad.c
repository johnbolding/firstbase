/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: pad.c,v 9.0 2001/01/09 02:56:20 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Pad_sid[] = "@(#) $Id: pad.c,v 9.0 2001/01/09 02:56:20 john Exp $";
#endif

#include <fb.h>

/* 
 *  pad - pad t to size and left justify/truncate into s. return s.
 */
 
   char *fb_pad(s, t, size)
      register char *s, *t;
      int size;
      
      {
         register int sz;
	 char *q;

         for (q = s, sz = 0; sz < size && *t; *q++ = *t++, sz++)
	    ;
	 for (; sz < size; *q++ = ' ', sz++)
	    ;
	 *q = 0;
	 return(s);
      }
