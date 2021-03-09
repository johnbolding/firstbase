/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbvedit.c,v 9.0 2001/01/09 02:55:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbvedit_sid[] = "@(#) $Id: dbvedit.c,v 9.0 2001/01/09 02:55:57 john Exp $";
#endif

/*
 *  dbvedit.c -   a full screen, multi page, visual dbedit
 */

#include <dbve_v.h>
#include <macro_v.h>

/*
 *  dbvedit - main driver
 */ 
     
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         initvi(argc, argv);
         editor();
	 endvi();
      }
