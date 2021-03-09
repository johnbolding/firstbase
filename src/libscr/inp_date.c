/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: inp_date.c,v 9.0 2001/01/09 02:57:03 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Inputdate_sid[] = "@(#) $Id: inp_date.c,v 9.0 2001/01/09 02:57:03 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_scr_inputclear;
extern short cdb_datedisplay;
extern short cdb_datestyle;
extern short cdb_centurymark;
extern char *cdb_centurybase;
extern char *cdb_centurynext;

extern char CHAR_BLANK;
extern char CHAR_MINUS;
extern char CHAR_0;
extern char CHAR_SLASH;

static char *FMT2 = "%02d%02d%02d";

extern short int cdb_edit_input;	/* OLD == 1, NEW == 2 */
extern char *cdb_e_buf;

#define FB_AMERICAN	1
#define FB_EUROPEAN	2

#if FB_PROTOTYPES
static noslash(char *s);
#else /* FB_PROTOTYPES */
static noslash();
#endif /* FB_PROTOTYPES */

/* 
 *  inputdate - interface with fb_input() for date input --
 *	xecho with slashes imbedded 
 */
 
   fb_inputdate (row, col, maxc, minc, fmt, addr, xecho, okend, confirm)
      int   row, col, maxc, minc, xecho, okend, confirm;
      char fmt, *addr;
      {
         int m = 0, d = 0, y = 0, st = 0, t, i;
         char x[3], buf[20], *fb_trim(char *p), o_e_buf[20];
	 int tcol, trow, sz;
         
	 trow = row;
	 tcol = col;
         if (trow < 0)
	    trow = -trow;
         if (tcol < 0)
	    tcol = -tcol;
         t = 0;
	 if (maxc < 0)
	    maxc = -8;
	 else
            maxc = 8;
         fmt = FB_SLASH_NUMERIC;
         o_e_buf[0] = NULL;
         if (cdb_edit_input){
            if (cdb_datestyle == FB_EUROPEAN && strlen(cdb_e_buf) == 6){
               /* FB_EUROPEAN is DDMMYY - swap to MMDDYY */
               x[0] = cdb_e_buf[0]; x[1] = cdb_e_buf[1];
               cdb_e_buf[0] = cdb_e_buf[2];
               cdb_e_buf[1] = cdb_e_buf[3];
               cdb_e_buf[2] = x[0];
               cdb_e_buf[3] = x[1];
               }
            strcpy(o_e_buf, cdb_e_buf);
            }
         while (t == 0){
	    if (cdb_scr_inputclear){
	       fb_move(trow, tcol+6);
	       fb_s_putw(FB_BLANK), fb_s_putw(FB_BLANK);
	       if (cdb_datedisplay > 8){	/* long date display */
                  for (i = 9; i <= cdb_datedisplay; i++)
		     fb_s_putw(FB_BLANK);
		  }
	       }
            strcpy(buf, addr);
            if (cdb_edit_input)
               strcpy(cdb_e_buf, o_e_buf);
            st = fb_input(row,col,maxc,minc,fmt,buf,xecho,okend,confirm);
            if (st == FB_END || st == FB_DEFAULT || st == FB_ABORT || 
	          st == FB_DSIGNAL || st == FB_ESIGNAL || st == FB_YSIGNAL ||
		  st == FB_QHELP || st == FB_FSIGNAL || st == FB_BSIGNAL ||
		  st == FB_SSIGNAL || st == FB_QSIGNAL ||
		  st == FB_PAGEUP || st == FB_PAGEDOWN || st == FB_PSIGNAL ||
		  st == FB_CSIGNAL || st == FB_WSIGNAL || st == FB_DELSIGNAL)
               return(st);
            if (st == FB_ESCAPE_END)
               return(FB_END);
	    fb_trim(buf);
	    noslash(buf);
	    sz = strlen(buf);
	    if (sz == 0 && minc == 0)
	       return(FB_AOK);
            if (sz == 5){
               o_e_buf[0] = CHAR_0;
               o_e_buf[1] = NULL;
               strcat(o_e_buf, buf);
               strcpy(buf, o_e_buf);
               sz = 6;
               }
            if (sz != 6){
	       fb_serror(FB_MESSAGE, SYSMSG[S_INPUT_ERROR],
                  SYSMSG[S_BAD_FORMAT]);
               continue;
               }
            if (cdb_datestyle == FB_EUROPEAN){
               /* FB_EUROPEAN is DDMMYY - swap to MMDDYY */
               x[0] = buf[0]; x[1] = buf[1];
               buf[0] = buf[2];
               buf[1] = buf[3];
               buf[2] = x[0];
               buf[3] = x[1];
               }
            x[2] = NULL;
            x[0] = buf[0]; x[1] = buf[1]; m = atoi(x);
            if (m < 1 || m > 12){
	       fb_serror(FB_MESSAGE, SYSMSG[S_INPUT_ERROR],
                  SYSMSG[S_BAD_FORMAT]);
               continue;
               }
            x[0] = buf[2]; x[1] = buf[3]; d = atoi(x);
            if (d >= 1 && d <= 30 && m != 2)
               t = 1;
            else if (d == 31 && 
               (m==1 || m==3 || m==5 || m==7 || m==8 || m==10 || m==12))
               t = 1;
            else if (m == 2 && d >= 1 && d <= 29)
               t = 1;
            if (t != 1){
	       fb_serror(FB_MESSAGE, SYSMSG[S_INPUT_ERROR],
                  SYSMSG[S_BAD_FORMAT]);
               continue;
               }
            t = 0;
            x[0] = buf[4]; x[1] = buf[5]; y = atoi(x);
            if (m != 2)
               break;
            if ((y%4 == 0 && y%100 != 0) || (y%400 == 0)){
               if (d >= 1 && d <= 29)
                  break;
               }
            else if (d >=1 && d <= 28)
               break;
	    fb_serror(FB_MESSAGE, SYSMSG[S_INPUT_ERROR], SYSMSG[S_BAD_FORMAT]);
            }
         sprintf((char *) addr, FMT2, m, d, y);
         fb_formfield(buf, (char *) addr, 'd', 6);
         fb_move(trow, tcol);
	 fb_standout();
         fb_prints(buf);
	 fb_standend();
	 fb_refresh();
         return(st);
      }

/*
 * noslash - slide numbers over any slashes/dashes. try and interpret
 *	any kind of date pattern at all.
 */

   static noslash(s)
      char *s;

      {
         char *p, *pc;
	 int lcount = 0;
	 
	 p = s;
	 for (; *s; s++){
	    if (*s == CHAR_SLASH || *s == CHAR_MINUS){
	       if (lcount == 1){
	          pc = p - 1;
	          *p++ = *pc;
		  *pc = CHAR_0;
		  }
	       lcount = 0;
	       }
	    else{
	       *p++ = *s;
	       lcount++;
	       }
	    }
	 if (lcount == 1){
	    pc = p - 1;
	    *p++ = *pc;
	    *pc = CHAR_BLANK;
	    }
	 *p = NULL;
      }
