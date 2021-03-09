/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: bulkrec.c,v 9.1 2001/02/16 19:34:18 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Bulkrec_sid[] = "@(#) $Id: bulkrec.c,v 9.1 2001/02/16 19:34:18 john Exp $";
#endif

/*
 * these routines enable the use of batch record processing
 */

#include <fb.h>
#include <fb_ext.h>

static int save_locklevel = -1;
extern short int cdb_use_rpc;
extern short int cdb_returnerror;
extern short int cdb_error;
extern short int cdb_locklevel;
extern short int cdb_secure;

/*
 * fb_bulkrec_begin - begin part for any bulk record processes
 */

   fb_bulkrec_begin(dp, fwait)
      fb_database *dp;
      int fwait;

      {
         int st = FB_AOK;

#if RPC
         if (cdb_use_rpc)
            return(fb_bulkrec_begin_clnt(dp, fwait));
#endif /* RPC */
         cdb_error = 0;
         st = fb_lock(0L, dp, fwait);
         if (st == FB_ERROR){
            if (fwait == FB_NOWAIT)
               cdb_error = FB_LOCKED_ERROR;
            else
               cdb_error = FB_ERROR;
            return(FB_ERROR);
            }
	 fb_setdirty(dp, 1);
	 fb_allsync(dp);
         st = fb_gethead(dp);
	 if (st == FB_ERROR){
            cdb_error = FB_READ_ERROR;
	    fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_HEADER], dp->dbase);
            return(FB_ERROR);
            }
         /*
          * this noise is due to fb_getrec() doing a lock on 0
          *    if the locklevel is 2 ... would cause deadlock here.
          */
         if (save_locklevel == -1)
            save_locklevel = cdb_locklevel;
         cdb_locklevel = 1;
         dp->inside_bulkrec = 1;
         return(st);
      }

/*
 * fb_bulkrec_end - end for any bulkrec processes
 */

   fb_bulkrec_end(dp)
      fb_database *dp;

      {
#if RPC
         if (cdb_use_rpc)
            return(fb_bulkrec_end_clnt(dp));
#endif /* RPC */
	 if (fb_puthead(dp->fd, dp->reccnt, dp->delcnt) == FB_ERROR){
            cdb_error = FB_WRITE_ERROR;
	    fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_HEADER], dp->dbase);
            return(FB_ERROR);
            }
	 fb_setdirty(dp, 0);
	 fb_allsync(dp);
	 fb_unlock_head(dp);
         cdb_locklevel = save_locklevel;
         dp->inside_bulkrec = 0;
         return(FB_AOK);
      }

/* 
 * b_addrec - bulk addrec - no locking or syncs done here.
 *	this is the same as fb_addrec() but no locking.
 */

   fb_b_addrec(dp)
      fb_database *dp;
   
      {
#if RPC
         if (cdb_use_rpc)
            return(fb_b_addrec_clnt(dp));
#endif /* RPC */
         if (dp->inside_bulkrec == 0)
            return(FB_ERROR);
         /* store a blank so that delrec works with no length thrashing */
         if (cdb_secure)
	    fb_recmode(dp, FB_BLANK, fb_getuid(), fb_getgid(), "666");
         else
            fb_store(dp->kp[dp->nfields], SYSMSG[S_STRING_BLANK], dp);
         /*
          * this is a new record; it cannot have previous auto values. clear.
          */
         fb_clear_autoindex(dp);
         fb_checkauto(dp);
         dp->reccnt++;
	 if (fb_putrec(dp->reccnt, dp) == FB_ERROR){
            cdb_error = FB_WRITE_ERROR;
	    fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_DATA], dp->dbase);
            return(FB_ERROR);
            }
         dp->rec = dp->reccnt;
	 if (fb_put_autoindex(dp) == FB_ERROR){
            cdb_error = FB_WRITE_ERROR;
	    fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_INDEX], dp->dbase);
            return(FB_ERROR);
            }
	 return(FB_AOK);
      }

/* 
 * b_delrec - bulk delrec - no locking or syncs done here.
 *	this is the same as fb_delrec() but no locking.
 */

   fb_b_delrec(dp)
      fb_database *dp;
      
      {
         int st = FB_AOK;

#if RPC
         if (cdb_use_rpc)
            return(fb_b_delrec_clnt(dp));
#endif /* RPC */
         if (dp->inside_bulkrec == 0)
            return(FB_ERROR);
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
         dp->delcnt++;
         if (dp->b_tree == 1 || dp->b_autoindex != NULL)
            st = fb_delidx(dp, dp->rec);
	 return(st);
      }
