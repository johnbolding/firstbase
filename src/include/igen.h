/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: igen.h,v 9.0 2001/01/09 02:56:12 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

struct self {				/* index tree entries */
   fb_field *fp, *cfp;
   char lword[FB_MAXNAME], rword[FB_MAXNAME];
   char *patcomp;
   struct self *orp, *andp, *sandp;
   };
