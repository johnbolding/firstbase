/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: undelete.c,v 9.0 2001/01/09 02:56:41 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Undelete_sid[] = "@(#) $Id: undelete.c,v 9.0 2001/01/09 02:56:41 john Exp $";
#endif

#include <dbve_ext.h>

/* 
 *  undelete - undelete a record 
 */
 
   int fb_undelete(k)
      fb_field *k;
      
      {
         k->fld[0] = FB_BLANK;
         fb_serror(FB_MESSAGE, SYSMSG[S_RECORD], SYSMSG[S_RESTORED]);
         return(FB_UNDELETED);
      }
