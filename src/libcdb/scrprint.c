/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: scrprint.c,v 9.1 2001/02/16 19:00:29 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Screenprint_sid[] = "@(#) $Id: scrprint.c,v 9.1 2001/02/16 19:00:29 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short cdb_screenprint_pause;	/* pause before screenprint flag */

   void fb_screenprint(f)
      char *f;

      {
         char buffer[FB_MAXLINE];

         if (cdb_screenprint_pause)
	    if (fb_mustbe('n',
	       "View/Print the generated results? <RETURN>=Yes, 'n'=No",
                  cdb_t_lines, 1) == FB_AOK)
	       return;
         sprintf(buffer, "scrprint %s", f);
	 fb_system(buffer, FB_NOROOT);
      }
