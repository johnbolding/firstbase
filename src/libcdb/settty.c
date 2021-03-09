/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: settty.c,v 9.3 2001/01/22 17:36:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Settty_sid[] = "@(#) $Id: settty.c,v 9.3 2001/01/22 17:36:57 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#if HAVE_TERMIOS_H
#include <sys/termios.h>
#include <sys/ioctl.h>
void fb_io_tcgeta(struct termios *pterm);
void fb_io_tcseta(struct termios *pterm);
#else
#include <termio.h>
void fb_io_tcgeta(struct termio *pterm);
void fb_io_tcseta(struct termio *pterm);
#endif /* HAVE_TERMIOS_H */

#include <sys/types.h>
#include <signal.h>

static char *MSG1 =    "bad tty";
#if TEST_SUN_CMD
static char *S_TERM = "TERM";
#endif /* TEST_SUN_CMD */
short int cdb_allow_interrupt = 0;
static int save_cc;

extern short int cdb_debugmode;
extern short int cdb_trap_xon;
extern short int cdb_okstop;
extern RETSIGTYPE fb_onintr(int disp);

static int settty_editmode = 0;

/* 
 *  settty - if m = 1 condition tty for FirstBase, else uncondition.
 */
 
   fb_settty(m)
      int m;

      {
#if HAVE_TERMIOS_H
         struct termios term;
#else
         struct termio term;
#endif /* HAVE_TERMIOS_H */

#if TEST_SUN_CMD
         char *p;
#endif /* TEST_SUN_CMD */

         if (m){			/* set FB_EDITMODE and signals */
            /* capture all the signals first */
            fb_catch_signal();
            if (!cdb_batchmode && isatty(1)){
               settty_editmode = 1;
               fb_io_tcgeta(&term);
               term.c_lflag &= ~ICANON;
               term.c_lflag &= ~ISIG;
               term.c_lflag &= ~ECHO;
               if (cdb_trap_xon){
                  term.c_iflag &= ~IXOFF;
                  term.c_iflag &= ~IXON;
                  }
               term.c_iflag &= ~ICRNL;
               save_cc = term.c_cc[4];
               term.c_cc[4] = '\001';
               fb_io_tcseta(&term);
#if TEST_SUN_CMD
               p = getenv(S_TERM);
               if (!cdb_debugmode && p != NULL && equal(p, "sun-cmd")){
                  printf("\033[>4l");		/* turn ScrollBar off */
                  fflush(stdout);
                  }
#endif /* TEST_SUN_CMD */
               }
            }
         else{					/* unset editmode, restor sig*/
            if (settty_editmode){
               /* clean up (release) the signals first */
               fb_release_signal();

               /* now clean up the terminal if applicable */
               if (!cdb_batchmode && isatty(1)){
                  fb_io_tcgeta(&term);
                  term.c_lflag |= ICANON;
                  term.c_lflag |= ISIG;
                  term.c_lflag |= ECHO;
                  if (cdb_trap_xon){
                     term.c_iflag |= IXOFF;
                     term.c_iflag |= IXON;
                     }
                  term.c_iflag |= ICRNL;
                  term.c_cc[4] = save_cc;
                  /*term.c_cc[4] = '\004';*/
                  fb_io_tcseta(&term);
#if TEST_SUN_CMD
                  p = getenv(S_TERM);
                  if (!cdb_debugmode && p != NULL && equal(p, "sun-cmd")){
                     fflush(stdout);
                     sleep(1);
                     printf("\033[>4h");	/* turn Scroll Bar on */
                     fflush(stdout);
                     }
#endif /* TEST_SUN_CMD */
                  }
               }
            }
      }

/*
 * allow_int - allow interrupt signal to go through
 */

   void fb_allow_int()
      {
#if HAVE_TERMIOS_H
         struct termios term;
#else
         struct termio term;
#endif /* HAVE_TERMIOS_H */

         if (!cdb_batchmode && isatty(1)){
            fb_io_tcgeta(&term);
            term.c_lflag |= ISIG;
            fb_io_tcseta(&term);
            }
         signal(SIGINT, fb_onintr);
      }

/*
 * catch_int - catch the interrupt signal - disallow interrupt signal
 */

   fb_catch_int()
      {
#if HAVE_TERMIOS_H
         struct termios term;
#else
         struct termio term;
#endif /* HAVE_TERMIOS_H */

         if (!cdb_batchmode && isatty(1)){
            fb_io_tcgeta(&term);
            term.c_lflag &= ~ISIG;
            fb_io_tcseta(&term);
            }
         signal(SIGINT, SIG_IGN);
      }

