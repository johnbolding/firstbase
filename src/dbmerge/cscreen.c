/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: cscreen.c,v 9.0 2001/01/09 02:55:40 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Cscreen_sid[] = "@(#) $Id: cscreen.c,v 9.0 2001/01/09 02:55:40 john Exp $";
#endif

#include <dbdmrg_e.h>

char dash[40] = "------------------------------------";

int t_rightcorn, t_leftcorn;
static test_leftcorn();
static set_rightcorn();
static put_status();

/*
 * set_screen - set the screen tops and bottoms, and do error checking
 */

   set_screen()
      {
         fb_aline *a, *la = NULL;
         int i;

         /* set the bottom */
         for (a = mpcur->mp_atop, i=3; i < cdb_t_lines - 1; i++, a=a->a_next){
            if (a == NULL)
               break;
            la = a;
            }
         mpcur->mp_abot = la;

         /* set the right corner */

         if (mpcur->mp_leftcorn <= 0)
            mpcur->mp_leftcorn = 1;
         else if (test_leftcorn() == FB_ERROR)
            mpcur->mp_leftcorn = t_leftcorn;
         set_rightcorn();
         
         if (mpcur->mp_col < mpcur->mp_leftcorn)
            mpcur->mp_col = mpcur->mp_leftcorn;
         else if (mpcur->mp_col > mpcur->mp_rightcorn)
            mpcur->mp_col = mpcur->mp_rightcorn;
      }

/*
 * test_leftcorn - test whether the left corner is past the optimal
 * 	rightmost fb_page placement of the left corner
 */
   static test_leftcorn()
      {
         int st = FB_AOK, t_col, o_len = 0, width;
         fb_token *t, *last_t;
         char *p, *q;

         p = mpcur->mp_acur->a_text;
         last_t = t = mpcur->mp_acur->a_thead;
         for (t_col = 1; *p; p++, t_col++){
            if (*p == CHAR_DOLLAR && t != NULL){
               if ((width = t->t_width) == 0 && t->t_field != NULL)
                  width = strlen(t->t_field->id) + 1;
               o_len += width;
               last_t = t;
               if (o_len < linewidth)
                  t = t->t_next;
               }
            else
               o_len++;
            if (o_len >= linewidth){
               if (o_len > linewidth)
                  t_col--;
               break;
               }
            }
         /* set the temp rightcorn to the calculated last possible col */
         t_rightcorn = t_col;

         /*
          * now count backwards from t_rightcorn (t_col) 'base_length' units
          */
         p = mpcur->mp_acur->a_text + t_col - 1;
         q = mpcur->mp_acur->a_text;
         t = last_t;
         for (o_len = 0; *p; p--, t_col--){
            if (*p == CHAR_DOLLAR && t != NULL){
               if ((width = t->t_width) == 0 && t->t_field != NULL)
                  width = strlen(t->t_field->id) + 1;
               o_len += width;
               /* marching backwards */
               t = t->t_prev;
               }
            else
               o_len++;
            if (o_len >= base_length || p == q){
               if (o_len > base_length)
                  t_col++;
               break;
               }
            }

         /* now t_col is set to the optimal leftcorn */
         t_leftcorn = t_col;

         if (mpcur->mp_leftcorn > t_leftcorn)
            st = FB_ERROR;
         return(st);
      }

/*
 * set_rightcorn - assume leftcorn is set. set the floating rightcorn
 */

   static set_rightcorn()
      {
         int t_col, o_len = 0, width, hidden_offset, ofc = 0;
         fb_token *t, *set_token();
         char *p;

         t = set_token(mpcur->mp_acur);
         p = mpcur->mp_acur->a_text + mpcur->mp_leftcorn - 1;
         hidden_offset = calc_hidden_offset();
         ofc = mpcur->mp_leftcorn + hidden_offset - 1;
         for (t_col = mpcur->mp_leftcorn; *p; p++, t_col++){
            if (*p == CHAR_DOLLAR && t != NULL){
               if ((width = t->t_width) == 0 && t->t_field != NULL)
                  width = strlen(t->t_field->id) + 1;
               o_len += width;
               ofc += width;
               t = t->t_next;
               }
            else{
               o_len++;
               ofc++;
               }
            if (o_len >= base_length || ofc >= linewidth){
               if (o_len > base_length)
                  t_col--;
               break;
               }
            }
         /* set the rightcorn to the calculated display col */
         mpcur->mp_rightcorn = t_col;
      }

/*
 * put_cursor - draw the cursor
 */

   put_cursor()
      {
         int col, offset, hidden_offset;

         mpcur->mp_row = whichrow(mpcur->mp_acur);
         /*fb_move(mpcur->mp_row,  mpcur->mp_col + mpcur->mp_leftcorn);*/
         col = base_left + mpcur->mp_col - mpcur->mp_leftcorn + 1;
         offset = calc_offset();
         hidden_offset = calc_hidden_offset();
         put_status(hidden_offset, offset);
         fb_move(mpcur->mp_row,  col + offset);
         fb_refresh();
      }

