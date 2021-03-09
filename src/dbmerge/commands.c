/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: commands.c,v 9.0 2001/01/09 02:55:40 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Commands_sid[] = "@(#) $Id: commands.c,v 9.0 2001/01/09 02:55:40 john Exp $";
#endif

#include <dbdmrg_e.h>

void step_down();
void step_up();

extern char *filen;
extern short int cdb_use_insert_char;
extern int fullsize, halfsize;

   cell_right(n)
      int n;

      {
         int nc, st = FB_ERROR, ofc, offset, full_offset;

         nc = mpcur->mp_col + n;
         offset = calc_offset();
         full_offset = calc_full_offset();
         ofc = nc + full_offset;
         if (ofc <= linewidth){
            if (ofc > mpcur->mp_rightcorn + offset)
               mpcur->mp_leftcorn += n;
            mpcur->mp_col = nc;
            st = FB_AOK;
            }
         return(st);
      }

   cell_left(n)
      int n;

      {
         int nc, st = FB_ERROR;

         nc = mpcur->mp_col - n;
         if (nc > 0){
            if (nc < mpcur->mp_leftcorn)
               mpcur->mp_leftcorn -= n;
            mpcur->mp_col = nc;
            st = FB_AOK;
            }
         return(st);
      }

   cell_up()
      {
         int st = FB_ERROR, offset, col;

         offset = calc_full_offset();
         col = mpcur->mp_col;
         if (mpcur->mp_acur->a_prev != NULL){
	    if (mpcur->mp_acur == mpcur->mp_atop){
	       step_up();
	       }
	    mpcur->mp_acur = mpcur->mp_acur->a_prev;
            set_column(offset + col);
	    st = FB_AOK;
	    }
	 return(st);
      }

   cell_down()
      {
         int st = FB_ERROR, offset, col;

         offset = calc_full_offset();
         col = mpcur->mp_col;
         if (mpcur->mp_acur->a_next != NULL){
	    if (mpcur->mp_acur == mpcur->mp_abot){
	       step_down();
	       }
	    mpcur->mp_acur = mpcur->mp_acur->a_next;
            set_column(offset + col);
	    st = FB_AOK;
	    }
	 return(st);
      }

   void step_down()

      {
         fb_aline *na;

	 if (mpcur->mp_acur == mpcur->mp_atail)
	    return;

	 if ((na = mpcur->mp_atop->a_next) == NULL)
	    return;
	 mpcur->mp_atop = na;

	 if ((na = mpcur->mp_abot->a_next) == NULL)
	    return;
	 mpcur->mp_abot = na;

         if (cdb_use_insert_char){
            fb_move(base_top, 1);
            fb_deleteln();
            fb_move(base_bottom, 1);
            fb_insertln();
            }
         mrg_display();
      }

   void step_up()

      {
         fb_aline *na;

	 if (mpcur->mp_acur == mpcur->mp_ahead)
	    return;

	 if ((na = mpcur->mp_atop->a_prev) == NULL)
	    return;
	 mpcur->mp_atop = na;

	 if ((na = mpcur->mp_abot->a_prev) == NULL)
	    return;
	 mpcur->mp_abot = na;
         if (cdb_use_insert_char){
            fb_move(base_bottom, 1);
            fb_deleteln();
            fb_move(base_top, 1);
            fb_insertln();
            }

         mrg_display();
      }

/*
 * scroll_down - do line motion in a scrolling manner
 */

   scroll_down(n, cflag)
      int n, cflag;

      {
         int j, reset = 0;
	 fb_aline *errora;

         if (cflag)			/* if gonna move cursor */
	    errora = mpcur->mp_acur;
	 else
	    errora = mpcur->mp_abot;
	 if (errora == mpcur->mp_atail)
	    return(FB_ERROR);
	 if (n == 0)
	    n = 1;
         for (j = n; j > 0; j--){
	    if (mpcur->mp_abot == mpcur->mp_atail)
	       break;
	    /* if the window slides past cursor, flag for reset below */
	    if (!cflag && mpcur->mp_acur == mpcur->mp_atop)
	       reset = 1;
	    step_down();
	    }
	 if (cflag){
	    for (j = n; j > 0; j--){
	       if (mpcur->mp_acur->a_next != NULL)
		  mpcur->mp_acur = mpcur->mp_acur->a_next;
	       else
		  break;
	       }
	    }
	 else if (reset)		/* reset cursor to top of p window */
	    mpcur->mp_acur = mpcur->mp_atop;
	 return(FB_AOK);
      }


