/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: make_rel.c,v 9.0 2001/01/09 02:55:48 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Make_rel_sid[] = "@(#) $Id: make_rel.c,v 9.0 2001/01/09 02:55:48 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * make_rel - make a relation out of the query represented by mat_head *h
 *	for each separate component of the list, build/blend relation
 *	return the final relation. 
 */

   make_rel(h, c_root, from)
      mat_head *h;
      node *c_root;
      node *from;

      {
         v_matrix *v;
         node c_tree, *ct, *t, *pt;
         int i, id;
         relation *relate();

         ct = &c_tree;
         clearnode(ct);
         ct->n_list = from;
         ct->n_type = C_ROOT;
         for (v = h->m_head; v != NULL; v = v->v_next){
            /* build a canonical node tree from the id list in v */
            for (i = 0; i < MAXCLAUSES && v->v_list[i] != 0; i++){
               id = v->v_list[i];
               if (id == -1)
                  continue;
               pt = c_root;
               t = c_root->n_narg[1];
               for (; t != NULL; t = t->n_narg[1]){
                  if (id == t->n_id)
                     break;
                  pt = t;
                  }
               if (t == NULL || id != t->n_id)
                  fb_xerror(FB_MESSAGE, "make_rel: cannot locate canonical id", NIL);
               /* unlink t from c_root */
               pt->n_narg[1] = t->n_narg[1];
               t->n_narg[1] = NULL;
               /* link t into c_tree */
               ct->n_narg[1] = t;
               ct = t;
               }
            }

         /* provide position number for all dbase elements in c_tree */
         u_position_cform(&c_tree, from);

#if DEBUG
         if (traceflag >= 4){
            fprintf(stderr, "make_rel - sub tree\n");
            trace_canon(&c_tree);
            }
#endif /* DEBUG */

         /*
          * build the relation, using c_tree, taking the current
          * relation list into account
          */
         relate(&c_tree);
      }
