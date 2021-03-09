/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: u_distin.c,v 9.0 2001/01/09 02:55:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char U_distinct_sid[] = "@(#) $Id: u_distin.c,v 9.0 2001/01/09 02:55:51 john Exp $";
#endif

#include "dbsql_e.h"

static node *a_sel_list;
static cell **usym;
static u_test_enter();

/*
 * u_distinct - create a 'distinct' index on the first sel_list field.
 *	tie this index to the database, making it ready for use
 *	algorithm: create an s_expr(sel_list) for each record in dp.
 *		   look up its printable value in symtable.
 *		      if there, ignore, else store f into a virtual index.
 *	           tie this virtual index to the database
 *		   store the index name in the selq node for later expunge.
 */

   u_distinct(selq, sel_list, dp, res)
      node *selq, *sel_list, *res;
      fb_database *dp;

      {
         cell **makesymtab();
         node *u_vir_index(), *qn;
         int u_test_enter();

         a_sel_list = sel_list;
         usym = makesymtab();
         qn = u_vir_index();
         selq->n_virlist = qn;
         if (init_vir_index(qn, dp) == FB_ERROR)
            return(FB_ERROR);
         whereeach(dp, u_test_enter, res);
         end_vir_index(dp);
         expunge_symtab(usym);
         return(FB_AOK);
      }

/*
 * u_test_enter - test whether to enter this record or not
 */

   static u_test_enter(dp)
      fb_database *dp;

      {
         char *s_expr(), *s;
         cell *u_lookup();

         s = s_expr(a_sel_list);
         if (!is_null(a_sel_list))
            if (u_lookup(s, usym) == (cell *) NULL){
               vir_enter(dp);
               u_install(s, usym);
               }
         return(FB_AOK);
      }
