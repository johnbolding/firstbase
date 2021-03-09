/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: catch.c,v 9.0 2001/01/09 02:57:05 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Catch_sid[] = "@(#) $Id: catch.c,v 9.0 2001/01/09 02:57:05 john Exp $";
#endif

#include <fb.h>
#include <sys/types.h>
#include <signal.h>

#if FB_PROTOTYPES
static RETSIGTYPE fb_sec_sigill(int disp);
static RETSIGTYPE fb_sec_sigfpe(int disp);
static RETSIGTYPE fb_sec_sigbus(int disp);
static RETSIGTYPE fb_sec_sigsegv(int disp);
static RETSIGTYPE fb_sec_sigsys(int disp);
static RETSIGTYPE fb_sec_sigterm(int disp);
static RETSIGTYPE fb_sec_sighup(int disp);
#else /* FB_PROTOTYPES */
static RETSIGTYPE fb_sec_sigill();
static RETSIGTYPE fb_sec_sigfpe();
static RETSIGTYPE fb_sec_sigbus();
static RETSIGTYPE fb_sec_sigsegv();
static RETSIGTYPE fb_sec_sigsys();
static RETSIGTYPE fb_sec_sigterm();
static RETSIGTYPE fb_sec_sighup();
#endif /* FB_PROTOTYPES */

char FATAL[] = {"Fatal Error."};

   fb_catch_sec()	/* catch all unwanted signals */
      {
	 signal(SIGHUP, fb_sec_sighup);
	 signal(SIGINT, SIG_IGN);
	 signal(SIGQUIT, SIG_IGN);
	 signal(SIGTRAP, SIG_IGN);
	 signal(SIGIOT, SIG_IGN);
#ifdef SIGABRT
	 signal(SIGABRT, SIG_IGN);
#endif /* SIGABRT */
#ifdef SIGEMT
	 signal(SIGEMT, SIG_IGN);
#endif /* SIGEMT */
#ifdef SIGTSTP
	 signal(SIGTSTP, SIG_IGN);
#endif /* SIGTSTP */
	 signal(SIGILL, fb_sec_sigill);
	 signal(SIGFPE, fb_sec_sigfpe);
	 signal(SIGBUS, fb_sec_sigbus);
	 signal(SIGSEGV, fb_sec_sigsegv);
#ifdef SIGSYS
	 signal(SIGSYS, fb_sec_sigsys);
#endif /* SIGSYS */
	 signal(SIGTERM, fb_sec_sigterm);
      }

/*
 * signal interrupt handlers
 */
   static RETSIGTYPE fb_sec_sigill(disp) int disp;
      { (void) disp; fprintf(stderr, FATAL); exit(0); }
   static RETSIGTYPE fb_sec_sigfpe(disp) int disp;
      { (void) disp; fprintf(stderr, FATAL); exit(0); }
   static RETSIGTYPE fb_sec_sigbus(disp) int disp;
      { (void) disp; fprintf(stderr, FATAL); exit(0); }
   static RETSIGTYPE fb_sec_sigsegv(disp) int disp;
      { (void) disp; fprintf(stderr, FATAL); exit(0); }
   static RETSIGTYPE fb_sec_sigsys(disp) int disp;
      { (void) disp; fprintf(stderr, FATAL); exit(0); }
   static RETSIGTYPE fb_sec_sigterm(disp) int disp;
      { (void) disp; exit(0); }
   static RETSIGTYPE fb_sec_sighup(disp) int disp;
      { (void) disp; exit(0); }
