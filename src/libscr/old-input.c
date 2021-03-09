/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: input.c,v 9.4 2002/12/29 17:22:41 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Input_sid[] = "@(#) $Id: input.c,v 9.4 2002/12/29 17:22:41 john Exp $";
#endif

/*
 *  input.c - standard input routines
 */

#include <fb.h>
#include <fb_ext.h>
#include <panic.h>
#include <keypad.h>

#include <setjmp.h>
#include <signal.h>
static jmp_buf jmp_env_stop, jmp_env_resize, jmp_env_read;

#if FB_PROTOTYPES
static format(char *buf, char *p, char *addr, int fmt, int maxc, int fcount);
static valid(int c, int fmt, int xecho, int okend, int maxc, int count);
#if HAVE_SETJMP_H
static RETSIGTYPE ontstp(int disp);
static RETSIGTYPE stopjob(void);
#if SIGWINCH
static RETSIGTYPE onresize(void);
static void ignoreresize(void);
static void setresize(void);
static void do_resize(void);
#endif /* SIGWINCH */
#endif HAVE_SETJMP_H
static void ignorestop(void);
static void setstop(void);
static kcheck(int c);
static read_kcheck(int);
static test_sigslot(char *buf);
RETSIGTYPE input_sigalrm(int disp);
RETSIGTYPE read_sigalrm(int disp);
#else /* FB_PROTOTYPES */
static format();
static valid();
static void ignorestop();
static void setstop();
static kcheck();
static read_kcheck();
static test_sigslot();
RETSIGTYPE input_sigalrm();
RETSIGTYPE read_sigalrm();
#if HAVE_SETJMP_H
static RETSIGTYPE ontstp();
static RETSIGTYPE stopjob();
#if SIGWINCH
static RETSIGTYPE onresize();
static void ignoreresize();
static void setresize();
static void do_resize();
#endif /* SIGWINCH */
#endif /* HAVE_SETJMP_H */
#endif /* FB_PROTOTYPES */

static short
	endcount = 0,			/* for tracking endkey'strokes */
	dotcount = 0,			/* for counting FB_DOLLARS decimals */
	okesc = 0, 			/* is ok to escape if 1 */
	okhelp = 0, 			/* is ok to help if 1 (row < 0) */
	okdsig = 0,			/* if 1, ok to dsignal out */
	okfsig = 0,			/* if 1, ok to fsignal out */
	okssig = 0,			/* if 1, ok to ssignal out */
	okctlsig = 0;			/* if 1, ok to <CTL>signal out */

static short int alarm_fired = 0;

char func_buf[FB_MAXLINE];		/* for capturing last signal */

extern short cdb_decimal;
extern short int cdb_scr_inputdots;
extern short int cdb_scr_inputclear;
extern short int cdb_scr_inputpastedge;
extern short int cdb_functionkeys;
extern short int cdb_okstop;
extern unsigned cdb_ualarm;
extern short int cdb_keypad[];
extern char cdb_K0[];
extern char cdb_K1[];
extern char cdb_K2[];
extern char cdb_K3[];
extern char cdb_K4[];
extern char cdb_K5[];
extern char cdb_K6[];
extern char cdb_K7[];
extern char cdb_K8[];
extern char cdb_K9[];
extern char cdb_KU[];
extern char cdb_KD[];
extern char cdb_KL[];
extern char cdb_KR[];
extern char cdb_CM[];
static char *I_PAGEUP = "\033\020";
static char *I_PAGEDOWN = "\033\016";
static char *I_WSIGNAL = "\033\027";
static char *I_DELSIGNAL = "\033\004";
static char *DECIMAL_ZERO = ".00";
static char *END_STRING = "END";

