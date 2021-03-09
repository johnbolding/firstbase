/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbvi.c,v 9.0 2001/01/09 02:56:05 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbvi_sid[] = "@(#) $Id: dbvi.c,v 9.0 2001/01/09 02:56:05 john Exp $";
#endif

/*
 *  dbvi.c -   a full screen, spreadsheet style editor.
 */

#include <dbvi_v.h>

/*
 *  dbvi - main driver
 */ 

   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         initvi(argc, argv);
         editor();
	 endvi();
      }
