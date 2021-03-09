/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: r_grp_by.c,v 9.0 2001/01/09 02:55:49 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char R_groupby_sid[] = "@(#) $Id: r_grp_by.c,v 9.0 2001/01/09 02:55:49 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * r_groupby - this is fb_forxeach() with a grouping twist.
 *      groups of indexed items projected as a single row.
 *	res is still applied as restrictions on the group projection.
 *	vp is the resultant virtual database
 *
 *	this is u_groupby, twisted for relations with psuedo indexes.
 */

   r_groupby(r, sel_list, having, from)
      relation *r;
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
         max = r->r_irecsiz + 13;

         optr = fb_malloc(max + 1);
         nptr = fb_malloc(max + 1);
         group_value = optr;
         optr[0] = nptr[0] = NULL;

         ifd = open(r->r_index, 0);
         if (ifd <= 0){
            fb_serror(FB_MESSAGE, "Could not open r_groupby() index.", NIL);
            return(FB_ERROR);
            }
         fb_r_init(ifd);
         group_expr = 1;
         while (fb_nextline(r->r_irec, max) != 0){
            strcpy(nptr, r->r_irec);
            nptr[r->r_irecsiz - 11] = NULL;
            if (!equal(nptr, optr)){
               /* process one group */
               strcpy(optr, nptr);
               rec = atol((char *) (r->r_irec + r->r_irecsiz - 11));
               getrec_relation(rec, r);
               getrec_loadrel(r);
#if DEBUG
               if (traceflag > 7){
                  fprintf(stderr, "\n r_groupby - before pre_where() \n");
                  trace_canon(h_cform);
                  }
#endif /* DEBUG */
               if (set_where(h_cform) != FB_AOK)
                  return(FB_ERROR);
               if (test_restrict(h_cform, (fb_database * ) NULL) == FB_AOK)
                  do_record(sel_list);
               }
	    }
          fb_free(optr);
          fb_free(nptr);
          group_value = NULL;
          close(ifd);
	  return(FB_AOK);
      }
