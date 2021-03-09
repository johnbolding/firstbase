/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_verify.c,v 9.1 2002/08/13 19:04:02 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Mac_verify_sid[] = "@(#) $Id: mac_verify.c,v 9.1 2002/08/13 19:04:02 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>

extern short int cdb_datestyle;	/* date style - 1 = american, 2 = european */
extern char *cdb_centurybase;	/* century base - 19 for now */
extern char *cdb_centurynext;	/* next century  - 20 for now */
extern short int cdb_centurymark; /* mark point of century assumptions */

/*
 * mf_verify_date - takes a possibly formatted date, strips out
 *	the slashes and dashes, and converts into a proper FirstBase MMDDYY
 *	string, storing into the return variable.
 *
 *	verify_date(ret, date_string)
 */

   mf_verify_date(n, r)
      fb_mnode *n, *r;

      {
         char month[FB_MAXLINE], day[FB_MAXLINE], year[FB_MAXLINE], *p, *q;
         char buf[FB_MAXLINE];
         int m, d, y, t, x, st, count, century, limit;
         fb_mnode *nret;

         (void) Mac_verify_sid;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         nret = mnode_to_var(n);
         if (nret == NULL)
            return(FB_ERROR);
         n = n->n_next;
         tostring(n);
         fb_rmlead(n->n_pval);
         fb_trim(n->n_pval);
         p = n->n_pval;
         month[0] = day[0] = year[0];
         if (strlen(p) == 5)
            limit = 1;
         else
            limit = 2;
         /* get month */
         q = month;
         for (count = 0; isdigit(*p); ){
            *q++ = *p++;
            if (++count >= limit)
               break;
            if (*p == '/' || *p == '-')
               break;
            }
         *q = NULL;

         if (*p == '/' || *p == '-')
            p++;
         /* get day */
         q = day;
         for (count = 0; isdigit(*p); ){
            *q++ = *p++;
            if (++count >= 2)
               break;
            if (*p == '/' || *p == '-')
               break;
            }
         if (*p == '/' || *p == '-')
            p++;
         *q = NULL;

         /* get year */
         q = year;
         for (count = 0; *p && isdigit(*p); ){
            *q++ = *p++;
            if (++count >= 4)
               break;
            }
         *q = NULL;

         m = atoi(month);
         d = atoi(day);
         y = atoi(year);
         if (y < 100){
            if (y > cdb_centurymark)
               century = (atoi(cdb_centurybase) * 100);
            else
               century = (atoi(cdb_centurynext) * 100);
            y += century;
            }
         else{
            sprintf(buf, "%d", y);
            buf[2] = buf[3] = '0';
            century = atoi(buf);
            }
         /* now y is a proper 4 digit year, and century is set as well */

         if (cdb_datestyle > 1){
            x = m;
            m = d;
            d = x;
            }
         st = FB_AOK;
         if (m < 1 || m > 12)
            st = FB_ERROR;
         t = FB_ERROR;
         if (d >= 1 && d <= 30 && m != 2)
            t = FB_AOK;
         else if (d == 31 && 
            (m==1 || m==3 || m==5 || m==7 || m==8 || m==10 || m==12))
            t = FB_AOK;
         else if (m == 2 && d >= 1 && d <= 29)
            t = FB_AOK;
         if (st == FB_AOK)
            st = t;
         if (st == FB_AOK && m == 2){
            t = FB_ERROR;
            if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0){
               if (d >= 1 && d <= 29)
                  t = FB_AOK;
               }
            else if (d >=1 && d <= 28)
               t = FB_AOK;
            st = t;
            }
         if (st == FB_ERROR)
            st = 0;
         r->n_fval = st;
         y -= century;
         if (st == FB_AOK){
            if (cdb_datestyle == 1)
               sprintf(buf, "%02d%02d%02d", m, d, y);
            else
               sprintf(buf, "%02d%02d%02d", d, m, y);
            fb_mkstr(&(nret->n_nval), buf);
            nret->n_tval |= T_STR;
            }
         return(FB_AOK);
      }

