/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: tdate.c,v 9.0 2001/01/09 02:56:21 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Tdate_sid[] = "@(#) $Id: tdate.c,v 9.0 2001/01/09 02:56:21 john Exp $";
#endif

#include <fb.h>
#include <time.h>

static char *FMT2 = "%02d:%02d";
static char *FMT3 = "%02d/%02d/%02d";
static char *FMT4 = "%02d%02d%02d";
static char *FMT5 = "%02d:%02d:%02d";

/*
* struct tm			
*         {
*         int     seconds,
*                 minutes,
*                 hours,
*                 day_month,      
*                 month,          
*                 year,           
*                 day_week,       
*                 day_year,       
*                 daylight;       
*         };
*/

/* 
 *  tdate - return a pointer to character time 
 */
 
   char *fb_tdate(p)
      register char *p;
   
      {
         time_t tme, *clock = &tme;
         struct tm xtm, *t = &xtm;
	 char *q;
         int yr;
   
         time(clock);
         t = (struct tm *) localtime(clock);
         sprintf(p, FMT2, t->tm_hour, t->tm_min);
	 q = p + 5;				/* result len of FMT2 */
	 *q++ = ' ';
         yr = t->tm_year;
         while (yr > 99)
            yr-= 100;
	 sprintf(q, FMT3, t->tm_mon+1, t->tm_mday, yr);
         return(p);
      }

/* 
 *  simpledate - return a pointer data string of time or date.
 *     0=time, 1=date, 2=mtime
 */
 
   char *fb_simpledate(p, m)
      register char *p;
      int m;
   
      {
         time_t tme, *clock = &tme;
         struct tm xtm, *t = &xtm;
         int yr;
   
         time(clock);
         t = (struct tm *) localtime(clock);
	 if (m == 2)		/* mtime */
            sprintf(p, FMT5, t->tm_hour, t->tm_min, t->tm_sec);
	 else if (m == 0)	/* time */
            sprintf(p, FMT2, t->tm_hour, t->tm_min);
	 else{			/* assume m==1, date */
            yr = t->tm_year;
            while (yr > 99)
               yr-= 100;
            sprintf(p, FMT4, t->tm_mon+1, t->tm_mday, yr);
            }
         return(p);
      }
