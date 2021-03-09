/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_load.c,v 9.0 2001/01/09 02:56:52 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Mac_load_sid[] = "@(#) $Id: mac_load.c,v 9.0 2001/01/09 02:56:52 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>

typedef struct s_loadnode loadnode;
struct s_loadnode {
   char *a_loadname;
   loadnode *a_next;
   };

loadnode *head_loadnode = NULL;
static fb_mnode *load_n_ghead = NULL;		/* mnode garbage head */
static fb_cell *load_c_ghead = NULL;		/* cell garbage head */

extern char *cdb_pgm;				/* program name */
extern char *cdb_VEDIT;

#if !FB_PROTOTYPES
static search_loadnode();
static install_loadnode();
static loadnode *make_loadnode();
#else
static search_loadnode(char *);
static install_loadnode(char *);
static loadnode *make_loadnode(void);
#endif

/*
 * mf_load - provide the load() function for macro language
 */

   mf_load(n, r)
      fb_mnode *n, *r;

      {
         int st = FB_AOK, vedit = 0;
         fb_cell **save_symtab, *save_c_ghead;
         fb_mnode *save_n_ghead;

         (void) Mac_load_sid;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         tostring(n);
         if (equal(cdb_pgm, cdb_VEDIT))
            vedit = 1;
         if (vedit || search_loadnode(n->n_pval) == 0){
            if (!vedit)
               install_loadnode(n->n_pval);
            /* save the state of things at the moment */
            save_symtab = symtab;
            save_n_ghead = n_ghead;
            save_c_ghead = c_ghead;
            n_ghead = NULL;
            c_ghead = NULL;
            symtab = p_symtab;
            st = fb_macrotree(n->n_pval);
            if (st == FB_AOK)
               fb_linknode(e_winner, winner);
            /* restore things the way they were */
            symtab = save_symtab;
            load_n_ghead = n_ghead;
            load_c_ghead = c_ghead;
            n_ghead = save_n_ghead;
            c_ghead = save_c_ghead;
            }
         r->n_fval = st;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

/*
 * make_loadnode - create a loadnode for storage
 */

   static loadnode *make_loadnode()
      {
         loadnode *a;

         a = (loadnode *) fb_malloc(sizeof(loadnode));
         a->a_loadname = NULL;
         a->a_next = NULL;
         return(a);
      }

/*
 * search_loadnode - search the loadnodes for a certain loadfile name
 */

   static search_loadnode(s)
      char *s;

      {
         loadnode *a;

         for (a = head_loadnode; a != NULL; a = a->a_next)
            if (equal(s, a->a_loadname))
               return(1);
         return(0);
      }

/*
 * install_loadnode - install a name into the loadnode series
 */

   static install_loadnode(s)
      char *s;

      {
         loadnode *a;

         a = make_loadnode();
         fb_mkstr(&(a->a_loadname), s);
         a->a_next = head_loadnode;
         head_loadnode = a;
      }

/*
 * gcollect_loadnode - garbage collection for the loadnode memory
 */

   void fb_gcollect_loadnode()

      {
         loadnode *a, *na;

         for (a = head_loadnode; a != NULL; a = na){
            na = a->a_next;
            fb_free((char *) a->a_loadname);
            fb_free((char *) a);
            }
         /* free up the load_n_ghead series */
         fb_gcollect_mnode(load_n_ghead);
         /* free up the load_c_ghead series */
         fb_gcollect_cell(load_c_ghead);
         head_loadnode = NULL;
      }
