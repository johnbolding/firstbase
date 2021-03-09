/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: ffetch.c,v 9.0 2001/01/09 02:56:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Ffetch_sid[] = "@(#) $Id: ffetch.c,v 9.0 2001/01/09 02:56:57 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/* 
 * ffetch - fetch the fb_field described by k from the buffer into s
 *	used only for fixed length record fetches now. (ie, indexes).
 *
 */
 
   fb_ffetch(k, s, buf, dp)
      fb_field *k;
      char *s, *buf;
      fb_database *dp;
   
      {
         register int i, p;

         if (k->type != FB_FORMULA){   
	    p = k->loc;
	    for (i = 0; i < k->size; i++)
	       *s++ = buf[p++];
	    *s = NULL;
	    }
	 else 
	    fb_getformula(k, k->idefault, s, 0, dp);
      }
