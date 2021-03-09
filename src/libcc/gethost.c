/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: gethost.c,v 9.0 2001/01/09 02:56:19 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Gethost_sid[] = "@(#) $Id: gethost.c,v 9.0 2001/01/09 02:56:19 john Exp $";
#endif

#include <fb.h>

/*
 * fb_gethostname - return the host, for all machines
 */

   void fb_gethostname(name, namelen)
      char *name;
      int namelen;

      {
#if GETHOSTNAME
         char *p;

         gethostname(name, namelen);
         for (p = name; *p && *p != '.'; p++)
            ;
         if (*p == '.')
            *p = 0;
#endif /* GETHOSTNAME */

#if !GETHOSTNAME
         int n, fd;

         name[0] = 0;
         fd = open("/etc/hostname", 0);
         if (fd < 0)
            return;
         if ((n = read(fd, name, namelen)) < 0)
            return;
         if (name[n - 1] == '\n')
            name[n - 1] = 0;
         close(fd);
#endif /* NOT GETHOSTNAME */
      }
