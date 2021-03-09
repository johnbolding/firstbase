/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dirname.c,v 9.1 2001/01/16 02:46:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dirname_sid[] = "@(#) $Id: dirname.c,v 9.1 2001/01/16 02:46:51 john Exp $";
#endif

#include <fb.h>

/* 
 *  dirname - get directory name from t and place in s. return s.
 */

   char *fb_dirname(s, t)
      register char *s, *t;
      
      {
         register char *p;
         
	 *s = 0;
	 if (t == 0 || !(*t))
	    return(s);
	 strcpy(s, t);
	 if ((p = strrchr(s, '/')) == 0)
	    p = s;
	 else
	    p++;
	 *p = 0;
	 return(s);
      }
