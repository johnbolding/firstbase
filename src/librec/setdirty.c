/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: setdirty.c,v 9.0 2001/01/09 02:57:02 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Setdirty_sid[] = "@(#) $Id: setdirty.c,v 9.0 2001/01/09 02:57:02 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/*
 *  setdirty - store dirty bit into slot
 *	new format 4-13 == rec_count, 14 = dirty bit, 15-23 = delcnt
 */
 
   fb_setdirty(dp, n)
      fb_database *dp;
      int n;
   
      {
         char buf[1];
	 
	 if (lseek(dp->fd, FB_HEADSTART + FB_HEADELEM, 0) < 0L){
	    fb_xerror(FB_SEEK_ERROR, SYSMSG[S_BAD_INDEX], dp->dbase);
	    return(FB_ERROR);
	    }
	 if (n == 0)
	    dp->dirty = buf[0] = CHAR_0;
	 else
	    dp->dirty = buf[0] = CHAR_1;
	 if (write(dp->fd, buf, 1) != 1){
	    fb_xerror(FB_WRITE_ERROR, SYSMSG[S_BAD_INDEX], dp->dbase);
	    return(FB_ERROR);
	    }
	 return(FB_AOK);
      }
