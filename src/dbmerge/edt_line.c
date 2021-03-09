/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: edt_line.c,v 9.0 2001/01/09 02:55:41 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Edit_cell_sid[] = "@(#) $Id: edt_line.c,v 9.0 2001/01/09 02:55:41 john Exp $";
#endif

#include <dbdmrg_e.h>

extern char *ebuf;			/* buffer to edit in */
extern int fullsize, halfsize;
extern short int cdb_use_insert_char;
extern fb_token *fb_maketoken();

/*
 * insert - each character typed until an escape is inserted
 */

   insert()
      {
         char inc, *p;
         fb_aline *a, *pa, *fb_makeline(), *na;
         int c, n, start_col, trow;
         fb_token *t, *nt, *pt;

         a = mpcur->mp_acur;
         start_col = mpcur->mp_col;
         fb_scrstat("Insert Level");
         put_cursor();
         while (read(0, &inc, 1) == 1){
            if (inc == '\033')
               break;
            if (inc == '\t')
               inc = FB_BLANK;
            else if (inc == '\010' || inc == '\177'){	/* DEL FB_BACKSPACE */
               if (mpcur->mp_col > 1){
                  mpcur->mp_col--;
                  fb_delete(1);
                  }
               continue;
               }
            else if (inc == '\025'){			/* ^U */
               if (mpcur->mp_col > 1){
                  for (mpcur->mp_col--; mpcur->mp_col >= start_col; mpcur->mp_col--)
                     fb_delete(1);
                  }
               mpcur->mp_col++;
               continue;
               }
            else if (inc == '\027'){			/* ^W */
               if (mpcur->mp_col <= start_col){
                  fb_bell();
                  fb_refresh();
                  continue;
                  }
               p = a->a_text + mpcur->mp_col - 2;
               if (mpcur->mp_col > 1){
                  /* skip white backward */
                  while (*p == FB_BLANK || *p == '\t'){
                     mpcur->mp_col--;
                     if (mpcur->mp_col < start_col){
                        mpcur->mp_col++;
                        break;
                        }
                     fb_delete(1);
                     p--;
                     }
                  /* skip the word */
                  while (*p != FB_BLANK && *p != '\t'){
                     mpcur->mp_col--;
                     if (mpcur->mp_col < start_col){
                        mpcur->mp_col++;
                        break;
                        }
                     fb_delete(1);
                     p--;
                     }
                  put_cursor();
                  }
               continue;
               }
	    else if (!isprint(inc) && inc != FB_NEWLINE && inc != FB_CRET){
               fb_bell();
               fb_refresh();
               continue;
               }
            modified = 1;
            c = mpcur->mp_col - 1;
            strncpy(ebuf, a->a_text, c);
            if (inc != FB_NEWLINE && inc != FB_CRET){
               if (cdb_use_insert_char)
                  fb_insertch();
               if (inc == CHAR_DOLLAR)
                  set_token_cur();     /* set before inserting the dollar */
               ebuf[c] = inc;
               ebuf[c+1] = NULL;
               strcat(ebuf, a->a_text + c);
               ebuf[linewidth] = NULL;
               strcpy(a->a_text, ebuf);
               cell_right(1);
               put_row(a);
               if (inc == CHAR_DOLLAR){
                  t = fb_maketoken();

                  fb_insert_token(t, token_cur, a);
                  edit_token(t, 1);
                  token_cur = t;

                  set_screen();
                  mrg_display();
                  fb_scrstat("Insert Level");
                  }
               }
            else{
               /* at FB_NEWLINE, build previous line and insert it */
               /* align the token lists - split them */
               set_token_cur();
               if (token_cur != NULL){
                  pt = token_cur->t_prev;
                  if (pt != NULL)
                     pt->t_next = NULL;
                  token_cur->t_prev = NULL;
                  }
               ebuf[c] = NULL;
               pa = fb_makeline();
               pa->a_text = fb_malloc(linewidth + 1);
               fb_pad(pa->a_text, ebuf, linewidth);

               if (cdb_use_insert_char){
                  if (mpcur->mp_row == base_bottom){
                     trow = mpcur->mp_row;
                     fb_move(base_top, 1);
                     fb_deleteln();
                     fb_move(trow, 1);
                     fb_insertln();
                     }
                  else{
                     trow = mpcur->mp_row + 1;
                     fb_move(base_bottom, 1);
                     fb_deleteln();
                     fb_move(trow, 1);
                     fb_insertln();
                     }
                  }

               fb_insert_line(pa, a, mpcur);
               /* set token list in previous line */
               if (token_cur == a->a_thead)
                  pa->a_thead = NULL;
               else
                  pa->a_thead = a->a_thead;

               /* set token tail in previous line */
               for (nt = pa->a_thead, pt = NULL; nt != NULL; nt = nt->t_next)
                  pt = nt;
               pa->a_ttail = pt;

               /* set token list and token tail in current line */
               a->a_thead = token_cur;
               for (nt = a->a_thead, pt = NULL; nt != NULL; nt = nt->t_next)
                  pt = nt;
               a->a_ttail = pt;

               /* check for screen conditions warranting redrawing */
               if (a == mpcur->mp_atop){
                  mpcur->mp_atop = pa;
                  set_screen();
                  }
               else if (a == mpcur->mp_abot && mpcur->mp_row == cdb_t_lines - 2){
                  na = mpcur->mp_atop;
                  for (n = 1; n < 2; na = na->a_next, n++)
                     ;
                  mpcur->mp_atop = na;
                  set_screen();
                  }

               /* build next line */
               strcpy(ebuf, a->a_text + c);
               fb_pad(a->a_text, ebuf, linewidth);
               mpcur->mp_col = start_col = 1;
               set_screen();
               mrg_display();
               }
            put_cursor();
            fb_refresh();
            }
         if (mpcur->mp_col > 1){
            p = mpcur->mp_acur->a_text + mpcur->mp_col - 2;
            if (*p != CHAR_DOLLAR)
               cell_left(1);
            }
         return(FB_AOK);
      }

