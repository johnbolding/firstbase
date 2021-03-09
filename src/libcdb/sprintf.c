/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: sprintf.c,v 9.2 2004/12/31 06:46:14 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Sprintf_sid[] = "@(#) $Id: sprintf.c,v 9.2 2004/12/31 06:46:14 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <sys/types.h>
#include <stdarg.h>

/*
 * fb_sprintf - sbuf - variable argument list of pp
 *	deposit into sbuf the proper printf manner of arguments.
 *	this version is intended to be machine independent
 */

   void fb_sprintf(char *sbuf, char *format, ...)

      {
         va_list pp;
         register char *c;
         char *mf, buf[FB_MAXLINE], m_format[FB_MAXLINE];
         int longflag, endflag, starflag, tmpint;
	 
         va_start(pp, format);
         /*
          * emulate printf here: tear apart format
          *    for each %NN[sldcf] - do the printf of it with its arg, emit
          *    for other characters, emit
          */
         for (c = format; *c; ){
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
            }
         *sbuf = NULL;
         va_end(pp);
      }
