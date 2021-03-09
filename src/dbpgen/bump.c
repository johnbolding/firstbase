/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: bump.c,v 9.0 2001/01/09 02:55:42 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Bump_sid[] = "@(#) $Id: bump.c,v 9.0 2001/01/09 02:55:42 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#define MORESIZE 15

/* 
 *  bump - bumps the addressed integer, if > MORESIZE, clears screen 
 */
 
   bump(lc)
      register int *lc;
      
      {
         (*lc) += 1;
	 if (*lc > MORESIZE){
	    (*lc) = 1;
	    FB_PAUSE();
	    fb_move(4, 1); fb_clrtobot(); fb_infoline();
	    }
      }
