/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: percent.c,v 9.1 2001/02/16 19:15:28 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Percentage_sid[] = "@(#) $Id: percent.c,v 9.1 2001/02/16 19:15:28 john Exp $";
#endif

#include <dbve_ext.h>

static char *FMT = "(page %d of %d)";

extern short int cdb_scr_help;

/* 
 *  percentage - print the percentage of screens onto lower right corner 
 */
 
   void fb_percentage(f, nmax)
      int f, nmax;
      
      {
         char buf[FB_MAXLINE];

         if (cdb_scr_help){
            sprintf(buf, FMT, f, nmax);
            fb_move(cdb_t_lines, (int) (cdb_t_cols - strlen(buf)));
            fb_clrtoeol();
            fb_prints(buf);
	    }
      }
