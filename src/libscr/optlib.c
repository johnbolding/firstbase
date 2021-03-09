/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: optlib.c,v 9.2 2004/12/31 06:46:24 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Optlib_sid[] = "@(#) $Id: optlib.c,v 9.2 2004/12/31 06:46:24 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <stdarg.h>

short int cdb_clr1, cdb_clr2, cdb_clr3;
short int cdb_refresh_clears = 0;
#if FB_PROTOTYPES
static p_clear(fb_pscreen *p);
#else /* FB_PROTOTYPES */
static p_clear();
#endif /* FB_PROTOTYPES */

extern char CHAR_b;
extern char CHAR_r;
extern char CHAR_s;
extern char CHAR_BLANK;
extern fb_pscreen cdb_curscr, cdb_newscr;

   fb_initopt()		/* initialize the optomization variables */
      {
	 cdb_curscr.s_ncount[FB_PROW] = cdb_newscr.s_ncount[FB_PROW] = 0;
         fb_nullscr(&cdb_curscr);
	 fb_nullscr(&cdb_newscr);
	 cdb_newscr.s_x = cdb_curscr.s_y = 0;
	 cdb_newscr.s_x = cdb_newscr.s_y = 0;
	 cdb_newscr.s_attron = CHAR_BLANK;
	 cdb_newscr.s_bell = CHAR_BLANK;
      }

/*
 * fb_printw - i could not figure how to connect this to fb_sprintf()
 *    so fb_sprintf is here inline as fb_printw
 */
   void fb_printw(char *format, ...)

      {
         va_list pp;
         register char *c;
         char *mf, buf[FB_MAXLINE], m_format[FB_MAXLINE], s_sbuf[FB_MAXLINE];
         char *sbuf;
         int longflag, endflag, starflag, tmpint;

         /*
          * emulate printf here: tear apart format
          *    for each %NN[sldcf] - do the printf of it with its arg, emit
          *    for other characters, emit
          */
         va_start(pp, format);
         for (c = format; *c; ){
            sbuf = s_sbuf;
            *sbuf = NULL;
            switch(*c){
               case '%':
                  c++;
                  if (*c == '%'){	/* handles %% */
                     *sbuf++ = '%';
                     c++;
                     break;
                     }
                  mf = m_format;
                  *mf++ = '%';		/* get the mini-format */
                  starflag = longflag = 0;
                  for (endflag = 0; *c && !endflag; c++){
                     *mf++ = *c;
                     *mf = NULL;
                     switch(*c){
                        case 'c':	/* terminals */
                           sprintf(buf, m_format, va_arg(pp, int));
                           endflag = 1;
                           break;
                        case 'd':
                           if (longflag)
                              sprintf(buf, m_format, va_arg(pp, long));
                           else
                              sprintf(buf, m_format, va_arg(pp, int));
                           endflag = 1;
                           break;
                        case 'f':
                           sprintf(buf, m_format, va_arg(pp, double));
                           endflag = 1;
                           break;
                        case 's':
                           if (!starflag)
                              sprintf(buf, m_format, va_arg(pp, char *));
                           else{
                              tmpint = va_arg(pp, int);
                              sprintf(buf, m_format,
                                 tmpint, va_arg(pp, char *));
                              }
                           endflag = 1;
                           break;
                        case 'u':
                           if (longflag)
                              sprintf(buf, m_format, va_arg(pp, u_long));
                           else
                              sprintf(buf, m_format, va_arg(pp, u_int));
                           endflag = 1;
                           break;

                        case 'l':	/* modifiers */
                           longflag = 1;
                        case '*':
                           starflag = 1;
                        case '.': case '-': case '0':
                        case '1': case '2': case '3':
                        case '4': case '5': case '6':
                        case '7': case '8': case '9':
                           break;
                        default:
                           printf("FB_ERROR");
                           break;
                        }
                     }
                  /* buf contains the single element formatted */
                  strcpy(sbuf, buf);
                  sbuf += strlen(buf);
                  break;
               default:
                  *sbuf++ = *c;
                  c++;
                  break;
               }
            *sbuf = NULL;
            fb_prints(s_sbuf);
            }
         va_end(pp);
      }

/* 
*    void fb_printw(char *format, ...)
* 
*       {
*          va_list ap;
*          char sbuf[200];
* 
* 	 va_start(ap, format);
*          fb_sprintf(sbuf, format, ap);
* 	 va_end(ap);
*          fb_prints(sbuf);
*       }
*/

