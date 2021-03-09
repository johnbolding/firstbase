/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mustwrte.c,v 9.0 2001/01/09 02:56:27 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Mustwrite_sid[] = "@(#) $Id: mustwrte.c,v 9.0 2001/01/09 02:56:27 john Exp $";
#endif

#include <fb.h>

static char *MSG = "mustwrite:";

/* 
 *  mustwrite - write s to fd or die trying.
 */

   fb_mustwrite(fd, s)
      register int fd;
      register char *s;
   
      {
         register int n;
	 
	 n = strlen(s);
	 if (write(fd, s, (unsigned) n) != n)
	    fb_xerror(FB_WRITE_ERROR, MSG, s);
      }
