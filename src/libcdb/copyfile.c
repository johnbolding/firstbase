/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: copyfile.c,v 9.1 2001/02/16 19:09:07 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "%W% %G% FB";
#endif

#include <fb.h>

/*
 * copyfile - copy a file from fname to tname
 */

   fb_copyfile(fname, tname)
      char *fname, *tname;

      {
         char buf[1024];
         int ffd, tfd, nchars;

         ffd = open(fname, READ);
         if (ffd < 0)
            return(-1);
	 close(creat(tname, 0666));
         tfd = open(tname, WRITE);
         if (tfd < 0)
            return(-1);
         for (;;){
            nchars = read(ffd, buf, 1024);
            if (nchars <= 0)
               break;
            write(tfd, buf, (unsigned) nchars);
            }
         close(ffd);
         fb_sync_fd(tfd);
         close(tfd);
         return(0);
      }
