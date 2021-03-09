/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: subscrn.c,v 9.0 2001/01/09 02:55:41 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Subscreen_sid[] = "@(#) $Id: subscrn.c,v 9.0 2001/01/09 02:55:41 john Exp $";
#endif

#include <dbdmrg_e.h>

#define SUB_WINDOW_SIZE		9

void sub_step_up();
void sub_step_down();

extern int sub_top_row, sub_bot_row, sub_col, sub_scr_top, sub_scr_cur;

extern short int cdb_sfields;
extern fb_field **cdb_sp;

   void sub_put_cursor(revflag)
      int revflag;

      {
         fb_field *f;
         int row;
         char buf[FB_MAXLINE];

         if (sub_scr_cur < 0 || sub_scr_cur >= cdb_sfields){
            fb_serror(FB_MESSAGE, "Cannot set the sub cursor", NIL);
            return;
            }
         row = sub_whichrow(sub_scr_cur);
         fb_move(row,  sub_col);
         f = cdb_sp[sub_scr_cur];
         sprintf(buf, "%-10s   %c   %4d", f->id, f->type, f->size);
         if (revflag)
            fb_reverse(buf);
         else
            fb_prints(buf);
         if (revflag){
            fb_move(row,  sub_col - 2);
            fb_reverse(">>");
            fb_move(row,  sub_col - 3);
            }
         else{
            fb_move(row,  sub_col - 2);
            fb_prints("  ");
            }
         fb_refresh();
      }

/*
 * sub_display module
 */

   sub_fb_display()
      {
         int r, n;
         fb_field *f;
         char buf[FB_MAXLINE];

         for (r = sub_top_row, n = sub_scr_top; r <= sub_bot_row; r++, n++){
            if (n >= cdb_sfields)
               break;
            f = cdb_sp[n];
            fb_move(r, sub_col);
            fb_clrtoeol();
            sprintf(buf, "%-10s   %c   %4d", f->id, f->type, f->size);
            fb_prints(buf);
            }
	 for (; r <= sub_bot_row; r++){
            fb_move(r, sub_col);
            fb_clrtoeol();
	    fb_prints("~");
	    }
      }

   sub_whichrow(s)
      int s;

      {
         int t, row;

         row = sub_top_row;
         for (t = sub_scr_top; row <= sub_bot_row; t++, row++){
	    if (t == s)
	       break;
	    if (row == sub_bot_row){
	       fb_serror(FB_MESSAGE, "Shouldnt Happen-Cant find sub-which-row", NIL);
	       row = sub_top_row;
	       break;
	       }
	    }
	 return(row);
      }

   sub_cell_up()
      {
         int st = FB_ERROR;

         sub_put_cursor(0);
         if (sub_scr_cur != 0){
	    if (sub_scr_cur == sub_scr_top){
	       sub_step_up();
	       }
	    sub_scr_cur--;
	    st = FB_AOK;
	    }
	 return(st);
      }

   sub_cell_down()
      {
         int st = FB_ERROR;

         sub_put_cursor(0);
         if (sub_scr_cur != cdb_sfields - 1){
	    if (sub_scr_cur == sub_scr_top + SUB_WINDOW_SIZE - 1){
	       sub_step_down();
	       }
	    sub_scr_cur++;
	    st = FB_AOK;
	    }
	 return(st);
      }

   void sub_step_down()

      {
         int na;

	 if (sub_scr_cur == cdb_sfields - 1)
	    return;

	 if ((na = sub_scr_top + 1) >= cdb_sfields)
	    return;
	 sub_scr_top = na;

         sub_fb_display();
      }

   void sub_step_up()

      {
         int na;

	 if (sub_scr_cur == 0)
	    return;

	 if ((na = sub_scr_top - 1) < 0)
	    return;
	 sub_scr_top = na;

         sub_fb_display();
      }

