/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getlin.c,v 9.0 2001/01/09 02:56:19 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getlin_sid[] = "@(#) $Id: getlin.c,v 9.0 2001/01/09 02:56:19 john Exp $";
#endif

#include <fb.h>

#define MAXLINE	120

/* 
 *  getlin - get a line from file fd
 */
 
   fb_getlin(line, fd, maxline)
      register int fd, maxline;
      register char *line;
   
      {
	 register int i, s = 0;
	 char c;
   
	 if (maxline == 0)
	    maxline = MAXLINE;
	 for (maxline--, c = 0, i = 1; i < maxline; i++) {
	    s = read(fd, &c, 1);
	    if (s != 1 || c == '\n')
	       break;
	    *line++ = c;
	    }
	 *line++ = '\n';
	 *line = 0;
	 if (i == 1 && s == 0)
	    return(EOF);
	 return(i);
      }
