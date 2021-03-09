/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mk_dbase.c,v 9.0 2001/01/09 02:56:49 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Make_dbase_sid[] = "@(#) $Id: mk_dbase.c,v 9.0 2001/01/09 02:56:49 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_secure;

/* 
 *  make_dbase - create a fb_database file
 */
 
   fb_make_dbase(dhp)
      fb_database *dhp;
      
      {
         char fname[FB_MAXNAME];
	 
         close(creat(dhp->dbase, 0666));	/* create dbase */
         dhp->fd = fb_mustopen(dhp->dbase, READWRITE);
         close(creat(dhp->dmap, 0666));		/* ditto for dmap */
         dhp->mfd = fb_mustopen(dhp->dmap, READWRITE);
         dhp->reccnt = dhp->delcnt = 0L;
         dhp->sequence = fb_putseq(dhp->fd);
	 if (dhp->sequence == FB_ERROR)
	    fb_xerror(FB_IO_ERROR, dhp->dbase, NIL);
         if (fb_puthead(dhp->fd, 0L, 0L) == FB_ERROR)
            fb_xerror(FB_IO_ERROR, dhp->dbase, NIL);
         if (cdb_secure)
            if (fb_putmode(dhp->fd, fb_getuid(), fb_getgid(),
                  "666") == FB_ERROR)
               fb_xerror(FB_IO_ERROR, dhp->dbase, NIL);
	 if (fb_bootmap(dhp->mfd) == FB_ERROR)
	    fb_xerror(FB_IO_ERROR, dhp->dmap, NIL);
	 fb_basename(fname, dhp->dbase);
      }
