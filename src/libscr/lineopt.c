/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: lineopt.c,v 9.0 2001/01/09 02:57:04 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Lineopt_sid[] = "@(#) $Id: lineopt.c,v 9.0 2001/01/09 02:57:04 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern fb_pscreen cdb_curscr, cdb_newscr;
extern char cdb_DL[];
extern char cdb_AL[];
extern char cdb_IC[];
extern char cdb_DC[];

extern char CHAR_BLANK;

   fb_deleteln()
      {
         register int y;
	 register char *p, *q, *pa, *qa;
         int x, px;

         fb_refresh();
	 tputs(cdb_DL, 1, fb_outc);
	 x = cdb_curscr.s_x;
	 for (px = x++; x < cdb_t_lines; x++, px++){
	    p = cdb_curscr.line[px];
	    q = cdb_curscr.line[x];
	    pa = cdb_curscr.attr[px];
	    qa = cdb_curscr.attr[x];
	    for (y = 0; y < cdb_t_cols; y++){
	       *p++ = *q++;
	       *pa++ = *qa++;
	       }
	    }
	 /* now null out the last line - pointed to by px still */
	 p = cdb_curscr.line[px];
	 pa = cdb_curscr.attr[px];
	 for (y = 0; y < cdb_t_cols; y++)
	    *p++ = *pa++ = CHAR_BLANK; 
      }

   fb_insertln()
      {
         register int y;
	 register char *p, *q, *pa, *qa;
         int x, px;

         fb_refresh();
	 tputs(cdb_AL, 1, fb_outc);
	 for (px = cdb_t_lines - 1; px > cdb_curscr.s_x; px--){
	    x = px - 1;
	    p = cdb_curscr.line[px];
	    q = cdb_curscr.line[x];
	    pa = cdb_curscr.attr[px];
	    qa = cdb_curscr.attr[x];
	    for (y = 0; y < cdb_t_cols; y++){
	       *p++ = *q++;
	       *pa++ = *qa++;
	       }
	    }
	 /* now null out the inserted line - pointed to by cdb_curscr.s_x */
	 p = cdb_curscr.line[cdb_curscr.s_x];
	 pa = cdb_curscr.attr[cdb_curscr.s_x];
	 for (y = 0; y < cdb_t_cols; y++)
	    *p++ = *pa++ = CHAR_BLANK; 
      }

/*
 * deletech - delete a char at the cursor using the DC termcap
 */

   fb_deletech()
      {
         register int y;
	 register char *p, *q, *pa, *qa;
         int x, last_y;

         /*fb_refresh();*/
	 tputs(cdb_DC, 1, fb_outc);
	 x = cdb_curscr.s_x;
         y = cdb_curscr.s_y;
         last_y = cdb_t_cols - 1;
	 p = cdb_curscr.line[x] + y;
         q = p + 1;
	 pa = cdb_curscr.attr[x] + y;
         qa = pa + 1;
         
         for (; y < last_y; y++){
            *p++ = *q++;
            *pa++ = *qa++;
            }

	 /* now null out the cell - pointed to by p and pa still */
	 *p = *pa = CHAR_BLANK; 
      }

/*
 * insertch - insert a blank at the cursor using IC
 */

   fb_insertch()
      {
         register int y;
	 register char *p, *q, *pa, *qa;
         int x, last_y;

         /*fb_refresh();*/
	 tputs(cdb_IC, 1, fb_outc);
	 x = cdb_curscr.s_x;
         y = cdb_curscr.s_y;
         last_y = cdb_t_cols - 1;
	 p = cdb_curscr.line[x] + last_y;
         q = p + 1;
	 pa = cdb_curscr.attr[x] + last_y;
         qa = pa + 1;
         
         for (y = last_y; y > cdb_curscr.s_y; y--){
            *q-- = *p--;
            *qa-- = *pa--;
            }

	 /* now null out the cell - pointed to by p and pa still */
	 *p = *pa = CHAR_BLANK; 
      }
