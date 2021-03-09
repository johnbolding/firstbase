/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: t_fld_ed.c,v 9.0 2001/01/09 02:55:41 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char T_field_editor_sid[] = "@(#) $Id: t_fld_ed.c,v 9.0 2001/01/09 02:55:41 john Exp $";
#endif

#include <dbdmrg_e.h>

#include <setjmp.h>
#include <signal.h>
static jmp_buf jmp_env;
static RETSIGTYPE sigalrm();

int sub_top_row, sub_bot_row, sub_col, sub_scr_top, sub_scr_cur;
int last_sub_scr_top;
static char *HLP_DBDMRG_FIELD = "dbdmrg_field.hlp";
int fullsize, halfsize;
static char command;

static checkarrow();

extern short int cdb_sfields;
extern fb_field **cdb_sp;

/*
 * t_field_editor - provide interface for actual fb_field selection.
 *
 *	attempt at an editor style interface of selecting the token
 */

   t_field_editor(t, row, col, nlines)
      fb_token *t;
      int row, col, nlines;

      {
         int i, count, st = FB_AOK;
	 char buffer[FB_MAXLINE];


         sub_top_row = row;
         sub_bot_row = row + nlines - 1;
         sub_col = col;
         sub_scr_top = 0;
         sub_scr_cur = 0;
	 sub_fb_display();
	 fullsize = nlines - 1;
	 halfsize = fullsize / 2;
	 command = NULL;
	 for (; st != FB_END && st != FB_ABORT; ){
	    st = FB_AOK;
	    sub_test_redraw();
	    sub_put_cursor(1);
            command = NULL;
	    setjmp(jmp_env);
	    signal(SIGALRM, sigalrm);
            if (command == '\033')
               break;
	    read(0, &command, 1);
	    count = 0;
	    if (clear_lastline){
	       fb_move(cdb_t_lines, 1);
	       fb_clrtoeol();
	       put_cursor();
	       clear_lastline = 0;
	       }
	    if (isdigit(command) && command != '0'){
	       buffer[0] = command;
	       i = 1;
	       for (; read(0, &command, 1) == 1;){
		  if (isdigit(command))
		     buffer[i++] = command;
		  else
		     break;
		  }
	       buffer[i] = NULL;
	       count = atoi(buffer);
	       }
	    switch (command){
               case '$':
                  return(CHAR_DOLLAR);
               case 'h':
	       case '-':
	       case '\012':
	       case '\015':
	          st = FB_END;
                  break;
	       case '\033':			/* FB_ESCAPE */
	          st = checkarrow();
	          break;
	       case 'j':			/* j ^N - move down */
               case '\016':			/* ^N */
	          st = sub_cell_down();
		  break;
	       case 'k':			/* k - move up */
               case '\020': 			/* ^P */
	          st = sub_cell_up();
		  break;
	       case '\002':			/* ^B - up full fb_page */
	          st = sub_line_up(fullsize, 1);
	          break;
	       case '\004':			/* ^D - down half fb_page */
	          st = sub_scroll_down(halfsize, 1);
	          break;
	       case '\005':			/* ^E - down one line */
	          st = sub_scroll_down(count, 0);
	          break;
	       case '\006':			/* ^F - foward one fb_page */
	          st = sub_line_down(fullsize, 1);
	          break;
	       case '\025':			/* ^U - up half fb_page */
	          st = sub_scroll_up(halfsize, 1);
	          break;
	       case '\030':			/* ^X - Abort out */
	          st = FB_ABORT;
	          break;
	       case '\031':			/* ^Y - up one line */
	          st = sub_scroll_up(count, 0);
	          break;
	       default:
	          st = FB_ERROR;
		  break;
	       }
	    if (st == FB_ERROR){
	       fb_bell();
               fb_refresh();
               }
	    }

         if (st != FB_ABORT){
            if (sub_scr_cur >= 0 && sub_scr_cur < cdb_sfields)
               t->t_field = cdb_sp[sub_scr_cur];
            else{
               t->t_field = NULL;
               fb_serror(FB_MESSAGE, "Could not set token field pointer.", NIL);
               }
            }
         return(st);
      }

   sub_test_redraw()
      {
	 if (last_sub_scr_top != sub_scr_top){
            sub_fb_display();
	    last_sub_scr_top = sub_scr_top;
	    }
       }

   static checkarrow()
      {
	 int size, i, st = FB_ERROR;
	 char buf[10], *p;

         if ((size = strlen(cdb_KU)) == 0)
	    return(FB_ERROR);
	 p = buf;
	 *p++ = '\033';
         alarm(1);
	 for (i = 1; i < size; i++){
	    read(0, p, 1);
	    if (*p == '\033')
	       break;
	    p++;
	    }
	 *p = NULL;
	 alarm(0);
	 if (equal(buf, cdb_KU))
	    st = sub_cell_up();
	 else if (equal(buf, cdb_KD))
	    st = sub_cell_down();
	 else if (equal(buf, cdb_KL))
	    st = FB_END;
	 else if (equal(buf, cdb_KR))
	    st = sub_cell_up();
	 else{
	    fb_bell();
	    }
	 return(st);
      }

   static RETSIGTYPE sigalrm()
      {
         command = '\033';
	 longjmp(jmp_env, 1);
      }
