/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: putfield.c,v 9.0 2001/01/09 02:56:41 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Putfield_sid[] = "@(#) $Id: putfield.c,v 9.0 2001/01/09 02:56:41 john Exp $";
#endif

#include <dbve_ext.h>

extern char *choice_attribute;		/* defined in choice.c */

static char out[251];			/* *s on 'F' type is too short */
	 				/* for comments, hence 'out' */
/* 
 *  putfield - print a fb_field value according to coordinates in node n.
 *	Note: putfield blindly Walks on s for FB_OFNUMERICS and FB_DATES.
 */

   void fb_putfield(n, k, s)
      fb_node *n;
      fb_field *k;
      char *s;
   
      {
         char *line, *p, *q, *r;
	 int row, col;

         line = cdb_bfld;		/* display uses cdb_afld--- careful */
         if (k->type != FB_BINARY){
            fb_formfield(line, s, k->type, k->size);
            strcpy(s, line);
            }
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
	    s[n->n_sub2] = NULL;
	    r = s + (n->n_sub1 - 1);
            for (q = s; q < r; q++)
               if (*q == NULL)
                  break;
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
	    if (k->type != FB_BINARY && strlen(s) > n->n_len)
	       s[n->n_len] = NULL;
	    q = s;
	    }
         if (k->type != FB_BINARY){
            for (p = q; *p; p++)
               if (*p == CHAR_NEWLINE || *p == '\t')
                  *p = FB_BLANK;
            if (n->n_reverse)
               fb_stand(q);
            else
               fb_prints(q);
            }
      }
