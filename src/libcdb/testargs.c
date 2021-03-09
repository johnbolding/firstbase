/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: testargs.c,v 9.0 2001/01/09 02:56:31 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Testargs_sid[] = "@(#) $Id: testargs.c,v 9.0 2001/01/09 02:56:31 john Exp $";
#endif

#include <fb.h>

/*
 * test the argument vector looking for a match of s
 */

   fb_testargs(argc, argv, s)
      int argc;
      char *argv[], *s;
      
      {
         for (argc--; argc > 0; argc--)
	    if (equal(s, argv[argc]))
	       return(argc);
	 return(0);
      }
