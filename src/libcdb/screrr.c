/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: screrr.c,v 9.1 2001/02/05 18:15:05 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Screrr_sid[] = "@(#) $Id: screrr.c,v 9.1 2001/02/05 18:15:05 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <panic.h>

extern short cdb_error_row_offset;
extern short cdb_screrr_cx_writeflag;
extern char *cdb_hitanykey;		/* to override HIT ANY KEY message */
extern short int cdb_cgi_flag;

#if PANIC
static char PANIC_KEY = 'X';
static char *PANIC_FMT1 = "%12s";
static char *BYE = "bye!";
#endif

/* 
 *  print error message in screen error location 
 */
 
   fb_screrr(s)
      char *s;

      {
         char c = NULL, *p;
	 char line[FB_MAXLINE], buf[10];
	 int erow, ecol, st;
         FILE *fs;
	 
	 erow = cdb_t_lines - cdb_error_row_offset;
         if (cdb_batchmode || cdb_t_lines <= 0 || cdb_t_cols <= 0 ||
               !isatty(1) || !isatty(0)){
            fs = stderr;
            if (cdb_cgi_flag){
               fs = stdout;
               fprintf(fs, "<P>\n");
               }
	    fprintf(fs, SYSMSG[S_FMT_S_S], s, SYSMSG[S_STRING_NEWLINE]);
	    fflush(fs);
	    }
	 else{
            fb_cx_push_env("C", CX_HITANY_SELECT, NIL);
            fb_cx_write(1);		/* since scanf is used, not input() */
            fb_move(erow, 1), fb_clrtoeol(), fb_bell();
            if (cdb_hitanykey == NULL)
               p = SYSMSG[S_HIT_ANY];
            else
               p = cdb_hitanykey;
            ecol = cdb_t_cols - strlen(p) - 2;
            fb_reverse(fb_pad(line, s, cdb_t_cols));

            fb_store_lastline();
            fb_move(cdb_t_lines, 1), fb_clrtoeol();
            fb_move(cdb_t_lines, ecol);
            fb_prints(p);
            fb_s_putw(FB_BLANK); fb_s_putw(CHAR_DOT);
            fb_move(cdb_t_lines, cdb_t_cols - 1);
            fb_refresh();

            st = fb_input(-cdb_t_lines, -(cdb_t_cols - 1),
               -1, 0, FB_ALPHA, buf, FB_ECHO, -FB_OKEND, FB_NOCONFIRM);

            if (st == FB_AOK)
               c = buf[0];
            else
               c = NULL;

#if PANIC
	    /* early on this was needed on some wimpy machines (morrow) */
	    if (c == PANIC_KEY)
               panic_count++;
	    if (panic_count >= PANIC_THRESH){
	       fb_move(cdb_t_lines, 60); fb_clrtoeol();
	       fb_printw(SYSMSG[S_FB]);
	       fb_refresh();
	       scanf(PANIC_FMT1, panic_area);
	       if (strncmp(panic_area, PANIC_PWD, 11) == 0)
	          fb_xerror(FB_MESSAGE, BYE, NIL);
               panic_count = 0;
	       }
#endif /* PANIC */

            fb_refresh_lastline();
	    fb_infoline();

            /*
             * long as I am here, disable this noise from flying window time.
             * fb_cx_pop_env();
             * if (cdb_screrr_cx_writeflag)
             *    fb_cx_write(1); # needs to be ... could be input calling #
             * cdb_screrr_cx_writeflag = 1;
             */
	    }
      }
