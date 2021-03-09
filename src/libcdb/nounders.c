/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: nounders.c,v 9.0 2001/01/09 02:56:28 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Nounders_sid[] = "@(#) $Id: nounders.c,v 9.0 2001/01/09 02:56:28 john Exp $";
#endif

#include <fb.h>

/* 
 *  nounders - remove underscores from id, default, and comment 
 */
 
   fb_nounders(k)
      register fb_field *k;
      
      {
	 if (k->id != NULL)
	    fb_underscore(k->id, 0);
	 if (k->comment != NULL)
	    fb_underscore(k->comment, 0);
	 if (k->idefault != NULL)
	    fb_underscore(k->idefault, 0);
      }
