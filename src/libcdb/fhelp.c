/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: fhelp.c,v 9.0 2001/01/09 02:56:26 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Fhelp_sid[] = "@(#) $Id: fhelp.c,v 9.0 2001/01/09 02:56:26 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

short clr_aft_fhelp = 1;	/* kludge around screen clear -- hmmm */
extern char *cdb_help;
extern short cdb_screrr_cx_writeflag;

static char *default_helpfile = "nohelp.hlp";

/* 
 *  fhelp - page a file to the screen...using only lines 4...t_lines-2
 *	and no touch of the timeline.
 */
 
   fb_fhelp(f)
      char *f;
      
      {
         FILE *fs;
	 register int i, lc;
	 int eof, j;
	 char line[FB_MAXLINE], c, ms[30], fname[FB_MAXNAME];

         if (f == NULL || *f == NULL){
            /*
             * This used to fail if no user defined object.
             * but, perhaps some kind of help is better than none.
             * even if the help is a no op... so use the default_helpfile.
             *
             * serror(CANT_OPEN, f, NIL);
	     * return(FB_ERROR);
             *
             */
            f = line;
            strcpy(line, default_helpfile);
	    }
         fname[0] = NULL;
         if (access(f, 0) != 0)
            strcpy(fname, cdb_help);
         strcat(fname, f);
         if ((fs = fopen(fname, FB_F_READ)) == NULL){
            strcpy(fname, cdb_help);
            strcat(fname, default_helpfile);
            if ((fs = fopen(fname, FB_F_READ)) == NULL){
               fb_serror(FB_CANT_OPEN, fname, NIL);
               return(FB_ERROR);
               }
	    }
	 for (lc = 0; fgets(line, FB_MAXLINE, fs) != NULL; lc++)
	    ;
	 rewind(fs);
         fb_cx_push_env("QS", CX_KEY_SELECT, NIL);
	 for (j = 0;;){
	    fb_move(3, 1); fb_clrtobot(); fb_refresh();
	    for (eof = 0, i = 4; i <= cdb_t_lines-2; i++){
	       if (fgets(line, FB_MAXLINE, fs) == NULL){
	          eof = 1;
		  break;
		  }
	       line[strlen(line) - 1] = NULL;
	       fb_move(i, 1);
	       fb_printw(FB_FSTRING, line);
	       if (lc == ++j){
	          eof = 1;
		  break;
		  }
	       }
	    if (eof == 1){
	       if (clr_aft_fhelp == 1){
                  cdb_screrr_cx_writeflag = 0;
	          fb_screrr(SYSMSG[S_END_FILE]);
                  }
	       break;
	       }
	    else{
               fb_cx_write(1);		/* since scanf is used, not input() */
	       fb_infoline();
	       sprintf(ms, SYSMSG[S_DISPLAY], j, lc);
	       fb_scrhlp(ms);
	       fb_move(cdb_t_lines, 1);
	       fb_printw(SYSMSG[S_SPACE_BAR]);
	       fb_refresh();
	       for (;;){
                  read(0, &c, 1);
		  if (c == FB_REDRAW1 || c == FB_REDRAW2)
		     fb_redraw();
		  else
		     break;
		  }
	       fb_move(cdb_t_lines, 1); fb_clrtoeol();
	       if (c != FB_BLANK)
	          break;
	       }
	    }
	 fclose(fs);
	 if (clr_aft_fhelp == 1){
	    fb_move(3, 1); fb_clrtobot(); fb_refresh();
	    }
         fb_cx_pop_env();
	 return(FB_AOK);
      }
