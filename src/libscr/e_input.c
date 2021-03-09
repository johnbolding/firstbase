/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: e_input.c,v 9.2 2001/02/05 18:20:31 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char E_input_sid[] = "@(#) $Id: e_input.c,v 9.2 2001/02/05 18:20:31 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <keyboard.h>
#include <signal.h>

#include <setjmp.h>
#include <signal.h>
static jmp_buf jmp_env_stop, jmp_env_resize;

#if FB_PROTOTYPES
static format(char *buf, char *p, char *addr, int fmt, int maxc, int fcount);
static e_valid(int c, int fmt, int okend);
static read_kseq(int c);
static e_invalid_char(void);
#if HAVE_SETJMP_H
static RETSIGTYPE ontstp(void);
static RETSIGTYPE stopjob(void);
#if SIGWINCH
static RETSIGTYPE onresize(void);
static void ignoreresize(void);
static void setresize(void);
static void do_resize(void);
#endif /* SIGWINCH */
#endif /* HAVE_SETJMP_H */
#else /* FB_PROTOTYPES */
static format();
static e_valid();
static read_kseq();
static e_invalid_char();
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

#define OKSIGPOS(t)		((t == 1) && (cdb_bpos == 0) && \
				 (!cdb_e_changes || equal(cdb_e_buf, o_buf)))
#define OKDEFPOS(t)		((t == 1) && (cdb_bpos == 0) && \
				 (!cdb_e_changes || equal(cdb_e_buf,o_buf) || \
                                  cdb_e_buf[0] == NULL))

short int
        cdb_bpos = 0,			/* position in the buffer */
        cdb_bmax = 0,			/* max buffer before bit pit */
	cdb_epos = 0,			/* end position of the buffer-NULL */
        cdb_e_changes;			/* flag denoting if changes */

static short int
	okesc = 0, 			/* is ok to escape if 1 */
	okhelp = 0, 			/* is ok to fb_help if 1 (row < 0) */
	okdsig = 0,			/* if 1, ok to dsignal out */
	okfsig = 0,			/* if 1, ok to fsignal out */
	okssig = 0,			/* if 1, ok to ssignal out */
	okctlsig = 0;			/* if 1, ok to <CTL>signal out */

static char func_buf[FB_MAXLINE];		/* for capturing last signal */
static int func_len;
static short cdb_allow_input_interrupt = 0;

extern short cdb_decimal;
extern short int cdb_scr_inputclear;
extern short int cdb_interrupt;
extern short int cdb_okstop;
static char *DECIMAL_ZERO = ".00";
static char *END_STRING = "END";

extern char k_quit_char;
extern char *cdb_e_buf;
extern short int cdb_e_st;
static char o_buf[FB_MAXLINE];		/* saved image of original e_buf */

extern char cdb_CL[];
extern char cdb_CM[];
extern char cdb_CD[];
extern char cdb_CE[];

