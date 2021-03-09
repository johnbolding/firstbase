/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: Free.c,v 9.0 2001/01/09 02:56:22 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Free_sid[] = "@(#) $Id: Free.c,v 9.0 2001/01/09 02:56:22 john Exp $";
#endif

#include <stdio.h>
#include <fb.h>
#include <fb_ext.h>

#undef free

/* 
 * Free - provide free() function with error checking
 */

   fb_free(p)
      register char *p;
      
      {
         if (p != 0 && p != (char *) 0 && p != NIL){
            /*
             * drop a NULL byte on the first memory slot
             * in order to help find bugs
             */
	    *p = 0;
	    free(p);
	    }
      }
