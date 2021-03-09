/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: optomize.c,v 9.0 2001/01/09 02:57:04 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Optomize_sid[] = "@(#) $Id: optomize.c,v 9.0 2001/01/09 02:57:04 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_clr1, cdb_clr2, cdb_clr3;

#if FB_PROTOTYPES
static optClear(void);
static void optClrtobot(void);
static optlines(void);
static optClrtoeol(int x);
#else /* FB_PROTOTYPES */
static optClear();
static void optClrtobot();
static optlines();
static optClrtoeol();
#endif /* FB_PROTOTYPES */

extern char cdb_CL[];
extern char cdb_CM[];
extern char cdb_CD[];
extern char cdb_CE[];
extern fb_pscreen cdb_newscr, cdb_curscr;

/*
 * Optomize - scan over the new screen and use Clr commands to optomize.
 */

   void fb_optomize()
      {
	 /* use flags to determin likelihood of optimization */
	 if (cdb_clr1)
	    optClear();
	 if (cdb_clr2)
	    optClrtobot();
	 if (cdb_clr3)
	    optlines();
      }

   static optClear()	/* provide an optomized Clear screen */
      {
         register int x, y;

         /* fix up both screens, ncounts -- and actual display */
	 TrueClear();
         for (x = 0; x < cdb_t_lines; x++){
	    for (y = 0; y < cdb_t_cols; y++){
               /*
                * before walking on the curscr values, check if the newscr
                * is trying to optimize with see-through nulls.
                * if so, copy the curscr values to the newscr before
                * the clear is done.
                */
               if (cdb_newscr.line[x][y] == NULL){
                  cdb_newscr.line[x][y] = cdb_curscr.line[x][y];
                  cdb_newscr.attr[x][y] = cdb_curscr.attr[x][y];
                  }
	       cdb_curscr.line[x][y] = cdb_curscr.attr[x][y] = CHAR_BLANK;
	       if (cdb_newscr.line[x][y] == CHAR_BLANK && 
	           cdb_newscr.attr[x][y] == CHAR_BLANK){
	          cdb_newscr.line[x][y] = NULL;
		  if (cdb_newscr.s_ncount[x] < cdb_t_cols)
		     cdb_newscr.s_ncount[x]++;
		  }
	       }
	    }
      }

   static void optClrtobot()	/* return FB_AOK when an optomized Clrtobot is used */
      {
         register int x, y;

         x = cdb_t_lines - 1; y = cdb_t_cols - 1;
	 /*
	 if (cdb_newscr.line[x][y] == NULL)
	    return;
	 */
	 /* locate last line with null character */
         for (; x >= 0; x--)
	    if (cdb_newscr.s_ncount[x] > 0)
	       break;
	 if (x < 0)
	    return;
	 for (; y >= 0; y--)
	    if (cdb_newscr.line[x][y] == NULL)
	       break;
	 if (y == cdb_t_cols - 1){
	    y = 0;
	    x++;
	    }
	 else
	    y++;
	 TrueMove(x + 1, y + 1);
	 TrueClrtobot();
         for (x++; x < cdb_t_lines; x++){
	    for (; y < cdb_t_cols; y++){
	       cdb_curscr.line[x][y] = cdb_curscr.attr[x][y] = CHAR_BLANK;
	       if (cdb_newscr.line[x][y] == CHAR_BLANK &&
	           cdb_newscr.attr[x][y] == CHAR_BLANK){
	          cdb_newscr.line[x][y] = NULL;
		  if (cdb_newscr.s_ncount[x] < cdb_t_cols)
		     cdb_newscr.s_ncount[x]++;
		  }
	       }
	    y = 0;
	    }
      }

   static optlines()
      {
         register int x, y;
         
	 for (x = 0, y = cdb_t_cols - 1; x < cdb_t_lines; x++)
	    if (cdb_newscr.line[x][y] != NULL)
	       optClrtoeol(x);
      }

   static optClrtoeol(x)	/* provide an optomized Clrtoeol */
      int x;

      {
         register int y, save_y;
	 int clrflag = 1;

         /* fix up both screens, and ncounts -- and actual display */
	 if (cdb_newscr.s_ncount[x] == 0)
	    y = 0;
	 else{
	    for (y = cdb_t_cols - 1; y >= 0; y--)
	       if (cdb_newscr.line[x][y] == NULL)
		  break;
	    y++;
	    }
	 save_y = y;
	 for (; y < cdb_t_cols; y++){
	    if (clrflag && 
	        (cdb_curscr.line[x][y] != CHAR_BLANK || 
		 cdb_curscr.attr[x][y] != CHAR_BLANK)){
	       TrueMove(x + 1, save_y + 1);
	       TrueClrtoeol();
	       clrflag = 0;
	       }
	    cdb_curscr.line[x][y] = cdb_curscr.attr[x][y] = CHAR_BLANK;
	    if (cdb_newscr.line[x][y] == CHAR_BLANK &&
	        cdb_newscr.attr[x][y] == CHAR_BLANK){
	       cdb_newscr.line[x][y] = NULL;
	       if (cdb_newscr.s_ncount[x] < cdb_t_cols)
		  cdb_newscr.s_ncount[x]++;
	       }
	    }
      }
