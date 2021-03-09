/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: xcommand.c,v 9.0 2001/01/09 02:56:42 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Xcommand_sid[] = "@(#) $Id: xcommand.c,v 9.0 2001/01/09 02:56:42 john Exp $";
#endif

#include <dbve_ext.h>

static char *PWD = 		"pwd";
static char *MSG = 		"extended command: ";
static char *SET = 		"set";
static char *PUTFILE = 		"PUTFILE";
static char *REGEXP = 		"REGEXP";
static char *VIPAUSE = 		"VIPAUSE";
static char *WRAPSCAN = 	"WRAPSCAN";

extern short int cdb_regexp;
extern short int cdb_vipause;
extern char *cdb_putfile;
extern short int cdb_wrapscan;

/*
 * xcommand - implement extended command set for command level dbvedit
 */
 
   void fb_xcommand()

      {
         char buf[FB_MAXLINE], e_var[FB_MAXLINE], word[FB_MAXLINE];
	 int tst, p, good;
	 
         fb_fmessage(MSG);
	 tst = fb_input(cdb_t_lines, 19, 20, 0, FB_ALPHA, buf, FB_ECHO,
            FB_OKEND, FB_CONFIRM);
	 good = 0;
	 if (tst == FB_AOK){
	    fb_trim(buf);
	    p = fb_getword(buf, 1, word);
	    if (equal(word, PWD)){
	       fb_fmessage(fb_getwd(buf));
	       fb_screrr(NIL);
	       good = 1;
	       }
	     else if (equal(word, SET)){
	       p = fb_getword(buf, p, e_var);
	       word[0] = NULL;
	       p = fb_getword(buf, p, word);
	       good = 1;
	       if (equal(e_var, REGEXP))
		  cdb_regexp = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	       else if (equal(e_var, VIPAUSE))
		  cdb_vipause = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	       else if (equal(e_var, PUTFILE))
		  fb_mkstr(&cdb_putfile, word);
	       else if (equal(e_var, WRAPSCAN))
		  cdb_wrapscan = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	       else
	          good = 0;
	       }
	    if (good == 0)
               fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
	    }
      }
