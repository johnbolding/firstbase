/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: u_grp_by.c,v 9.0 2001/01/09 02:55:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char U_groupby_sid[] = "@(#) $Id: u_grp_by.c,v 9.0 2001/01/09 02:55:51 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * u_groupby - this is fb_forxeach() with a grouping twist.
 *      groups of indexed items projected as a single row.
 *	res is still applied as restrictions on the group projection.
 *	vp is the resultant virtual database
 */

   u_groupby(dp, sel_list, having, from)
      fb_database *dp;
      node *sel_list, *having, *from;

      {
         long rec;
         int max, ifd;
         char *optr, *nptr;
         node *make_canon(), *h_cform;

         /* disable global restrictions - use having on group basis below */
         g_restrict = NULL;
         h_cform = make_canon(having);
         u_verify_cform(h_cform, from);
         max = dp->irecsiz + 13;

         optr = fb_malloc(max + 1);
         nptr = fb_malloc(max + 1);
         group_value = optr;
         optr[0] = nptr[0] = NULL;

         ifd = open(dp->dindex, 0);
         if (ifd <= 0){
            fb_serror(FB_MESSAGE, "Could not open u_groupby() index.", NIL);
            return(FB_ERROR);
            }
         fb_r_init(ifd);
         group_expr = 1;
         while (fb_nextline(dp->irec, max) != 0){
            strcpy(nptr, dp->irec);
            nptr[dp->irecsiz - 11] = NULL;
            if (!equal(nptr, optr)){
               /* process one group */
               strcpy(optr, nptr);
               rec = atol((char *) (dp->irec + dp->irecsiz - 11));
#if DEBUG
               if (traceflag > 7){
                  fprintf(stderr, "\n groupby - before pre_where() \n");
                  trace_canon(h_cform);
                  }
#endif /* DEBUG */
               if (set_where(h_cform) != FB_AOK)
                  return(FB_ERROR);
               u_getrec(rec, dp);
               if (test_restrict(h_cform, dp) == FB_AOK)
                  do_record(sel_list);
               }
	    }
          fb_free(optr);
          fb_free(nptr);
          group_value = NULL;
          close(ifd);
	  return(FB_AOK);
      }