/*
 *  e_input.c - the new, editable input mechanism.
 *	provides extensible editing commands and signals for outer layers.
 *
 *  formalized input routine - all keystrokes are monitored.
 *     hacks include: -row, HELP signal accepted, +row HELP caught
 *                    -col, FB_DSIGNAL is ok, +col FB_DSIGNAL is caught
 *                    -maxc, <CTL>SIGNAL is ok, +col <CTL>SIGNAL is caught
 *                    -xecho, FB_ESCAPEs are ok, +xecho FB_ESCAPEs are caught
 *                    -okend,FB_FSIGNAL/FB_BSIGNAL are ok, +okend are caught
 */
 
   fb_e_input(row, col, maxc, minc, fmt, addr, xecho, okend, confirm)
      int   row, col, maxc, minc, xecho, okend, confirm;
      char fmt, *addr;

      {
         char *q, c;
         int ic, *iptr, cdiff, kfunction, e_empty = 1, i;
         long *lp;

	 *addr = 0;
         /* get dates out of the way */
         if (fmt == FB_DATE)
            return(fb_inputdate(row, col, maxc, minc, fmt, addr, xecho,
               okend, confirm));
         okhelp = okdsig = okctlsig = 0;
         okesc = okfsig = okssig = 0;
         cdb_bpos = cdb_epos = cdb_e_changes = 0;
         cdb_e_st = 0;
         /* decode the various "flags" passed in */
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
         cdb_bmax = maxc;
         if (xecho < 0){
	    okesc = 1;
	    xecho = -xecho;
	    }
         if (okend < 0){
	    okfsig = okssig = 1;
	    okend = -okend;
	    }

         /* if e_buf is editable, some outside layer will have set it */
         o_buf[0] = NULL;
         if (cdb_e_buf[0] != NULL){
            strcpy(o_buf, cdb_e_buf);
            cdb_epos = strlen(cdb_e_buf);	/* points to NULL */
            if (cdb_epos > maxc){
               cdb_e_buf[maxc] = NULL;
               cdb_epos = maxc;
               }
            e_empty = 0;
            }

         fb_cx_write(1);			/* write to cx (cdbtool) */
         for (;;){
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
            if (xecho != FB_NOECHO)
               fb_e_display(row, col);
            if (cdb_bpos == maxc - 1 && cdb_epos == maxc &&
                  confirm == FB_NOCONFIRM)
               break;
#ifdef SIGWINCH
            setresize();
#endif /* SIGWINCH */
            if (read(0, &c, 1) != 1)
	       fb_xerror(FB_READ_ERROR, "Error reading in e_input()", NIL);
            ic = c;
#ifdef SIGWINCH
            ignoreresize();
#endif /* SIGWINCH */

            /*
             * special case - sometimes FB_ESCAPE is overloaded to mean
             *    the FB_END keystroke, kind of. this is usually for
             *    vi type of tools ... dbdmrg(1), dbvi(1).
             */
            if (c == FB_ESCAPE && OKSIGPOS(okesc))
               return(FB_ESCAPE_END);

            else if ((c == '\015' || (c == FB_ESCAPE && okesc)) &&
                  cdb_epos >=minc){
               fb_move(row, col + cdb_epos);
               for (i = cdb_epos; i < maxc; i++)
                  fb_s_putw(FB_BLANK);			/* blank out dots */
               if (OKDEFPOS(1) && cdb_epos == 0){
                  if (fmt == FB_DOLLARS){
		     fb_move(row, col + maxc - 3);
                     fb_printw(DECIMAL_ZERO);
		     fb_refresh();
                     }
                  return(FB_DEFAULT);
                  }
               else
                  break;
               }

            /* process printable characters */
            else if ((c >= 040 && c < 0177) || (ic >= 160 && ic <= 255)){
               if (fmt == FB_UPPERCASE && islower(c))
                  c = toupper(c);
               /* if a valid character, insert it into edit buffer */
               if (e_valid(c, fmt, okend) != FB_ERROR){
                  if (fmt == FB_DOLLARS && c != CHAR_DOT &&
                        fb_e_dotcount() == 1){
                     /* make sure pos is within range, else disallow */
                     if (fb_e_cur_past_dot() && fb_e_num_past_dot() >= 2){
                        e_invalid_char();
                        continue;
                        }
                     }
                  if ((cdb_epos <= maxc && fmt != FB_DOLLARS) ||
                         (cdb_epos < maxc && fmt == FB_DOLLARS))
                     fb_e_insert_char(c);
                  }
               /*
                * the following are done if e_valid returns FB_ERROR
                *
                * FB_ENDKEY is kind of special here since - is printable ascii
                * other non-printable signals are now caught below ...
                */
               else if (c == FB_ENDKEY && okend == FB_OKEND &&
                     ((cdb_bpos == 0) ||
                        (cdb_bpos == 1 && cdb_e_buf[0]==FB_ENDKEY))){
                  if (xecho != FB_NOECHO && maxc > 2 && e_empty &&
                        (fmt != FB_NUMERIC || maxc > 3)){
                     fb_printw(END_STRING);
                     fb_refresh();
                     }
                  return(FB_END);
                  }
               else				/* printable, but invalid */
                  e_invalid_char();
               }

            /*
             * special case - ^H allowed for HELP iff
             *    in column 0, no changes, and ^H is a "backward" event
             */
            else if (c == FB_HELPKEY && OKSIGPOS(okhelp) && fb_test_help())
               return(FB_QHELP);

            /*
             * special case - ^E allowed for FB_ESIGNAL iff
             *    in column 0, no changes, and nothing is being edited.
             *    basically, this is for the dbshell fb_help line.
             */
            else if (c == FB_ESIGNAL && OKSIGPOS(okctlsig) &&
                  cdb_e_buf[0] == NULL)
               return(FB_ESIGNAL);

            /*
             * special case - ^D allowed for FB_DSIGNAL iff
             *    in column 0, no changes, and nothing is being edited.
             *    basically, this is for the dbedit autodefault keystroke
             */
            else if (c == FB_DSIGNAL && OKSIGPOS(okdsig) &&
                  cdb_e_buf[0] == NULL)
               return(FB_DSIGNAL);

            /*
             * special case - ^F and ^B allowed for FB_FSIGNAL/FB_BSIGNAL iff
             *    in column 0, no changes, and nothing is being edited.
             *    basically, this is for the dbedit record level mode
             */
            else if ((c == FB_FSIGNAL || c == FB_BSIGNAL) &&
                  OKSIGPOS(okfsig) && cdb_e_buf[0] == NULL)
               return(c);

            /*
             * special case - ^S and ^Q allowed for FB_SSIGNAL/FB_QSIGNAL iff
             *    in column 0, no changes, and nothing is being edited.
             *    basically, this is for the dbedit record level mode
             */
            else if ((c == FB_SSIGNAL || c == FB_QSIGNAL) &&
                  OKSIGPOS(okssig) && cdb_e_buf[0] == NULL)
               return(c);
            else if (c == FB_SSIGNAL1 && OKSIGPOS(okssig) &&
                  cdb_e_buf[0] == NULL)
               return(FB_SSIGNAL);
            /* process function sequences - non printable characters */
            else if ((c >= 00 && c < 040) || (c >= 0177)){
	       kfunction = FB_ERROR;
	       func_buf[0] = NULL;
	       kfunction = read_kseq(c);
               /* if kfunction has been turned into an e_signal interpret */
	       if (kfunction >= E_ACTION_START && kfunction <= E_ACTION_STOP){
                  /*
                   * these are the standard internal cdb functions
                   * which need to be judged whether they are acceptable
                   *
                   * at the bottom of this mess, if kfunction > 0 return it.
                   */
		  kfunction = fb_interpret_esig(kfunction);
                  
                  if (kfunction == FB_REDRAW1){
                     fb_redraw();
                     continue;			/* continue so no error */
                     }
                  else if (kfunction == FB_END){
                     if (okend && equal(cdb_e_buf, o_buf)) /* simp function */
                        ;
                     else if (okend == 0)
                        kfunction = 0;
                     else if (cdb_e_changes > 0){	/* compound function */
                        cdb_e_st = kfunction;
                        c = '\015';
                        fb_move(row, col + cdb_epos);
                        for (i = cdb_epos; i < maxc; i++)
                           fb_s_putw(FB_BLANK);	/* blank out dots */
                        break;			/* get out of loop */
                        }
                     }
                  else if (kfunction == FB_DSIGNAL && OKSIGPOS(okdsig))
                     ;
                  else if (kfunction == FB_ABORT)
                     ;
                  else if (kfunction == FB_QHELP && OKSIGPOS(okhelp))
                     ;
                  else if (kfunction == FB_ESIGNAL || kfunction == FB_YSIGNAL){
                     if (okctlsig && equal(cdb_e_buf, o_buf)) /* simple func */
                        ;
                     else if (okctlsig == 0)
                        kfunction = 0;
                     else if (cdb_e_changes > 0){	/* compound function */
                        cdb_e_st = kfunction;
                        c = '\015';
                        fb_move(row, col + cdb_epos);
                        for (i = cdb_epos; i < maxc; i++)
                           fb_s_putw(FB_BLANK);	/* blank out dots */
                        break;			/* get out of loop */
                        }
                     }
                  else if ((kfunction==FB_FSIGNAL || kfunction==FB_BSIGNAL) &&
                        OKSIGPOS(okfsig))
                     ;
                  else if ((kfunction==FB_SSIGNAL || kfunction==FB_QSIGNAL) &&
                        OKSIGPOS(okssig))
                     ;
                  else if ((kfunction==FB_PAGEUP || kfunction==FB_PAGEDOWN ||
                        kfunction == FB_CSIGNAL || kfunction == FB_WSIGNAL ||
                        kfunction == FB_DELSIGNAL) && OKSIGPOS(1))
                     ;
                  else		/* must be a bad signal */
                     kfunction = 0;
                  if (kfunction != 0){
                     /* if the FB_END keystroke, do the END_STRING thing */
                     if (kfunction == FB_END && xecho != FB_NOECHO &&
                           maxc > 2 && e_empty &&
                           (fmt != FB_NUMERIC || maxc > 3)){
                        fb_printw(END_STRING);
                        fb_refresh();
                        }
		     return(kfunction);
                     }
                  else
                     e_invalid_char();
		  }	 /* end of the standard cdb signals section */

               else if (kfunction >= 040 && kfunction < 0177)
                  c = kfunction;
               else if (kfunction == E_MOVE_CHAR_BACKWARD)
                  fb_e_move_char_backward();
               else if (kfunction == E_MOVE_CHAR_FORWARD)
                  fb_e_move_char_forward();
               else if (kfunction == E_DELETE_CHAR_BACKWARD)
                  fb_e_delete_char_backward();
               else if (kfunction == E_DELETE_CHAR_FORWARD)
                  fb_e_delete_char_forward();
               else if (kfunction == E_MOVE_WORD_BACKWARD)
                  fb_e_move_word_backward();
               else if (kfunction == E_MOVE_WORD_FORWARD)
                  fb_e_move_word_forward();
               else if (kfunction == E_DELETE_WORD_BACKWARD)
                  fb_e_delete_word_backward();
               else if (kfunction == E_DELETE_WORD_FORWARD)
                  fb_e_delete_word_forward();
               else if (kfunction == E_END_OF_LINE)
                  fb_e_end_of_line();
               else if (kfunction == E_BEGINNING_OF_LINE)
                  fb_e_beginning_of_line();
               else if (kfunction == E_DELETE_TO_BEGINNING_OF_LINE)
                  fb_e_delete_to_beginning_of_line();
               else if (kfunction == E_DELETE_TO_END_OF_LINE)
                  fb_e_delete_to_end_of_line();
               else if (kfunction == E_UPCASE_WORD)
                  fb_e_upcase_word();
               else if (kfunction == E_DOWNCASE_WORD)
                  fb_e_downcase_word();
               else if (kfunction == E_CAPITALIZE_WORD)
                  fb_e_capitalize_word();
               else if (kfunction == E_QUIT)
                  fb_bell();
               else if (kfunction == E_SYS_INTERRUPT && cdb_interrupt &&
                     cdb_allow_input_interrupt)
                  fb_onintr(0);
#if HAVE_SETJMP_H
               else if (kfunction == E_SYS_STOP && cdb_okstop)
                  stopjob();
#endif /* HAVE_SETJMP_H */
               else
                  e_invalid_char();
               }
            /* this is another FB_QSIGNAL in case cdb_trapxon is 1 */
            else if (c == FB_VSIGNAL && OKSIGPOS(okssig) &&
                  cdb_e_buf[0] == NULL)
               return(FB_QSIGNAL);

            else
               e_invalid_char();
            }
         switch (fmt) {
            case 'l':			/* CHAR_l FB_LONG */ 
               lp = (long *) addr;
               *lp = atol(cdb_e_buf); 
               break;
            case 'i':			/* CHAR_i FB_INTEGER */
               iptr = (int *) addr;
               *iptr = atoi(cdb_e_buf); 
               break;
            default:
               q = cdb_e_buf + cdb_epos; 
	       if (cdb_decimal && fmt == FB_DOLLARS){ /* zeros added-decimal */
	          cdiff = fb_nodecimal(cdb_e_buf);
		  cdb_epos += cdiff;
		  q += cdiff;
	          }
               format(cdb_e_buf, q, addr, fmt, maxc, cdb_epos);
               if (fmt == FB_DOLLARS){
                  fb_move(row, col);
		  fb_standout();
                  fb_printw(FB_FSTRING, cdb_e_buf);
		  fb_standend();
                  }
               break;
            }
	 if (c == FB_ESCAPE)
	    return(FB_ESCAPE_AOK);
         return(FB_AOK);
      }

