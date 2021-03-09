/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_date.c,v 9.1 2001/01/12 22:52:01 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macro_date_sid[] = "@(#) $Id: mac_date.c,v 9.1 2001/01/12 22:52:01 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>

#include <sys/types.h>

#if !HAVE_TM_ZONE
extern long timezone;
#endif /* !HAVE_TM_ZONE */

#define NSECONDS 	86400

static char *FMT4 = "%02d%02d%02d";
static char *DATE_MESG = "Calculated date is before Jan 1, 1970";

#if !FB_PROTOTYPES
static long date_to_seconds();
static void seconds_to_date();
#else /* FB_PROTOTYPES */
static long date_to_seconds(char *);
static void seconds_to_date(char *, long);
#endif /* FB_PROTOTYPES */

   mf_now(n, r)
      fb_mnode *n, *r;

      {
         time_t tme, *clock = &tme;

         (void) Macro_date_sid;

         r->n_fval = (double) time(clock);
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_cdbdate(n, r)
      fb_mnode *n, *r;

      {
         struct tm xtm, *t = &xtm;
         time_t tme, *clock = &tme;
         char buf[FB_MAXLINE];
         int yr;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tonumber(n);
         tme = (time_t) n->n_fval;
         t = (struct tm *) localtime(clock);
         yr = t->tm_year;
         while (yr > 99)
            yr-= 100;
         sprintf(buf, FMT4, t->tm_mon+1, t->tm_mday, yr);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         r->n_tval |= T_DTE;
         return(FB_AOK);
      }

   mf_year(n, r)
      fb_mnode *n, *r;

      {
         struct tm xtm, *t = &xtm;
         time_t tme, *clock = &tme;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tonumber(n);
         tme = (time_t) n->n_fval;
         t = (struct tm *) localtime(clock);
         r->n_fval = t->tm_year + 1900;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_month(n, r)
      fb_mnode *n, *r;

      {
         struct tm xtm, *t = &xtm;
         time_t tme, *clock = &tme;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tonumber(n);
         tme = (time_t) n->n_fval;
         t = (struct tm *) localtime(clock);
         r->n_fval = t->tm_mon + 1;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_day(n, r)
      fb_mnode *n, *r;

      {
         struct tm xtm, *t = &xtm;
         time_t tme, *clock = &tme;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tonumber(n);
         tme = (time_t) n->n_fval;
         t = (struct tm *) localtime(clock);
         r->n_fval = t->tm_mday;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_hour(n, r)
      fb_mnode *n, *r;

      {
         struct tm xtm, *t = &xtm;
         time_t tme, *clock = &tme;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tonumber(n);
         tme = (time_t) n->n_fval;
         t = (struct tm *) localtime(clock);
         r->n_fval = t->tm_hour;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_minute(n, r)
      fb_mnode *n, *r;

      {
         struct tm xtm, *t = &xtm;
         time_t tme, *clock = &tme;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tonumber(n);
         tme = (time_t) n->n_fval;
         t = (struct tm *) localtime(clock);
         r->n_fval = t->tm_min;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_second(n, r)
      fb_mnode *n, *r;

      {
         struct tm xtm, *t = &xtm;
         time_t tme, *clock = &tme;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tonumber(n);
         tme = (time_t) n->n_fval;
         t = (struct tm *) localtime(clock);
         r->n_fval = t->tm_sec;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_date(n, r)
      fb_mnode *n, *r;

      {
         struct tm xtm, *t = &xtm;
         time_t tme, *clock = &tme;
         char *p, buf[FB_MAXLINE];
         int lc;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tonumber(n);
         tme = (time_t) n->n_fval;
         t = (struct tm *) localtime(clock);
         p = asctime(t);
         strcpy(buf, p);
         lc = strlen(buf) - 1;
         if (buf[lc] == FB_NEWLINE)
            buf[lc] = NULL;
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_dts(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2, *n3;
         int j70, new, y;
         struct tm xtm, *t = &xtm;
         time_t tme, *clock = &tme;

         if (fb_realnodes(n) < 3)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         n2 = n; n = n->n_next;
         n3 = n;
         j70 = fb_julian(1, 1, 1970);
         tonumber(n1);
         tonumber(n2);
         tonumber(n3);
         y = (int) n3->n_fval;
         if (y < 100)
            y += 1900;
         new = fb_julian((int) n1->n_fval, (int) n2->n_fval, y);
         if (new < j70){
            fb_serror(FB_MESSAGE, DATE_MESG, NIL);
            return(FB_ERROR);
            }
         r->n_fval = (double) ((new - j70) * NSECONDS);
         tme = (time_t) r->n_fval;
         t = (struct tm *) localtime(clock);
#if HAVE_TM_ZONE
         r->n_fval -= (double) t->tm_gmtoff;
#else /* not HAVE_TM_ZONE */
         r->n_fval += (double) timezone;
#endif
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_cdbdts(n, r)
      fb_mnode *n, *r;

      {
         long c_sec;
         char buf[20];

         if (!istype_str(n))
            return(FB_ERROR);
         strcpy(buf, n->n_nval);
         if ((c_sec = date_to_seconds(buf)) == FB_ERROR)
            return(FB_ERROR);
         r->n_fval = (double) c_sec;
         return(FB_AOK);
      }

   mf_tts(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2, *n3;

         if (fb_realnodes(n) < 3)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         n2 = n; n = n->n_next;
         n3 = n;
         tonumber(n1);
         tonumber(n2);
         tonumber(n3);
         r->n_fval = (double) (n1->n_fval * 60 + n2->n_fval) * 60 + n3->n_fval;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

/*
 * newdate - calculate a newdate given a cdbdate and a number (of days)
 */

   mf_newdate(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2;
         long c_sec, n_sec, add_sec;
         char buf[20];

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         n2 = n;
         tonumber(n2);
         if (!istype_str(n1))
            return(FB_ERROR);
         strcpy(buf, n1->n_nval);
         if ((c_sec = date_to_seconds(buf)) == FB_ERROR)
            return(FB_ERROR);
         add_sec = (n2->n_fval * NSECONDS);
         n_sec = c_sec + add_sec;
         seconds_to_date(buf, n_sec);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_DTE;
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * ndays - calculate number of days between two cdbdate points
 */

   mf_ndays(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2;
         long s_sec, e_sec;
         char buf[20];

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         n2 = n;
         if (!istype_str(n1))
            return(FB_ERROR);
         strcpy(buf, n1->n_nval);
         s_sec = date_to_seconds(buf);
         if (!istype_str(n2))
            return(FB_ERROR);
         strcpy(buf, n2->n_nval);
         e_sec = date_to_seconds(buf);
         r->n_fval = (double) ((e_sec - s_sec) / NSECONDS);
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

/*
 * date_to_seconds 
 */

   static long date_to_seconds(s)
      char *s;

      {
         int m, d, y, j70, new, len;
         struct tm xtm, *t = &xtm;
         time_t tme, *clock = &tme;
         char ns[FB_MAXLINE];

         if (s == NULL || s[0] == NULL || (len = strlen(s)) < 6)
            return((long) 0);
         if (len == 6)
            fb_longdate(ns, s);
         else
            strcpy(ns, s);
         y = atoi(ns + 4);
         *(ns + 4) = NULL;
         d = atoi(ns + 2);
         *(ns + 2) = NULL;
         m = atoi(ns);
         j70 = fb_julian(1, 1, 1970);
         new = fb_julian(m, d, y);
         if (new < j70){
            fb_serror(FB_MESSAGE, DATE_MESG, NIL);
            return(FB_ERROR);
            }
         tme = (time_t) ((new - j70) * NSECONDS);
         t = (struct tm *) localtime(clock);
#if HAVE_TM_ZONE
         tme -= t->tm_gmtoff;
#else /* not HAVE_TM_ZONE */
         tme += timezone;
#endif
         return(tme);
      }

   static void seconds_to_date(buf, n)
      char *buf;
      long n;

      {
         struct tm xtm, *t = &xtm;
         time_t tme, *clock = &tme;
         int yr;

         tme = (time_t) n;
         t = (struct tm *) localtime(clock);
         yr = t->tm_year;
         while (yr >= 100)
            yr -= 100;
         sprintf(buf, FMT4, t->tm_mon+1, t->tm_mday, yr);
      }
