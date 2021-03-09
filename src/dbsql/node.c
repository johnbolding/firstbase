/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: node.c,v 9.0 2001/01/09 02:55:48 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Node_sid[] = "@(#) $Id: node.c,v 9.0 2001/01/09 02:55:48 john Exp $";
#endif

/*
 * node - the node portion of dbsql - mostly parser stuff
 */

#include "dbsql_e.h"

/*
 * series of functions for node generation and settings.
 *    generally, called from dbsql.g.y
 */

   node *makenode()
      {
         node *n;

         n = (node *) fb_malloc(sizeof(node));
         n->n_glink = NULL;
         clearnode(n);
         n->n_glink = n_ghead;
         n_ghead = n;
         return(n);
      }

   clearnode(n)
      node *n;

      {
         int i;

         n->n_type = 0;
         n->n_next = NULL;
         n->n_list = NULL;
         n->n_group = NULL;
         n->n_label = NULL;
         n->n_obj = 0;
         for (i = 0; i < NARGS; i++)
            n->n_narg[i] = NULL;
         n->n_lvarc = 0;
         n->n_tvarc = 0;

         n->n_nval = NULL;
         n->n_pval = NULL;
         n->n_fval = 0;
         n->n_tval = NULL;
         n->n_virlist = NULL;
         n->n_width = 0;
         n->n_vwidth = 0;
         n->n_scale = 0;
         n->n_height = 0;
         n->n_id = -1;
         n->n_p1 = 0;
         n->n_p2 = 0;
      }

   node *nullnode()
      {
         node *n, *makenode();

         n = makenode();
         n->n_type = S_NULL;
         return(n);
      }

   node *vnode(type, obj)
      int type, obj;

      {
         node *n, *makenode();
         cell *c;

         n = makenode();
         n->n_type = type;
         n->n_obj = obj;
         if (type == V_FCON || type == V_CON){
            c = (cell *) obj;
            if (type == V_CON)
               n->n_fval = (float) atoi(c->c_sval);
            else
               n->n_fval = (float) atof(c->c_sval);
            }
         return(n);
      }

   node *vnode1(type, obj, obj_a)
      int type, obj, obj_a;

      {
         node *n, *makenode(), *vnode();

         n = makenode();
         n->n_type = type;
         n->n_obj = obj;
         n->n_narg[0] = vnode(type, obj_a);
         return(n);
      }

   node *vnode2(type, obj, obj_a, obj_b)
      int type, obj, obj_a, obj_b;

      {
         node *n, *makenode(), *vnode();

         n = makenode();
         n->n_type = type;
         n->n_obj = obj;
         n->n_narg[0] = vnode(type, obj_a);
         n->n_narg[1] = vnode(type, obj_b);
         return(n);
      }

   node *node0(type)
      char type;

      {
         node *n, *makenode();

         n = makenode();
         n->n_type = type;
         return(n);
      }

   node *node1(type, a)
      char type;
      node *a;

      {
         node *n, *makenode();

         n = makenode();
         n->n_type = type;
         n->n_narg[0] = a;
         return(n);
      }

   node *node2(type, a, b)
      char type;
      node *a, *b;

      {
         node *n, *makenode();

         n = makenode();
         n->n_type = type;
         n->n_narg[0] = a;
         n->n_narg[1] = b;
         return(n);
      }

   node *node3(type, a, b, c)
      char type;
      node *a, *b, *c;

      {
         node *n, *makenode();

         n = makenode();
         n->n_type = type;
         n->n_narg[0] = a;
         n->n_narg[1] = b;
         n->n_narg[2] = c;
         return(n);
      }

   node *node4(type, a, b, c, d)
      char type;
      node *a, *b, *c, *d;

      {
         node *n, *makenode();

         n = makenode();
         n->n_type = type;
         n->n_narg[0] = a;
         n->n_narg[1] = b;
         n->n_narg[2] = c;
         n->n_narg[3] = d;
         return(n);
      }

/*
 * linknode - connect two nodes at the n_next link
 */

   node *linknode(a, b)
      node *a, *b;

      {
         node *n;

         if (a == NULL)
            return(b);
         else if (b == NULL)
            return(a);
         for (n = a; n->n_next != NULL; n = n->n_next)
            ;
         n->n_next = b;
         return(a);
      }

/*
 * listnode - connect two nodes at the n_list link
 */

   node *listnode(a, b)
      node *a, *b;

      {
         node *n;

         if (a == NULL)
            return(b);
         else if (b == NULL)
            return(a);
         for (n = a; n->n_list != NULL; n = n->n_list)
            ;
         n->n_list = b;
         return(a);
      }

/*
 * groupnode - connect two nodes at the n_group link
 */

   node *groupnode(a, b)
      node *a, *b;

      {
         node *n;

         if (a == NULL)
            return(b);
         else if (b == NULL)
            return(a);
         for (n = a; n->n_group != NULL; n = n->n_group)
            ;
         n->n_group = b;
         return(a);
      }

/*
 * testnull - return 0 or 1 depending on null value. n is well defined.
 */

   testnull(n)
      node *n;

      {
         int ret = 0;
         fb_field *f;

         if (n->n_type == V_ID){
            f = (fb_field *) n->n_p2;
            if (f->fld[0] == NULL)
               ret = 1;
            }
         return(ret);
      }

/*
 * copy_nodelist - make a copy of the node list f, return pointer to its root.
 */

   node *copy_nodelist(f)
      node *f;

      {
         cell *tc, *fc, *makecell();
         node *t, *pt, *makenode(), *nroot;

         pt = NULL;
         nroot = (node *) NULL;
         for (; f != (node *) NULL; f = f->n_list){
            t = makenode();
            tc = makecell();
            fc = (cell *) f->n_obj;
            fb_mkstr(&(tc->c_sval), fc->c_sval);
            t->n_obj = (int) tc;
            if (nroot == (node *) NULL)
               nroot = t;
            if (pt != NULL)
               pt->n_list = t;
            pt = t;
            }
         return(nroot);
      }

/*
 * countlist - count the nodes in the list n
 */

   countlist(n)
      node *n;

      {
         int i = 0;

         for (; n != NULL; n = n->n_list)
            i++;
         return(i);
      }

/*
 * gcollect_node - garbage collect all of the nodes that have been made
 */

   gcollect_node()
      {
         node *n, *nn;

         for (n = n_ghead; n != NULL; n = nn){
            nn = n->n_glink;
            fb_free(n->n_pval);
            fb_free(n->n_nval);
            fb_free((char *) n);
            }
         n_ghead = NULL;
      }
