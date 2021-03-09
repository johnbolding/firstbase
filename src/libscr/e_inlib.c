/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: e_inlib.c,v 9.0 2001/01/09 02:57:03 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char E_inlib_sid[] = "@(#) $Id: e_inlib.c,v 9.0 2001/01/09 02:57:03 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <keyboard.h>

#define iswhite(c)   ((c) == FB_BLANK || (c) == '\t')
#define isfunny(c)   (!isupper(c) && !islower(c) && !isdigit(c) && !iswhite(c))

extern short int cdb_bpos, cdb_bmax, cdb_epos, cdb_e_changes;
extern char *cdb_e_buf;
extern short int cdb_scr_inputdots;

/*
 * fb_e_inlib - these routines comprise the extensible editing commands
 *	available for fb_e_input processing.
 */

   fb_e_display(row, col)
      int row, col;

      {
         int i;

         fb_move(row, col);
         fb_prints(cdb_e_buf);
         for (i = cdb_epos; i < cdb_bmax; i++){
            if (cdb_scr_inputdots)
               fb_s_putw(CHAR_DOT);
            else
               fb_s_putw(CHAR_BLANK);
            }
         if (cdb_bpos >= cdb_bmax)
            cdb_bpos = cdb_bmax - 1;
         i = col + cdb_bpos;
         fb_move(row, i);
         fb_refresh();
      }

   fb_e_insert_char(c)
      int c;

      {
         char *e, *q, *b;

         /* if already full, delete last char in the line */
         if (cdb_epos >= cdb_bmax){
            e = cdb_e_buf + cdb_epos - 1;
            *e = NULL;
            cdb_epos--;
            }
         /* slide any existing characters one right, from end backwards */
         e = cdb_e_buf + cdb_epos;
         q = e + 1;
         b = cdb_e_buf + cdb_bpos;
         for (;; e--, q--){
            *q = *e;
            if (e == b)
               break;
            }
         cdb_epos++;
         /* now insert the character at cdb_bpos */
         *b = c;
         cdb_bpos++;
         /* defensively, place a NULL at cdb_epos, by definition */
         q = cdb_e_buf + cdb_epos;
         *q = NULL;
         cdb_e_changes = 1;
      }

   fb_e_move_char_backward()
      {
         if (--cdb_bpos < 0)
            cdb_bpos = 0;
      }

   fb_e_move_char_forward()
      {
         if (++cdb_bpos > cdb_epos)
            cdb_bpos = cdb_epos;
      }

   void fb_e_delete_char_backward()
      {
         char *p;

         if (cdb_bpos == 0)
            return;
         if (cdb_bpos != cdb_epos){
            cdb_bpos--;
            fb_e_delete_char_forward();
            }
         else{
            p = cdb_e_buf + cdb_bpos - 1;
            *p = NULL;
            cdb_bpos--;
            cdb_epos--;
            }
         cdb_e_changes = 1;
      }

   void fb_e_delete_char_forward()
      {
         char *b, *q;

         /* slide existing left one, going forwards */
         if (cdb_bpos == cdb_epos)
            return;
         b = cdb_e_buf + cdb_bpos;
         q = b + 1;
         for (;; b++, q++){
            *b = *q;
            if (*q == NULL)
               break;
            }
         if (--cdb_epos <= 0)
            cdb_bpos = cdb_epos = 0;
         else if (cdb_bpos > cdb_epos)
            cdb_bpos = cdb_epos;
         cdb_e_changes = 1;
      }

   fb_e_end_of_line()
      {
         cdb_bpos = cdb_epos;
      }

   fb_e_beginning_of_line()
      {
         cdb_bpos = 0;
      }

   void fb_e_move_word_forward()
      {
         char *p;

         if (cdb_bpos == cdb_epos)
            return;
         p = cdb_e_buf + cdb_bpos;
         if (isfunny(*p))	 /* if in a funny, skip funny characters */
            for (; cdb_bpos != cdb_epos && isfunny(*p); p++, cdb_bpos++)
               ;
         else	 		/* if in a word, skip this word */
            for (; cdb_bpos != cdb_epos && !iswhite(*p) && !isfunny(*p);
                  p++, cdb_bpos++)
               ;

         /* skip white space */
         for (; cdb_bpos != cdb_epos && iswhite(*p); p++, cdb_bpos++)
            ;
      }

   void fb_e_move_word_backward()
      {
         char *p;

         if (cdb_bpos == 0)
            return;
         cdb_bpos--;
         p = cdb_e_buf + cdb_bpos;

         /* skip white space */
         for (; cdb_bpos > 0 && iswhite(*p); p--, cdb_bpos--)
            ;

         if (isfunny(*p)){ 	/* if in a funny, skip funny characters */
            for (; cdb_bpos > 0 && isfunny(*p); p--, cdb_bpos--)
               ;
            if (!isfunny(*p))
               cdb_bpos++;
            }
         else{		 	/* if in a word, skip this word */
            for (; cdb_bpos > 0 && !iswhite(*p) && !isfunny(*p); p--, cdb_bpos--)
               ;
            if (iswhite(*p) || isfunny(*p))
               cdb_bpos++;
            }
      }

   void fb_e_move_end_word_forward()
      {
         char *p;

         if (cdb_bpos == cdb_epos)
            return;
         p = cdb_e_buf + cdb_bpos;
         /* if needed, skip white space */
         if (iswhite(*p))
            for (; cdb_bpos != cdb_epos && iswhite(*p); p++, cdb_bpos++)
               ;
         if (isfunny(*p))	 /* if in a funny, skip funny characters */
            for (; cdb_bpos != cdb_epos && isfunny(*p); p++, cdb_bpos++)
               ;
         else	 		/* if in a word, skip this word */
            for (; cdb_bpos != cdb_epos && !iswhite(*p) && !isfunny(*p); p++, cdb_bpos++)
               ;
      }

   void fb_e_delete_word_forward()
      {
         int lpos, rpos;
         char *b, *q;

         if (cdb_bpos == cdb_epos)
            return;
         lpos = cdb_bpos;
         b = cdb_e_buf + lpos;
         fb_e_move_end_word_forward();
         rpos = cdb_bpos;
         q = cdb_e_buf + rpos;
         for (;; b++, q++){
            *b = *q;
            if (*q == NULL)
               break;
            }
         cdb_epos -= (rpos - lpos);
         cdb_bpos = lpos;
         if (cdb_epos <= 0)
            cdb_bpos = cdb_epos = 0;
         else if (cdb_bpos > cdb_epos)
            cdb_bpos = cdb_epos;
         cdb_e_changes = 1;
      }

   void fb_e_delete_word_backward()
      {
         int lpos, rpos;
         char *b, *q;

         if (cdb_bpos == 0)
            return;
         rpos = cdb_bpos;
         fb_e_move_word_backward();
         lpos = cdb_bpos;
         b = cdb_e_buf + lpos;
         q = cdb_e_buf + rpos;
         for (;; b++, q++){
            *b = *q;
            if (*q == NULL)
               break;
            }
         cdb_epos -= (rpos - lpos);
         cdb_bpos = lpos;
         if (cdb_epos <= 0)
            cdb_bpos = cdb_epos = 0;
         else if (cdb_bpos > cdb_epos)
            cdb_bpos = cdb_epos;
         cdb_e_changes = 1;
      }

   void fb_e_delete_to_end_of_line()
      {
         char *p;
         int i;

         if (cdb_bpos == cdb_epos)
            return;
         p = cdb_e_buf + cdb_bpos;
         for (i = cdb_bpos; i < cdb_epos; i++, p++)
            *p = NULL;
         cdb_epos = cdb_bpos;
         cdb_e_changes = 1;
      }

   void fb_e_delete_to_beginning_of_line()
      {
         int lpos, rpos;
         char *b, *q;

         if (cdb_bpos == 0)
            return;
         rpos = cdb_bpos;
         lpos = 0;
         b = cdb_e_buf + lpos;
         q = cdb_e_buf + rpos;
         for (;; b++, q++){
            *b = *q;
            if (*q == NULL)
               break;
            }
         cdb_epos -= (rpos - lpos);
         cdb_bpos = lpos;
         if (cdb_epos <= 0)
            cdb_bpos = cdb_epos = 0;
         else if (cdb_bpos > cdb_epos)
            cdb_bpos = cdb_epos;
         cdb_e_changes = 1;
      }

   fb_e_upcase_word()
      {
         char *b, *e;

         b = cdb_e_buf + cdb_bpos;
         if (iswhite(*b)){
            fb_e_move_word_forward();
            b = cdb_e_buf + cdb_bpos;
            }
         fb_e_move_word_forward();
         e = cdb_e_buf + cdb_bpos - 1;
         for (;;b++){
            if (islower(*b))
               *b = toupper(*b);
            if (b >= e)
               break;
            }
         cdb_e_changes = 1;
      }

   fb_e_downcase_word()
      {
         char *b, *e;

         b = cdb_e_buf + cdb_bpos;
         if (iswhite(*b)){
            fb_e_move_word_forward();
            b = cdb_e_buf + cdb_bpos;
            }
         fb_e_move_word_forward();
         e = cdb_e_buf + cdb_bpos - 1;
         for (;;b++){
            if (isupper(*b))
               *b = tolower(*b);
            if (b >= e)
               break;
            }
         cdb_e_changes = 1;
      }

   fb_e_capitalize_word()
      {
         char *b, *e;

         b = cdb_e_buf + cdb_bpos;
         if (iswhite(*b)){
            fb_e_move_word_forward();
            b = cdb_e_buf + cdb_bpos;
            }
         fb_e_move_word_forward();
         e = cdb_e_buf + cdb_bpos - 1;
         /* do first char to upper */
         if (islower(*b))
            *b = toupper(*b);
         /* do the rest in lower */
         for (b++ ;; b++){
            if (isupper(*b))
               *b = tolower(*b);
            if (b >= e)
               break;
            }
         cdb_e_changes = 1;
      }

/*
 * some other utility functions
 */

   fb_e_dotcount()
      {
         char *p;
         int d = 0;

         for (p = cdb_e_buf; *p; p++)
            if (*p == CHAR_DOT)
               d++;
         return(d);
      }

   fb_e_num_past_dot()
      {
         char *p;
         int d = 0, n = 0;

         for (p = cdb_e_buf; *p; p++){
            if (*p == CHAR_DOT)
               d = 1;
            else if (d == 1 && isdigit(*p))
               n++;
            }
         return(n);
      }

   fb_e_cur_past_dot()			/* 1 if past dot, else 0 */
      {
         char *p;

         if (cdb_bpos == 0)
            return(0);
         for (p = cdb_e_buf + cdb_bpos - 1; ; p--){
            if (*p == CHAR_DOT)
               return(1);
            if (p == cdb_e_buf)
               break;
            }
         return(0);
      }
