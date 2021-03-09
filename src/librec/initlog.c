/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: initlog.c,v 9.0 2001/01/09 02:56:59 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Initlog_sid[] = "@(#) $Id: initlog.c,v 9.0 2001/01/09 02:56:59 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_reclog;

/*
 * initlog - initialize the log (create) or at least open.
 */

   void fb_initlog(hp)
      fb_database *hp;

      {
         if (!cdb_reclog)
	    return;
         if (access(hp->dlog, 0) != 0)
            close(creat(hp->dlog, 0666));	/* create log */
         hp->logfd = fb_mustopen(hp->dlog, READWRITE);
      }
