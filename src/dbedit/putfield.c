/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: putfield.c,v 9.1 2001/01/16 02:46:27 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Putfield_sid[] = "@(#) $Id: putfield.c,v 9.1 2001/01/16 02:46:27 john Exp $";
#endif

#include <dbedit.h>

static char out[51];			/* *s on 'F' type is too short */
	 				/* for comments, hence 'out' */
/* 
 *  db_putfield - local to dbedit.
 *	print a fb_field on row. row is doubled before use.
 *	Note: putfield blindly Walks on s for FB_OFNUMERICS and DATES.
 */

   void db_putfield(fld, k, s, row)
      fb_field *k;
      int row, fld;
      char *s;
   
      {
         char *line, c = FB_BLANK, *p, link_c;
	 int size = 0;

         row *= 2;
         line = cdb_bfld;		/* display uses cdb_afld--- careful */
         if (k->type != FB_BINARY){
            fb_formfield(line, s, k->type, k->size);
            strcpy(s, line);
            }
	 if (k->type == FB_FORMULA){
	    strcpy(out, s);
	    s = out;			/* make s point to out for space */
	    }
         fb_move(row, 1), fb_clrtoeol();
         fb_printw(SYSMSG[S_PUT_FMT1], fld, k->id);
	 /*
	 if (fld <= 99 && k->help != NULL && strlen(k->help) > 0){
	    fb_move(row, 1); fb_s_putw(CHAR_STAR);
	    }
	 */
         if (k->type != FB_BINARY){
            size = strlen(s);
            if (size <= 50)
               c = FB_BLANK;
            else
               c = CHAR_BACKSLASH, s[50] = NULL;
            if ((p = strchr(s, CHAR_NEWLINE)) != 0){
               *p = NULL;
               c = CHAR_BACKSLASH;
               }
            }
	 fb_move(row, 17);
         switch(k->comloc) {
            case 'a':
	       if (size > 0 && k->type != FB_BINARY)
	          fb_stand(s);
	       fb_move(row, 17 + k->size + 2);
	       fb_printw(SYSMSG[S_BLANK_S], k->comment);
               break;
            case 'b':		/* print comment and fall through */
	       fb_printw(SYSMSG[S_S_BLANK], k->comment);
	    default:
	       if (size > 0 && k->type != FB_BINARY)
	          fb_stand(s);
               break;
            }
	 fb_move(row, 67);
	 fb_printw(FB_FCHAR, c);
	 fb_move(row, 71);
	 if (k->dflink == NULL)
	    link_c = FB_BLANK;
	 else
	    link_c = FB_LINK;
	 fb_printw(SYSMSG[S_PUT_FMT2], k->type, link_c, k->size);
      }
