/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: cscreen.c,v 9.1 2001/02/16 19:50:11 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Cscreen_sid[] = "@(#) $Id: cscreen.c,v 9.1 2001/02/16 19:50:11 john Exp $";
#endif

#include <dbvi_ext.h>

/*
 * fb_cell record screen routines
 */

static char CalcLine[81] = {NULL};
static char pline[FB_MAXLINE];
static char fline[FB_MAXLINE];

extern short cdb_datedisplay;

/*
 * set_screen - set the tails and ioloc points according to the crec_mhead,
 *	col_mhead, taking into account the width of each fb_field.
 */

   void set_screen()
      {
         column *p, *np;
	 crec *c;
	 int i, io = FIRST_IOLOC;	/* 7 is "NNNN> " for numbers */
	 char *fb_pad(), header[FB_MAXLINE];

	 if (crec_mhead == NULL)
	    return;
	 if (crec_phead == NULL){
	    col_phead = col_mhead;
	    crec_phead = crec_mhead;
	    }
	 /* set the column tail pointer, iolocs and CalcLine */
	 strcpy(CalcLine, "      ");	/* magic 7 again - means 6 spaces */
	 p = col_phead;
	 for (; p != NULL ;){
	    p->p_ioloc = io;
	    strcat(CalcLine, fb_pad(header, p->p_label, p->p_width));
	    io += p->p_width;	/* set for next pass thru */
	    if (p->p_array + 1 >= ncolumns)
	       break;
	    np = p->p_next;
	    if (np == NULL || io + np->p_width >= cdb_t_cols)
	       break;
	    p = np;
	    }
	 col_ptail = p;
	 /* this code catches a cursor beyond right edge */
	 if (col_current->p_array > col_ptail->p_array)
	    col_current = col_ptail;
	 /* touch up the calc line */
	 strcpy(header, CalcLine);
	 fb_pad(CalcLine, header, cdb_t_cols);
	 if (col_phead == col_mhead)
	    CalcLine[0] = CalcLine[1] = '|';
	 else
	    CalcLine[0] = CalcLine[1] = '<';
	 if (col_ptail == col_mtail)
	    CalcLine[79] = CalcLine[78] = '|';
	 else
	    CalcLine[79] = CalcLine[78] = '>';

	 /* set the fb_cell record tail */
	 for (i = calc_row + 1, c = crec_phead; ;i++){
	    if (c->c_next == NULL)
	       break;
	    if (i >= cdb_t_lines - 1)
	       break;
	    c = c->c_next;
	    }
	 crec_ptail = c;
      }

   set_rightcorner(rp)
      column *rp;

      {
         column *p;
	 int i;

	 for (p = rp, i = FIRST_IOLOC; ;){
	    i += p->p_width;
	    if (i >= cdb_t_cols){
	       p = p->p_next;
	       break;
	       }
	    if (p->p_prev == NULL)
	       break;
	    p = p->p_prev;
	    }
	 col_phead = p;
      }

/*
 * put_cline - CalcLine fb_prints on calc_row.
 */

   put_cline()
      {
	 fb_move(calc_row, 1), fb_clrtoeol(), fb_reverse(CalcLine);
	 fb_move(calc_row, cdb_t_cols);
      }

/*
 * draw_cells - draw all of the physical fb_cell records.
 */

   draw_cells()
      {
         crec *c;
	 column *p;
	 int row;
	 
	 row = calc_row + 1;
	 for (c = crec_phead; c != NULL; c = c->c_next, row++){
	    for (p = col_phead; p != NULL; p = p->p_next){
	       put_cell(c->c_cell, row, p, 0);
	       if (p == col_ptail)
		  break;
	       }
	    checklink(2, c);
	    if (c == crec_ptail)
	       break;
	    }
	 draw_numbers();
      }

/*
 * clear_cells - clear all of the physical screen fb_cell locations
 */

   clear_cells()
      {
	 int row;
	 
	 for (row = calc_row + 1; row <= cdb_t_lines - 1; row++){
	    fb_move(row, FIRST_IOLOC);
	    fb_clrtoeol();
	    }
      }

   put_row(c)
      crec *c;

      {
	 column *p;
	 int row;
	 
	 row = whichrow(c);
	 for (p = col_phead; p != NULL; p = p->p_next){
	    put_cell(c->c_cell, row, p, 0);
	    if (p == col_ptail)
	       break;
	    }
      }

/*
 * put_cell - fb_put an individual fb_cell to the screen
 */

   put_cell(fv, row, p, revflag)
      char *fv[];
      int row, revflag;
      column *p;
      
      {
         int tsize;

         strcpy(pline, fv[p->p_array]);
	 tsize = p->p_field->size;
	 if (p->p_field->type ==FB_DATE)
	    tsize = cdb_datedisplay;
	 fb_formfield(fline, pline, p->p_field->type, tsize);
	 fb_move(row, p->p_ioloc);
	 fb_pad(pline, fline, tsize);
	 if (!revflag)
	    fb_prints(pline);
	 else
	    fb_reverse(pline);
      }

   draw_numbers()
      {
         
	 crec *c;
	 int pos = 1, row;

         for (c = crec_mhead; c != crec_phead; c = c->c_next, pos++)
	    ;
	 row = calc_row + 1;
	 for (c = crec_phead; c != NULL; c = c->c_next, pos++){
	    fb_move(row++, 1);
	    fb_printw("%3d>", pos);
	    if (c == crec_ptail)
	       break;
	    }
	 for (; row < cdb_t_lines; row++){
	    fb_move(row, 1);
	    fb_clrtoeol();
	    fb_printw("   ~");
	    }
      }

   put_cursor(revflag)		/* draw the cursor */
      int revflag;

      {
         put_cell(crec_current->c_cell, whichrow(crec_current),
	    col_current, revflag);
	 fb_refresh();
      }

   sput_cursor(revflag)		/* draw the cursor - no fb_refresh - */
      int revflag;

      {
         put_cell(crec_current->c_cell, whichrow(crec_current),
	    col_current, revflag);
      }

   whichrow(c)
      crec *c;

      {
         crec *tc;
	 int row;

         row = calc_row + 1;
         for (tc = crec_phead; tc != NULL; tc = tc->c_next, row++){
	    if (tc == c)
	       break;
	    if (tc == crec_ptail){
	       fb_serror(FB_MESSAGE, "Shouldnt Happen - Cant find which-row", NIL);
	       row = calc_row + 1;
	       break;
	       }
	    }
	 return(row);
      }

/*
 * onpage - return number of elements drawn on the current page
 */

   dbvi_onpage()
      {
         crec *tc;
	 int count;

         count = 0;
         for (tc = crec_phead; tc != NULL; tc = tc->c_next, count++)
	       ;
	 return(count);
      }

   void dbvi_display()
      {
         fb_move(3, 1); fb_clrtobot();
	 dbvi_scanset();
	 put_cline();
	 draw_cells();
	 /* fb_checkformula(crec_current); */
      }
