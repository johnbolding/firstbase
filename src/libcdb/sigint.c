/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: sigint.c,v 9.0 2001/01/09 02:56:30 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "%W% %G% FB";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <sys/types.h>
#include <signal.h>

/*
 * signal interrupt handlers
 */

   RETSIGTYPE fb_sigill(d) int d;	{ fb_xerror(SIGILL, NIL, NIL); }
   RETSIGTYPE fb_sigfpe(d) int d;	{ fb_xerror(SIGFPE, NIL, NIL); }
   RETSIGTYPE fb_sigbus(d) int d;	{ fb_xerror(SIGBUS, NIL, NIL); }
   RETSIGTYPE fb_sigsegv(d) int d;	{ fb_xerror(SIGSEGV, NIL, NIL); }
#ifdef SIGSYS
   RETSIGTYPE fb_sigsys(d) int d;	{ fb_xerror(SIGSYS, NIL, NIL); }
#endif
   RETSIGTYPE fb_sigterm(d) int d;	{ fb_lxerror(SIGTERM, NIL, NIL); }
   RETSIGTYPE fb_sighup(d) int d;	{ fb_lxerror(SIGHUP, NIL, NIL); }
#ifdef SIGLOST
   RETSIGTYPE fb_siglost(d) int d;	{ fb_xerror(SIGLOST, NIL, NIL); }
#endif
