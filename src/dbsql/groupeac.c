/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: groupeac.c,v 9.0 2001/01/09 02:55:47 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Groupeach_sid[] = "@(#) $Id: groupeac.c,v 9.0 2001/01/09 02:55:47 john Exp $";
#endif

#include "dbsql_e.h"

extern short int cdb_secure;

/*
 * groupeach - this is like whereeach except it is done for
 *	records that match the curent group_value only.
 *
 *	restrictions (g_restrict) are applied above this layer.
 *
 *	assume that the current loaded rec is important, so
 *	retain that record after all said and done.
 */

   groupeach(dp, f)
      fb_database *dp;
      int (*f)();

      {
         int isize, st;
         long rec;

         rec = dp->rec;
	 lseek(dp->ifd, 0L, 0);
         fb_getxrec(group_value, dp);
         isize = dp->irecsiz - 11;
         for (;;){
            /* assume record is loaded */
	    if (!(FB_ISDELETED(dp))){
               st = FB_AOK;
               if (cdb_secure && !fb_record_permission(dp, READ))
                  st = FB_ERROR;
               if (st == FB_AOK)
                  if (((*f)(dp)) == FB_ERROR)
                     return(FB_ERROR);
               }
            if (fb_nextxrec(dp) != FB_AOK)
               break;
            if (strncmp(group_value, dp->irec, isize) != 0)
               break;
            }
          u_getrec(rec, dp);
	  return(FB_AOK);
      }
