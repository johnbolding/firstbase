/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: scanset.c,v 9.0 2001/01/09 02:55:36 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Scanset_sid[] = "@(#) $Id: scanset.c,v 9.0 2001/01/09 02:55:36 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char *FMT1 = "%3d> %s";
static char *FMT2 = "<%c%c%6d>";
/* 
 *  scanset - set up emtpy screen of cdb_keymap id's 
 */

   db_scanset(akp, top, maxc)
      fb_field *akp[];
      int   top, maxc;
      
      {
         int   row;
	 fb_field *k;
	 char link_c;
         
	 fb_move(4, 1), fb_clrtobot();
         for (row = 4; row <= 22; row += 2){
            if (top >= maxc)		/* akp[maxc] == del or null */
               break;
            fb_move(row, 1);
            k = akp[top];		/* top is zero based */
            fb_printw(FMT1, top + 1, k->id);
	    /*
	    if (top < 99 && k->help != NULL && strlen(k->help) > 0){
	       fb_move(row, 1); fb_s_putw(CHAR_STAR);
	       }
	    */
	    fb_move(row, 71);
	    if (k->dflink == NULL)
	       link_c = FB_BLANK;
	    else
	       link_c = FB_LINK;
	    fb_printw(FMT2, k->type, link_c, k->size);
            top++;
            }
	 fb_infoline();
      }