/*
 * calc_offset - calculate the offset in the cursor position.
 *	count only those tokens that APPEAR on the screen.
 *	not their image, the dollar sign must appear!
 */

   calc_offset()

      {
         char *p, *q;
         int offset = 0, width;
         fb_token *t, *set_token();

         p = mpcur->mp_acur->a_text + mpcur->mp_leftcorn - 1;
         q = mpcur->mp_acur->a_text + mpcur->mp_col - 1;
         t = set_token(mpcur->mp_acur);
         for (; p < q; p++)
            if (*p == CHAR_DOLLAR && t != NULL){
               if ((width = t->t_width) == 0 && t->t_field != NULL)
                  width = strlen(t->t_field->id) + 1;
               offset += (width - 1);
               t = t->t_next;
               }
         return(offset);
      }

/*
 * calc_full_offset - calculate the full offset at the cursor position.
 *	both off screen and on.
 */

   calc_full_offset()

      {
         char *p, *q;
         int offset = 0, width;
         fb_token *t;

         p = mpcur->mp_acur->a_text;
         q = mpcur->mp_acur->a_text + mpcur->mp_col - 1;
         for (t = mpcur->mp_acur->a_thead; p < q; p++)
            if (*p == CHAR_DOLLAR && t != NULL){
               if ((width = t->t_width) == 0 && t->t_field != NULL)
                  width = strlen(t->t_field->id) + 1;
               offset += (width - 1);
               t = t->t_next;
               }
         return(offset);
      }

/*
 * calc_hidden_offset - calculate the hidden offset, left of screen
 */

   calc_hidden_offset()

      {
         char *p, *q;
         int offset = 0, width;
         fb_token *t;

         p = mpcur->mp_acur->a_text;
         q = mpcur->mp_acur->a_text + mpcur->mp_leftcorn - 1;
         for (t = mpcur->mp_acur->a_thead; p < q; p++)
            if (*p == CHAR_DOLLAR && t != NULL){
               if ((width = t->t_width) == 0 && t->t_field != NULL)
                  width = strlen(t->t_field->id) + 1;
               offset += (width - 1);
               t = t->t_next;
               }
         return(offset);
      }

/*
 * set_token - return the 'first' token on the display page, i.e., rest of list
 */

   fb_token *set_token(a)
      fb_aline *a;

      {
         char *p, *q;
         fb_token *t;

         p = a->a_text;
         q = a->a_text + mpcur->mp_leftcorn - 1;
         for (t = a->a_thead; p < q; p++)
            if (*p == CHAR_DOLLAR && t != NULL)
               t = t->t_next;
         return(t);
      }

/*
 * set_column - set the column to n to appear seamless in fb_cell motion
 */

   set_column(n)
      int n;

      {
         int dflag = 0, width, tloc;
         char *p, *q;
         fb_token *t;

         tloc = mpcur->mp_col = 1;
         p = mpcur->mp_acur->a_text;
         q = mpcur->mp_acur->a_text + linewidth - 1;
         t = mpcur->mp_acur->a_thead;	/* start t at beginning */
         for (; p <= q && tloc < n; p++){
            if (*p == CHAR_DOLLAR && t != NULL){
               if ((width = t->t_width) == 0 && t->t_field != NULL)
                  width = strlen(t->t_field->id) + 1;
               tloc += width;
               t = t->t_next;
               }
            else
               tloc++;
            mpcur->mp_col++;
            if (tloc >= n)
               break;
            }
         if (*p == CHAR_DOLLAR && tloc > n)
            mpcur->mp_col--;

         /* if out of the confines of the previous line, reset edges */
         if (mpcur->mp_col < mpcur->mp_leftcorn){
            mpcur->mp_leftcorn = mpcur->mp_col;
            dflag = 1;
            }
         else if (mpcur->mp_col > mpcur->mp_rightcorn){
            mpcur->mp_rightcorn = mpcur->mp_col;
            /*mpcur->mp_col = mpcur->mp_rightcorn;*/
            dflag = 1;
            }
         if (dflag){
            set_screen();
            mrg_display();
            }
      }

