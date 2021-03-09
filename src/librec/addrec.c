/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: addrec.c,v 9.1 2001/02/16 19:34:18 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Addrec_sid[] = "@(#) $Id: addrec.c,v 9.1 2001/02/16 19:34:18 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_use_rpc;
extern short int cdb_secure;
extern short int cdb_returnerror;
extern short int cdb_error;
extern short int cdb_usrlog;

/* 
 * addrec - does read of header, and adds the record in dp to
 *	- the end of the dbase. bumps header, and does autoindexes.
 *	- also does AUTOINCR if AUTOMARK is in place.
 *	- does not maintain uniqueness within a field.
 */

   fb_addrec(dp)
      fb_database *dp;
   
      {
         int st;

#if RPC
         if (cdb_use_rpc)
            return(fb_addrec_clnt(dp));
#endif
         /* store a blank so that delrec works with no length thrashing */
         if (cdb_secure)
	    fb_recmode(dp, FB_BLANK, fb_getuid(), fb_getgid(), "666");
         else
            fb_store(dp->kp[dp->nfields], SYSMSG[S_STRING_BLANK], dp);
         /*
          * this is a new record; it cannot have previous auto values. clear.
          */
         fb_clear_autoindex(dp);
	 fb_lock_head(dp);
         if (cdb_usrlog > 10)
            fb_usrlog_msg("CS-begin (addrec)");
	 fb_allsync(dp);
	 fb_setdirty(dp, 1);
	 st = fb_gethead(dp);
         fb_checkauto(dp);
	 if (st == FB_ERROR){
            cdb_error = FB_READ_ERROR;
            if (cdb_returnerror){
	       fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_HEADER], dp->dbase);
               return(FB_ERROR);
               }
	    fb_xerror(FB_READ_ERROR, SYSMSG[S_BAD_HEADER], dp->dbase);
            }
	 if (fb_putrec(++dp->reccnt, dp) == FB_ERROR){
            cdb_error = FB_WRITE_ERROR;
	    fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_DATA], dp->dbase);
            return(FB_ERROR);
            }
         dp->rec = dp->reccnt;
	 if (fb_puthead(dp->fd, dp->reccnt, dp->delcnt) == FB_ERROR){
            cdb_error = FB_WRITE_ERROR;
	    fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_HEADER], dp->dbase);
            return(FB_ERROR);
            }
	 fb_setdirty(dp, 0);
	 if (fb_put_autoindex(dp) == FB_ERROR){
            cdb_error = FB_WRITE_ERROR;
	    fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_INDEX], dp->dbase);
            return(FB_ERROR);
            }
	 fb_allsync(dp);
         if (cdb_usrlog > 10)
            fb_usrlog_msg("CS-end (addrec)");
	 fb_unlock_head(dp);
	 return(FB_AOK);
      }
