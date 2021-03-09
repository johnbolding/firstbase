/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: countlin.c,v 9.0 2001/01/09 02:56:19 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Countlines_sid[] = "@(#) $Id: countlin.c,v 9.0 2001/01/09 02:56:19 john Exp $";
#endif

#include <fb.h>

/*
 * countlines - count the number of attr seperated lines in buf. attr != NULL.
 */

   fb_countlines(s, attr)
      char *s;
      int attr;

      {
         char *p;
         int count = 0;

         for (p = s; *p; p++)
            if (*p == attr)
               count++;
         return(count);
      }
