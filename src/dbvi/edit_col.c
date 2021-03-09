/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: edit_col.c,v 9.1 2002/09/04 04:48:14 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Edit_col[] = "@(#)edit_col.c	8.1 8/19/93 FB";
#endif

#include <dbvi_ext.h>

short int dbvi_column_mode = 0;

/* 
 *  edit_col - edit a column of values, starting with the current col/rec.
 */
    edit_col()

      {
         int st;

         dbvi_column_mode = 1;
	 for (;;){
	    st = edit_cell();
	    if (st != FB_AOK && st != FB_ESIGNAL && st != FB_YSIGNAL)
	       break;
	    if (st == FB_AOK){
	       if (cell_down() != FB_AOK)
	          break;
	       }
	    else if (st == FB_YSIGNAL){
	       if (cell_up() != FB_AOK)
	          fb_bell();
	       }
	    else if (st == FB_ESIGNAL){
	       if (cell_down() != FB_AOK)
	          fb_bell();
	       }
	    }
         dbvi_column_mode = 0;
	 return(st);
      }