/*
 * sub_line_down - do line motion as a new page
 */

   sub_line_down(n, cflag)
      int n, cflag;

      {
         int j, reset = 0, bot, errora, na;

         sub_put_cursor(0);
         if (cflag)			/* if gonna move cursor */
	    errora = sub_scr_cur;
	 else
	    errora = sub_scr_top + SUB_WINDOW_SIZE - 1;
	 if (errora == cdb_sfields)
	    return(FB_ERROR);
	 if (n == 0)
	    n = 1;
         for (j = n; j > 0; j--){
            bot = sub_scr_top + SUB_WINDOW_SIZE - 1;
	    if (bot == cdb_sfields)
	       break;
	    /* if the window slides past cursor, flag for reset below */
	    if (!cflag && sub_scr_cur == sub_scr_top)
	       reset = 1;
	    na = sub_scr_top + 1;
	    if (na < cdb_sfields)
	       sub_scr_top = na;
	    }
	 if (cflag){
	    for (j = n; j > 0; j--){
	       if (sub_scr_cur + 1 != cdb_sfields)
		  sub_scr_cur++;
	       else
		  break;
	       }
	    }
	 else if (reset)		/* reset cursor to top of p window */
	    sub_scr_cur = sub_scr_top;
	 return(FB_AOK);
      }

   sub_line_up(n, cflag)
      int n, cflag;

      {
         int j, reset = 0, bot, na, errora;

         sub_put_cursor(0);
         if (cflag)			/* if gonna move cursor */
	    errora = sub_scr_cur;
	 else
	    errora = sub_scr_top;
	 if (errora == 0)
	    return(FB_ERROR);
	 if (n == 0)
	    n = 1;
         for (j = n; j > 0; j--){
	    if (sub_scr_top == 0)
	       break;
	    /* if the window slides past cursor, flag for reset below */
            bot = sub_scr_top + SUB_WINDOW_SIZE - 1;
	    if (!cflag && sub_scr_cur == bot)
	       reset = 1;
	    na = sub_scr_top - 1;
	    if (na >= 0)
	       sub_scr_top = na;
	    }
	 if (cflag){
	    for (j = n; j > 0; j--){
	       if (sub_scr_cur - 1 >= 0)
		  sub_scr_cur--;
	       else
		  break;
	       }
	    }
	 else if (reset)		/* reset cursor to top of p window */
            sub_scr_cur = sub_scr_top + SUB_WINDOW_SIZE - 1;
	 return(FB_AOK);
      }

/*
 * sub_scroll_down - do line motion in a scrolling manner
 */

   sub_scroll_down(n, cflag)
      int n, cflag;

      {
         int j, reset = 0, errora, bot;

         sub_put_cursor(0);
         if (cflag)			/* if gonna move cursor */
	    errora = sub_scr_cur;
	 else
	    errora = sub_scr_top + SUB_WINDOW_SIZE - 1;
	 if (errora == cdb_sfields)
	    return(FB_ERROR);
	 if (n == 0)
	    n = 1;
         for (j = n; j > 0; j--){
            bot = sub_scr_top + SUB_WINDOW_SIZE - 1;
	    if (bot == cdb_sfields)
	       break;
	    /* if the window slides past cursor, flag for reset below */
	    if (!cflag && sub_scr_cur == sub_scr_top)
	       reset = 1;
	    sub_step_down();
	    }
	 if (cflag){
	    for (j = n; j > 0; j--){
	       if (sub_scr_cur + 1 != cdb_sfields)
		  sub_scr_cur++;
	       else
		  break;
	       }
	    }
	 else if (reset)		/* reset cursor to top of p window */
	    sub_scr_cur = sub_scr_top;
	 return(FB_AOK);
      }

/*
 * sub_scroll_up - do line motion in a scrolling manner
 */

   sub_scroll_up(n, cflag)
      int n, cflag;

      {
         int j, reset = 0, errora, bot;

         sub_put_cursor(0);
         if (cflag)			/* if gonna move cursor */
	    errora = sub_scr_cur;
	 else
	    errora = sub_scr_top;
	 if (errora == 0)
	    return(FB_ERROR);
	 if (n == 0)
	    n = 1;
         for (j = n; j > 0; j--){
	    if (sub_scr_top == 0)
	       break;
	    /* if the window slides past cursor, flag for reset below */
            bot = sub_scr_top + SUB_WINDOW_SIZE - 1;
	    if (!cflag && sub_scr_cur == bot)
	       reset = 1;
	    sub_step_up();
	    }
	 if (cflag){
	    for (j = n; j > 0; j--){
	       if (sub_scr_cur - 1 >= 0)
		  sub_scr_cur--;
	       else
		  break;
	       }
	    }
	 else if (reset)		/* reset cursor to top of p window */
            sub_scr_cur = sub_scr_top + SUB_WINDOW_SIZE - 1;
	 return(FB_AOK);
      }
