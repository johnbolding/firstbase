/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: edit_row.c,v 9.0 2001/01/09 02:56:05 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Edit_row[] = "@(#)edit_row.c	8.1 8/19/93 FB";
#endif

#include <dbvi_ext.h>

/* 
 *  edit_row - edit a whole row of values, starting with the current col/rec.
 */
    edit_row()

      {
         int i, st;

	 for (;;){
	    i = col_current->p_array;
	    /* i is used to keep track of the array pos */
	    st = edit_cell();
	    if (st != FB_AOK && st != FB_ESIGNAL && st != FB_YSIGNAL)
	       break;
	    if (st == FB_AOK){
	       if (cell_right() != FB_AOK)
	          break;
	       }
	    else if (st == FB_YSIGNAL){
	       if (cell_left() != FB_AOK)
	          fb_bell();
	       }
	    else if (st == FB_ESIGNAL){
	       if (cell_right() != FB_AOK)
	          fb_bell();
	       }
	    test_redraw();
	    if (st == FB_ESIGNAL || st ==FB_AOK)
	       while (i == col_current->p_array){
	          if (cell_right() != FB_AOK)
		     break;
	          test_redraw();
	          }
	    }
	 return(st);
      }