/*
 * replace - each character typed until an escape is replaced
 */

   replace(n)
      int n;

      {
         char inc, *p;
         fb_aline *a;
         int c, start_col, orig_n;
         fb_token *t;

         a = mpcur->mp_acur;
         start_col = mpcur->mp_col;
         if (n == 0)
            n = 1;
         orig_n = n;
         for (; n > 0; n--){
            if (read(0, &inc, 1) != 1)
               break;
            if (inc == '\033' || inc == FB_RETURN || inc == FB_CRET)
               break;
            if (inc == '\t')
               inc = FB_BLANK;
            else if (inc == '\010' || inc == '\177'){	/* DEL FB_BACKSPACE */
               if (mpcur->mp_col > 1){
                  mpcur->mp_col--;
                  fb_delete(1);
                  }
               continue;
               }
            else if (inc == '\025'){			/* ^U */
               if (mpcur->mp_col > 1){
                  for (mpcur->mp_col--; mpcur->mp_col >= start_col; mpcur->mp_col--)
                     fb_delete(1);
                  }
               mpcur->mp_col++;
               continue;
               }
            else if (inc == '\027'){			/* ^W */
               if (mpcur->mp_col <= start_col){
                  fb_bell();
                  fb_refresh();
                  continue;
                  }
               p = a->a_text + mpcur->mp_col - 2;
               if (mpcur->mp_col > 1){
                  /* skip white backward */
                  while (*p == FB_BLANK || *p == '\t'){
                     mpcur->mp_col--;
                     if (mpcur->mp_col < start_col){
                        mpcur->mp_col++;
                        break;
                        }
                     fb_delete(1);
                     p--;
                     }
                  /* skip the word */
                  while (*p != FB_BLANK && *p != '\t'){
                     mpcur->mp_col--;
                     if (mpcur->mp_col < start_col){
                        mpcur->mp_col++;
                        break;
                        }
                     fb_delete(1);
                     p--;
                     }
                  put_cursor();
                  }
               continue;
               }
	    else if (!isprint(inc) && inc != FB_NEWLINE){
               fb_bell();
               fb_refresh();
               continue;
               }
            modified = 1;
            c = mpcur->mp_col - 1;
            if (inc == CHAR_DOLLAR)
               set_token_cur();     /* set before inserting the dollar */
            a->a_text[c] = inc;
            if (n > 1)
               cell_right(1);
            put_row(a);
            if (inc == CHAR_DOLLAR){
               t = fb_maketoken();

               fb_insert_token(t, token_cur, a);
               edit_token(t, 1);
               token_cur = t;

               set_screen();
               mrg_display();
               }
            put_cursor();
            fb_refresh();
            }
         if (mpcur->mp_col > 1 && orig_n > 1){
            p = mpcur->mp_acur->a_text + mpcur->mp_col - 2;
            if (*p != CHAR_DOLLAR)
               cell_left(1);
            }
         return(FB_AOK);
      }

/*
 * delete - delete n characters
 */

   fb_delete(n)
      int n;

      {
         fb_aline *a;
         int c;

         a = mpcur->mp_acur;
         if (n == 0)
            n = 1;
         modified = 1;
         for (; n > 0; n--){
            if (cdb_use_insert_char)
               fb_deletech();
            c = mpcur->mp_col - 1;
            strncpy(ebuf, a->a_text, c);
            ebuf[c] = NULL;
            strcat(ebuf, a->a_text + c + 1);
            ebuf[linewidth - 1] = FB_BLANK;
            ebuf[linewidth] = NULL;
            strcpy(a->a_text, ebuf);
            }
         set_screen();
         put_row(a);
         put_cursor();
         fb_refresh();
         return(FB_AOK);
      }

/*
 * delete_rest_of_line - the vi(1) D command
 */

   delete_rest_of_line()
      {
         fb_aline *a;
         int st = FB_AOK, n;
         char *p;

         a = mpcur->mp_acur;
         modified = 1;
         p = a->a_text + mpcur->mp_col - 1;
         n = mpcur->mp_col;
         for (; n < linewidth; n++)
            *p++ = FB_BLANK;
         /* set_screen(); */
         put_row(a);
         return(FB_AOK);
      }

/*
 * word_delete - the vi(1) dw command
 *       delete everything up to but not including the place
 *       a w command would go to at this moment.
 */

   fb_word_delete()
      {
         fb_aline *a;
         int n, ofc, qcol;
         char *p, *q;

         a = mpcur->mp_acur;
         modified = 1;
         /* p is the point to start deleteing */
         p = a->a_text + mpcur->mp_col - 1;

         /* now find where q should be .. start with p */
         q = p;
         ofc = mpcur->mp_col + calc_full_offset();
         if (ofc >= linewidth)
            return(FB_ERROR);

         /* if in word, skip this word */
         qcol = mpcur->mp_col;
         for (; ofc < linewidth && *q != FB_BLANK && *q != '\t'; ){
            q++;
            qcol++;
            ofc = qcol + calc_full_offset();
            if (ofc >= linewidth)
               break;
            }
         /* skip white space */
         for (; ofc < linewidth && *q == FB_BLANK || *q == '\t'; ){
            q++;
            qcol++;
            ofc = qcol + calc_full_offset();
            if (ofc >= linewidth)
               break;
            }

         n = 0;
         for (; p != q; p++, n++)
            ;
         fb_delete(n);
         return(FB_AOK);
      }

