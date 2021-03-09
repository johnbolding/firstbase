/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbvemit.c,v 9.0 2001/01/09 02:56:00 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbvemit_sid[] = "@(#) $Id: dbvemit.c,v 9.0 2001/01/09 02:56:00 john Exp $";
#endif

/*
 *  dbvemit.c -   a full screen, multi fb_page, visual dbemit -
 *	i.e., dbvemit edits a record in edit style add mode and emits vals
 *            in Cdb loadable format. get it?
 */

#include <dbve_v.h>
#include <macro_v.h>

/*
 *  dbvemit - main driver
 */ 
     
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         initvi(argc, argv);
         editor();
	 endvi();
      }
