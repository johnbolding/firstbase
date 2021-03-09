/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: relate.c,v 9.0 2001/01/09 02:55:50 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Relate_sid[] = "@(#) $Id: relate.c,v 9.0 2001/01/09 02:55:50 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * relate - create a new relation, using any relations already built.
 *	if all components of a relation are used, it is purged at end.
 */

   relate(c_root)
      node *c_root;

      {
         relation *nr, *pr, *r, *r_select();

         /* select/create a relation into nr as needed */
         nr = r_select(c_root);

         /* link nr into place at front of list */
         nr->r_next = r_head;
         r_head = nr;

         /* test for used relations and remove them */
         pr = NULL;
         for (r = r_head; r != NULL; r = nr){
            nr = r->r_next;
            /* if used then unlink this r */
            if (r->r_used){
               if (pr == NULL)
                  r_head = nr;
               else
                  pr->r_next = nr;
               }
            else
               pr = r;
            }
      }
