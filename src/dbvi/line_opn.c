/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: line_opn.c,v 9.0 2001/01/09 02:56:06 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Line_open_sid[] = "@(#) $Id: line_opn.c,v 9.0 2001/01/09 02:56:06 john Exp $";
#endif

#include <dbvi_ext.h>

   line_open(c_dot)
      crec *c_dot;
      
      {
	 int krow, st, pcount;
	 crec *khead, *ktail, *makecrec(), *c_new;
	 
	 modified = 1;
	 for (;;){
	    sput_cursor(0);
	    c_new = makecrec();
	    ktail = c_new; khead = c_new;
	    if (c_dot == NULL){			/* must be at head of mem */
	       ktail->c_next = crec_mhead;
	       crec_mhead->c_prev = ktail;
	       crec_mhead = crec_phead = khead;
	       }
	    else{				/* normal case */
	       ktail->c_next = c_dot->c_next;
	       if (c_dot->c_next != NULL)
		  c_dot->c_next->c_prev = ktail;
	       c_dot->c_next = khead;
	       khead->c_prev = c_dot;
	       }
	    if (ktail->c_next == crec_phead)	/* boundry conditions */
	       crec_phead = khead;
	    else if (c_dot == crec_ptail){
	       pcount = dbvi_onpage();
	       if (pcount >= cdb_t_lines - calc_row){
		  crec_phead = crec_phead->c_next;
		  crec_ptail = khead;
		  fb_move(calc_row + 1, 1);
		  fb_deleteln();
		  }
	       else 
		  crec_ptail = ktail;
	       }
	    if (c_dot == crec_mtail)		/* check for tail of mem */
	       crec_mtail = ktail;
	    crec_current = khead;
	    crec_leftcorn = crec_phead;
	    if (col_phead == col_mhead){
	       if (c_dot == NULL)
		  krow = calc_row + 1;
	       else
		  krow = whichrow(c_dot) + 1;
	       fb_move(krow, 1);
	       fb_insertln();
	       fb_move(cdb_t_lines, 1); 
	       fb_deleteln();
	       col_current = col_mhead;
	       set_screen();
	       draw_numbers();
	       }
	    else{
	       col_phead = col_current = col_mhead;
	       test_redraw();
	       }
	    checklink(3, c_new);
	    st = edit_row();
	    if (st != FB_AOK)
	       break;
	    c_dot = c_new;
	    }
	 return(FB_AOK);
      }