/* clear screen, redraw current screen */
   fb_redraw()
      {
         register int i;

         cdb_newscr = cdb_curscr;
	 p_clear(&cdb_curscr);
         cdb_clr1 = 1;
         for (i = 0; i < cdb_t_lines; i++)
	    cdb_newscr.s_ncount[i] = 0;
	 fb_refresh();
      }

   fb_gcounter(j)
      long j;

      {
         fb_move(20, 49);
	 fb_printw("%ld", j);
	 fb_refresh();
      }

   fb_stand(s)
      char *s;

      {
         fb_standout();
	 fb_prints(s);
	 fb_standend();
      }

   fb_force(s)
      char *s;

      {
	 fb_prints(s);
	 fb_refresh();
      }

   fb_reverse(s)
      char *s;

      {
         cdb_newscr.s_attron = CHAR_r;
	 fb_prints(s);
         cdb_newscr.s_attron = CHAR_BLANK;
      }

   fb_bell()
      {
         cdb_newscr.s_bell = CHAR_b;
      }

   fb_standout()
      {
         cdb_newscr.s_attron = CHAR_s;
      }

   fb_standend()
      {
         cdb_newscr.s_attron = CHAR_BLANK;
      }

   fb_reverseout()
      {
         cdb_newscr.s_attron = CHAR_r;
      }

   fb_reverseend()
      {
         cdb_newscr.s_attron = CHAR_BLANK;
      }

   fb_clear()
      {
	 cdb_clr1 = 1;
         p_clear(&cdb_newscr);
	 /*
	 if (cdb_refresh_clears)
	    fb_refresh(); */
      }

   static p_clear(p)
      fb_pscreen *p;

      {
         register int x, y;

         for (x = 0; x < cdb_t_lines; x++){
	    for (y = 0; y < cdb_t_cols; y++)
	       p->line[x][y] = p->attr[x][y] = CHAR_BLANK;
	    p->s_ncount[x] = 0;
	    }
	 p->s_ncount[FB_PROW] = 0;		/* set total nulls to 0 */
      }

   fb_clrtobot()
      {
         register int x, y;

         cdb_clr2 = 1;
         y = cdb_newscr.s_y;
         for (x = cdb_newscr.s_x; x < cdb_t_lines; x++){
            for (; y < cdb_t_cols; y++){
	       if (cdb_newscr.line[x][y] == NULL){
	          cdb_newscr.s_ncount[x]--;
	          cdb_newscr.s_ncount[FB_PROW]--;
		  }
	       cdb_newscr.line[x][y] = cdb_newscr.attr[x][y] = CHAR_BLANK;
	       }
	    y = 0;
	    }
      }

   fb_clrtoeol()
      {
         register int x, y;

         cdb_clr3 = 1;
         for (x = cdb_newscr.s_x, y = cdb_newscr.s_y; y < cdb_t_cols; y++){
	    if (cdb_newscr.line[x][y] == NULL){
	       cdb_newscr.s_ncount[x]--;
	       cdb_newscr.s_ncount[FB_PROW]--;
	       }
	    cdb_newscr.line[x][y] = cdb_newscr.attr[x][y] = CHAR_BLANK;
	    }
	 if (cdb_refresh_clears)
	    fb_refresh();
      }

   fb_move(x, y)
      int x, y;

      {
         cdb_newscr.s_x = x - 1;
	 cdb_newscr.s_y = y - 1;
      }

/*
 * nullscr - null out the given screen
 */
   fb_nullscr(s)
      fb_pscreen *s;

      {
         register int i, j;

         for (i = 0; i < cdb_t_lines; i++)
            if (s->s_ncount[i] < cdb_t_cols || s->s_ncount[FB_PROW] == 0){
	       for (j = 0; j < cdb_t_cols; j++){
	          s->line[i][j] = NULL;
	          s->attr[i][j] = FB_BLANK;	/* assume normal start attr */
		  }
	       s->s_ncount[i] = cdb_t_cols;
	       }
	 s->s_ncount[FB_PROW] = cdb_t_cols * cdb_t_lines;
	 s->s_bell = CHAR_BLANK;
	 s->s_attron = CHAR_BLANK;
	 cdb_clr1 = cdb_clr2 = cdb_clr3 = 0;
      }

   void fb_prints(buf)
      char *buf;
      
      {
	 register char *p;
	 
	 for (p = buf; *p; p++)
	    fb_s_putw(*p);
      }

   void fb_s_putw(c)
      int c;
      
      {
	 register int x, y;

	 if (c == FB_NEWLINE){
	    ++cdb_newscr.s_x;
	    cdb_newscr.s_y = 0;
	    }
	 else if (c == '\t'){
	    for (;;){
	       fb_s_putw(CHAR_BLANK);
	       if (cdb_newscr.s_y % 8 == 0 || cdb_newscr.s_y >= cdb_t_cols - 1)
	          break;
	       }
	    }
	 else if (cdb_newscr.s_y < cdb_t_cols){
	    x = cdb_newscr.s_x;
	    y = cdb_newscr.s_y;
	    if (cdb_curscr.line[x][y] != c || 
	          cdb_curscr.attr[x][y] != cdb_newscr.s_attron){
	       if (cdb_newscr.line[x][y] == NULL){
		  cdb_newscr.s_ncount[x]--;
		  cdb_newscr.s_ncount[FB_PROW]--;
		  }
	       cdb_newscr.line[x][y] = c;
	       cdb_newscr.attr[x][y] = cdb_newscr.s_attron;
	       }
	    else{
	       if (cdb_newscr.line[x][y] != NULL){
		  cdb_newscr.s_ncount[x]++;
		  cdb_newscr.s_ncount[FB_PROW]++;
		  }
	       cdb_newscr.line[x][y] = NULL;
	       cdb_newscr.attr[x][y] = cdb_newscr.s_attron;
	       }
	    if (++y < cdb_t_cols)
	       cdb_newscr.s_y = y;
	    else
	       cdb_newscr.s_y = cdb_t_cols - 1;
	    }
      }

   void fb_outc(c)		/* outc is used for tputs() function */
      int c;

      {
         putc(c, stdout);
      }

   void fb_spause()		/* simple pasue -- with no movement */
      {
         char c;

	 fb_refresh();
         scanf(FB_FCHAR, &c);
      }
