/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: node.c,v 9.0 2001/01/09 02:56:54 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Node_sid[] = "@(#) $Id: node.c,v 9.0 2001/01/09 02:56:54 john Exp $";
#endif

/*
 * node - the node portion of dbmacro - mostly parser stuff
 */

#include <fb.h>
#include <macro_e.h>

static fb_mnode *n_2nd_ghead = NULL;
static fb_mnode *m_pool = NULL;

/*
 * series of functions for node generation and settings.
 */

   fb_mnode *fb_makenode()
      {
         fb_mnode *n;

         (void) Node_sid;
         if (m_pool == NULL){
            n = (fb_mnode *) fb_malloc(sizeof(fb_mnode));
            }
         else{
            n = m_pool;
            m_pool = m_pool->n_glink;
            }
         n->n_glink = NULL;
         fb_clearnode(n);
         if (n->n_glink == NULL){
            n->n_glink = n_ghead;
            n_ghead = n;
            }
         n->n_lineno = lineno;
         n->n_fname = m_filenames[m_current_filename];
         return(n);
      }

/*
 * fb_s_makenode - make a node without linking into the garbage link
 *	used for return list of nodes
 */

   fb_mnode *fb_s_makenode()
      {
         fb_mnode *n;

         if (m_pool == NULL){
            n = (fb_mnode *) fb_malloc(sizeof(fb_mnode));
            }
         else{
            n = m_pool;
            m_pool = m_pool->n_glink;
            }
         n->n_glink = NULL;
         fb_clearnode(n);
         n->n_lineno = 0;
         n->n_fname = NULL;
         return(n);
      }

   fb_clearnode(n)
      fb_mnode *n;

      {
         int i;

         n->n_type = 0;
         n->n_next = NULL;
         n->n_list = NULL;
         n->n_obj = 0;
         for (i = 0; i < NARGS; i++)
            n->n_narg[i] = NULL;
         n->n_nval = NULL;
         n->n_pval = NULL;
         n->n_fval = 0;
         n->n_tval = 0;
         n->n_width = 0;
         n->n_scale = 0;
         n->n_virlist = NULL;
         n->n_fname = NULL;
         n->n_p1 = 0;
         n->n_p2 = 0;
      }

   void fb_freenode(n)
      fb_mnode *n;

      {
         if (n == NULL)
            return;
         if (n->n_nval != NULL){
            fb_free((char *) n->n_nval);
            n->n_nval = NULL;
            }
         if (n->n_pval != NULL){
            fb_free((char *) n->n_pval);
            n->n_pval = NULL;
            }
         fb_free((char *) n);
      }

   void fb_free_globalnode(n)
      fb_mnode *n;

      {
         if (n == NULL)
            return;
         if (n->n_nval != NULL){
            fb_free((char *) n->n_nval);
            n->n_nval = NULL;
            }
         if (n->n_pval != NULL){
            fb_free((char *) n->n_pval);
            n->n_pval = NULL;
            }
         /*
          * set the garbage indicator to ON for later collection
          *
          * n->n_garbage = 1;
          */
      }

   fb_copynode(t, f)
      fb_mnode *t, *f;

      {
         t->n_type = f->n_type;
         if (f->n_nval != NULL)
            fb_mkstr(&(t->n_nval), f->n_nval);
         t->n_fval = f->n_fval;
         t->n_tval = f->n_tval;
         t->n_width = f->n_width;
         t->n_scale = f->n_scale;
         t->n_fname = f->n_fname;
      }

   fb_mnode *fb_nullnode()
      {
         fb_mnode *n;

         n = fb_makenode();
         n->n_type = S_NULL;
         return(n);
      }

   fb_mnode *fb_vnode(type, obj)
      int type, obj;

      {
         fb_mnode *n;
         fb_cell *c;

         n = fb_makenode();
         n->n_type = type;
         n->n_obj = obj;
         if (type == V_FCON || type == V_CON){
            c = (fb_cell *) obj;
            if (type == V_CON)
               n->n_fval = (double) atoi(c->c_sval);
            else
               n->n_fval = (double) atof(c->c_sval);
            }
         return(n);
      }

   fb_mnode *fb_vnode1(type, obj, obj_a)
      int type, obj, obj_a;

      {
         fb_mnode *n;

         n = fb_makenode();
         n->n_type = type;
         n->n_obj = obj;
         n->n_narg[0] = fb_vnode(type, obj_a);
         return(n);
      }

   fb_mnode *fb_vnode2(type, obj, obj_a, obj_b)
      int type, obj, obj_a, obj_b;

      {
         fb_mnode *n;

         n = fb_makenode();
         n->n_type = type;
         n->n_obj = obj;
         n->n_narg[0] = fb_vnode(type, obj_a);
         n->n_narg[1] = fb_vnode(type, obj_b);
         return(n);
      }

   fb_mnode *fb_s_node0(type)
      char type;

      {
         fb_mnode *n;

         n = fb_s_makenode();
         n->n_type = type;
         n->n_glink = n_2nd_ghead;
         n_2nd_ghead = n;
         return(n);
      }

   fb_mnode *fb_node0(type)
      char type;

      {
         fb_mnode *n;

         n = fb_makenode();
         n->n_type = type;
         return(n);
      }

   fb_mnode *fb_node1(type, a)
      char type;
      fb_mnode *a;

      {
         fb_mnode *n;

         n = fb_makenode();
         n->n_type = type;
         n->n_narg[0] = a;
         return(n);
      }

   fb_mnode *fb_node2(type, a, b)
      char type;
      fb_mnode *a, *b;

      {
         fb_mnode *n;

         n = fb_makenode();
         n->n_type = type;
         n->n_narg[0] = a;
         n->n_narg[1] = b;
         return(n);
      }

   fb_mnode *fb_node3(type, a, b, c)
      char type;
      fb_mnode *a, *b, *c;

      {
         fb_mnode *n;

         n = fb_makenode();
         n->n_type = type;
         n->n_narg[0] = a;
         n->n_narg[1] = b;
         n->n_narg[2] = c;
         return(n);
      }

   fb_mnode *fb_node4(type, a, b, c, d)
      char type;
      fb_mnode *a, *b, *c, *d;

      {
         fb_mnode *n;

         n = fb_makenode();
         n->n_type = type;
         n->n_narg[0] = a;
         n->n_narg[1] = b;
         n->n_narg[2] = c;
         n->n_narg[3] = d;
         return(n);
      }