/*
 * fb_ignore_fpe - used on some motherboard/machines that emit FPE's
 *	when casting a float to an int.
 */

   void fb_ignore_fpe()
      {
         signal(SIGFPE, SIG_IGN);
      }

/*
 * fb_catch_signal - catch important signals
 */

   fb_catch_signal()
      {
	 RETSIGTYPE fb_sigill(int d), fb_sigbus(int d), fb_sigsegv(int d);
         RETSIGTYPE fb_sigfpe(int d);
         RETSIGTYPE fb_sigsys(int d), fb_sigterm(int d), fb_sighup(int d);
#ifdef SIGLOST
         RETSIGTYPE fb_siglost(int d);
#endif

         signal(SIGHUP, fb_sighup);
         signal(SIGINT, SIG_IGN);
         signal(SIGQUIT, SIG_IGN);
         signal(SIGTERM, SIG_IGN);
         signal(SIGTRAP, SIG_IGN);
         signal(SIGIOT, SIG_IGN);
#ifdef SIGTSTP
         signal(SIGTSTP, SIG_IGN);
#endif
#ifdef SIGABRT
         signal(SIGABRT, SIG_IGN);
#endif
#ifdef SIGEMT
         signal(SIGEMT, SIG_IGN);
#endif
         signal(SIGILL, fb_sigill);
         signal(SIGFPE, fb_sigfpe);
         signal(SIGBUS, fb_sigbus);
         signal(SIGSEGV, fb_sigsegv);
#ifdef SIGSYS
         signal(SIGSYS, fb_sigsys);
#endif
         signal(SIGTERM, fb_sigterm);
#ifdef SIGLOST
         signal(SIGLOST, fb_siglost);
#endif
      }

/*
 * fb_release_signal - release important signals
 */

   fb_release_signal(void)
      {
#if HAVE_SETJMP_H
         if (cdb_okstop)
            signal(SIGTSTP, SIG_DFL);
#endif /* HAVE_SETJMP_H */
         signal(SIGHUP, SIG_DFL);
         signal(SIGINT, SIG_DFL);
         signal(SIGQUIT, SIG_DFL);
         signal(SIGTRAP, SIG_DFL);
         signal(SIGIOT, SIG_DFL);
         signal(SIGINT, SIG_DFL);
         signal(SIGALRM, SIG_DFL);
#ifdef SIGABRT
         signal(SIGABRT, SIG_DFL);
#endif
#ifdef SIGEMT
         signal(SIGEMT, SIG_DFL);
#endif
         signal(SIGILL, SIG_DFL);
         signal(SIGFPE, SIG_DFL);
         signal(SIGBUS, SIG_DFL);
         signal(SIGSEGV, SIG_DFL);
#ifdef SIGSYS
         signal(SIGSYS, SIG_DFL);
#endif
         signal(SIGTERM, SIG_DFL);
#ifdef SIGLOST
         signal(SIGLOST, SIG_DFL);
#endif
      }

#if HAVE_TERMIOS_H
   void fb_io_tcgeta(pterm)
         struct termios * pterm;
      
      {
         int st;

#ifdef TCGETS
         st = ioctl(1, TCGETS, pterm);
#else
# ifdef TIOCGETA
         st = ioctl(1, TIOCGETA, pterm);
# else
#  ifdef TCGETA
         st = ioctl(1, TCGETA, pterm);
#  endif
# endif
#endif
         if (st == -1)
            fb_xerror(FB_BAD_TTY, MSG1, NIL);
      }

   void fb_io_tcseta(pterm)
         struct termios * pterm;
      
      {
         int st;
         
#ifdef TCSETS
         st = ioctl(1, TCSETS, pterm);
#else
# ifdef TIOCSETA
         st = ioctl(1, TIOCSETA, pterm);
# else
#  ifdef TCSETA
         st = ioctl(1, TCSETA, pterm);
#  endif
# endif
#endif
         if (st == -1)
            fb_xerror(FB_BAD_TTY, MSG1, NIL);
      }

#else /* if ! HAVE_TERMIOS_H */
   void fb_io_tcgeta(pterm)
         struct termio * pterm;
      
      {
         if (ioctl(1, TCGETA, pterm) == -1)
            fb_xerror(FB_BAD_TTY, MSG1, NIL);
      }

   void fb_io_tcseta(pterm)
         struct termio * pterm;
      
      {
         if (ioctl(1, TCSETA, pterm) == -1)
            fb_xerror(FB_BAD_TTY, MSG1, NIL);
      }
#endif /* HAVE_TERMIOS_H */
