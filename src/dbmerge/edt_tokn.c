/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: edt_tokn.c,v 9.0 2001/01/09 02:55:41 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Edit_token_sid[] = "@(#) $Id: edt_tokn.c,v 9.0 2001/01/09 02:55:41 john Exp $";
#endif

#include <dbdmrg_e.h>

#include <setjmp.h>
#include <signal.h>
static jmp_buf jmp_env;
static RETSIGTYPE sigalrm();

static int tok_row, len_row, sub_state;
static char command;
static char *DELMSG  = "Delete this Token? y=Yes, <RETURN>=No";

#define SUB_WINDOW_SIZE		9
#define SUB_COL_OFFSET		15

static sub_edit();
static input_sub();
static display_sub();
static clear_sub();
static checkarrow();
static flipstate();

/*
 * edit_token - edit/create the token information for the current token
 */

   edit_token(t, new)
      fb_token *t;
      int new;

      {
         fb_aline *a;
         int row, i, j, start_col = 40, start_row, top_row;

         fb_scrstat("Token Level");
         if (t == NULL){
            fb_serror(FB_MESSAGE, "Cannot edit NULL token.", NIL);
            return(FB_ERROR);
            }
         put_row(mpcur->mp_acur);
         a = mpcur->mp_acur;
         row = whichrow(a);
         if (row <= 12)
            start_row = 13;
         else
            start_row = 3;
         j = start_row + 10;
         for (i = start_row; i < j; i++){
            fb_move(i, start_col); fb_clrtoeol(); fb_reverse("|");
            }
         if (start_row == 3){
            fb_move(--i, start_col + 1);
            top_row = start_row;
            }
         else{
            fb_move(start_row, start_col + 1);
            top_row = start_row + 1;
            }
         fb_reverse("----------------------------------------");
         return(sub_edit(t, top_row, start_col, new));
      }

/*
 * sub_edit - edit a token object main loop
 */

   static sub_edit(t, trow, tcol, new)
      fb_token *t;
      int trow, tcol, new;

      {
         int col, sub_tcol, st, firstnew = 0;

         tok_row = trow + 2;
         len_row = tok_row + 4;
         col = tcol + 2;
         fb_move(tok_row - 2, col);
         fb_prints("Token:");
         fb_move(tok_row - 1, col);
         fb_prints("======");
         fb_move(len_row - 2, col);
         fb_prints("Length:");
         fb_move(len_row - 1, col);
         fb_prints("=======");
         sub_tcol = tcol + 15;
         if (new){
            firstnew = 1;
            }
         sub_state = 1;
         for (;;){
            st = 0;
            clear_sub(trow, sub_tcol);
            display_sub(t, col, 0, 0);
            if (firstnew == 0)
               fb_scrhlp("Edit token, -=END, <ESC>=END");
            display_sub(t, col, sub_state, 1);
            fb_refresh();
            command = NULL;
	    setjmp(jmp_env);
	    signal(SIGALRM, sigalrm);
            if (command == '\033')
               break;
            if (firstnew > 0 && firstnew <= 2){
               sub_state = firstnew;
               st = input_sub(t, col);
               if (++firstnew > 2){
                  firstnew = 0;
                  sub_state = 1;
                  }
               continue;			/* CONTINUE */
               }
	    read(0, &command, 1);
            if (command == '-' || command == 'q')
               break;
            else if (command == CHAR_DOLLAR){
               t->t_field = NULL;
               continue;			/* CONTINUE */
               }
            switch(command){
               case '\016':		/* ^N */
               case '\020': 		/* ^P */
                  flipstate();
                  break;
	       case 'j':
	       case '\012':
	       case '\015':
                  if (sub_state == 2 && !new)
                     return(command);
                  flipstate();
                  break;
               case 'k':
                  if (sub_state == 1 && !new)
                     return('k');
                  flipstate();
                  break;
               case 'h':
                  if (new)
                     flipstate();
                  else
                     return(command);
                  break;
               case 'l':
                  if (new)
                     st = input_sub(t, col);
                  else
                     return(command);
                  break;
               case 'w':
               case 'b':
                  if (new)
                     flipstate();
                  else
                     return(command);
                  break;
	       case '\033':			/* FB_ESCAPE */
	          st = checkarrow();
                  switch(st){
                     case FB_ARROWDOWN:
                        if (sub_state == 2 && !new)
                           return('j');
                        flipstate();
                        break;
                     case FB_ARROWUP:
                        if (sub_state == 1 && !new)
                           return('k');
                        flipstate();
                        break;
                     case FB_ARROWLEFT:
                        if (new)
                           flipstate();
                        else
                           return('h');
                        break;
                     case FB_ARROWRIGHT:
                        if (new)
                           st = input_sub(t, col);
                        else
                           return('l');
                        break;
                     }
	          break;
               case 'i': case 'a':
               case 'o': case 'O':
               case 'r': case 'R':
               case 'c': case 'C':
               case '@':
                  st = input_sub(t, col);
                  break;
	       case 'd':			/* dw - word only */
	          read(0, &command, 1);
		  if (command != 'w'){
                     st = FB_ERROR;
                     break;
                     }
                  /* else flow through to the 'x' command */
               case 'x':
                  if (!new && fb_mustbe(CHAR_y, DELMSG, cdb_t_lines, 1) == FB_AOK)
                     st = FB_DELETED;
                  break;
	       default:
	          st = FB_ERROR;
		  break;
               }
            if (command == '\033' && st == FB_ERROR)
               break;
            if (st == CHAR_DOLLAR)
               t->t_field = NULL;
	    else if (st == FB_ERROR){
	       fb_bell();
               fb_refresh();
               }
            else if (st == FB_DELETED)
               break;
            }
         if (st == CHAR_DOLLAR || (!new && st != FB_DELETED))
            st = 'l';
         if (t->t_field == NULL)
            t->t_width = 1;
         return(st);
      }

