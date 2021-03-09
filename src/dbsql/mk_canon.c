/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mk_canon.c,v 9.0 2001/01/09 02:55:48 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Make_canon_sid[] = "@(#) $Id: mk_canon.c,v 9.0 2001/01/09 02:55:48 john Exp $";
#endif

#include "dbsql_e.h"

static node *c_current;
static int c_id;

static cparse();
static attach();
static void set_lvarc();
static void set_tvarc();
void build_vars();
void add_var();

/*
 * make_canon - convert the tree given to a canonical form,
 *	conjunctive normal form.
 */

   node *make_canon(n)
      node *n;

      {
         node *root, *makenode();

         root = makenode();
         root->n_type = C_ROOT;
         root->n_narg[0] = NULL;
         c_current = root;
         cparse(n);
         set_lvarc(root);
         c_id = 0;
         set_tvarc(root, root->n_narg[1]);
         root->n_tvarc = root->n_narg[1]->n_tvarc;
         return(root);
      }

/*
 * cparse - recursive part of making canon structures
 */

   static cparse(n)
      node *n;

      {
         node *c_andtree(), *right, *left, *t;

         switch(n->n_type){
            case O_AND:
               cparse(n->n_narg[0]);
               cparse(n->n_narg[1]);
               break;
            case O_OR:
               /* if only one or the other side is an AND, then burst apart */
               left = n->n_narg[0];
               right = n->n_narg[1];
               if (left->n_type == O_AND && right->n_type != O_AND){
                  t = n->n_narg[0];
                  n->n_narg[0] = n->n_narg[1];
                  n->n_narg[1] = t;
                  left = n->n_narg[0];
                  right = n->n_narg[1];
                  }
               if (left->n_type != O_AND && right->n_type == O_AND){
                  cparse(c_andtree(n));
                  }
               else
                  attach(n);
               break;
            default:
               attach(n);
               break;
            }
      }

/*
 * attach - attach the canonical node to the right hand side
 */

   static attach(n)
      node *n;

      {
         node *right, *makenode();

         right = makenode();
         right->n_type = C_AND;
         right->n_narg[0] = n;
         c_current->n_narg[1] = right;
         c_current = right;
      }

/*
 * c_andtree - take a tree of the form x OR (a and b)
 *	and convert to (x OR a) AND (x OR b)
 */

   node *c_andtree(n)
      node *n;

      {
         node *a, *o1, *o2, *makenode();

         a = makenode();
         o1 = makenode();
         o2 = makenode();
         a->n_type = O_AND;
         o1->n_type = o2->n_type = O_OR;
         o1->n_narg[0] = o2->n_narg[0] = n->n_narg[0];
         o1->n_narg[1] = n->n_narg[1]->n_narg[0];
         o2->n_narg[1] = n->n_narg[1]->n_narg[1];
         a->n_narg[0] = o1;
         a->n_narg[1] = o2;
         return(a);
      }

/*
 * set_lvarc - set the left variable arc of node n
 */

   static void set_lvarc(n)
      node *n;

      {
         if (n == NULL)
            return;
         build_vars(n, n->n_narg[0]);
         n->n_lvarc = countlist(n->n_list);
         set_lvarc(n->n_narg[1]);
      }

/*
 * set_tvarc - set the total variable arcs of node n
 */

   static void set_tvarc(r, n)
      node *r, *n;

      {
         node *a;

         if (n == NULL)
            return;
         n->n_id = ++c_id;
         set_tvarc(r, n->n_narg[1]);
         for (a = n->n_list; a != NULL; a = a->n_list)
            add_var(r, a);
         n->n_tvarc = countlist(r->n_list);
      }

/*
 * build_vars - recursive part of building the variables for
 *	canonical node tress
 */

   void build_vars(a, t)
      node *a, *t;

      {
         if (t == NULL)
            return;
         if (t->n_type == S_SUBQ)
            return;
         if (t->n_type == V_ID)
            add_var(a, t->n_narg[0]);		/* a.b, a is the fb_database */
         else{
            build_vars(a, t->n_narg[0]);
            build_vars(a, t->n_narg[1]);
            /*
            build_vars(a, t->n_narg[2]);
            build_vars(a, t->n_narg[3]);
            */
            }
      }

/*
 * add_var - add var id to list a if not already there
 */

   void add_var(a, id)
      node *a, *id;

      {
         cell *c, *tc;
         node *n, *makenode();

         if (id == NULL || id->n_obj == NULL)
            return;
         /* else add to the list of dbase symbols if its unique */
         c = (cell *) id->n_obj;
         for (n = a->n_list; n != NULL; n = n->n_list){
            tc = (cell *) n->n_obj;
            if (equal(c->c_sval, tc->c_sval))
               return;
            }
         n = makenode();
         *n = *id;
         n->n_list = a->n_list;
         a->n_list = n;
      }