/*
 * fb_linknode - connect two nodes at the n_next link
 */

   fb_mnode *fb_linknode(a, b)
      fb_mnode *a, *b;

      {
         fb_mnode *n;

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

   fb_mnode *fb_listnode(a, b)
      fb_mnode *a, *b;

      {
         fb_mnode *n;

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
 * testnull - return 0 or 1 depending on null value. n is well defined.
 */

   fb_testnull(n)
      fb_mnode *n;

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
 * countnodes - count the nodes in the link list n_next
 */

   fb_countnodes(n)
      fb_mnode *n;

      {
         int i = 0;

         for (; n != NULL; n = n->n_next)
            i++;
         return(i);
      }

/*
 * realnodes - count the non S_NULL nodes in the link list n_next
 */

   fb_realnodes(n)
      fb_mnode *n;

      {
         int i = 0;

         for (; n != NULL; n = n->n_next){
            if (n->n_type != S_NULL)
               i++;
            }
         return(i);
      }

/*
 * gcollect_mnode - garbage collect all of the nodes that have been made
 */

   void fb_gcollect_mnode(g)
      fb_mnode *g;

      {
         fb_mnode *n;

         if (g == NULL)
            return;
         /* get n to the last node in the glink list */
         for (n = g; n != NULL; n = n->n_glink){
            if (n->n_nval != NULL){
               fb_free((char *) n->n_nval);
               n->n_nval = NULL;
               }
            if (n->n_pval != NULL){
               fb_free((char *) n->n_pval);
               n->n_pval = NULL;
               }
            if (n->n_glink == NULL)
               break;
            }
         /* g is the first node, n is the last node */
         n->n_glink = m_pool;
         m_pool = g;
         n_ghead = NULL;
      }

   void fb_gcollect_m_pool()
      {
         fb_mnode *n, *nn;

         for (n = m_pool; n != NULL; n = nn){
            nn = n->n_glink;
            fb_freenode(n);
            }
      }

/*
 * gcollect - garbage collection.
 *      symtab is not expunged in the idea that some symbols
 *      are likely to be repeated in this session
 */

   void fb_gcollect(g, c)
      fb_mnode *g;
      fb_cell *c;

      {
         fb_gcollect_mnode(g);
         fb_gcollect_cell(c);
      }

   void fb_2nd_gcollect()
      {
         fb_gcollect_mnode(n_2nd_ghead);
      }