/*
 * scroll_up - do line motion in a scrolling manner
 */

   scroll_up(n, cflag)
      int n, cflag;

      {
         int j, reset = 0;
	 fb_aline *errora;

         if (cflag)			/* if gonna move cursor */
	    errora = mpcur->mp_acur;
	 else
            errora = mpcur->mp_atop;
	 if (errora == mpcur->mp_ahead)
	    return(FB_ERROR);
	 if (n == 0)
	    n = 1;
         for (j = n; j > 0; j--){
	    if (mpcur->mp_atop == mpcur->mp_ahead)
	       break;
	    /* if the window slides past cursor, flag for reset below */
	    if (!cflag && mpcur->mp_acur == mpcur->mp_abot)
	       reset = 1;
	    step_up();
	    }
	 if (cflag){
	    for (j = n; j > 0; j--){
	       if (mpcur->mp_acur->a_prev != NULL)
		  mpcur->mp_acur = mpcur->mp_acur->a_prev;
	       else
		  break;
	       }
	    }
	 else if (reset)		/* reset cursor to bottom of p window*/
	    mpcur->mp_acur = mpcur->mp_abot;
	 return(FB_AOK);
      }

/*
 * line_down - do line motion as a new page
 */

   line_down(n, cflag)
      int n, cflag;

      {
         int j, reset = 0;
	 fb_aline *na, *errora;

         if (cflag)			/* if gonna move cursor */
	    errora = mpcur->mp_acur;
	 else
	    errora = mpcur->mp_abot;
	 if (errora == mpcur->mp_atail)
	    return(FB_ERROR);
	 if (n == 0)
	    n = 1;
         if (cdb_use_insert_char && n > halfsize)
            clear_all_lines();
         for (j = n; j > 0; j--){
	    if (mpcur->mp_abot == mpcur->mp_atail)
	       break;
	    /* if the window slides past cursor, flag for reset below */
	    if (!cflag && mpcur->mp_acur == mpcur->mp_atop)
	       reset = 1;
	    na = mpcur->mp_atop->a_next;
	    if (na != NULL)
	       mpcur->mp_atop = na;
	    na = mpcur->mp_abot->a_next;
	    if (na != NULL)
	       mpcur->mp_abot = na;
	    }
	 if (cflag){
	    for (j = n; j > 0; j--){
	       if (mpcur->mp_acur->a_next != NULL)
		  mpcur->mp_acur = mpcur->mp_acur->a_next;
	       else
		  break;
	       }
	    }
	 else if (reset)		/* reset cursor to top of p window */
	    mpcur->mp_acur = mpcur->mp_atop;
	 return(FB_AOK);
      }

   line_up(n, cflag)
      int n, cflag;

      {
         int j, reset = 0;
	 fb_aline *na, *errora;

         if (cflag)			/* if gonna move cursor */
	    errora = mpcur->mp_acur;
	 else
	    errora = mpcur->mp_atop;
	 if (errora == mpcur->mp_ahead)
	    return(FB_ERROR);
	 if (n == 0)
	    n = 1;
         if (cdb_use_insert_char && n > halfsize)
            clear_all_lines();
         for (j = n; j > 0; j--){
	    if (mpcur->mp_atop == mpcur->mp_ahead)
	       break;
	    /* if the window slides past cursor, flag for reset below */
	    if (!cflag && mpcur->mp_acur == mpcur->mp_abot)
	       reset = 1;
	    na = mpcur->mp_atop->a_prev;
	    if (na != NULL)
	       mpcur->mp_atop = na;
	    na = mpcur->mp_abot->a_prev;
	    if (na != NULL)
	       mpcur->mp_abot = na;
	    }
	 if (cflag){
	    for (j = n; j > 0; j--){
	       if (mpcur->mp_acur->a_prev != NULL)
		  mpcur->mp_acur = mpcur->mp_acur->a_prev;
	       else
		  break;
	       }
	    }
	 else if (reset)		/* reset cursor to top of p window */
            mpcur->mp_acur = mpcur->mp_abot;
	 return(FB_AOK);
      }

   scroll_right(n)
      int n;

      {
         int nc, st = FB_ERROR, ofc;

         nc = mpcur->mp_leftcorn + n;
         ofc = nc + calc_offset();
         if (ofc <= linewidth){
            mpcur->mp_leftcorn = nc;
            st = FB_AOK;
            }
         return(st);
      }

   scroll_left(n)
      int n;

      {
         int nc, st = FB_ERROR;

         nc = mpcur->mp_leftcorn - n;
         if (nc > 0 || mpcur->mp_leftcorn > 1){
            if (nc <= 0)
               nc = 1;
            mpcur->mp_leftcorn = nc;
            st = FB_AOK;
            }
         return(st);
      }

   word_right()
      {
         char *p;
         int ofc;

         ofc = mpcur->mp_col + calc_full_offset();
         if (ofc >= linewidth)
            return(FB_ERROR);
         p = mpcur->mp_acur->a_text + mpcur->mp_col - 1;
         /* if in word, skip this word */
         for (; ofc < linewidth && *p != FB_BLANK && *p != '\t'; ){
            p++;
            mpcur->mp_col++;
            ofc = mpcur->mp_col + calc_full_offset();
            if (ofc >= linewidth)
               break;
            }
         /* skip white space */
         for (; ofc < linewidth && *p == FB_BLANK || *p == '\t'; ){
            p++;
            mpcur->mp_col++;
            ofc = mpcur->mp_col + calc_full_offset();
            if (ofc >= linewidth)
               break;
            }
         /* if out of the confines of the this line, reset edges */
         if (mpcur->mp_col > mpcur->mp_rightcorn)
            mpcur->mp_leftcorn = mpcur->mp_col;
         set_screen();
         return(FB_AOK);
      }

   end_word_right()
      {
         char *p, *np;

         p = mpcur->mp_acur->a_text + mpcur->mp_col - 1;
         np = p + 1;
         if ((*p != FB_BLANK && *p != '\t') && (*np == FB_BLANK || *np == '\t')){
            p++;
            mpcur->mp_col++;
            }
         while (*p == FB_BLANK || *p == '\t'){
            p++;
            mpcur->mp_col++;
            if (mpcur->mp_col >= linewidth)
               break;
            }
         while (*p != FB_BLANK && *p != '\t'){
            p++;
            mpcur->mp_col++;
            if (mpcur->mp_col >= linewidth)
               break;
            }
         if (*p == FB_BLANK || *p == '\t')
            mpcur->mp_col--;
         if (mpcur->mp_col < linewidth)
            st = FB_AOK;
         if (mpcur->mp_col > mpcur->mp_rightcorn)
            mpcur->mp_leftcorn = mpcur->mp_col - base_length;
         return(FB_AOK);
      }

   word_left()
      {
         char *p;

         p = mpcur->mp_acur->a_text + mpcur->mp_col - 2;
         /* skip blanks backwards */
         while (*p == FB_BLANK || *p == '\t'){
            p--;
            mpcur->mp_col--;
            if (mpcur->mp_col <= 0)
               break;
            }
         /* skip the word */
         while (*p != FB_BLANK && *p != '\t'){
            p--;
            mpcur->mp_col--;
            if (mpcur->mp_col <= 0)
               break;
            }
         if (mpcur->mp_col < mpcur->mp_leftcorn)
            mpcur->mp_leftcorn = mpcur->mp_col - 10;
         return(FB_AOK);
      }

   genstat(type)
      int type;

      {
         int i, count;
         fb_aline *a;
         char buffer[FB_MAXLINE];

         i = count = 0;
         for (a = mpcur->mp_ahead; a != NULL; a = a->a_next){
            count++;
            if (a == mpcur->mp_acur)
               i = count;
            }
         if (type == 0 || (type == -1 && count > 0))
            sprintf(buffer,"\"%s\" %d lines",filen,count);
         else if (type == 1)
            sprintf(buffer,"\"%s\" line %d of %d",filen,i,count);
         else if (type == -1 && count == 0)
            sprintf(buffer,"\"%s\" [New file]",filen);
         fb_move(cdb_t_lines, 1);
         fb_prints(buffer);
         clear_lastline = 1;
      }
