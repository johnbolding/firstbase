/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbdmrg.c,v 9.0 2001/01/09 02:55:40 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbdmrg_sid[] = "@(#) $Id: dbdmrg.c,v 9.0 2001/01/09 02:55:40 john Exp $";
#endif

/*
 *  dbdmrg.c -   a full screen editor used to create dbmerge fb_input files.
 */

#include <dbdmrg_v.h>

/*
 *  dbdmrg - main driver
 */ 

   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         initmrg(argc, argv);
         editor();
	 endmrg();
      }
