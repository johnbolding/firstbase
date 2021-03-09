/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dfield.c,v 9.1 2001/02/16 19:41:42 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dfield_sid[] = "@(#) $Id: dfield.c,v 9.1 2001/02/16 19:41:42 john Exp $";
#endif

/*
 *  dfield.c - display a very long fb_field (>300) to screen without
 * 	popping into a shell.
 */

#include <dbedit.h>

static char *MSG1 = "Display what field? ";

/* 
 *  dfield - pump a fb_field to the screen
 */
 
   void dfield(p)
      char *p;
      
      {
	 int st, num, savescan;
	 fb_field *f;
	 
	 if (p[1] == NULL || !(isdigit(p[1]))){
	    fb_fmessage(MSG1);
	    st = fb_input(cdb_t_lines, 27, 3, 0, FB_INTEGER, (char *) &num, 
	          FB_ECHO, FB_END, FB_CONFIRM);
	    if (st == FB_END || st == FB_ABORT)
	       return;
	    }
	 else
	    num = atoi(p+1);
	 num--;
	 if (num >= 0 && num < cdb_sfields){
            f = cdb_sp[num];
	    if (f->size > FB_LONGFIELD){
	       fb_d_dfield(f);
	       return;
	       }
	    else if (f->size > FB_SCREENFIELD){
	       savescan = scanner;
	       scanner = 1;
	       fb_longinput(f, cdb_afld, num, 0);
	       scanner = savescan;
	       return;
	       }
	    }
	 fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
      }
