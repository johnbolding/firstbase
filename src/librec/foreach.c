/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: foreach.c,v 9.0 2001/01/09 02:56:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Foreach_sid[] = "@(#) $Id: foreach.c,v 9.0 2001/01/09 02:56:57 john Exp $";
#endif

#include <fb.h>

extern short int cdb_secure;		/* flag used by cdb, not settable! */

/*
 * foreach - foreach record in dp database, get record and call f(dp)
 *	as long as f(dp) returns non FB_ERROR code or until reccnt is reached.
 *	return FB_AOK or FB_ERROR.
 */

   fb_foreach(dp, f)
      fb_database *dp;
      int (*f)(fb_database *dp);
      
      {
         
         long rec;
	 
	 for (rec = 1; rec <= dp->reccnt; rec++){
	    if (fb_getrec(rec, dp) == FB_ERROR)
	       fb_xerror(FB_FATAL_GETREC, dp->dbase, (char *) &rec);
	    if (!(fb_isdeleted(dp))){
               if (cdb_secure && fb_record_permission(dp, READ) == FB_ERROR)
                  continue;
	       if (((*f)(dp)) == FB_ERROR)
		  return(FB_ERROR);
               }
	    }
	  return(FB_AOK);
      }
