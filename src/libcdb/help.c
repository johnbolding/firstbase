/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: help.c,v 9.0 2001/01/09 02:56:26 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Help_sid[] = "@(#) $Id: help.c,v 9.0 2001/01/09 02:56:26 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char *FMT3 = "%3d> %-10s %c %6d";
static char *FMT4 = " [%s] ";
static char *FMT5 = "%3d) %-10s %c .....";
char *HLP_HELP = "help.hlp";

extern short cdb_screrr_cx_writeflag;
extern char *cdb_tempdir;			/* setup ENVIRONMENT var */
extern short int cdb_edit_input;
static char *TNAME = "cdbXXXXXX";

#if FB_PROTOTYPES
static show_help(int tp, fb_database *hp);
#else /* FB_PROTOTYPES */
static show_help();
#endif /* FB_PROTOTYPES */

/* 
 *  fb_help - print help page to screen. 
 *    s is '?i' or '?d' or '?#' or '?' or '??'
 *    ?# where # is an integer pulls fb_help file from that field
 */
 
   void fb_help(s, hp)
      char *s;
      fb_database *hp;
   
      {
	 register row;
         int n, lastone, n_pauses = 0;
         register fb_field **k;
         char msg[20], c, fname[FB_MAXNAME];
         FILE *fs;

         fb_move(3, 1); fb_clrtobot(); fb_refresh();
         switch(s[1]){
	    case 'i':
	       k = cdb_ip;
	       lastone = hp->ifields - 2;
	       fb_basename(fname, hp->dindex);
	       break;
	    case 'd':
	    case NULL:
	    case '\040':		/* FB_BLANK */
	       k = cdb_kp;
	       lastone = hp->nfields - 1;
	       fb_basename(fname, hp->dbase);
	       break;
	    case 'h':		/* fb_help files */
	    case 'f':		/* formulas */
	       show_help(s[1], hp);
	       return;
	    case 'b':		/* describe bindings */
               if (!cdb_edit_input)
                  return;
	       sprintf(fname, SYSMSG[S_FMT_SSLASHS], cdb_tempdir, TNAME);
               close(mkstemp(fname));
               fs = fopen(fname, "w");
               if (fs != NULL){
                  fb_describe_bindings(fs);
                  fclose(fs);
                  fb_fhelp(fname);
                  unlink(fname);
                  }
               return;
	    default:
	       if (!isdigit(s[1]))
	          fb_fhelp(HLP_HELP);
	       else {
		  n = atoi(s+1) - 1;
		  if (n >= 0 && n < hp->nfields)
		     fb_fhelp(cdb_kp[n]->help);
		  }
	       return;
	    }
         fb_cx_push_env("QS", CX_KEY_SELECT, NIL);
	 for(n = 0; k != NULL;){
	    fb_move(3, 1); fb_clrtobot(); fb_refresh();
	    for (row = 4; row <= cdb_t_lines - 2; row++){
	       fb_move(row, 15);
	       fb_printw(FMT3, n+1, (*k)->id, (*k)->type, 
	          (*k)->size);
	       if ((*k)->idefault != NULL)
	          fb_printw(FMT4, (*k)->idefault);
	       if (++n > lastone){
		  if (s[1] != NULL || n_pauses == 0){
                     cdb_screrr_cx_writeflag = 0;
		     fb_screrr(NIL);
		     fb_move(3, 1); fb_clrtobot(); fb_refresh();
		     }
                  fb_cx_pop_env();
		  return;
		  }
	       k++;
	       }
	    fb_infoline();
	    sprintf(msg, SYSMSG[S_DISPLAY], n, lastone);
	    fb_scrhlp(msg);
	    fb_fmessage(SYSMSG[S_SPACE_BAR]);
            fb_cx_write(1);		/* since scanf is used, not input() */
            n_pauses++;
	    for (;;){
               read(0, &c, 1);
	       if (c == FB_REDRAW1 || c == FB_REDRAW2)
		  fb_redraw();
	       else
		  break;
	       }
	    if (c != FB_BLANK)
	       break;
	    }
         fb_cx_pop_env();
	 if (s[1] != NULL){
	    fb_move(3, 1); fb_clrtobot(); fb_refresh();
	    }
      }

/*
 *  show_help - show all the help files or formulas in the fields 
 *      tp == h means help files, else formulas
 */
 
    static show_help(tp, hp)
      int tp;
      fb_database *hp;
      
      {
         register int i, j;
	 char ms[30], c;
	 
         fb_cx_push_env("QS", CX_KEY_SELECT, NIL);
	 for (j = 0; j < hp->nfields; ){
	    fb_move(3, 1); fb_clrtobot(); fb_refresh();
	    for (i = 4; i <= cdb_t_lines-2 && j < hp->nfields; i++, j++){
	       fb_move(i, 1);
	       fb_printw(FMT5, j+1, cdb_kp[j]->id, cdb_kp[j]->type);
	       if (tp == CHAR_h){
		  if (cdb_kp[j]->help != NULL)
		     fb_printw(FB_FSTRING, cdb_kp[j]->help);
		  }
	       else{	/* else tp is 'f' for formula */
	          if (cdb_kp[j]->type == FB_FORMULA ||
                        cdb_kp[j]->type == FB_LINK)
		     fb_printw(FB_FSTRING, cdb_kp[j]->idefault);
		  }
	       }
	    if (j < hp->nfields){
               fb_cx_write(1);		/* since scanf is used, not input() */
	       fb_infoline();
	       sprintf(ms, SYSMSG[S_DISPLAY], j, hp->nfields-1);
	       fb_scrhlp(ms);
	       fb_fmessage(SYSMSG[S_SPACE_BAR]);
	       fb_refresh();
	       scanf(FB_FCHAR, &c);
	       if (c != FB_BLANK)
		  break;
	       }
	    else{
               cdb_screrr_cx_writeflag = 0;
	       fb_screrr(SYSMSG[S_END_FILE]);
               }
	    }
         fb_cx_pop_env();
         fb_move(3, 1); fb_clrtobot(); fb_refresh();
      }
