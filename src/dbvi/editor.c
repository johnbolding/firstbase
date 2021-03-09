/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: editor.c,v 9.0 2001/01/09 02:56:05 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Editor_sid[] = "@(#) $Id: editor.c,v 9.0 2001/01/09 02:56:05 john Exp $";
#endif

#include <dbvi_ext.h>

#include <setjmp.h>
#include <signal.h>
jmp_buf jmp_env;
static RETSIGTYPE sigalrm();

static char *HLP_DBVI = "dbvi.hlp";
static char *EXITMSG  = "Write File and Exit? y=Yes, <RETURN>=No";

/* 
 *  editor - main loop of the dbvi spreadsheet style editor.
 */
 
   editor()
      
      {      
         int i, st = FB_AOK, count, dotcom;
	 char com, buffer[FB_MAXLINE], lastcom;
	 column *p;
	 crec *c, *crec_last = NULL;

	 dbvi_display();
	 genstat(-1);
	 col_leftcorn = col_mhead;
	 crec_leftcorn = crec_mhead;
	 fullsize = (cdb_t_lines - 1) - (calc_row + 1);
	 halfsize = fullsize / 2;
	 com = NULL;
	 for (; st != FB_END; ){
	    st = FB_AOK;
	    test_redraw();
	    fb_scrstat("Command Level");
	    if (crec_last != crec_current)
	       checklink(3, crec_current);
	    crec_last = crec_current;
	    put_cursor(1);
	    setjmp(jmp_env);
	    signal(SIGALRM, sigalrm);
	    read(0, &com, 1);
	    if (com != '.'){
	       count = 0;
	       dotcom = 0;
	       }
	    else{
	       dotcom = 1;
	       com = lastcom;
	       }
	    if (clear_lastline){
	       fb_move(cdb_t_lines, 1);
	       fb_clrtoeol();
	       put_cursor(1);
	       clear_lastline = 0;
	       }
	    if (isdigit(com) && com != '0'){
	       buffer[0] = com;
	       i = 1;
	       for (; read(0, &com, 1) == 1;){
		  if (isdigit(com))
		     buffer[i++] = com;
		  else
		     break;
		  }
	       buffer[i] = NULL;
	       count = atoi(buffer);
	       }
	    switch (com){
	       case '-':
	          st = FB_END;
	          if (modified){
		     if (fb_mustbe(CHAR_y, EXITMSG, cdb_t_lines, 1) == FB_AOK)
			writefile();
		     else
		        st = FB_AOK;
		     }
		  break;
	       case '\002':			/* ^B - up full fb_page */
	          st = line_up(fullsize, 1);
	          break;
	       case '\004':			/* ^D - down half fb_page */
	          st = scroll_down(halfsize, 1);
	          break;
	       case '\005':			/* ^E - down one line */
	          st = scroll_down(count, 0);
	          break;
	       case '\006':			/* ^F - foward one fb_page */
	          st = line_down(fullsize, 1);
	          break;
	       case '\007':			/* ^G - print stat line */
	          st = genstat(1);
	          break;
	       case '\014':
	       case '\022':			/* ^L ^R */
	          fb_redraw();
		  break;
	       case '\016':			/* ^N - next col page right */
	          sput_cursor(0);
		  if (col_phead == col_mtail)
		     st = FB_ERROR;
		  else{
		     col_phead = col_ptail;
		     col_current = col_phead;
		     }
		  break;
	       case '\020':			/* ^P - prev col page rleft */
	          sput_cursor(0);
		  if (col_phead == col_mhead)
		     st = FB_ERROR;
		  else{
		     set_rightcorner(col_phead);
		     col_current = col_phead;
		     }
		  break;
	       case '\025':			/* ^U - up half page */
	          st = scroll_up(halfsize, 1);
	          break;
	       case '\031':			/* ^Y - up one line */
	          st = scroll_up(count, 0);
	          break;
	       case '\033':			/* FB_ESCAPE */
	          st = checkarrow();
	          break;
	       case 'C':			/* C - modify down til EOC */
	          modified = 1;
	          fb_scrstat("Field Level: COL");
		  st = edit_col();
		  break;
	       case 'G':			/* G - end of file */
	          st = line_goto(count);
	          break;
	       case 'H':			/* J - last line of window */
	          sput_cursor(0);
		  col_current = col_phead;
	          break;
	       case 'I':			/* I - mid column of window */
	          sput_cursor(0);
		  for (i = 1, p = col_phead; p != col_ptail; i++)
		     p = p->p_next;
		  for (i = i / 2, p = col_ptail; i > 0; i--)
		     p = p->p_prev;
		  col_current = p;
	          break;
	       case 'J':			/* L - left edge of window */
	          sput_cursor(0);
		  crec_current = crec_ptail;
	          break;
	       case 'K':			/* K - first line of window */
	          sput_cursor(0);
		  crec_current = crec_phead;
	          break;
	       case 'L':			/* L - right edge of window */
	          sput_cursor(0);
		  col_current = col_ptail;
	          break;
	       case 'M':			/* M - mid line of window */
	          sput_cursor(0);
		  for (i = 1, c = crec_phead; c != crec_ptail; i++)
		     c = c->c_next;
		  for (i = i / 2, c = crec_ptail; i > 0; i--)
		     c = c->c_prev;
		  crec_current = c;
	          break;
	       case 'O':			/* O - open lines above */
	          st = line_open(crec_current->c_prev);
		  break;
	       case 'o':			/* o - open lines here */
	          st = line_open(crec_current);
		  break;
	       case 'P':			/* P  - fb_put lines above */
	          st = line_put(crec_current->c_prev);
		  lastcom = com;
		  break;
	       case 'p':			/* p  - fb_put lines here */
	          st = line_put(crec_current);
		  lastcom = com;
		  break;
	       case 'R':			/* R - modify until EOR */
	          modified = 1;
	          fb_scrstat("Field Level: ROW");
		  st = edit_row();
		  break;
	       case 'Y':			/* Y  - yank lines */
	          st = line_yank(crec_current, count);
		  break;
	       case 'Z':			/* ZZ - write and exit */
	          read(0, &com, 1);
		  if (com == 'Z'){
		     sput_cursor(0);
		     if (modified)
		        writefile();
		     st = FB_END;
		     }
		  else
		     st = FB_ERROR;
		  break;
	       case 'd':			/* dd - fb_delete line */
	          if (!dotcom || lastcom != 'd')
	             read(0, &com, 1);
		  if (com == 'd'){
		     st = line_delete(crec_current, count);
		     lastcom = 'd';
		     }
		  else
		     st = FB_ERROR;
		  break;
	       case 'i':			/* a,i,r - add/insert fb_cell */
	       case 'a':
	       case 'r':
	       case 'c':
		  modified = 1;
	          fb_scrstat("Field Level: CELL");
		  st = edit_cell();
		  break;
	       case 'l':			/* l w - move right */
	       case 'w':
	          st = cell_right();
		  break;
	       case 'h':			/* h b - move left */
	       case 'b':
	          st = cell_left();
		  break;
	       case 'j':			/* j ^J ^M - move down */
	       case '\012':
	       case '\015':
	          st = cell_down();
		  break;
	       case 'k':			/* k - move up */
	          st = cell_up();
		  break;
	       case 'z':
	          read(0, &com, 1);
		  sput_cursor(0);
		  if (com == FB_NEWLINE || com == FB_CRET) /* zCR - at top */
		     i = 0;
		  else if (com == '-')		/* z- - at bottom */
		     i = fullsize;
		  else if (com == '.')		/* z. - at middle */
		     i = halfsize;
		  else
		     st = FB_ERROR;
		  if (st != FB_ERROR){
		     for (c = crec_current; i > 0; i--){
		        if (c->c_prev == NULL)
			   break;
			c = c->c_prev;
			}
		     crec_phead = c;
		     }
		  break;
	       case '0':			/* 0 - first mem column */
	          sput_cursor(0);
	          col_phead = col_current = col_mhead;
		  break;
	       case '$':			/* $ - last mem column */
	          sput_cursor(0);
		  set_rightcorner(col_mtail);
		  col_current = col_mtail;
		  break;
	       case ':':			/* : - colon commands */
		  st = colon_command();
		  break;
	       case '?':			/* ? - fb_help file display */
	          fb_fhelp(HLP_DBVI);
		  dbvi_display();
		  break;
	       default:
	          st = FB_ERROR;
		  break;
	       }
	    if (st == FB_ERROR)
	       fb_bell();
	    lastcom = com;
	    }
      }

   test_redraw()
      {
	 if (col_leftcorn != col_phead || crec_leftcorn != crec_phead){
	    set_screen();
	    clear_cells();
	    put_cline();
	    draw_cells();
	    col_leftcorn = col_phead;
	    crec_leftcorn = crec_phead;
	    }
       }

   checkarrow()
      {
	 int size, i;
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
	    st = cell_up();
	 else if (equal(buf, cdb_KD))
	    st = cell_down();
	 else if (equal(buf, cdb_KL))
	    st = cell_left();
	 else if (equal(buf, cdb_KR))
	    st = cell_right();
	 else{
	    fb_bell();
	    }
	 return(st);
      }

   static RETSIGTYPE sigalrm()
      {
         fb_bell();
	 fb_refresh();
	 longjmp(jmp_env, 1);
      }
