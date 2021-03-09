/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: onpage.c,v 9.0 2001/01/09 02:56:38 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Onpage_sid[] = "@(#) $Id: onpage.c,v 9.0 2001/01/09 02:56:38 john Exp $";
#endif

#include <fb.h>

/* 
 *  onpage - determine what page f is on -- return its 'dot' 
 */
 
   fb_onpage(f)
      register int f;
      
      {
      switch(f % 10){
         case 0:  f = f - 9; break;
         default: f = f - ((f % 10) - 1);
         }
      if (f < 1)
         f = 1;
      return(f);
      }
