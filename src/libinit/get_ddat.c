/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: get_ddat.c,v 9.0 2001/01/09 02:56:46 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getd_data_sid[] = "@(#) $Id: get_ddat.c,v 9.0 2001/01/09 02:56:46 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char *DIRTY_HELP = "dirty.hlp";
extern short clr_aft_fhelp;	/* kludge around screen clear -- hmmm */

#if FB_PROTOTYPES
static testmap(fb_database *dhp);
#else
static testmap();
#endif /* FB_PROTOTYPES */

short cdb_allow_dirty = 0;
extern short int cdb_returnerror;
extern short int cdb_error;
extern short int cdb_dbase_byteorder;
extern short int cdb_cpu_byteorder;

/* 
 *  getd_data - attempt to open and initialize a fb_database file in mode.
 */

   fb_getd_data(mode, dhp)
      int mode;
      fb_database *dhp;
      
      {
         int st;

         if ((dhp->fd = open(dhp->dbase, mode)) < 0){
#ifdef FB_INIT_DEBUG
            if (!cdb_returnerror)
	       fb_serror(FB_CANT_OPEN, dhp->dbase, NIL);
#endif /* FB_INIT_DEBUG */
            cdb_error = FB_CANT_OPEN;
            return(FB_ERROR);
	    }
	 dhp->sequence = fb_getseq(dhp->fd);
         if ((dhp->mfd = open(dhp->dmap, READWRITE)) < 0){
#ifdef FB_INIT_DEBUG
            if (!cdb_returnerror)
	       fb_serror(FB_CANT_OPEN, dhp->dmap, NIL);
#endif /* FB_INIT_DEBUG */
            cdb_error = FB_CANT_OPEN;
            return(FB_ERROR);
	    }
	 st = fb_lock(0L, dhp, FB_WAIT);
         if (st == FB_ERROR)
            return(FB_LOCKED_ERROR);
	 st = fb_gethead(dhp);
	 if (fb_unlock(0L, dhp) == FB_ERROR){
            st = FB_ERROR;
#ifdef FB_INIT_DEBUG
            fb_serror(FB_CANT_OPEN, dhp->dmap, "Can't UNLOCK?");
#endif /* FB_INIT_DEBUG */
            }
	 if (st == FB_ERROR){
            if (cdb_returnerror){
	       fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_HEADER], dhp->dbase);
               return(FB_ERROR);
               }
            else
	       fb_xerror(FB_READ_ERROR, SYSMSG[S_BAD_HEADER], dhp->dbase);
            }
	 st = testmap(dhp);
	 if (st == FB_ERROR){
            if (cdb_returnerror){
	       fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_MAP], dhp->dbase);
               return(FB_ERROR);
               }
            else
	       fb_xerror(FB_READ_ERROR, SYSMSG[S_BAD_MAP], dhp->dbase);
            }
	 if (dhp->dirty == CHAR_1 && !cdb_allow_dirty){
            if (!cdb_batchmode){
	       fb_scrhdr(dhp, NIL);
               clr_aft_fhelp = 0;
	       fb_fhelp(DIRTY_HELP);
               }
            cdb_error = FB_DIRTY_DBASE;
            if (cdb_returnerror){
	       fb_lerror(FB_DIRTY_DBASE, dhp->dbase, NIL);
               return(FB_DIRTY_DBASE);
               }
            else
	       fb_xerror(FB_DIRTY_DBASE, dhp->dbase, NIL);
	    }
         return(FB_AOK);
      }

/*
 * testmap - test the map by attempting to get a map entry FOR LAST Record
 */

   static testmap(dhp)
      fb_database *dhp;
      
      {
         long avail, freep, rpos, rlen;

	 return(fb_getmap(dhp->mfd, dhp->reccnt, &avail, &freep, &rpos,&rlen));
      }
