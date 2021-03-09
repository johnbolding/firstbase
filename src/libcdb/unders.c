/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: unders.c,v 9.0 2001/01/09 02:56:31 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Unders_sid[] = "@(#) $Id: unders.c,v 9.0 2001/01/09 02:56:31 john Exp $";
#endif

#include <fb.h>

/* 
 *  unders - replace underscores into id, default, and comment 
 */
 
   fb_unders(k)
      register fb_field *k;
      
      {
	 if (k->id != NULL)
	    fb_underscore(k->id, 1);
	 if (k->comment != NULL)
	    fb_underscore(k->comment, 1);
	 if (k->idefault != NULL)
	    fb_underscore(k->idefault, 1);
      }
