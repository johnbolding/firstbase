/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mkstr.c,v 9.0 2001/01/09 02:56:27 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Mkstr_sid[] = "@(#) $Id: mkstr.c,v 9.0 2001/01/09 02:56:27 john Exp $";
#endif

#include <fb.h>

/*
 *  mkstr - alloc space/store s at p, freeing previous p. return new p.
 */

   char *fb_mkstr(p, s)
      char **p, *s;
      
      {
         char *fb_malloc(unsigned s);
	 
	 fb_free((char *) *p);
         if (s != 0 && s != (char *) 0){
	    *p = fb_malloc((unsigned) (strlen(s) + 1));
	    strcpy(*p, s);
            }
         else{
	    *p = fb_malloc(2);
	    strcpy(*p, "");
            }
	 return(*p);
      }
