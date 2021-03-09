/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: isdel.c,v 9.0 2001/01/09 02:56:59 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Isdel_sid[] = "@(#) $Id: isdel.c,v 9.0 2001/01/09 02:56:59 john Exp $";
#endif


#include <fb.h>
#include <fb_ext.h>

extern short int cdb_use_rpc;

/*
 * fb_isdeleted - glosses over the FB_ISDELETED() function and
 *	the use of RPC.
 *	returns either 1 if deleted, 0 if not.
 */

   fb_isdeleted(dp)
      fb_database *dp;

      {
         char buf[10];
         int st = 0;

         if (cdb_use_rpc){
            buf[0] = NULL;
            fb_fetch(dp->kp[dp->nfields], buf, dp);
            if (buf[0] == CHAR_STAR)
               st = 1;
            }
         else
            st = FB_ISDELETED(dp);
         return(st);
      }
