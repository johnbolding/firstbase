/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getctime.c,v 9.0 2001/01/09 02:56:26 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getctime_sid[] = "@(#) $Id: getctime.c,v 9.0 2001/01/09 02:56:26 john Exp $";
#endif

#include <fb.h>
#include <sys/stat.h>

extern short int cdb_use_rpc;

/*
 * fb_getctime - returns the time_t of the file path
 */

   time_t fb_getctime(path)
      char *path;

      {
	 struct stat sbuf;
         time_t tval = -1;
         int st;

#if RPC
         time_t fb_getctime_clnt(char *path);

         if (cdb_use_rpc)
            return(fb_getctime_clnt(path));
#endif /* RPC */
	 st = stat(path, &sbuf);
         if (st == 0)
            tval = sbuf.st_ctime;
         return(tval);
      }
