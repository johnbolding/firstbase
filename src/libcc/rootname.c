/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: rootname.c,v 9.0 2001/01/09 02:56:21 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Rootname_sid[] = "@(#) $Id: rootname.c,v 9.0 2001/01/09 02:56:21 john Exp $";
#endif

#include <fb.h>
#define MAXNAME 240

/* 
 *  rootname - take dirname and basename of pathname p and put back into r.
 *       this leaves the root name, minus the extension, with dirname intact.
 */

   char *fb_rootname(r, p)
      char *p, *r;
      
      {
	 char b[MAXNAME];
	 
	 fb_dirname(r, p);
	 fb_basename(b, p);
	 strcat(r, b);
	 return(r);
      }
