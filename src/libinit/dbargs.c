/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbargs.c,v 9.0 2001/01/09 02:56:46 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "%W% %G% FB";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_max_file_name;
extern short int cdb_setup_done;   /* flag set after fb_setup() called */

/*
 * dbargs - expand the passed in fname and iname into the database.
 *	does not walk on dbase names for empty fname/iname.
 */

   fb_dbargs(fname, iname, dhp)
      char *fname, *iname;
      fb_database *dhp;
      
      {
         char tname[FB_MAXNAME];
	 int sz;

         if (!cdb_setup_done)
            fb_setup();
	 if ((sz = strlen(fname)) > 0){
	    fb_basename(tname, fname);
	    sz = strlen(tname);
	    if (sz > cdb_max_file_name)
	       fb_xerror(FB_MESSAGE, SYSMSG[S_TOO_LONG], fname);
	    sprintf(tname, SYSMSG[S_FMT_2S], fname, SYSMSG[S_EXT_CDB]);
	    fb_mkstr(&(dhp->dbase), tname);
	    sprintf(tname, SYSMSG[S_FMT_2S], fname, SYSMSG[S_EXT_DDICT]);
	    fb_mkstr(&(dhp->ddict), tname);
	    sprintf(tname, SYSMSG[S_FMT_2S], fname, SYSMSG[S_EXT_MAP]);
	    fb_mkstr(&(dhp->dmap), tname);
	    sprintf(tname, SYSMSG[S_FMT_2S], fname, SYSMSG[S_EXT_LOG]);
	    fb_mkstr(&(dhp->dlog), tname);
	    }
	 if ((sz = strlen(iname)) > 0){
	    fb_basename(tname, iname);
	    sz = strlen(tname);
	    if (sz > cdb_max_file_name)
	       fb_xerror(FB_MESSAGE, SYSMSG[S_TOO_LONG], iname);
	    sprintf(tname, SYSMSG[S_FMT_2S], iname, SYSMSG[S_EXT_IDX]);
	    fb_mkstr(&(dhp->dindex), tname);
	    sprintf(tname, SYSMSG[S_FMT_2S], iname, SYSMSG[S_EXT_IDICT]);
	    fb_mkstr(&(dhp->idict), tname);
	    }
      }