/*
 * input_sub - interface to fb_input information into the node
 */

   static input_sub(t, col)
      fb_token *t;
      int col;

      {
         int st = FB_AOK;
         char buf[10];

         modified = 1;
         if (sub_state == 1){
            fb_scrhlp("Select field token, <CTL>-X=Abort");
            st = t_field_editor(t, tok_row - 2, col + SUB_COL_OFFSET,
               SUB_WINDOW_SIZE);
            fb_move(cdb_t_lines, 1); fb_clrtoeol();
            }
         else if (sub_state == 2){
            /* get fb_input for the length of the token */
            fb_scrhlp("Enter token length, 0 for variable");
            fb_move(len_row, col);
            fb_prints("        ");
            st = fb_input(len_row, col, 4, 0, FB_POS_NUM,buf,-FB_ECHO, FB_OKEND, FB_CONFIRM);
            if (st == FB_AOK || st == FB_ESCAPE_AOK)
               t->t_width = atoi(buf);
            fb_move(cdb_t_lines, 1); fb_clrtoeol();
            }
         put_row(mpcur->mp_acur);
         if (st != CHAR_DOLLAR)
            st = FB_AOK;
         return(st);
      }

/*
 * display_sub - display the entire token unit
 */

   static display_sub(t, col, state, rflag)
      fb_token *t;
      int col, state, rflag;

      {

         char buf[FB_MAXLINE];
         fb_field *f;

         if (state == 0 || state == 1){
            fb_move(tok_row, col); fb_clrtoeol();
            if (t != NULL && t->t_field != NULL){
               f = t->t_field;
               sprintf(buf, "%c   %4d", f->type, f->size);
               fb_move(tok_row, col + SUB_COL_OFFSET);
               fb_prints(buf);
               fb_move(tok_row, col);
               if (rflag)
                  fb_reverse(t->t_field->id);
               else
                  fb_prints(t->t_field->id);
               }
            }
         if (state == 0 || state == 2){
            buf[0] = NULL;
            if (t != NULL && t->t_field != NULL){
               if (t->t_width <= 0)
                  sprintf(buf, "variable");
               else
                  sprintf(buf, " %3d    ", t->t_width);
               }
            fb_move(len_row, col); fb_clrtoeol();
            if (rflag)
               fb_reverse(buf);
            else
               fb_prints(buf);
            }
      }

/*
 * clear_sub - clear the entire sub token unit
 */

   static clear_sub(trow, tcol)
     int trow, tcol;

      {
         int j;

         j = trow + SUB_WINDOW_SIZE;
         for (; trow < j; trow++){
            fb_move(trow, tcol);
            fb_clrtoeol();
            }
      }

/*
 * test_dollar - if cursor is on a dollar token, edit the token
 */

   test_dollar()
      {
         char *p;
         int st = 0;

         p = mpcur->mp_acur->a_text + mpcur->mp_col - 1;
         if (*p == CHAR_DOLLAR){

            set_token_cur();
            st = edit_token(token_cur, 0);

            if (st == FB_DELETED){
               fb_delete_token(token_cur, mpcur->mp_acur);
               fb_delete(1);
               set_token_cur();
               }
            set_screen();
            mrg_display();
            put_cursor();
            fb_refresh();
            }
         return(st);
      }

   set_token_cur()
      {

         char *p, *b;
         fb_token *t, *set_token();
         int j, n = 0;

         p = mpcur->mp_acur->a_text + mpcur->mp_col - 1;
         /* count the number of previous dollar signs here */
         b = mpcur->mp_acur->a_text;
         for (; b != p; b++)
            if (*b == CHAR_DOLLAR)
               n++;
         /* then skip n tokens */
         t = mpcur->mp_acur->a_thead;
         for (j = 0; t != NULL; t = t->t_next){
            if (++j > n)
               break;
            }
         token_cur = t;
      }

   static RETSIGTYPE sigalrm()
      {
         command = '\033';
	 longjmp(jmp_env, 1);
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
	 if (equal(buf, cdb_KL))
            st = FB_ARROWLEFT;
         else if (equal(buf, cdb_KR))
            st = FB_ARROWRIGHT;
         else if (equal(buf, cdb_KU))
            st = FB_ARROWUP;
         else if (equal(buf, cdb_KD))
            st = FB_ARROWDOWN;
	 else
	    st = FB_ERROR;
	 return(st);
      }

   static flipstate()
      {
         if (++sub_state > 2)
            sub_state = 1;
      }
