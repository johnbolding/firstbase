/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: pathname.c,v 9.0 2001/01/09 02:56:28 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Pathname_sid[] = "@(#) $Id: pathname.c,v 9.0 2001/01/09 02:56:28 john Exp $";
#endif

#include <fb.h>

static char *FMT = "%s/%s";
extern char *cdb_work_dir;		/* working directory */
extern char *cdb_user_home;		/* working directory */

/*
 * pathname - try and locate a file in environ HOME then in working directory.
 *	return file name or null.
 */
 
   char *fb_pathname(fp, f)
      char *fp, *f;
      
      {
	 *fp = 0;
	 if (cdb_user_home != NULL){
	    sprintf(fp, FMT, cdb_user_home, f);
	    if (access(fp, 0) == 0)
	       return(fp);
	    }
	 *fp = 0;
         if (cdb_work_dir != NULL){
	    sprintf(fp, FMT, cdb_work_dir, f);
            if (access(fp, 0) != 0)
               *fp = 0;
            }
	 return(fp);
      }