/* 
 *  formalized input routine - all keystrokes are monitored.
 *     hacks include: -row, HELP signal accepted, +row HELP caught
 *                    -col, FB_DSIGNAL is ok, +col FB_DSIGNAL is caught
 *                    -maxc, <CTL>SIGNAL is ok, +col <CTL>SIGNAL is caught
 *                    -xecho, FB_ESCAPEs are ok, +xecho FB_ESCAPEs are caught
 *                    -okend,FB_FSIGNAL/FB_BSIGNAL are ok, +okend are caught
 */
 
   fb_o_input(row, col, maxc, minc, fmt, addr, xecho, okend, confirm)
      int   row, col, maxc, minc, xecho, okend, confirm;
      char fmt, *addr;

      {
         char buf[FB_MAXINPUT], *q, c, *p = buf;
         int *iptr, bs, i, cdiff, kfunction, orig_maxc;
         long *lp;
	 int count = 0;			/* for counting keystrokes */
         
         okhelp = 0; okdsig = 0; okctlsig = 0;
         okesc = okfsig = okssig = 0;
	 dotcount = 0;
	 *addr = 0;
         alarm_fired = 0;
#if PANIC
         panic_count = 0;
#endif	 
         fb_cx_write(1);
         if (fmt == FB_DATE)
            return(fb_inputdate(row, col, maxc, minc, fmt, addr, xecho,
               okend, confirm));
         if (row < 0){
	    okhelp = 1;
	    row = -row;
	    }
         if (col < 0){
	    okdsig = 1;
	    col = -col;
	    }
         if (maxc < 0){
	    okctlsig = 1;
	    maxc = -maxc;
	    }
	 orig_maxc = maxc;
         if (xecho < 0){
	    okesc = 1;
	    xecho = -xecho;
	    }
         if (okend < 0){
	    okfsig = okssig = 1;
	    okend = -okend;
	    }
	 endcount = 0;
	 if (cdb_scr_inputclear){
	    fb_move(row, col);
	    for (i = 1; i <= maxc; i++)
	       if (cdb_scr_inputdots)
		  fb_s_putw(CHAR_DOT);
	       else
		  fb_s_putw(CHAR_BLANK);
	    }
         count = 0;
         for (; alarm_fired == 0; ){
#if HAVE_SETJMP_H
	    if (cdb_okstop){
	       if (setjmp(jmp_env_stop)){/* return from TTYSTOP sig handler */
		  stopjob();
		  }
	       signal(SIGTSTP, ontstp);
	       }
#ifdef SIGWINCH
            if (setjmp(jmp_env_resize)){
               do_resize();
               }
#endif /* SIGWINCH */
#endif /* HAVE_SETJMP_H */
            if (cdb_functionkeys)
	       signal(SIGALRM, input_sigalrm);
            if (count < maxc || cdb_scr_inputpastedge)
	       i = col + count;
	    else
	       i = col + maxc - 1;
            fb_move(row, i);
	    fb_refresh();
#ifdef SIGWINCH
            setresize();
#endif /* SIGWINCH */
            if (read(0, &c, 1) != 1)
	       fb_xerror(FB_READ_ERROR, "Error reading in input()", NIL);
#ifdef SIGWINCH
            ignoreresize();
#endif /* SIGWINCH */
            count++;
            if (cdb_functionkeys && c == FB_ESCAPE){
	       kfunction = FB_ERROR;
	       func_buf[0] = NULL;
	       if (setjmp(jmp_env_stop)){
                  /* value of 1 means return from longjmp jmp_env_stop */
                  alarm((unsigned) 0);
                  signal(SIGALRM, SIG_DFL);
		  if (func_buf[0] == FB_ESCAPE && func_buf[1] == NULL){
		     /*kfunction = FB_KEY_ESCAPE;*/
		     kfunction = FB_ESCAPE_END;
                     }
		  else
	             kfunction = test_sigslot(func_buf);
		  }
	       else 
	          kfunction = read_kcheck(count);
	       if (kfunction >= 0 && cdb_keypad[kfunction] != 0){
		  ignorestop();
		  return(cdb_keypad[kfunction]);
		  }
               else if (kfunction != FB_ERROR)
                  return(kfunction);
               }
	    if (fmt == FB_UPPERCASE && islower(c))
	       c = toupper(c);
            if (valid(c, fmt, xecho, okend, maxc, count) != FB_ERROR){
	       if (c == CHAR_DOT && fmt == FB_DOLLARS){
	          dotcount++;
		  if (count + 3 < maxc)
		     maxc = count + 3;
		  }
               if ((count <= maxc  && fmt != FB_DOLLARS) ||
                   (fmt == FB_DOLLARS && count < maxc)){
                  *p++ = c;
                  if (count == maxc && confirm == FB_NOCONFIRM)
                     break;
                  }
               else{
	          ignorestop();
	          fb_serror(FB_MESSAGE, SYSMSG[S_INPUT_ERROR],
                     SYSMSG[S_TOO_LONG]);
		  setstop();
                  if (fmt != FB_DOLLARS)
		     i = col + maxc;
                  else
		     i = col + maxc - 1;
		  if (!cdb_scr_inputpastedge)
		     i--;
                  fb_move(row, i);
		  if (!cdb_scr_inputpastedge)
		     fb_s_putw(*(p-1));
		  else
		     fb_s_putw(FB_BLANK);
                  fb_move(row, i);
                  count--;
                  }
               }
            else if (cdb_functionkeys && count == 1 && c != FB_ESCAPE && 
	          (kfunction = kcheck(c)) >= 0 &&
                  cdb_keypad[kfunction] != 0){
	       ignorestop();
               return(cdb_keypad[kfunction]);
               }
#if PANIC	       
	    else if (c == FB_PANICKEY){
	       ignorestop();
	       --count;
	       if (count <= 3 && ++panic_count >= PANIC_THRESH){
	          fb_move(cdb_t_lines, 60); fb_clrtoeol();
		  fb_printw(SYSMSG[S_FB]);
		  fb_refresh();
		  scanf("%12s", panic_area);
		  if (equal(panic_area, PANIC_PWD))
	             fb_xerror(FB_MESSAGE, "bye!", NIL);
		  panic_count = 0;
		  }
	       ignorestop();
	       fb_serror(FB_MESSAGE, SYSMSG[S_INPUT_ERROR], SYSMSG[S_NOHELP]);
	       setstop();
	       fb_move(row, col + count);
	       }
#endif	       
            else if (c == FB_HELPKEY && count == 1){
	       if (okhelp){
	          ignorestop();
                  return(FB_QHELP);	/* helpkey == column 1 backspace */
		  }
	       else{
	          --count;
		  ignorestop();
	          fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], "");
		  setstop();
                  fb_move(row, col + count);
		  }
	       }
            else if (c == FB_BACKSPACE || c == FB_RUBOUT){
               count--;
               if (xecho != FB_NOECHO && count > 0){
	          fb_move(row, col + count - 1);
		  if (cdb_scr_inputdots)
	             fb_s_putw(CHAR_DOT);
		  else
	             fb_s_putw(CHAR_BLANK);
	          fb_move(row, col + count - 1);
		  }
               if (count > 0)
                  count--;
               if (p != buf)
                  p--;
	       if (*p == CHAR_DOT){
	          dotcount = 0;
		  maxc = orig_maxc;
		  }
               }
            else if (c == FB_BACKWORD){
               count--;
	       for (bs = 0, q = p-1; count > 0 && p != buf; ){
		  if (*q == FB_BLANK && bs == 1)
		     break;
		  else if (*q != FB_BLANK)
		     bs = 1;
                  if (xecho != FB_NOECHO){
	             fb_move(row, col + count - 1);
		     if (cdb_scr_inputdots)
			fb_s_putw(CHAR_DOT);
		     else
			fb_s_putw(CHAR_BLANK);
	             fb_move(row, col + count - 1);
		     }
		  p--;
		  q--;
		  count--;
		  if (*p == CHAR_DOT){
		     dotcount = 0;
		     maxc = orig_maxc;
		     }
		  }
               }
            else if (c == FB_KILL || c == FB_KILL2){
	       ignorestop();
               return(FB_ABORT);
	       }
            else if (c == FB_DSIGNAL && okdsig == 1 && count == 1){
	       ignorestop();
               return(FB_DSIGNAL);
	       }
            else if ((c == FB_ESIGNAL || c == FB_ESIGNAL2 || c == FB_ESIGNAL3) && 
	          okctlsig == 1 && count == 1){
	       ignorestop();
               return(FB_ESIGNAL);
	       }
            else if ((c == FB_YSIGNAL || c == FB_YSIGNAL2) && 
	          okctlsig == 1 && count == 1){
	       ignorestop();
               return(FB_YSIGNAL);
	       }
            else if ((c == FB_FSIGNAL || c == FB_BSIGNAL) && 
	          okfsig == 1 && count == 1){
	       ignorestop();
               return(c);
	       }
            else if ((c == FB_SSIGNAL || c == FB_QSIGNAL) && 
	          okssig == 1 && count == 1){
	       ignorestop();
               return(c);
	       }
            else if (c == FB_SSIGNAL1 && okssig == 1 && count == 1){
	       ignorestop();
               return(FB_SSIGNAL);
	       }
            else if (c == FB_VSIGNAL && okssig == 1 && count == 1){
	       ignorestop();
               return(FB_QSIGNAL);
	       }
            else if (c == FB_REDRAW1 || c == FB_REDRAW2){
	       count--;
	       fb_redraw();
	       }
            else if (c == FB_ERASE){
               fb_move(row, col);
	       maxc = orig_maxc;
               for(i = 1; i <= maxc; i++)
		  if (cdb_scr_inputdots)
	             fb_s_putw(CHAR_DOT);
		  else
	             fb_s_putw(CHAR_BLANK);
               fb_move(row, col);
               count = dotcount = 0;
               p = buf;
               }
            else if (c == FB_ENDKEY && okend == FB_OKEND && count == 1){
               if (xecho != FB_NOECHO && maxc > 2 &&
                     (fmt != FB_NUMERIC || maxc > 3)){
                  fb_printw(END_STRING);
		  fb_refresh();
		  }
	       ignorestop();
               return(FB_END);
               }
            else if (c == FB_ESCAPE && count == 1 && okesc){
	       ignorestop();
               return(FB_ESCAPE_END);
               }
            else if (((c==FB_RETURN || c=='\015') || (c == FB_ESCAPE && okesc)) && 
	          count-1 >= minc){
               for (i = count; i <= maxc; i++)
                  fb_s_putw(FB_BLANK);			/* blank out dots */
               if (--count == 0){
                  if (fmt == FB_DOLLARS){
		     fb_move(row, col + maxc - 3);
                     fb_printw(DECIMAL_ZERO);
		     fb_refresh();
                     }
	          ignorestop();
                  return(FB_DEFAULT);
                  }
               else
                  break;
               }
            else{
               --count;
	       ignorestop();
	       fb_serror(FB_MESSAGE, SYSMSG[S_INPUT_ERROR], SYSMSG[S_INVALID]);
	       setstop();
               fb_move(row, col + count);
               }
            }
         *p = NULL;
         switch (fmt) {
            case 'l':			/* CHAR_l FB_LONG */ 
               lp = (long *) addr;
               *lp = atol(buf); 
               break;
            case 'i':			/* CHAR_i FB_INTEGER */
               iptr = (int *) addr;
               *iptr = atoi(buf); 
               break;
            default:  
	       if (cdb_decimal && fmt == FB_DOLLARS){ /* zeros added - decimal */
	          cdiff = fb_nodecimal(buf);
		  count += cdiff;
		  p += cdiff;
	          }
               format(buf, p, addr, fmt, maxc, count);
               if (fmt == FB_DOLLARS){
                  fb_move(row, col);
		  fb_standout();
                  fb_printw(FB_FSTRING, buf);
		  fb_standend();
                  }
               break;
            }
	 ignorestop();
	 if (c == FB_ESCAPE)
	    return(FB_ESCAPE_AOK);
         return(FB_AOK);
      }

