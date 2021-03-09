/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: Strcmp.c,v 9.0 2001/01/09 02:56:18 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Strcmp_sid[] = "@(#) $Id: Strcmp.c,v 9.0 2001/01/09 02:56:18 john Exp $";
#endif

#include <fb.h>
#undef strcmp

/*
 * Strcmp - to fix the (char *) 0 problem with some (sun!) processors.
 */

   fb_strcmp(a, b)
      const char *a, *b;

      {
	 if (a == 0 && b == 0)
	    return(0);
	 else if (a == 0)
	    return(1);
	 else if (b == 0)
	    return(-1);
	 else
            return(strcmp(a, b));
      }
