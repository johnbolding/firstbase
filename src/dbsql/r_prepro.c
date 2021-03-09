/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: r_prepro.c,v 9.0 2001/01/09 02:55:49 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char R_preprocess_sid[] = "@(#) $Id: r_prepro.c,v 9.0 2001/01/09 02:55:49 john Exp $";
#endif

#include "dbsql_e.h"

static relation *r_cur;			/* current relation */
static r_add_single();

/*
 * r_preprocess - preprocess the single variable component of a subQ.
 *	if the rel_single[slot] is filled, use it, else use whole dbase.
 */

   r_preprocess(rn, nvars)
      node *rn;
      int nvars;

      {
         int slot, r_add_single(), i;
         relation *r, *create_relation();
         node *n, *dn;
         fb_database *dp;
         v_matrix *vm, *make_matrix();

#if DEBUG
         if (traceflag >= 6){
            fprintf(stderr, "r_preprocess - preprocess single var subq\n");
            trace_canon(rn);
            }
#endif /* DEBUG */
         if (set_where(rn) != FB_AOK)
            fb_xerror(FB_MESSAGE, "r_preprocess - set_where() failed.", NIL);
         n = rn->n_narg[1];
         slot = n->n_list->n_id;
         vm = make_matrix();
         vm->v_array[slot] = 1;
         /*
          * by definition, these are all "slot" single vars to do,
          * so, use the whole database
          */
         dn = rn->n_list;
         for (i = 0; i < slot; i++, dn = dn->n_list)
            ;
         dp = (fb_database *) dn->n_p1;
         if (dp == NULL)
            fb_xerror(FB_MESSAGE, "r_preprocess - Null database pointer", NIL);
         r = create_relation(1, vm, nvars);
         r->r_dbase[0] = dp;
         r->r_vars = n->n_list;		/* vars for just this part */
         r_cur = r;
         whereeach(dp, r_add_single, rn);
         /*
          * i dont think a flush_relation is needed here, really.
          * flush_relation(r);
          */
         rel_single[slot] = r;
#if DEBUG
         if (traceflag >= 6)
            trace_relation(r, "Single Component of SubQ Relation Trace");
#endif /* DEBUG */
      }

/*
 * r_add_single - add the current dp record number to the r_cur relation
 */

   static r_add_single(dp)
      fb_database *dp;

      {
         r_cur->r_rec[0] = dp->rec;
         addrec_relation(r_cur);
      }
