/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: readmrg.c,v 9.0 2001/01/09 02:56:49 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Readmrg_sid[] = "@(#) $Id: readmrg.c,v 9.0 2001/01/09 02:56:49 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <mdict.h>
#include <mdict_e.h>

#define LEFT_SQUARE	'['
#define RIGHT_SQUARE	']'
#define LEFT_BRACE	'{'
#define RIGHT_BRACE	'}'

#if FB_PROTOTYPES
static read_tokens(fb_aline *a, char *buf);
static int inrange(int c);
#else
static read_tokens();
static inrange();
#endif /* FB_PROTOTYPES */

/*
 * readmrg - read a merge file into internal representation
 */

   fb_readmrg(filen)
      char *filen;

      {
         fb_aline *a;
         int fd, len, nlines = 0;
         char buf[1000], sbuf[1000];

         mphead = fb_make_mpage();
         fb_insert_page(mphead, NULL);
         mphead->mp_row = 2;
         mphead->mp_col = 1;

         killpage = fb_make_mpage();
         killbuffer = NULL;
         copypage = fb_make_mpage();
         copybuffer = NULL;

         if (access(filen, 0) == 0){
            fd = fb_mustopen(filen, READ);
            fb_r_init(fd);
            while (fb_nextline(buf, FB_MAXLINE) != 0){
               nlines++;
               a = fb_makeline();
               fb_insert_line(a, (fb_aline *) NULL, mphead);
               read_tokens(a, buf);
               len = strlen(buf);
               if (len > linewidth)
                  fb_serror(FB_MESSAGE,
                     "Linewidth Exceeded - Truncating Input Line", NIL);
               fb_pad(sbuf, buf, linewidth);
               strcpy(a->a_text, sbuf);
               }
            fb_r_end();
            }
         if (nlines <= 0){
            a = fb_makeline();
            fb_insert_line(a, NULL, mphead);
            }
      }

/*
 * read_tokens - read the tokens from buf, store into token list in a.
 *	modify buf to contain only the dollar sign
 */

   static read_tokens(a, buf)
      fb_aline *a;
      char *buf;

      {
         char nbuf[1000], *p, *q, tokbuf[FB_MAXLINE], *tp;
         fb_token *t;

         strcpy(nbuf, buf);
         for (p = nbuf, q = buf; *p; p++){
            *q++ = *p;
            if (*p == CHAR_DOLLAR){
               t = fb_maketoken();
               fb_insert_token(t, (fb_token *) NULL, a);
               p++;				/* get past DOLLAR sign */
               if (*p == CHAR_DOLLAR){
                  t->t_width = 1;
                  t->t_field = NULL;
                  continue;
                  }
               for (tp = tokbuf; *p; tp++, p++){
                  if (!inrange(*p))
                     break;
                  *tp = *p;
                  }
               *tp = NULL;
               if (*p == LEFT_SQUARE){		/* capture length also */
                  t->t_width = atoi(++p);
                  for (; *p && *p != RIGHT_SQUARE; p++)
                     ;
                  if (*p != RIGHT_SQUARE)
                     fb_serror(FB_MESSAGE, "Badly formed token:", tokbuf);
                  }
               else
                  *q++ = *p;
               if ((t->t_field = fb_findfield(tokbuf, cdb_db)) == NULL)
                  fb_serror(FB_MESSAGE, "Could not locate database field:",
                     tokbuf);
               }
            }
         *q = NULL;
      }

/* 
 *  inrange - check if c is inrange with valid character string, or == xc.
 */
 
   static int inrange(c)
      int c;
      
      {
	 if (c &&
	       ((c >= 'A' && c <= 'Z') ||
	       (c >= 'a' && c <= 'z') ||
	       (c >= '0' && c <= '9') ||
	       (c == '_')))
	    return(1);
	 else
	    return(0);
      }
