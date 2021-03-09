/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: gethead.c,v 9.0 2001/01/09 02:56:58 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Gethead_sid[] = "@(#) $Id: gethead.c,v 9.0 2001/01/09 02:56:58 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_use_rpc;

/* 
 *  get header record (record 4: 23) from fd.
 *	new format 4-13 == rec_count, 14 = dirty bit, 15-23 = delcnt
 */

   fb_gethead(dp)
      fb_database *dp;
   
      {
         char rc[11], dc[11], buf[25];
         int rlen = FB_HEADELEM + FB_HEADELEM;
	 register int i;
	 
#if RPC
         if (cdb_use_rpc)
            return(fb_gethead_clnt(dp));
#endif /* RPC */
	 if (lseek(dp->fd, FB_HEADSTART, 0) < 0L){
	    fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_INDEX], dp->dbase);
	    return(FB_ERROR);
	    }
	 if (read(dp->fd, buf, (unsigned) rlen) != rlen){
	    fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_INDEX], dp->dbase);
	    return(FB_ERROR);
	    }
         memcpy(rc, buf, FB_HEADELEM);
         memcpy(dc, buf + FB_HEADELEM, FB_HEADELEM);
         rc[FB_HEADELEM] = NULL;
         dc[FB_HEADELEM] = NULL;
	 for (i = 0; i < 10; i++)
	    if ((!(isdigit(rc[i])) && rc[i] != CHAR_BLANK) ||
	        (!isdigit(dc[i]) && dc[i] != CHAR_BLANK))
	       return(FB_ERROR);
	 dp->reccnt = atol(rc);
	 if (dp->dirty == CHAR_0)
	    dp->dirty = dc[0];
	 dp->delcnt = atol(dc + 1);
	 return(FB_AOK);
      }