/*
 * mf_verify_dollar - takes a possibly formatted dollar, strips out
 *	any leading $, commas, and the decimal.
 *	a proper FB dollar string is returned in nret.
 * 	once caveat: the macro assignment mechanism already handles dollars
 *	   so return the decimal in place here!
 *	verify_dollar(buf, dollar_string, [maxsize])
 *	   maxsize represents the size of a FORMATTED dollar string
 */

   mf_verify_dollar(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *nret, *nmax, *nstr;
         char buf[FB_MAXLINE], *p, *q;
         int dotcount = 0, pastdot = 0, st, maxlen = 0, cc = 0, slen, flen;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         nret = mnode_to_var(n);n = n->n_next;
         if (nret == NULL)
            return(FB_ERROR);
         nstr = n;		n = n->n_next;
         if (n != NULL){
            nmax = n;
            tostring(nmax);
            maxlen = atoi(nmax->n_pval);
            }
         tostring(nstr);
         fb_rmlead(nstr->n_pval);
         fb_trim(nstr->n_pval);
         p = nstr->n_pval;
         if (*p == CHAR_DOLLAR)
            p++;
         q = buf;
         st = FB_AOK;
         for (dotcount = 0; *p; p++){
            if (*p == CHAR_DOT)
               dotcount++;
            else if (*p == CHAR_COMMA){
               if (dotcount)
                  st = FB_ERROR;
               continue;
               }
            else if (!isdigit(*p))
               st = FB_ERROR;
            else if (dotcount)
               pastdot++;
            if (++cc >= FB_MAXLINE)
               break;
            *q++ = *p;
            }
         *q = NULL;
         if (dotcount > 1 || pastdot > 2)
            st = FB_ERROR;
         if (maxlen > 0){
            slen = strlen(buf);
            flen = 1 + ((slen - 3) / 3) + slen;
            if (flen > maxlen)
               st = FB_ERROR;
            }
         if (st == FB_ERROR)
            st = 0;
         r->n_fval = st;
         if (st == FB_AOK){
            fb_mkstr(&(nret->n_nval), buf);
            nret->n_tval |= T_STR;
            }
         return(FB_AOK);
      }

/*
 * mf_verify_float - takes a possibly formatted float, strips out
 *	any leading or trailing blanks, checks for digits + decimal.
 *	a proper FB float string is returned in nret.
 */

   mf_verify_float(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *nret, *nstr, *nmax;
         char buf[FB_MAXLINE], *p, *q;
         int dotcount = 0, st, maxlen = 0, cc = 0;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         nret = mnode_to_var(n); n = n->n_next;
         if (nret == NULL)
            return(FB_ERROR);
         nstr = n;		 n = n->n_next;
         if (n != NULL){
            nmax = n;
            tostring(nmax);
            maxlen = atoi(nmax->n_pval);
            }
         tostring(nstr);
         fb_rmlead(nstr->n_pval);
         fb_trim(nstr->n_pval);
         p = nstr->n_pval;
         q = buf;
         st = FB_AOK;
         for (dotcount = 0; *p; p++){
            if (*p == CHAR_DOT)
               dotcount++;
            else if (!isdigit(*p))
               st = FB_ERROR;
            if (++cc >= FB_MAXLINE)
               break;
            *q++ = *p;
            }
         *q = NULL;
         if (dotcount > 1)
            st = FB_ERROR;
         if (maxlen > 0 && strlen(buf) > maxlen)
            st = FB_ERROR;
         if (st == FB_ERROR)
            st = 0;
         r->n_fval = st;
         if (st == FB_AOK){
            fb_mkstr(&(nret->n_nval), buf);
            nret->n_tval |= T_STR;
            }
         return(FB_AOK);
      }

