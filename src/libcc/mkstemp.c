/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mkstemp.c,v 9.3 2001/01/22 23:27:59 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Mkstemp_sid[] = "@(#) $Id: mkstemp.c,v 9.3 2001/01/22 23:27:59 john Exp $";
#endif

#include <fb.h>

/*
 * mkstemp - to gloss over missing mkstemp functions.
 */

   fb_mkstemp(fname)
      char *fname;

      {
         int fd;

         mktemp(fname);
         fd = creat(fname, 0666);
         return(fd);
      }
