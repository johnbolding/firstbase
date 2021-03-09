/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: plib.c,v 9.2 2004/12/31 06:44:19 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)plib.c	8.2 11/26/93 FB";
#endif

#include <dbvf_ext.h>
#include <stdarg.h>

   F_fb_initscreen()		/* initialize the vscreen variables */
      {
	 vfscr.s_ccount[F_PROW] = 0;
         F_nullscr(&vfscr);
      }

   F_fb_move(x, y)
      int x, y;

      {
         vfscr.s_x = x - 1;
	 vfscr.s_y = y - 1;
      }

/*
 * nullscr - null out the given screen
 */
   F_nullscr(s)
      fscreen *s;

      {
         register int i, j;

         for (i = 0; i < p_lines; i++){
	    for (j = 0; j < p_cols; j++)
	       s->line[i][j] = NULL;
	    s->s_ccount[i] = 0;
	    }
      }

/*
 * fb_printw - i could not figure how to connect this to fb_sprintf()
 *    so fb_sprintf is here inline as fb_printw
 */
   void F_fb_printw(char *format, ...)

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
            F_fb_prints(s_sbuf);
            }
         va_end(pp);
      }

/*
*   F_fb_printw(va_alist)
*      va_dcl
*
*      {
*         va_list ap;
*         char sbuf[200];
*
*	 va_start(ap);
*         fb_sprintf(sbuf, &ap);
*	 va_end(ap);
*         F_fb_prints(sbuf);
*      }
*/

   F_fb_prints(buf)
      char *buf;
      
      {
	 register char *p;
	 
	 for (p = buf; *p; p++)
	    F_fb_s_putw(*p);
      }

   F_fb_s_putw(c)
      char c;
      
      {
	 register int x, y;

	 if (c == FB_NEWLINE){
	    ++vfscr.s_x;
	    vfscr.s_y = 0;
	    }
	 else if (c == '\t'){
	    for (;;){
	       F_fb_s_putw(CHAR_BLANK);
	       if (vfscr.s_y % 8 == 0 || vfscr.s_y >= p_cols - 1)
	          break;
	       }
	    }
	 else if (vfscr.s_y < p_cols){
	    x = vfscr.s_x;
	    y = vfscr.s_y;
	    vfscr.line[x][y] = c;
	    vfscr.s_ccount[x]++;
	    if (++y < p_cols)
	       vfscr.s_y = y;
	    else
	       vfscr.s_y = p_cols - 1;
	    }
      }

/*
 * screendump - spit out complete vfscr.
 */

   screendump()

      {
         register int i, j;
         fscreen *s;
	 char c;
	 int lastrow, lastcol, nlines = 0;

         s = &vfscr;
         for (lastrow = p_lines - 1; lastrow >= 0; lastrow--)
	    if (s->s_ccount[lastrow] > 0)
	       break;
         for (i = 0; i <= lastrow; i++){
	    if (s->s_ccount[i] > 0){
	       for (lastcol = p_cols - 1; lastcol >= 0; lastcol--)
		  if (s->line[i][lastcol] != NULL)
		     break;
	       for (j = 0; j <= lastcol; j++){
		  c = s->line[i][j];
		  if (c == NULL)
		     c = FB_BLANK;
		  putc(c, stdout);
		  }
	       }
	    putc(FB_NEWLINE, stdout);
	    nlines++;
	    }
	 if (formfeed_flag && nlines < p_lines)
	    putc('\014', stdout);
	 else{
	    /* simulate a formfeed with blank lines */
	    for (; nlines < p_lines; nlines++)
	       putc(FB_NEWLINE, stdout);
	    }
	 F_nullscr(s);
      }
