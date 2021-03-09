/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: Malloc.c,v 9.0 2001/01/09 02:56:23 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Malloc_sid[] = "@(#) $Id: Malloc.c,v 9.0 2001/01/09 02:56:23 john Exp $";
#endif

/* 
 *  Malloc - must malloc or die trying.
 */

#include <fb.h>
#include <fb_ext.h>

/* extern char *malloc(unsigned s); */

   char *fb_malloc(s)
      unsigned s;
      
      {
         register char *p;
	 
	 if ((p = (char *) malloc((unsigned) s)) == 0)
	    fb_xerror(FB_OUT_OF_MEMORY, SYSMSG[S_NOMEM], NIL);
	 return(p);
      }
