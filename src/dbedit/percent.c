/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: percent.c,v 9.1 2001/02/16 19:41:43 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Percentage_sid[] = "@(#) $Id: percent.c,v 9.1 2001/02/16 19:41:43 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/* 
 *  percentage - print the percentage of screens onto lower right corner 
 */
 
   void fb_percentage(f, nmax)
      int f, nmax;
      
      {
         char buf[FB_MAXLINE];

	 sprintf(buf, SYSMSG[S_DISPLAY], f, nmax);
         fb_move(cdb_t_lines, (int) (cdb_t_cols - strlen(buf)));
	 fb_prints(buf);
      }
