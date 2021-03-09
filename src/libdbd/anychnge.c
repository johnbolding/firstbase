/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: anychnge.c,v 9.1 2001/02/16 18:57:06 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Anychange_sid[] = "@(#) $Id: anychnge.c,v 9.1 2001/02/16 18:57:06 john Exp $";
#endif

#include <fb.h>
#include <dbd.h>
#include <fb_ext.h>

/* 
 *  anychange - set up anychange screen 
 */
 
   fb_anychange(inp)
      char inp[];

      {
         register int st;
         
         fb_fmessage("Any Change (#, -=End, <CTL>-H=HELP) ?");
         st = fb_input(-cdb_t_lines, 39, 4, 0, FB_ALPHA, inp, FB_ECHO,
            FB_OKEND, FB_CONFIRM);
	 				/* neg row allows FB_QHELP signal */
         if (st == FB_ABORT || st == FB_END || st == FB_QHELP)
            return(st);
         else if (st == FB_DEFAULT)
            return(PAGEF);
	 switch(inp[0]){
	    case PAGEF:
	    case PAGEB:
	    case ADDMODE:
	    case DELMODE:
	    case INSERTMODE:
	    case HELPMODE:
	    case MINFOMODE:
	    case TRACEMODE:
	    case SORTBYMODE:
	       return(inp[0]);
	    default:
               return(st);
	    }
      }
