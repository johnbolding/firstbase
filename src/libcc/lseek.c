/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: lseek.c,v 9.0 2001/01/09 02:56:20 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char lseek_sid[] = "@(#) $Id: lseek.c,v 9.0 2001/01/09 02:56:20 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#if LONG_LONG_LSEEK

#undef lseek

   long LSEEK(fd, pos, whence)
      int fd, whence;
      long pos;

      {
         long result;
         if ((result = lseek(fd, (long long) pos, whence)) < 0)
            fb_xerror(FB_SEEK_ERROR, SYSMSG[S_BAD_DATA], NIL);
         return(result);
      }
#endif
