/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: r_distin.c,v 9.0 2001/01/09 02:55:49 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char R_distinct_sid[] = "@(#) $Id: r_distin.c,v 9.0 2001/01/09 02:55:49 john Exp $";
#endif

#include "dbsql_e.h"

static node *a_sel_list;
static relation *vr;
static char *vbuf;
static cell **rsym;
static r_test_enter();

/*
 * r_distinct - create a 'distinct' relation using a given relation.
 *	return the newly create relation.
 *	algorithm: create an s_expr(sel_list) for each relation entry.
 *		   look up its printable value in symtable.
 *		      if there, ignore, else store f into a virtual index.
 */

   relation *r_distinct(r, sel_list)
      node *sel_list;
      relation *r;

      {
         cell **makesymtab();
         int r_test_enter(), i;
         relation *create_relation();

         vr = create_relation(r->r_nrecs, r->r_vm, r->r_nvars);
         for (i = 0; i < r->r_nrecs; i++)
            vr->r_dbase[i] = r->r_dbase[i];
         a_sel_list = sel_list;
         rsym = makesymtab();
         vbuf = (char *) fb_malloc(FB_MAXLINE);
         relateeach(r, r_test_enter);
         fb_free(vbuf);
         expunge_symtab(rsym);
         return(vr);
      }

/*
 * r_test_enter - test whether to enter this record or not
 */

   static r_test_enter(r)
      relation *r;

      {
         char *s_expr(), *s;
         node *n;
         int size;
         cell *u_lookup();

         vbuf[0] = NULL;
         size = 0;
         for (n = a_sel_list; n != NULL; n = n->n_list){
            s = s_expr(n);
            if (is_null(n))
               return(FB_AOK);
            size += strlen(s);
            if (size < FB_MAXLINE)
               strcat(vbuf, s);
            }
         if (u_lookup(vbuf, rsym) == (cell *) NULL){
            rel_enter(vr, r);
            u_install(vbuf, rsym);
            }
         return(FB_AOK);
      }
