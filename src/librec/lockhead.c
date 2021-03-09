/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: lockhead.c,v 9.0 2001/01/09 02:57:00 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Lockhead_sid[] = "@(#) $Id: lockhead.c,v 9.0 2001/01/09 02:57:00 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/*
 * fb_lock_head - lock the firstbase header or die trying
 *    note: these always compile into here.
 */

   fb_lock_head(d)
      fb_database *d;

      {
         int st;

         if ((st = fb_lock(0L, d, FB_WAIT)) == FB_ERROR)
            fb_xerror(FB_ABORT_ERROR, "Lock Header Time Out", d->dbase);
         return(st);
      }

   fb_unlock_head(d)
      fb_database *d;

      {
         int st;

         if ((st = fb_unlock(0L, d)) == FB_ERROR)
            fb_xerror(FB_ABORT_ERROR, "UnLock Header Time Out", d->dbase);
         return(st);
      }
