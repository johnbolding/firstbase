/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: sfatal.c,v 9.0 2001/01/09 02:56:30 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Sfatal_sid[] = "@(#) $Id: sfatal.c,v 9.0 2001/01/09 02:56:30 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern char *cdb_hitanykey;		/* to override HIT ANY KEY message */
extern short int cdb_cgi_flag;

/* 
 *  sfatal - provide a fatal message and die.
 */
 
   fb_sfatal(e, s)
      int e;
      char *s;
   
      {
         char c, *p;
	 int ecol;
         FILE *fs;
	 
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
	 else {
            fb_cx_push_env("C", CX_HITANY_SELECT, NIL);
            fb_cx_write(1);		/* since scanf is used, not input() */
            fb_move(cdb_t_lines-1, 1), fb_clrtoeol(), fb_bell();
            fb_force(s);
            if (cdb_hitanykey == NULL)
               p = SYSMSG[S_HIT_ANY];
            else
               p = cdb_hitanykey;
            fb_move(cdb_t_lines, 1), fb_clrtoeol();
            ecol = cdb_t_cols - strlen(p) - 2;
            fb_move(cdb_t_lines, ecol), fb_clrtoeol();
            fb_force(p);
            fb_s_putw(FB_BLANK); fb_s_putw(CHAR_DOT); fb_move(cdb_t_lines, 79);
            fb_refresh();
            read(0, &c, 1);
            fb_cx_pop_env();
            fb_move(cdb_t_lines, 1), fb_clrtoeol(); fb_refresh();
	    }
         fb_exit(e);
      }