/* 
 *  format a string into a buffer fb_padding as needed 
 */
 
   static format(buf, p, addr, fmt, maxc, fcount)
      char *buf, *addr, *p;
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
 
   static e_valid(c, fmt, okend)
      int fmt, c;
      int okend;

      {
         if (isdigit(c)){
            if (fmt == FB_STRICTALPHA)
               return(FB_ERROR);
            }
         else if ((c >= CHAR_A && c <= CHAR_Z) || 
	          (c >= CHAR_a && c <= CHAR_z)){
            if (fmt != FB_ALPHA && fmt != FB_STRICTALPHA &&
                  fmt != FB_UPPERCASE)
               return(FB_ERROR);
            }
	 else if (c == FB_ENDKEY){
	    switch (fmt){
	       /* FB_FORMULA, FB_NUMERIC, FB_DOLLARS */
	       case 'f': case 'n': case '$':
                  /* for these types, pos 0 is the only valid place */
                  if (cdb_bpos == 0)
                     break;
                  return(FB_ERROR);
	       case 'S':	/* FB_SLASH_NUMERIC */
	          if (cdb_bpos == 0 && okend == FB_OKEND)
		     return(FB_ERROR);
		  break;
	       default:
	          if (cdb_bpos == 0 || fmt == FB_STRICTALPHA || 
		        fmt == FB_POS_NUM || fmt == FB_SLASH_NUMERIC)
		     return(FB_ERROR);
		  break;
	       }
	    }
         else if (c == CHAR_SLASH && fmt == FB_SLASH_NUMERIC)
            ;
         else if ((c == CHAR_MINUS || c == CHAR_PLUS) &&
                  (fmt == FB_FLOAT || fmt == FB_NUMERIC || fmt==FB_DOLLARS) &&
		   cdb_bpos == 0)
            ;
	 else if (c == CHAR_DOT && fmt == FB_FLOAT && fb_e_dotcount() == 0)
	    ;
	 else if (c == CHAR_DOT && fmt == FB_DOLLARS && cdb_decimal &&
               fb_e_dotcount() == 0 && cdb_epos - cdb_bpos <= 2)
	    ;
	 else if (c == FB_BLANK &&
               (fmt == FB_POS_NUM || fmt == FB_SLASH_NUMERIC) && cdb_bpos == 0)
	    ;	/* allows blanking of date */
         else if (fmt != FB_ALPHA && fmt != FB_UPPERCASE)
            return(FB_ERROR);
         else if (c >= 0 && c <= 31)
            return(FB_ERROR);
         return(FB_AOK);
      }

