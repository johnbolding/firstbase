/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: opendb.c,v 9.0 2001/01/09 02:56:49 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Opendb_sid[] = "@(#) $Id: opendb.c,v 9.0 2001/01/09 02:56:49 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char *MSG1 = "creating new database";
extern short int cdb_opendb_level;
extern short int cdb_allow_links;
extern short int cdb_fixedwidth;
extern short int cdb_initscreen_done;
extern short int cdb_use_rpc;
extern short int cdb_returnerror;
extern short int cdb_error;
extern short int cdb_locklevel;
extern short int cdb_setup_done;   /* flag set after fb_setup() called */

extern int errno;

/*
 *  opendb - open a fb_database using given mode.
 *
 *	- if ixflag == NOINDEX, open index also.
 *	- if ixflag == WITHINDEX, open index also.
 *	- if ixflag == ALLINDEX, open index and autoindexes.
 *
 *      - if ixoption == OPTIONAL_INDEX, return
 *	- if ixoption == MAYBE_OPTIONAL_INDEX, fb_serror out
 *	- if ixoption == MUST_INDEX, fb_xerror out
 */

   fb_opendb(dp, mode, ixflag, ixoption)
      fb_database *dp;
      int mode, ixflag, ixoption;
      
      {
         char hname[FB_MAXNAME];
	 fb_database *save_dp;
	 int ignore_flag = 0, permissions = 0, simple_index = 0, st;

         if (cdb_t_lines == 0 || cdb_initscreen_done == 0)
	    cdb_batchmode = 1;
	 cdb_error = 0;
         errno = 0;
#if RPC
         if (cdb_use_rpc)
            return(fb_opendb_clnt(dp, mode, ixflag, ixoption));
#endif /* RPC */
         if (dp == NULL){
	    if (cdb_returnerror){
	       cdb_error = FB_BAD_DATA;
	       return(FB_ERROR);
	       }
	    fb_xerror(FB_BAD_DATA, SYSMSG[S_NO_INDEX], NIL);
	    }
         if (!cdb_setup_done)
	    fb_setup();		/* process /usr/lib/.cdbrc setup file */
         dp->dirty = CHAR_0;
	 if (cdb_opendb_level == 0){
	    if (cdb_locklevel <= 0)
	       ignore_flag = 1;
	    fb_initlock(ignore_flag, (fb_database *) dp);
	    }
         dp->fixedwidth = cdb_fixedwidth;
	 cdb_opendb_level++;
	 save_dp = cdb_db;
	 /* expand the dbase file name if needed */
         if ( (dp->dmap == NULL  || dp->dmap[0] == NULL)  ||
	      (dp->ddict == NULL || dp->ddict[0] == NULL)){
	    if (dp->dbase == NULL  || dp->dbase[0] == NULL){
	       if (cdb_returnerror){
		  cdb_error = FB_CANT_OPEN;
		  return(FB_ERROR);
		  }
	       fb_xerror(FB_CANT_OPEN, NIL, NIL);	/* panic if no name */
	       }
	    strcpy(hname, dp->dbase);
	    fb_dbargs(hname, NIL, dp);
	    }

	 /* expand the dbase index file name if needed */
         if (ixflag > 0)
            simple_index = 1;
	 if (ixflag > 0 && (dp->idict == NULL || dp->idict[0] == NULL)){
	    if ( (dp->dindex == NULL || dp->dindex[0] == NULL)){
	       /* error - no name to expand */
	       switch(ixoption){
	          case FB_MUST_INDEX:
		     if (cdb_returnerror){
			cdb_error = FB_CANT_OPEN;
			return(FB_ERROR);
			}
		     fb_xerror(FB_CANT_OPEN, NIL, NIL);
                     break;
		  case FB_MAYBE_OPTIONAL_INDEX:
		     if (cdb_returnerror){
			cdb_error = FB_CANT_OPEN;
			return(FB_ERROR);
			}
		     fb_serror(FB_CANT_OPEN, NIL, NIL);
		     break;
		  }
               simple_index = 0;
	       /* ixflag = 0; */
	       }
	    else{ /* expand index name */
	       strcpy(hname, dp->dindex);
	       fb_dbargs(NIL, hname, dp);
	       }
	    }
	    
         if (fb_getd_dict(dp) == FB_ERROR){
	    if (!cdb_batchmode)
	       fb_clear();
	    if (cdb_returnerror){
	       cdb_error = FB_BAD_DICT;
	       return(FB_ERROR);
	       }
	    fb_xerror(FB_BAD_DICT, dp->ddict, NIL);
	    }
         st = fb_getd_data(mode, dp);
         if (st == FB_LOCKED_ERROR){
            cdb_error = FB_LOCKED_ERROR;
            if (cdb_returnerror)
               return(FB_ERROR);
            fb_xerror(FB_CANT_OPEN, dp->dbase, "locked");
            }
         else if (st == FB_DIRTY_DBASE){
            cdb_error = FB_DIRTY_DBASE;
            return(FB_ERROR);
            }
         if (st == FB_ERROR){			/* attempt to create it */
	    if (!cdb_batchmode)
	       fb_clear();
	    if (access(dp->dbase, 0) != 0 && access(dp->dmap, 0) != 0 &&
	          (mode == READWRITE || mode == WRITE)){
	       if (!cdb_batchmode)
	          fb_fmessage(MSG1);
               fb_make_dbase(dp);
	       }
	    else {
               permissions = 0;
               if (mode == READ)
                  permissions = 4;
               else if (mode == READWRITE || mode == WRITE)
                  permissions = 6;
	       if (access(dp->dbase, permissions) != 0){
		  if (cdb_returnerror){
		     cdb_error = FB_CANT_OPEN;
		     return(FB_ERROR);
		     }
	          fb_xerror(FB_CANT_OPEN, dp->dbase, NIL);
		  }
	       if (access(dp->dmap, 6) != 0){	/* write needed for locks */
		  if (cdb_returnerror){
		     cdb_error = FB_CANT_OPEN;
		     return(FB_ERROR);
		     }
	          fb_xerror(FB_CANT_OPEN, dp->dmap, NIL);
		  }
	       /* this catches too many open files */
	       if (cdb_returnerror){
		  cdb_error = FB_CANT_OPEN;
		  return(FB_ERROR);
		  }
	       fb_xerror(FB_CANT_OPEN, dp->dbase, NIL);
	       }
	    }
	 if (ixflag > 1 && fb_getauto(dp, mode) == FB_ERROR){	/* ALLINDEX */
	    if (cdb_returnerror){
	       cdb_error = FB_BAD_INDEX;
	       return(FB_ERROR);
	       }
	    fb_xerror(FB_BAD_INDEX, dp->dbase, NIL);
	    }
	 if (simple_index > 0){				/* WITHINDEX */
	    strcpy(hname, dp->dindex);	/* save name for pos error msg */
	    if (fb_geti_dict(mode, dp) == FB_ERROR){
	       if (!cdb_batchmode)
		  fb_clear();
	       switch(ixoption){
	          case FB_MUST_INDEX:
		     if (cdb_returnerror){
			cdb_error = FB_BAD_DICT;
			return(FB_ERROR);
			}
		     fb_xerror(FB_BAD_DICT, dp->idict, NIL);
                     break;
		  case FB_MAYBE_OPTIONAL_INDEX:
		     break;
		  }
	       ixflag = 0;
	       }
	    else if (fb_geti_data(mode, dp) == FB_ERROR){
	       if (!cdb_batchmode)
		  fb_clear();
	       switch(ixoption){
	          case FB_MUST_INDEX:
		     if (cdb_returnerror){
			cdb_error = FB_CANT_OPEN;
			return(FB_ERROR);
			}
                     if (dp->b_tree && dp->b_seq != NULL){
                        fb_rootname(hname, dp->b_seq->bs_name);
		        fb_xerror(FB_BAD_INDEX, hname, NIL);
                        }
                     else
		        fb_xerror(FB_CANT_OPEN, hname, NIL);
                     break;
		  case FB_MAYBE_OPTIONAL_INDEX:
		     if (cdb_returnerror){
			cdb_error = FB_CANT_OPEN;
			return(FB_ERROR);
			}
		     fb_serror(FB_CANT_OPEN, hname, NIL);
                     break;
		  }
	       ixflag = 0;
	       }
	    }
	 if (ixflag == 0)
	    fb_emptyi_dict(dp);
         if (cdb_allow_links){
	    st = fb_initlink(dp);
            if (st == FB_ERROR){
               /*
                * if an FB_ERROR and NOT cdb_returnerror, fb_xerror will
                * have been already called in fb_initlink
                */
               if (cdb_returnerror){
                  cdb_error = FB_LINK_ERROR;
                  return(FB_ERROR);
                  }
	       fb_xerror(FB_LINK_ERROR, dp->ddict, NIL);
               }
            }
	 fb_nobuf(dp);			/* turn off buffering */
	 /* restore top dbase global variables  */
	 cdb_db = save_dp;
         if (cdb_db != NULL){
            cdb_kp = cdb_keymap = cdb_db->kp;
            cdb_ip = cdb_keyindx = cdb_db->ip;
            }
	 dp->refcnt++;
	 return(FB_AOK);
      }