/* 
 *  format a string into a buffer fb_padding as needed 
 */
 
   static format(buf, p, addr, fmt, maxc, fcount)
      char *buf, *addr;
      char *p;
      int maxc, fcount, fmt;

      {
         char *t;
	 char fb_padc;

         if (!(FB_OFNUMERIC(fmt))){
            for (; fcount < maxc; fcount++)
               *p++ = FB_BLANK;
            *p = NULL;
            }
         else {
            for (t = buf + (int) maxc ; p >= buf; )
               *t-- = *p--;
            fb_padc = FB_BLANK;
            if ((*(t+1) == CHAR_MINUS || *(t+1) == CHAR_PLUS) && 
	        (fmt == FB_NUMERIC || fmt == FB_FLOAT))
               if (*(t+1) == CHAR_PLUS)		/* dont let pluses show */
                  *(t+1) = fb_padc;
            for (; t >= buf; t--)
               *t = fb_padc;
            p = buf + (int) maxc;
            if (fmt == FB_DOLLARS){
               if (*(p-1) == FB_BLANK)
                  *(p-1) = CHAR_0;
               if (*(p-2) == FB_BLANK)
                  *(p-2) = CHAR_0;
               }
            }
         sprintf((char *) addr, FB_FSTRING, buf);
         if (fmt == FB_DOLLARS){
            for (t = buf + 1; t <= p-3; t++)
               *(t-1) = *(t);
            *(p-3) = CHAR_DOT;
            }
      }