/*
 * display module
 */

   mrg_display()
      {
         fb_aline *a;
         int r, col, width;
         fb_token *t, *set_token();
         char buf[FB_MAXLINE], *p, tbuf[FB_MAXLINE], pbuf[FB_MAXLINE];

         for (r = base_top, a = mpcur->mp_atop; a != NULL; a = a->a_next){
            col = base_left + 1;
            fb_move(r, col);
            fb_clrtoeol();
            strncpy(buf, a->a_text + mpcur->mp_leftcorn - 1, base_length);
            buf[base_length] = NULL;
            t = set_token(a);
            for (p = buf; *p; p++){
               if (*p == CHAR_DOLLAR && t != NULL){
                  if (t->t_field != NULL)
                     sprintf(tbuf, "$%s", t->t_field->id);
                  else
                     sprintf(tbuf, "$$");
                  if ((width = t->t_width) <= 0)
                     strcpy(pbuf, tbuf);
                  else
                     fb_pad(pbuf, tbuf, width);
                  fb_reverse(pbuf);
                  t = t->t_next;
                  col += width;
                  }
               else{
                  fb_s_putw(*p);
                  col++;
                  }
               if (col > cdb_t_cols)
                  break;
               }
            if (++r >= cdb_t_lines - 1)
               break;
            }
         draw_numbers();
         fb_infoline();
      }

/*
 * onscreen - test if a is on screen
 */

   onscreen(test_a)
      fb_aline *test_a;

      {
         int r;
         fb_aline *a;

         for (r = base_top, a = mpcur->mp_atop; a != NULL; a = a->a_next){
            if (a == test_a)
               return(1);
            if (++r >= cdb_t_lines - 1)
               break;
            }
         return(0);
      }

/*
 * clear all lines
 */

   clear_all_lines()
      {
         int r;

         for (r = base_top; r <= base_bottom; r++){
            fb_move(r, 1);
            fb_clrtoeol();
            }
         fb_refresh();
      }

   draw_numbers()
      {
         
	 fb_aline *a;
	 int pos = 1, row;

         for (a = mpcur->mp_ahead; a != mpcur->mp_atop; a = a->a_next, pos++)
	    ;
	 row = 3;
	 for (a = mpcur->mp_atop; a != NULL; a = a->a_next, pos++){
	    /* fb_move(row, 1); */
	    /* fb_printw("%2d> ", pos); */
            a->a_lineno = pos;
            if (++row >= cdb_t_lines - 1)
               break;
	    }
	 for (; row < cdb_t_lines - 1; row++){
	    fb_move(row, 1);
	    fb_clrtoeol();
	    fb_prints("~");
	    }
      }

   whichrow(a)
      fb_aline *a;

      {
         fb_aline *ta;
	 int row;

         row = 3;
         for (ta = mpcur->mp_atop; ta != NULL; ta = ta->a_next, row++){
	    if (ta == a)
	       break;
	    if (ta == mpcur->mp_abot){
	       fb_serror(FB_MESSAGE, "Shouldnt Happen - Cant find which-row", NIL);
	       row = 3;
	       break;
	       }
	    }
	 return(row);
      }

   put_row(a)
      fb_aline *a;

      {
	 int row, col, width;
         char buf[FB_MAXLINE], *p, tbuf[FB_MAXLINE], pbuf[FB_MAXLINE];
         fb_token *t, *set_token();
	 
	 row = whichrow(a);
         col = base_left + 1;
         fb_move(row, col);
         fb_clrtoeol();
         strncpy(buf, a->a_text + mpcur->mp_leftcorn - 1, base_length);
         buf[base_length] = NULL;
         t = set_token(a);
         for (p = buf; *p; p++){
            if (*p == CHAR_DOLLAR && t != NULL){
               if (t->t_field != NULL)
                  sprintf(tbuf, "$%s", t->t_field->id);
               else
                  sprintf(tbuf, "$$");
               if ((width = t->t_width) <= 0)
                  strcpy(pbuf, tbuf);
               else
                  fb_pad(pbuf, tbuf, width);
               fb_reverse(pbuf);
               t = t->t_next;
               col += width;
               }
            else{
               fb_s_putw(*p);
               col++;
               }
            if (col > cdb_t_cols)
               break;
            }
      }

   onpage()
      {
         int i;
         fb_aline *a;

         for (a = mpcur->mp_atop, i = 1; a != NULL; a = a->a_next, i++){
            if (a == mpcur->mp_abot)
               break;
            }
         return(i);
      }

   static put_status(hidden_offset, offset)
      int hidden_offset, offset;

      {
         char buf[10], lc, rc;

         if (mpcur->mp_leftcorn == 1)
            lc = '|';
         else
            lc = '<';

         if (linewidth > cdb_t_cols &&
               linewidth - cdb_t_cols + 1 > mpcur->mp_leftcorn + hidden_offset)
            rc = '>';
         else
            rc = '|';

         buf[0] = lc; buf[1] = lc; buf[2] = NULL;
         fb_move(2, 1);
         fb_clrtoeol();
         fb_reverse(buf);
         buf[0] = rc; buf[1] = rc; buf[2] = NULL;
         fb_move(2, cdb_t_cols - 1);
         fb_reverse(buf);
         sprintf(buf, "%d-%d", mpcur->mp_acur->a_lineno,
            mpcur->mp_col + offset + hidden_offset);
         fb_move(2, 38); fb_reverse(buf);
      }
