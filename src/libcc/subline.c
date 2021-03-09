/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: subline.c,v 9.2 2006/09/19 17:43:22 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Subline_sid[] = "@(#) $Id: subline.c,v 9.2 2006/09/19 17:43:22 john Exp $";
#endif

#include <fb.h>

#if !FB_PROTOTYPES
static int fb_rearline();
#else
static int fb_rearline(char *line, char *s, int snum, int attr);
#endif /* FB_PROTOTYPES */

#define FB_MAXLINE 	250		/* general mac line length */
int fb_subline_maxline = FB_MAXLINE;

/*
 * subline - get attr separated field snum from s into line.
 */

   fb_subline(line, s, snum, attr)
      char *line, *s;
      int snum, attr;
 
      {
         char *p, *left_p, *right_p;
         int count;

         if (fb_subline_maxline < FB_MAXLINE)
            fb_subline_maxline = FB_MAXLINE;
         /* null out line in advance */
         line[0] = '\0';
         if (*s == 0)
            return(0);
         if (snum < 0)
            return(fb_rearline(line, s, snum, attr));
         /*
          * put left_p at slot 1 of snum-1 attr
          * put right_p at last char of the line.
          * copy from left_p to right_p inclusive into line
          */
         for (p = s, count = 0; count < snum - 1 && *p; p++){
            if (*p == attr)
               count++;
            }
         if (count == snum - 1){
            left_p = p;
            if (!*p)
               return(0);
            }
         else
            return(0);
         /*
          * left_p points to the first char of subline we want
          * so, march right_p to the right until EOS or attr.
         */
         for (right_p = p; *p && *p != attr; p++)
            right_p = p;

         /*
          * now copy from left_p to right_p.
          * test right_p to make sure it is not an attr character.
          */
         for (p = line; ; left_p++){
            if (*left_p != attr)
               *p++ = *left_p;
            if (left_p == right_p)
               break;
            }
         *p = '\0';
         return(1);
      }

   static fb_rearline(line, s, snum, attr)
      char *line, *s;
      int snum, attr;
 
      {
         char *p, *left_p, *right_p;
	 int count;

         for (p = s; *p; p++)
            ;
         if (p != s)
            p--;
         if (*p == attr)
            p--;
         /*
          * locate where left_p should go. then do right_p.
          * copy from left_p to right_p inclusive into line
          */
         for (count = 0; count > snum; p--){
            if (*p == attr || p == s){
               if (--count <= snum)
                  break;
               }
            if (p == s)
               break;
            }
         if (count == snum){
            if (p != s)
               left_p = ++p;
            else
               left_p = p;
            if (!*p)
               return(0);
            }
         else
            return(0);
         /*
          * left_p points to the first char of subline we want
          * so, march right_p to the right until EOS or attr.
         */
         for (right_p = p; *p && *p != attr; p++)
            right_p = p;

         /*
          * now copy from left_p to right_p.
          * test right_p to make sure it is not an attr character.
          */
         for (p = line; ; left_p++){
            if (*left_p != attr)
               *p++ = *left_p;
            if (left_p == right_p)
               break;
            }
         *p = '\0';
         return(1);
      }