/* 
 *  check validity of an input character
 */
 
   static valid(c, fmt, xecho, okend, maxc, count)
      int c, fmt;
      int xecho, okend, maxc, count;

      {
         int i;

         switch (c) {
            case FB_BACKSPACE :
	    case FB_BACKWORD  :
            case FB_RETURN    :
            case FB_ERASE     :
            case FB_KILL      :
            case FB_KILL2     :
	    case FB_DSIGNAL   :
	    case FB_REDRAW1   :
	    case FB_REDRAW2   :
	    
	    /*
	     *  FB_HELPKEY is same as backspace --- except on column 1 
	     */

#if PANIC	    
	    case FB_PANICKEY  :
#endif	    
            case FB_RUBOUT    : return(FB_ERROR);
            }
         if (c == FB_ESCAPE && okesc)
	    return(FB_ERROR);
         if (isdigit(c)){
            if (fmt == FB_STRICTALPHA)
               return(FB_ERROR);
            }
         else if ((c >= CHAR_A && c <= CHAR_Z) || 
	          (c >= CHAR_a && c <= CHAR_z)){
            if (fmt != FB_ALPHA && fmt != FB_STRICTALPHA && fmt != FB_UPPERCASE)
               return(FB_ERROR);
            }
	 else if (c == FB_ENDKEY){
	    switch (fmt){
	       /* FB_FORMULA, FB_NUMERIC, FB_DOLLARS */
	       case 'f': case 'n': case '$':
		  if (count == 2 && endcount == 1){
		     if (okend == FB_OKEND)
		        count = 1;
		     return(FB_ERROR);
		     }
		  endcount = 1;
		  if (count > 1)
		     return(FB_ERROR);
		  break;
	       case 'S':	/* FB_SLASH_NUMERIC */
	          if (count == 1 && okend == FB_OKEND)
		     return(FB_ERROR);
		  break;
	       default:
	          if (count == 1 || fmt == FB_STRICTALPHA || 
		        fmt == FB_POS_NUM || fmt == FB_SLASH_NUMERIC)
		     return(FB_ERROR);
		  break;
	       }
	    }
         else if (c == CHAR_SLASH && fmt == FB_SLASH_NUMERIC)
            ;
         else if ((c == CHAR_MINUS || c == CHAR_PLUS) &&
                  (fmt == FB_FLOAT || fmt == FB_NUMERIC || fmt==FB_DOLLARS) &&
		   count == 1)
            ;
	 else if (c == CHAR_DOT && fmt == FB_FLOAT)
	    ;
	 else if (c == CHAR_DOT && fmt == FB_DOLLARS && cdb_decimal && !dotcount)
	    ;
	 else if (c == FB_BLANK && (fmt == FB_POS_NUM || fmt == FB_SLASH_NUMERIC) &&
	       count == 1)	/* allows blanking of date */
	    ;
         else if (fmt != FB_ALPHA && fmt != FB_UPPERCASE)
            return(FB_ERROR);
         else if (c >= 0 && c <= 31)
            return(FB_ERROR);
         if (xecho != FB_NOECHO){
            fb_printw(FB_FCHAR, c);
	    if (count == 1 && !cdb_scr_inputclear){
	       for (i = 2; i <= maxc; i++)
		  fb_s_putw(CHAR_BLANK);
	       }
	    fb_refresh();
	    }
         return(FB_AOK);
      }

