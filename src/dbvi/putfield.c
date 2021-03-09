/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: putfield.c,v 9.0 2001/01/09 02:56:06 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Putfield_sid[] = "@(#) $Id: putfield.c,v 9.0 2001/01/09 02:56:06 john Exp $";
#endif

#include <dbvi_ext.h>

char choice_attribute[1];		/* defined in choice.c */

static char out[251];			/* *s on 'F' type is too short */
	 				/* for comments, hence 'out' */
/* 
 *  putfield - print a fb_field value according to coordinates in fb_node n.
 *	Note: putfield blindly Walks on s for FB_OFNUMERICS and DATES.
 */

   putfield(n, k, s)
      fb_node *n;
      fb_field *k;
      char *s;
   
      {
         char *line, *fb_trim(), *p, *q;
	 int row, col;

         line = cdb_bfld;		/* display uses cdb_afld--- careful */
	 choice_attribute[0] = '|';
	 fb_formfield(line, s, k->type, k->size);
	 strcpy(s, line);
	 if (k->type == FB_FORMULA){
	    strcpy(out, s);
	    s = out;			/* make s point to out for space */
	    }
	 row = n->n_row; col = n->n_col;
         fb_move(row, col);
	 fb_printw("%*s", n->n_len, NIL);
         fb_move(row, col);
	 if (n->n_sub2 > 0){
	    /* take subset of s, assign to p */
	    q = s + (n->n_sub1 - 1);
	    s[n->n_sub2] = NULL;
	    fb_trim(q);
	    }
	 else{
	    if (n->n_type == T_SUBELEMENT_R || n->n_type == T_SUBELEMENT_N){
	       if (k->type == FB_CHOICE || k->type == FB_SILENTCHOICE){
		  fb_subline(line, s, n->n_sub1, choice_attribute[0]);
		  strcpy(s, line);
		  }
	       else{
	          fb_subline(line, s, n->n_sub1, CHAR_NEWLINE);
		  strcpy(s, line);
		  }
	       }
	    if (strlen(s) > n->n_len)
	       s[n->n_len] = NULL;
	    q = s;
	    }
	 for (p = q; *p; p++)
	    if (*p == CHAR_NEWLINE || *p == '\t')
	       *p = FB_BLANK;
	 if (n->n_reverse)
	    fb_stand(q);
	 else
	    fb_prints(q);
      }
