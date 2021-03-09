/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: delrec.c,v 9.1 2001/02/16 19:34:19 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Delrec_sid[] = "@(#) $Id: delrec.c,v 9.1 2001/02/16 19:34:19 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_use_rpc;
extern short int cdb_returnerror;
extern short int cdb_error;
extern short int cdb_secure;

/*
 * delrec - delete the loaded record from the *dp.
 *	- decrement the header and restore.
 *	- NOT responsibility of delrec to do mutual exclusion anywhere.
 */

   fb_delrec(dp)
      fb_database *dp;
      
      {
         int st = FB_AOK;

#if RPC
         if (cdb_use_rpc)
            return(fb_delrec_clnt(dp));
#endif
         if (dp->rec < 1 || dp->rec > dp->reccnt)
            return(FB_ERROR);
	 fb_lock_head(dp);
	 fb_setdirty(dp, 1);
	 fb_allsync(dp);
	 st = fb_gethead(dp);
	 if (st == FB_ERROR){
            cdb_error = FB_READ_ERROR;
            if (cdb_returnerror){
	       fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_HEADER], dp->dbase);
               return(FB_ERROR);
               }
	    fb_xerror(FB_READ_ERROR, SYSMSG[S_BAD_HEADER], dp->dbase);
            }
         if (cdb_secure)
            fb_recmode(dp, CHAR_STAR, -1, -1, NIL);	/* update del only */
         else{
            /* make sure there is enogh room to store the '*' */
            fb_store(dp->kp[dp->nfields], SYSMSG[S_STRING_BLANK], dp);
            dp->kp[dp->nfields]->fld[0] = CHAR_STAR;
            }
	 if (fb_putrec(dp->rec, dp) == FB_ERROR){
            cdb_error = FB_WRITE_ERROR;
	    fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_DATA], dp->dbase);
            return(FB_ERROR);
            }
	 if (fb_puthead(dp->fd, dp->reccnt, ++dp->delcnt) == FB_ERROR){
            cdb_error = FB_WRITE_ERROR;
	    fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_HEADER], dp->dbase);
            return(FB_ERROR);
            }
	 fb_setdirty(dp, 0);
         if (dp->b_tree == 1)
            st = fb_delidx(dp, dp->rec);
	 fb_allsync(dp);
	 fb_unlock_head(dp);
	 return(st);
      }