#if HAVE_SETJMP_H
   static RETSIGTYPE ontstp(disp)		/* set up jump point */
      int disp;

      {
	 longjmp(jmp_env_stop, 1);
      }

   static RETSIGTYPE stopjob()		/* stop this job */
      {
	 TrueMove(cdb_t_lines, 1);
	 fflush(stdout);
         alarm(0);
	 fb_settty(FB_ENDMODE);
         sleep(1);
	 kill(getpid(), SIGTSTP);
         sleep(2);
	 fb_settty(FB_EDITMODE);
	 fb_redraw();
      }

#ifdef SIGWINCH
   static void ignoreresize()
      {
	 signal(SIGWINCH, SIG_IGN);
      }

   static void setresize()
      {
	 signal(SIGWINCH, onresize);
      }

   static RETSIGTYPE onresize()		/* set up jump point */

      {
	 longjmp(jmp_env_resize, 1);
      }

   static void do_resize()
      {
	 fb_move(1, 1);
         fb_clrtoeol();
	 fb_move(2, 1);
         fb_clrtoeol();
	 fb_move(cdb_t_lines - 1, 1);
         fb_clrtoeol();
         fb_refresh();

         fb_winsize();
         
         fb_restore_header();
         fb_infoline();
         fb_refresh();
	 fb_redraw();
      }

