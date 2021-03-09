/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: setecho.c,v 9.1 2001/01/12 22:52:02 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Setecho_sid[] = "@(#) $Id: setecho.c,v 9.1 2001/01/12 22:52:02 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#if HAVE_TERMIOS_H
#include <sys/termios.h>
#include <sys/ioctl.h>
static void io_tcgeta(struct termios *pterm);
static void io_tcseta(struct termios *pterm);
#else
#include <termio.h>
static void io_tcgeta(struct termio *pterm);
static void io_tcseta(struct termio *pterm);
#endif /* HAVE_TERMIOS_H */

#include <sys/types.h>
#include <signal.h>

static char *MSG1 =    "bad tty";

extern short int cdb_debugmode;
extern short int cdb_trap_xon;
extern short int cdb_okstop;

static int settty_editmode = 0;

/* 
 *  fb_setecho - if m = 1 condition tty for FirstBase, else uncondition.
 */
 
   fb_setecho(m)
      int m;

      {
#if HAVE_TERMIOS_H
         struct termios term;
#else
         struct termio term;
#endif /* HAVE_TERMIOS_H */


         if (m){			/* set FB_EDITMODE and signals */
            if (isatty(1)){
               io_tcgeta(&term);
               term.c_lflag &= ~ECHO;
               io_tcseta(&term);
               }
            }
         else{					/* unset editmode, restor sig*/
            /* now clean up the terminal if applicable */
            if (isatty(1)){
               io_tcgeta(&term);
               term.c_lflag |= ECHO;
               io_tcseta(&term);
               }
            }
      }

#if HAVE_TERMIOS_H
   static void io_tcgeta(pterm)
         struct termios * pterm;
      
      {
         int st;

#ifdef TCGETS
         st = ioctl(1, TCGETS, pterm);
#endif
#ifdef TIOCGETA
         st = ioctl(1, TIOCGETA, pterm);
#endif
#ifdef TCGETA
         st = ioctl(1, TCGETA, pterm);
#endif
         if (st == -1)
            fb_xerror(FB_BAD_TTY, MSG1, NIL);
      }

   static void io_tcseta(pterm)
         struct termios * pterm;
      
      {
         int st;
         
#ifdef TCSETS
         st = ioctl(1, TCSETS, pterm);
#endif
#ifdef TIOCSETA
         st = ioctl(1, TIOCSETA, pterm);
#endif
#ifdef TCSETA
         st = ioctl(1, TCSETA, pterm);
#endif
         if (st == -1)
            fb_xerror(FB_BAD_TTY, MSG1, NIL);
      }

#else /* if ! HAVE_TERMIOS_H */
   static void io_tcgeta(pterm)
         struct termio * pterm;
      
      {
         if (ioctl(1, TCGETA, pterm) == -1)
            fb_xerror(FB_BAD_TTY, MSG1, NIL);
      }

   static void io_tcseta(pterm)
         struct termio * pterm;
      
      {
         if (ioctl(1, TCSETA, pterm) == -1)
            fb_xerror(FB_BAD_TTY, MSG1, NIL);
      }
#endif /* HAVE_TERMIOS_H */
