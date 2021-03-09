/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: editor.c,v 9.0 2001/01/09 02:55:40 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Editor_sid[] = "@(#) $Id: editor.c,v 9.0 2001/01/09 02:55:40 john Exp $";
#endif

#include <dbdmrg_e.h>

#include <setjmp.h>
#include <signal.h>
static jmp_buf jmp_env;
static RETSIGTYPE sigalrm();

static char *HLP_DBDMRG = "dbdmrg.hlp";
static char *EXITMSG  = "Write File and Exit? y=Yes, <RETURN>=No";
int fullsize, halfsize;
extern short int cdb_use_insert_char;

char com, lastcom;

static fb_aline *acur = NULL;

static checkarrow();
static checkmeta();

/* 
 *  editor - main loop of the dbdmrg editor.
 */
 
   editor()
      
      {      
         int i, count, dotcom;
	 char buffer[FB_MAXLINE], subcom = NULL;
         fb_aline *a;

	 mrg_display();
	 genstat(-1);
	 fullsize = (cdb_t_lines - 5);
	 halfsize = fullsize / 2;
	 com = NULL;
         leftcorn = mpcur->mp_leftcorn;
         atop = mpcur->mp_atop;
	 for (; st != FB_END; ){
	    st = FB_AOK;
	    test_redraw();
            fb_scrstat("Command Level");
	    put_cursor();
            st = test_dollar();
	    setjmp(jmp_env);
	    signal(SIGALRM, sigalrm);
            if (st > 0)
               com = st;
            else
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
	       put_cursor();
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
	       case '\016':			/* ^N - next col fb_page right */
	       case '>':			/* > - next col fb_page right */
                  scroll_right(15);
		  break;
	       case '\020':			/* ^P - prev col fb_page rleft */
	       case '<':			/* < - next col fb_page right */
                  scroll_left(15);
		  break;
	       case '\025':			/* ^U - up half fb_page */
	          st = scroll_up(halfsize, 1);
	          break;
	       case '\030':			/* ^X - meta X commands */
                  st = checkmeta();
	          break;
	       case '\031':			/* ^Y - up one line */
	          st = scroll_up(count, 0);
	          break;
	       case '\033':			/* FB_ESCAPE */
	          st = checkarrow();
	          break;
	       case 'G':			/* G - end of file */
	       case 'g':
	          st = line_goto(count);
	          break;
	       case 'H':			/* H - top line of window */
                  mpcur->mp_acur = mpcur->mp_atop;
	          break;
               case 'J':
                  st = line_join();
	          lastcom = com;
                  break;
	       case '(':			/* ( - left edge of window */
                  mpcur->mp_col = mpcur->mp_leftcorn;
	          break;
	       case ')':			/* ) - right edge of window */
                  mpcur->mp_col = mpcur->mp_rightcorn;
                  set_screen();
	          break;
	       case 'D':			/* D - bottom line of window */
                  st = delete_rest_of_line();
	          break;
	       case 'L':			/* L - bottom line of window */
                  mpcur->mp_acur = mpcur->mp_abot;
	          break;
	       case 'M':			/* M - mid line of window */
		  for (i = 1, a = mpcur->mp_atop; a != mpcur->mp_abot; i++)
		     a = a->a_next;
		  for (i = i / 2, a = mpcur->mp_abot; i > 0; i--)
		     a = a->a_prev;
		  mpcur->mp_acur = a;
	          break;
	       case 'O':			/* O - open lines above */
	          st = line_open(mpcur->mp_acur->a_prev);
		  break;
	       case 'o':			/* o - open lines here */
	          st = line_open(mpcur->mp_acur);
		  break;
	       case 'P':			/* P  - fb_put lines above */
	          st = line_put(mpcur->mp_acur->a_prev);
		  lastcom = com;
		  break;
	       case 'p':			/* p  - fb_put lines here */
	          st = line_put(mpcur->mp_acur);
		  lastcom = com;
		  break;
	       case 'Y':			/* Y  - yank lines */
	       case 'y':
	          st = line_yank(mpcur->mp_acur, count);
		  break;
	       case 'Z':			/* ZZ - write and exit */
	          read(0, &com, 1);
		  if (com == 'Z'){
		     if (modified)
		        writefile();
		     st = FB_END;
		     }
		  else
		     st = FB_ERROR;
		  break;
	       case 'd':			/* dd - delete line */
	          if (!dotcom || lastcom != 'd')
	             read(0, &subcom, 1);
		  if (subcom == 'd'){
		     st = line_delete(count);
		     lastcom = 'd';
                     subcom = 'd';
		     }
		  else if (subcom == 'w'){
		     st = fb_word_delete();
		     lastcom = 'd';
                     subcom = 'w';
                     }
		  else
		     st = FB_ERROR;
		  break;
	       case 'i':			/* i - insert */
                  st = insert();
	          lastcom = com;
                  break;
	       case 'a':			/* a - append */
                  cell_right(1);
                  test_redraw();
                  put_cursor();
                  fb_refresh();
                  st = insert();
	          lastcom = com;
                  break;
	       case 'r':			/* r - one replace */
		  st = replace(count);
	          lastcom = com;
		  break;
	       case 'R':			/* R - continuous Replace */
		  st = replace(linewidth - mpcur->mp_col + 1);
	          lastcom = com;
		  break;
               case 'x':			/* x - delete one character */
                  st = fb_delete(count);
	          lastcom = com;
                  break;
	       case 'w':			/* w - word right */
                  st = word_right();
                  break;
	       case 'e':			/* e - end of word */
                  st = end_word_right();
                  break;
	       case 'l':			/* l - move right */
	       case ' ':			/* FB_SPACE - move right */
	          st = cell_right(1);
		  break;
	       case 'b':			/* b - back one word */
                  st = word_left();
                  break;
	       case 'h':			/* h -  move left */
	          st = cell_left(1);
		  break;
	       case 'j':			/* j ^J ^M - move down */
	          st = cell_down();
		  break;
	       case 'k':			/* k - move up */
	          st = cell_up();
		  break;
	       case '\012':			/* FB_NEWLINE */
	       case '\015':			/* FB_CRET */
	          st = cell_down();
                  if (st == FB_AOK){
                     mpcur->mp_leftcorn = 1;
                     mpcur->mp_col = 1;
                     }
                  break;
	       case 'z':
	          read(0, &com, 1);
		  if (com == FB_NEWLINE || com == FB_CRET)	/* zCR - at top */
		     i = 0;
		  else if (com == '-')		/* z- - at bottom */
		     i = fullsize;
		  else if (com == '.')		/* z. - at middle */
		     i = halfsize;
		  else
		     st = FB_ERROR;
		  if (st != FB_ERROR){
                     if (cdb_use_insert_char)
                        clear_all_lines();
		     for (a = mpcur->mp_acur; i > 0; i--){
		        if (a->a_prev == NULL)
			   break;
			a = a->a_prev;
			}
		     mpcur->mp_atop = a;
		     }
		  break;
	       case '0':			/* 0 - first mem column */
                  mpcur->mp_leftcorn = 1;
                  mpcur->mp_col = 1;
		  break;
	       case '$':			/* $ - last mem column */
                  mpcur->mp_leftcorn = linewidth;
                  mpcur->mp_col = linewidth;
                  set_screen();
		  break;
	       case ':':			/* : - colon commands */
		  st = colon_command();
		  break;
	       case '?':			/* ? - fb_help file display */
	          fb_fhelp(HLP_DBDMRG);
		  mrg_display();
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
      }

   test_redraw()
      {
	 if (leftcorn != mpcur->mp_leftcorn || atop != mpcur->mp_atop ||
               acur != mpcur->mp_acur){
	    set_screen();
            mrg_display();
	    leftcorn = mpcur->mp_leftcorn;
            atop = mpcur->mp_atop;
            acur = mpcur->mp_acur;
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
	    st = cell_up();
	 else if (equal(buf, cdb_KD))
	    st = cell_down();
	 else if (equal(buf, cdb_KL))
	    st = cell_left(1);
	 else if (equal(buf, cdb_KR))
	    st = cell_right(1);
	 return(st);
      }

/*
 * checkmeta - invoked by ^X - times out if no other command given
 */

   static checkmeta()
      {
	 char p;
         int st = FB_ERROR;

         st = FB_ERROR;
         alarm(5);
	 read(0, &p, 1);
         switch(p){
            case '\005':				/* ^X ^E */
               alarm(0);
               mface();
               st = FB_AOK;
               break;
            }
	 alarm(0);
	 return(st);
      }

   static RETSIGTYPE sigalrm()
      {
         fb_bell();
	 fb_refresh();
	 longjmp(jmp_env, 1);
      }