#endif /* SIGWINCH */

#endif /* HAVE_SETJMP_H */

   static void ignorestop()
      {
#if HAVE_SETJMP_H
         if (cdb_okstop)
	    signal(SIGTSTP, SIG_IGN);
#endif /* HAVE_SETJMP_H */
      }

   static void setstop()
      {
#if HAVE_SETJMP_H
         if (cdb_okstop)
	    signal(SIGTSTP, ontstp);
#endif /* HAVE_SETJMP_H */
      }

/*
 * kcheck - check the K0 ... K9 termcap settings for possible single
 *	character functions. Any other known single character
 *	keystrokes could be defined here, like TAB.
 */

   static int kcheck(c)
      int c;

      {
         char tbuf[2];
	 
	 tbuf[0] = c; tbuf[1] = NULL;
	 if (equal(tbuf, cdb_K0))
	    return(0);
	 else if (equal(tbuf, cdb_K1))
	    return(1);
	 else if (equal(tbuf, cdb_K2))
	    return(2);
	 else if (equal(tbuf, cdb_K3))
	    return(3);
	 else if (equal(tbuf, cdb_K4))
	    return(4);
	 else if (equal(tbuf, cdb_K5))
	    return(5);
	 else if (equal(tbuf, cdb_K6))
	    return(6);
	 else if (equal(tbuf, cdb_K7))
	    return(7);
	 else if (equal(tbuf, cdb_K8))
	    return(8);
	 else if (equal(tbuf, cdb_K9))
	    return(9);
	 else if (tbuf[0] == '\t')
	    return(FB_KEY_TAB);
	 else
	    return(FB_ERROR);
      }

