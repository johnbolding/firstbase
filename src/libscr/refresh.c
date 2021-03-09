/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: refresh.c,v 9.1 2001/02/05 18:21:28 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Refresh_sid[] = "@(#) $Id: refresh.c,v 9.1 2001/02/05 18:21:28 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#if FB_PROTOTYPES
static void fixrow(int x);
#else /* FB_PROTOTYPES */
static void fixrow();
#endif /* FB_PROTOTYPES */

extern fb_pscreen cdb_curscr, cdb_newscr;
extern char cdb_CM[];
extern char cdb_SO[];
extern char cdb_SE[];
extern char cdb_RV[];
extern char cdb_RE[];
extern char cdb_VB[];

static char lastline[FB_PCOL+1];

/*
 * Refresh - overlay cdb_newscr onto cdb_curscr, painting as needed.
 */

   void fb_refresh()
      {
         int i;

         if (cdb_batchmode)
	    return;
	 /* lineopt(); */
         fb_optomize();
         for (i = 0; i < cdb_t_lines; i++){
	    if (cdb_newscr.s_ncount[i] != cdb_t_cols)
	       fixrow(i);
	    }
	 cdb_curscr.s_x = cdb_newscr.s_x;
	 cdb_curscr.s_y = cdb_newscr.s_y;
	 TrueMove(cdb_curscr.s_x + 1, cdb_curscr.s_y + 1);
	 if (cdb_newscr.s_bell == 'b')
	    Truebell();
	 fflush(stdout);
	 fb_nullscr(&cdb_newscr);
      }

/*
 * fixrow - fix up a row, provide optomized result
 */
   static void fixrow(x)
      int x;

      {
         int y, r_cols;
         int py = 0, pstate, attr;

	 for (pstate = 0, y = 0, r_cols = cdb_t_cols; y < r_cols; y++){
	    switch (pstate){
	       case 0: 
	          if (cdb_newscr.line[x][y] != NULL)
                     if (cdb_curscr.line[x][y] != cdb_newscr.line[x][y] ||
                         cdb_curscr.attr[x][y] != cdb_newscr.attr[x][y]){
		        py = y;
			pstate = 1;
	                cdb_curscr.line[x][y] = cdb_newscr.line[x][y];
	                cdb_curscr.attr[x][y] = cdb_newscr.attr[x][y];
			}
		  break;

	       case 1: 
	          if (cdb_newscr.line[x][y] == NULL || 
	             (cdb_curscr.line[x][y] == cdb_newscr.line[x][y] &&
	              cdb_curscr.attr[x][y] == cdb_newscr.attr[x][y])){
		        TrueMove(x + 1, py + 1);
			attr = CHAR_BLANK;
		        for (; py < y; py++){
			   if (attr != cdb_curscr.attr[x][py]){
			      if (attr == CHAR_s)	/* turn off old attr*/
				 TrueStandend();
			      else if (attr == CHAR_r)
				 TrueReverseend();
			      attr = cdb_curscr.attr[x][py];
			      if (attr == CHAR_s)	/* turn on new attr */
				 TrueStandout();
			      else if (attr == CHAR_r)
				 TrueReverseout();
			      }
			   putc(cdb_curscr.line[x][py], stdout);
			   }
			if (attr == CHAR_s)
			   TrueStandend();
			else if (attr == CHAR_r)
			   TrueReverseend();
			attr = CHAR_BLANK;
			pstate = 0;
			}
		  else{
	             cdb_curscr.line[x][y] = cdb_newscr.line[x][y];
	             cdb_curscr.attr[x][y] = cdb_newscr.attr[x][y];
		     }
		  break;
	       }
	    }
	 if (pstate == 1){
	    TrueMove(x + 1, py + 1);
            attr = CHAR_BLANK;
	    for (y = py; y < r_cols; y++){
	       if (attr != cdb_curscr.attr[x][y]){
		  if (attr == CHAR_s)
		     TrueStandend();
		  else if (attr == CHAR_r)
		     TrueReverseend();
		  attr = cdb_curscr.attr[x][y];
		  if (attr == CHAR_s)
		     TrueStandout();
		  else if (attr == CHAR_r)
		     TrueReverseout();
		  }
	       putc(cdb_curscr.line[x][y], stdout);
	       }
	    if (attr == CHAR_s)
	       TrueStandend();
	    else if (attr == CHAR_r)
	       TrueReverseend();
	    attr = CHAR_BLANK;
	    }
      }

   void fb_store_lastline()

      {
         strncpy(lastline, cdb_curscr.line[cdb_t_lines - 1], cdb_t_cols);
         lastline[cdb_t_cols] = NULL;
      }

   void fb_refresh_lastline()

      {
         fb_move(cdb_t_lines, 1);
         fb_clrtoeol();
         fb_prints(lastline);
      }
