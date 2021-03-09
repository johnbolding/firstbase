/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: chainto.c,v 9.1 2001/02/16 18:57:06 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Chainto_sid[] = "@(#) $Id: chainto.c,v 9.1 2001/02/16 18:57:06 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern char *cdb_pgm;

/* 
 *  chainto - possibly chain to another program. dbds only.
 */
 
   void fb_chainto(msg, topgm, argv)
      char *msg, *topgm, *argv[];
      
      {
         char line[FB_MAXLINE];
	 
	 fb_move(2, 1); fb_clrtobot(); fb_infoline();
	 sprintf(line, "Do you want to %s? (y/n)", msg);
	 if (fb_mustbe(CHAR_y, line, cdb_t_lines, 1) != FB_AOK)
	    return;
	 if (strlen(topgm) > 6)
	    topgm[6] = NULL;
	 fb_basename(argv[0], topgm);	/* assumes topgm names = 6 length */
	 sprintf(line, "Attempting to %s...", msg);
	 fb_fmessage(line);
	 execvp(topgm, argv);
	 
	 /* should not return here if exec works well */
	 fb_xerror(FB_EXEC_FAILURE, cdb_pgm, topgm);
      }