/*
 * read_kseq - read in a keyseq or time out trying. c is the first char.
 */

   static read_kseq(c)
      int c;

      {
	 int st = FB_ERROR;
	 char cbuf, *p;

	 p = func_buf;
	 *p++ = c;
         func_len = 1;
	 *p = NULL;
	 for (;;){
	    st = fb_test_keyboard(func_buf, func_len);
            if (st == FB_ERROR){
               func_len = 0;
               break;
               }
            else if (st != FB_AOK)
               break;
            if (read(0, &cbuf, 1) != 1)
	       fb_xerror(FB_READ_ERROR, "Error reading in e_input()", NIL);
            if (cbuf == k_quit_char){
               func_len = 0;
               st = E_QUIT;
               break;
               }
            func_len++;
	    *p++ = cbuf;
	    *p = NULL;
	    }
	 return(st);
      }

   static e_invalid_char()
      {
         fb_bell();
      }

   fb_allow_input_interrupt()
      {
         cdb_allow_input_interrupt = 1;
      }

#if HAVE_SETJMP_H
   static RETSIGTYPE ontstp()		/* set up jump point */

      {
	 longjmp(jmp_env_stop, 1);
      }

   static RETSIGTYPE stopjob()			/* stop this job */
      {
	 TrueMove(cdb_t_lines, 1);
	 fflush(stdout);
	 fb_settty(FB_ENDMODE);
	 kill(getpid(), SIGTSTP);
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

