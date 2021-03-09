/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: d_dfield.c,v 9.0 2001/01/09 02:56:39 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char d_dfield_sid[] = "@(#) $Id: d_dfield.c,v 9.0 2001/01/09 02:56:39 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#define MAXSTACK 33
static long seekstack[MAXSTACK];
static long topstack;
static long basestack;
static int displaypages;
static long offset;
static long n_offset;
static char *MSG1 = "Display Command:";
static char *MSG2 = "End of Field:";
static char *cur_p;
static int lines;

static d_page(fb_field *f);
static d_push(long v);
static long d_pop(void);

/* 
 * d_dfield - meat of dfield routint to display a fb_field to screen.
 *	allows page forward and backward commands.
 */

   void fb_d_dfield(f)
      fb_field *f;
      
      {
         char inp[3], buf[FB_MAXLINE];
	 int st, okend, d_st, pos;
         char *p;

         if (f->fld == NIL)
            return;
         topstack = 0;
         basestack = 0;
         offset = 0;
         displaypages = -1;
         d_st = d_page(f);
         fb_cx_push_env("Q", CX_KEY_SELECT, NIL);
         if (displaypages > 1)
            fb_cx_add_buttons("SU");
         for (; ; ){
            buf[0] = NULL;
            if (displaypages > 1){
               strcpy(buf, "<Space>=Next Page, <CTL>-B=Prev Page, ");
               okend = -FB_OKEND;
               }
            else
               okend = FB_OKEND;
            fb_infoline();
            strcat(buf, "<RETURN>=Quit");
            fb_move(cdb_t_lines, 1);
            fb_clrtoeol();
            if (d_st != EOF){
               p = MSG1;
               fb_printw(p);
               }
            else{
               p = MSG2;
               fb_reverse(p);
               }
            pos = strlen(p) + 2;
            fb_scrhlp(buf);
            st = fb_input(cdb_t_lines, pos, 1, 0, FB_ALPHA, inp,
               FB_ECHO, okend, FB_NOCONFIRM);
            if (st == FB_PAGEUP || st == FB_BSIGNAL){
               if (topstack == basestack)
                  continue;	/* restart without redrawing page */
               offset = d_pop();
               }
            else if (st == FB_PAGEDOWN || st == FB_FSIGNAL ||
                  (st == FB_AOK && inp[0] == FB_BLANK)){
               if (d_st == EOF)
                  continue;	/* restart without redrawing page */
               /* since not EOF save the state */
               d_push(offset);
               offset += n_offset;
               }
            else
               break;
            d_st = d_page(f);
            }
	 fb_move(3, 1); fb_clrtobot(); fb_refresh();
         fb_cx_pop_env();
      }

/*
 * d_page - print a single page of the fb_field to the Cdb screen
 */

   static d_page(f)
      fb_field *f;

      {
         int eof = 0, clear = 0;

         cur_p = f->fld + offset;
         n_offset = 0;
	 for (; *cur_p; cur_p++, n_offset++){
	    if (clear == 0){
	       clear = 1;
	       lines = 4;
	       fb_move(3, 1); fb_clrtobot(); fb_refresh();
	       fb_move(4, 1);
	       }
	    if (*cur_p != FB_NEWLINE)
	       fb_s_putw(*cur_p);
            else {
               if (++lines >= cdb_t_lines-1)
                  break;
               fb_move(lines, 1);
               }
            }
         if (*cur_p == FB_NEWLINE){
            cur_p++;
            n_offset++;
            }
         if (*cur_p == NULL)
            eof = 1;
         /* for return codes use EOF or FB_AOK */
         if (displaypages == -1){
            if (eof)
               displaypages = 1;
            else
               displaypages = 2;	/* rather, at least 2 */
            }
         if (eof)
            return(EOF);
         else
            return(FB_AOK);
      }

/*
 * push and pop routines for keeping track of pages
 *	these implement a rotating stack that keeps the last MAXSTACK values
 */

   static d_push(v)
      long v;

      {
         seekstack[topstack] = v;
         topstack = (topstack + 1) % MAXSTACK;
         if (topstack == basestack)
            basestack = (basestack + 1) % MAXSTACK;
      }

   static long d_pop()
      {
         long v;
         
         if (topstack == basestack){
            fb_serror(FB_MESSAGE, "Nothing to Pop - d_dfield", NIL);
            return(0);
            }
         topstack--;
         if (topstack < 0)
            topstack = MAXSTACK - 1;
         v = seekstack[topstack];
         seekstack[topstack] = 0;
         return(v);
      }
