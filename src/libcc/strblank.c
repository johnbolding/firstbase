/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: strblank.c,v 9.0 2001/01/09 02:56:21 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Strblank_sid[] = "@(#) $Id: strblank.c,v 9.0 2001/01/09 02:56:21 john Exp $";
#endif

#include <fb.h>

/*
 * fb_str_is_blanks - return a 1 if all blanks (or NULL), else a 0
 */

   fb_str_is_blanks(s)
      char *s;

      {
         for (; *s; s++)
            if (*s != ' ')
               return(0);
         return(1);
      }
