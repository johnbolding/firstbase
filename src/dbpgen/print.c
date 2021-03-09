/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: print.c,v 9.0 2001/01/09 02:55:43 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Print_sid[] = "@(#) $Id: print.c,v 9.0 2001/01/09 02:55:43 john Exp $";
#endif

/* 
 *  standard print routine - used only in dbpgen 
 */

#include <fb.h>
#include <fb_ext.h>

int pagelength = 60;
int formlength = 66;
int formfeed = 0;

static int firsttime = 0;			/* to mark first pass */
static int pagcnt = 0;				/* for fb_page count */
static int lincnt = 9999;			/* for line count */
static char tline[FB_PRINTLINE+2] = {""};	/* for temp pline */

/* 
 *  print - standard prinout routine - handles headers, fb_page numbers, etc 
 */
 
   print(s, fs, title, hdr1, hdr2, legnd, legnd2, coname, pagewidth)
      char *s, *title, *hdr1, *hdr2, *legnd, *legnd2, *coname;
      FILE *fs;
      int pagewidth;
      
      {
         char *p, date[FB_MAXNAME], *fb_tdate();
         int i, j;
         int   legctl;

         if (lincnt >= pagelength){
            if (firsttime < 2){
               if (firsttime == 0){
                  p = coname;
		  legnd2 = title;
		  }
               else
                  p = title;
               sprintf(tline, FB_FSTRING, fb_tdate(date));
               i = (pagewidth/2) - (strlen(p)/2) + 1;
               for (j = strlen(tline); j <= i; tline[j++] = FB_BLANK)
                  ;
               tline[j] = NULL;
               strcat(tline, p);
               for (j = strlen(tline); j < (pagewidth-9); tline[j++]=FB_BLANK)
                  ;
               tline[j] = NULL;
               strcat(tline, "Page ");
               firsttime++;
               }
            while ((lincnt++) <= formlength){
               if (formfeed){
                  fprintf(fs, "\014");
                  break;
                  }
               fprintf(fs, "\n");
               }
            lincnt = 6; legctl = 0;
            fprintf(fs, "\n");
            fprintf(fs, "%s%3d\n\n", tline, ++pagcnt);
            if (strlen(legnd) > 0){
               i = (pagewidth/2) - (strlen(legnd)/2) + 1;
	       fprintf(fs, "%s", fb_pad(tline, " ", i));
               fprintf(fs, "%s\n", legnd);
               lincnt++; legctl = 1;
               }
            if (strlen(legnd2) > 0){
               i = (pagewidth/2) - (strlen(legnd2)/2) + 1;
	       fprintf(fs, "%s", fb_pad(tline, " ", i));
               fprintf(fs, "%s\n", legnd2);
               lincnt++; legctl = 1;
               }
            if (legctl == 1){
               fprintf(fs, "\n");
               lincnt++;
               }
            fprintf(fs, "%s\n", hdr1);
            if (strlen(hdr2) > 0){
               fprintf(fs, "%s\n", hdr2);
               lincnt++;
               }
            fprintf(fs, "\n");
            }
         fprintf(fs, "%s\n", s);
         lincnt++;
      }

/*
 * testpage - return number of lines left on page
 */

   testpage()
      {
         return(pagelength - lincnt);
      }
