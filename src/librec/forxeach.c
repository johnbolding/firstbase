/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: forxeach.c,v 9.0 2001/01/09 02:56:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Forxeach_sid[] = "@(#) $Id: forxeach.c,v 9.0 2001/01/09 02:56:57 john Exp $";
#endif

#include <fb.h>

extern short int cdb_secure;		/* flag used by cdb, not settable! */

/*
 * forxeach - foreach record in dp's index, get record and call f(dp)
 *	as long as f(dp) returns non FB_ERROR code or until eof of idx.
 */

   fb_forxeach(dp, f)
      fb_database *dp;
      int (*f)(fb_database *dp);
      
      {
         long rec;
         int max;
         char crec[FB_RECORDPTR+1];
	 
         /* b_tree handled if needed */
         if (dp->b_tree){
            if (fb_firstxrec(dp) != FB_ERROR)
               for (;;){
                  if (cdb_secure && fb_record_permission(dp, READ) == FB_ERROR)
                     continue;
                  if (!(fb_isdeleted(dp)))
                     if (((*f)(dp)) == FB_ERROR)
                        return(FB_ERROR);
                  if (fb_nextxrec(dp) == FB_ERROR)
                     break;
                  }
            }
         else{
            lseek(dp->ifd, 0L, 0);
            fb_r_init(dp->ifd);
            max = dp->irecsiz + 2;
            while (fb_nextline(dp->irec, max) != 0){
               fb_ffetch(dp->ip[((dp->ifields) - 1)], crec, dp->irec, dp);
               rec = atol(crec);
               if (rec > 0L && rec <= dp->reccnt){	/* ignore rec of 0 */
                  if (fb_getrec(rec, dp) == FB_ERROR)
                     fb_xerror(FB_FATAL_GETREC, dp->dbase, (char *) &rec);
                  if (cdb_secure && fb_record_permission(dp, READ) == FB_ERROR)
                     continue;
                  if (!(fb_isdeleted(dp)))
                     if (((*f)(dp)) == FB_ERROR)
                        return(FB_ERROR);
                  }
               }
            }
	  return(FB_AOK);
      }
