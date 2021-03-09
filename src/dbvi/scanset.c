/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: scanset.c,v 9.0 2001/01/09 02:56:06 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Scanset_sid[] = "@(#) $Id: scanset.c,v 9.0 2001/01/09 02:56:06 john Exp $";
#endif

#include <dbvi_ext.h>

/* 
 *  scanset - set up emtpy screen of any text on the current fb_page.
 */

   void dbvi_scanset()
      
      {
	 fb_node *n;
         
	 if (pcur == NULL)
	    return;
	 for (n = pcur->p_nhead; n; n = n->n_next){
	    if (n->n_fp == NULL){	/* i.e., if its not a fb_field */
	       fb_move(n->n_row, n->n_col);
	       if (n->n_reverse)
	          fb_reverse(n->n_text);
	       else
	          fb_printw(FB_FSTRING, n->n_text);
	       }
	    }
      }
