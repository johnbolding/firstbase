/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: formfld.c,v 9.0 2001/01/09 02:56:26 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Formfield_sid[] = "@(#) $Id: formfld.c,v 9.0 2001/01/09 02:56:26 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_datedisplay;
extern short int cdb_datestyle;
extern short int cdb_e_negative;
static char *L_FB_FDATE = "%c%c/%c%c/%c%c%c%c";
static char *F_FB_FDATE = "%c%c %s %c%c%c%c";

#define EMIT		'e'

static char *emitfield(char *s, char *t, int type, int size);
extern char *fb_formdollar(char *s, char *t, int size);
extern char *fb_longdate(char *s, char *t);

/* 
 *  formfield - format any cdb string t into s by type. return s.
 */
 
   char *fb_formfield(s, t, f_type, size)
      char *s, *t, f_type;
      int size;
      
      {
         char ldate[10];
         char mon[4];
         short int m;

         s[0] = NULL;
	 if (*t){
	    if (f_type == FB_DOLLARS){
	       fb_formdollar(s, t, size);
	       strcpy(t, s);
	       fb_rjustify(s, t, size, 0);
	       }
	    else if (FB_OFNUMERIC(f_type))
	       fb_rjustify(s, t, size, f_type);
	    else if (f_type == FB_DATE){
	       if (strlen(t) == 6){
                  switch(cdb_datedisplay){
                     case 8:
                        if (cdb_datestyle == FB_AMERICAN)
                           sprintf(s, FB_FDATE, t[0],t[1],t[2],t[3],t[4],t[5]);
                        else 		/* FB_EUROPEAN */
                           sprintf(s, FB_FDATE,t[2],t[3],t[0],t[1],t[4],t[5]);
                        break;
                     case 10:		 /* long date display */
                        fb_longdate(ldate, t);
                        if (cdb_datestyle == FB_AMERICAN)
                           sprintf(s, L_FB_FDATE, ldate[0], ldate[1], ldate[2], 
                              ldate[3],ldate[4],ldate[5],ldate[6],ldate[7]);
                        else 		/* FB_EUROPEAN */
                           sprintf(s, L_FB_FDATE, ldate[2], ldate[3], ldate[0], 
                              ldate[1],ldate[4],ldate[5],ldate[6],ldate[7]);
                        break;
                     case 11:		 /* formal style  - 05 May 1960 */
                        /* thanks to a suggestion from wendy robin! */
                        fb_longdate(ldate, t);
                        mon[0] = ldate[0];
                        mon[1] = ldate[1];
                        mon[2] = NULL;
                        m = atoi(mon);
                        switch(m){
                           case 1: strcpy(mon, "Jan"); break;
                           case 2: strcpy(mon, "Feb"); break;
                           case 3: strcpy(mon, "Mar"); break;
                           case 4: strcpy(mon, "Apr"); break;
                           case 5: strcpy(mon, "May"); break;
                           case 6: strcpy(mon, "Jun"); break;
                           case 7: strcpy(mon, "Jul"); break;
                           case 8: strcpy(mon, "Aug"); break;
                           case 9: strcpy(mon, "Sep"); break;
                           case 10: strcpy(mon, "Oct"); break;
                           case 11: strcpy(mon, "Nov"); break;
                           case 12: strcpy(mon, "Dec"); break;
                           }
                        sprintf(s, F_FB_FDATE, ldate[2], ldate[3], mon,
                           ldate[4], ldate[5], ldate[6], ldate[7]);
                        break;
                     }
                  }
	       }
	    else if (f_type == FB_FORMULA)
	       fb_rjustify(s, t, size, FB_NUMERIC);
	    else if (f_type == FB_BINARY)
               s[0] = NULL;
	    else if (f_type == EMIT)
	       emitfield(s, t, f_type, size);
	    else
	       strcpy(s, t);
	    }
	 return(s);
      }

/*
 * format a dollar string from t into s. use the form "$###,###,###.##"
 */
   char *fb_formdollar(s, t, size)
      char *s, *t;
      int size;
      
      {
	 int neg = 0, len, count, j;
	 double val;
	 char *p, *q, pennies[5], dollars[FB_MAXLINE];

         if (equal(t, "-000"))
            val = 0;
         else
	    val = atof(t);
	 if (val < 0){
	    neg = 1;
	    val = -val;
	    strcpy(s, t);
	    sprintf(t, "%.0f", val);
	    }
	 len = strlen(t);
	 if (len <= 5){
	    if (neg){
	       if (!cdb_e_negative)
	          sprintf(s, "(%.2f)", val / 100);
	       else
	          sprintf(s, "-%.2f ", val / 100);
	       }
	    else
	       sprintf(s, "%.2f ", val / 100);
	    return(s);
	    }
	 p = t + strlen(t);	/* p = &t[strlen(t)] */
	 q = dollars + FB_MAXLINE - 1;
	 *q-- = NULL;
	 pennies[2] = *p--;
	 pennies[1] = *p--;
	 pennies[0] = *p--;
	 len -= 2;
	 for (count = 0; len > 0; len--, count++){
	    if (count >= 3){
	       count = 0;
	       *q-- = ',';
	       }
	     *q-- = *p--;
	     }
	  q++;
	  j = 3; 	/* length of '.## ' */
	  if (neg)
	     j += 2;
	  if (strlen(q) + j > size){
	     p = q + size - 4;
	     *p++ = pennies[0] = pennies[1] = CHAR_PERCENT;
	     *p = NULL;
	     }
	  if (!neg)
	     sprintf(s, "%s.%s ", q, pennies);
	  else{
	     if (!cdb_e_negative)
	        sprintf(s, "(%s.%s)",  q, pennies);
	     else
	        sprintf(s, "-%s.%s ",  q, pennies);
	     }
	  return(s);
       }

/* 
 *  emitfield - format any cdb string t into emittable/loadable s by type.
 *	return s.
 */
      
   static char *emitfield(s, t, f_type, size)
      register char *s, *t;
      int f_type;
      int size;
      
      {
         char *q;

         *s = NULL;
         q = s;
         for (; *t; ){
            if (*t == FB_NEWLINE){
               *s++ = CHAR_BACKSLASH;
               *s++ = CHAR_n;
               t++;
               }
            else
               *s++ = *t++;
            }
         *s = NULL;
	 return(q);
      }