/*
 * mf_verify_pos_numeric - takes a possibly formatted numeric, strips out
 *	any leading or trailing blanks, checks for digits.
 *	a proper FB numeric string is returned in nret.
 */

   mf_verify_pos_numeric(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *nret, *nstr, *nmax;
         char buf[FB_MAXLINE], *p, *q;
         int st, cc = 0, maxlen = 0;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         nret = mnode_to_var(n); n = n->n_next;
         if (nret == NULL)
            return(FB_ERROR);
         nstr = n;		 n = n->n_next;
         if (n != NULL){
            nmax = n;
            tostring(nmax);
            maxlen = atoi(nmax->n_pval);
            }
         tostring(nstr);
         fb_rmlead(nstr->n_pval);
         fb_trim(nstr->n_pval);
         p = nstr->n_pval;
         q = buf;
         st = FB_AOK;
         for (; *p; p++){
            if (!isdigit(*p))
               st = FB_ERROR;
            if (++cc >= FB_MAXLINE)
               break;
            *q++ = *p;
            }
         *q = NULL;
         if (maxlen > 0 && strlen(buf) > maxlen)
            st = FB_ERROR;
         if (st == FB_ERROR)
            st = 0;
         r->n_fval = st;
         if (st == FB_AOK){
            fb_mkstr(&(nret->n_nval), buf);
            nret->n_tval |= T_STR;
            }
         return(FB_AOK);
      }

/*
 * mf_verify_numeric - takes a possibly formatted numeric, strips out
 *	any leading or trailing blanks, checks for digits.
 *	a proper FB numeric string is returned in nret.
 */

   mf_verify_numeric(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *nret, *nstr, *nmax;
         char buf[FB_MAXLINE], *p, *q;
         int st, cc = 0, maxlen = 0;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         nret = mnode_to_var(n);	n = n->n_next;
         if (nret == NULL)
            return(FB_ERROR);
         nstr = n;		 	n = n->n_next;
         if (n != NULL){
            nmax = n;
            tostring(nmax);
            maxlen = atoi(nmax->n_pval);
            }
         tostring(nstr);
         fb_rmlead(nstr->n_pval);
         fb_trim(nstr->n_pval);
         p = nstr->n_pval;
         q = buf;
         if (*p == CHAR_PLUS || *p == CHAR_MINUS)
            *q++ = *p++;
         st = FB_AOK;
         for (; *p; p++){
            if (!isdigit(*p))
               st = FB_ERROR;
            if (++cc >= FB_MAXLINE)
               break;
            *q++ = *p;
            }
         *q = NULL;
         if (maxlen > 0 && strlen(buf) > maxlen)
            st = FB_ERROR;
         if (st == FB_ERROR)
            st = 0;
         r->n_fval = st;
         if (st == FB_AOK){
            fb_mkstr(&(nret->n_nval), buf);
            nret->n_tval |= T_STR;
            }
         return(FB_AOK);
      }

/*
 * mf_verify_ascii - takes an ascii string and makes sure there are no
 *	characters outside of decimal:10,32-126 ...
 *	check size if requested, too
 *	converts those characters to ~, never fails for non-ascii now.
 */

   mf_verify_ascii(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *nret, *nstr, *nmax;
         char *buf, *p, *q;
         int st, maxlen = 0;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         nret = mnode_to_var(n);	n = n->n_next;
         if (nret == NULL)
            return(FB_ERROR);
         nstr = n;		 	n = n->n_next;
         if (n != NULL){
            nmax = n;
            tostring(nmax);
            maxlen = atoi(nmax->n_pval);
            }
         tostring(nstr);
         p = nstr->n_pval;
         buf = (char *) fb_malloc((unsigned) (strlen(p) + 1));
         q = buf;
         st = FB_AOK;
         for (; *p; p++){
            if (*p == 10 || *p == 13 || (*p >= 32 && *p <= 126))
               ;
            else{
               /*
               st = FB_ERROR;
               break;
               */
               *p = '~';
               }
            if (*p != 13)	/* strip CR's */
               *q++ = *p;
            }
         *q = NULL;
         if (maxlen > 0 && strlen(buf) > maxlen)
            st = FB_ERROR;
         if (st == FB_ERROR)
            st = 0;
         r->n_fval = st;
         if (st == FB_AOK){
            fb_mkstr(&(nret->n_nval), buf);
            nret->n_tval |= T_STR;
            }
         fb_free(buf);
         return(FB_AOK);
      }