/*
 * read_kcheck - check the K0 ... K9 termcap settings for possible escape
 *	functions. Any other multiple character functions
 *	could be defined here, like KEY_UP, etc.
 */

   static read_kcheck(count)
      {
	 int st = FB_ERROR;
	 char cbuf, *p;

	 p = func_buf;
	 *p++ = FB_ESCAPE;
	 *p = NULL;
	 for (; alarm_fired == 0; ){
            signal(SIGALRM, SIG_DFL);
	    signal(SIGALRM, read_sigalrm);
            if (setjmp(jmp_env_read)){
               /* value of 1 means return from longjmp jmp_env_read */
               alarm((unsigned) 0);
               signal(SIGALRM, SIG_DFL);
               break;
               }
            st = FB_ERROR;
            st = FB_ERROR;
	    signal(SIGALRM, read_sigalrm);
alarm((unsigned) 0);
alarm((unsigned) 10);
            if (read(0, &cbuf, 1) != 1)
	       fb_xerror(FB_READ_ERROR, "Error reading in input()", NIL);
	    *p++ = cbuf;
	    *p = NULL;
	    if ((st = test_sigslot(func_buf)) != FB_ERROR)
	       break;
	    }
	 alarm((unsigned) 0);
         signal(SIGALRM, SIG_DFL);
	 if (st != FB_ERROR){
            if (st >= 0 && count != 1 && cdb_keypad[st] != FB_ABORT &&
	          cdb_keypad[st] != FB_CSIGNAL)
	       return(FB_ERROR);
	    return(st);
	    }
	 *p = NULL;
	 if ((st = test_sigslot(func_buf)) != FB_ERROR){
            if (st >= 0 && count != 1 && cdb_keypad[st] != FB_ABORT &&
	          cdb_keypad[st] != FB_CSIGNAL)
	       return(FB_ERROR);
	    }
	 return(st);
      }

   static test_sigslot(buf)
      char *buf;

      {
	 if (equal(buf, cdb_KU))
	    return(FB_KEY_UP);
	 else if (equal(buf, cdb_KD))
	    return(FB_KEY_DOWN);
	 else if (equal(buf, cdb_KL))
	    return(FB_KEY_LEFT);
	 else if (equal(buf, cdb_KR))
	    return(FB_KEY_RIGHT);
	 else if (equal(buf, cdb_K0))
	    return(0);
	 else if (equal(buf, cdb_K1))
	    return(1);
	 else if (equal(buf, cdb_K2))
	    return(2);
	 else if (equal(buf, cdb_K3))
	    return(3);
	 else if (equal(buf, cdb_K4))
	    return(4);
	 else if (equal(buf, cdb_K5))
	    return(5);
	 else if (equal(buf, cdb_K6))
	    return(6);
	 else if (equal(buf, cdb_K7))
	    return(7);
	 else if (equal(buf, cdb_K8))
	    return(8);
	 else if (equal(buf, cdb_K9))
	    return(9);
	 else if (equal(buf, I_PAGEUP))
            return(FB_PAGEUP);
	 else if (equal(buf, I_PAGEDOWN))
            return(FB_PAGEDOWN);
	 else if (equal(buf, I_WSIGNAL))
            return(FB_WSIGNAL);
	 else if (equal(buf, I_DELSIGNAL))
            return(FB_DELSIGNAL);
	 return(FB_ERROR);
      }

   RETSIGTYPE input_sigalrm(disp)
      int disp;

      {
         alarm_fired = 1;
         longjmp(jmp_env_stop, 1);
      }

   RETSIGTYPE read_sigalrm(disp)
      int disp;

      {
         alarm_fired = 1;
         longjmp(jmp_env_read, 1);
      }
