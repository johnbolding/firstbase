/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: libsym.c,v 9.0 2001/01/09 02:56:53 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Libsym_sid[] = "@(#) $Id: libsym.c,v 9.0 2001/01/09 02:56:53 john Exp $";
#endif

#include <fb.h>
#include <macro_e.h>

/*
 * libsym - the symbol table library
 */

#if !FB_PROTOTYPES
static int hash();
static dump_one();
#else
static int hash(char *);
static dump_one(fb_cell **);
#endif

static fb_cell *c_pool = NULL, **s_pool = NULL;

/*
 * makesymtab - make a fb_cell symbol table, return it
 */

   fb_cell **fb_makesymtab()
      {
         int i;
         fb_cell **cp, *c;
         
         (void) Libsym_sid;
         if (s_pool == NULL)
            cp = (fb_cell **) fb_malloc(MAXSYM * sizeof(fb_cell *));
         else{
            /* get a symtab off the s_pool list */
            cp = s_pool;
            s_pool = s_pool[0]->c_symtab;
            c = cp[0];
            /* link the used c node back into the c_pool list */
            c->c_chain = c_pool;
            c_pool = c;
            }
         for (i = 0; i < MAXSYM; i++)
            cp[i] = NULL;
         return(cp);
      }

/*
 * expunge_symtab - remove a fb_cell symbol table
 */

   void fb_expunge_symtab(stab)
      fb_cell *stab[];

      {
         int i;
         fb_cell *c, *nc;

         if (stab == NULL)
            return;
         for (i = 0; i < MAXSYM; i++){
            for (c = stab[i]; c != NULL; c = nc){
               nc = c->c_chain;
               if (c->c_symtab != NULL)
                  fb_expunge_symtab(c->c_symtab);
               fb_free((char *) c->c_sval);
               c->c_sval = NULL;
               if (nc == NULL)
                  break;
               }
            /* stab[i] is head, c is tail */
            if (c != NULL){
               c->c_chain = c_pool;
               c_pool = stab[i];
               stab[i] = NULL;
               }
            }
         c = fb_s_makecell();
         c->c_symtab = s_pool;
         stab[0] = c;
         s_pool = stab;
      }

   void fb_gcollect_s_pool()
      {
         fb_cell **s, *c, **ns;

         for (s = s_pool; s != NULL; s = ns){
            c = s[0];
            ns = c->c_symtab;
            fb_free((char *) c);
            fb_free((char *) s);
            }
      }

/*
 * lookup - lookup a symbol, return its fb_cell or 0
 */

   fb_cell *fb_lookup(s)
      char *s;

      {
         return(fb_u_lookup(s, symtab));
      }

/*
 * fb_glookup - lookup a GLOBAL symbol, return its fb_cell or 0
 */

   fb_cell *fb_glookup(s)
      char *s;

      {
         return(fb_u_lookup(s, g_symtab));
      }

/*
 * sym_install - install a symbol
 */

   fb_cell *fb_sym_install(s)
      char *s;

      {
         return(fb_u_install(s, symtab));
      }

/*
 * sinstall - symbol lookup/install
 */

   fb_cell *fb_sinstall(s)
      char *s;

      {
         fb_cell *c;

         if ((c = fb_u_lookup(s, symtab)) == (fb_cell *) NULL)
            c = fb_u_install(s, symtab);
         return(c);
      }

/*
 * ginstall - GLOBAL symbol lookup/install
 */

   fb_cell *fb_ginstall(s)
      char *s;

      {
         fb_cell *c;

         if ((c = fb_u_lookup(s, g_symtab)) == (fb_cell *) NULL)
            c = fb_u_install(s, g_symtab);
         return(c);
      }

/*
 * u_sinstall - u_lookup and u_install if needed.
 */

   fb_cell *fb_u_sinstall(s, stab)
      char *s;
      fb_cell *stab[];

      {
         fb_cell *c;

         if ((c = fb_u_lookup(s, stab)) == (fb_cell *) NULL)
            c = fb_u_install(s, stab);
         return(c);
      }

/*
 * u_lookup - lookup a symbol, return its fb_cell or 0
 */

   fb_cell *fb_u_lookup(s, stab)
      char *s;
      fb_cell *stab[];

      {
         int i;
         fb_cell *c;

         i = hash(s);
         for (c = stab[i]; c != NULL; c = c->c_chain)
            if (equal(c->c_sval, s))
               return(c);
         return((fb_cell *) NULL);
      }

/*
 * u_install - install a symbol
 */

   fb_cell *fb_u_install(s, stab)
      char *s;
      fb_cell *stab[];

      {
         fb_cell *c, *nc;
         int i;

         c = fb_s_makecell();
         fb_mkstr(&(c->c_sval), s);
         i = hash(s);
         nc = stab[i];
         stab[i] = c;
         c->c_chain = nc;
         return(c);
      }

/*
* s_makecell - make a fb_cell without linking to others. for symtab use.
*/

   fb_cell *fb_s_makecell()
      {
         fb_cell *c;

         if (c_pool == NULL)
            c = (fb_cell *) fb_malloc(sizeof(fb_cell));
         else{
            c = c_pool;
            c_pool = c_pool->c_chain;
            }
         c->c_sval = NULL;
         c->c_chain = NULL;
         c->c_vid = NULL;
         c->c_symtab = NULL;
         return(c);
      }

/*
* makecell - make a cell
*/

   fb_cell *fb_makecell()
      {
         fb_cell *c;

         c = fb_s_makecell();
         c->c_chain = c_ghead;
         c_ghead = c;
         return(c);
      }

/*
 * gcollect_cell - garbage collection for symbol table cells
 */

   void fb_gcollect_cell(cg)
      fb_cell *cg;

      {
         fb_cell *c, *nc;

         if (cg == NULL)
            return;
         for (c = cg; c != NULL; c = nc){
            nc = c->c_chain;
            fb_free(c->c_sval);
            c->c_sval = NULL;
            if (nc == NULL)
               break;
            }
         /* cg is head, c is tail */
         if (c != NULL){
            c->c_chain = c_pool;
            c_pool = cg;
            }
      }

   void fb_gcollect_c_pool()
      {
         fb_cell *c, *nc;

         for (c = c_pool; c != NULL; c = nc){
            nc = c->c_chain;
            fb_free((char *) c);
            }
      }

/*
 * fb_freecell - place c into the cell pool
 */

   void fb_freecell(c)
      fb_cell *c;

      {
         c->c_chain = c_pool;
         c_pool = c;
      }

/*
 * hash - provide a simple hash function
 */

   static int hash(s)
      register  char *s;
      {
         int hashval;

         for (hashval = 0; *s != '\0'; )
             hashval += (unsigned) *s++;
         return(hashval % MAXSYM);
      }

   void fb_dump_symtab()
      {
         printf("Global Symbol Table: %d\n", (int) g_symtab);
         dump_one(g_symtab);
         printf("Symbol Table: %d\n", (int) symtab);
         dump_one(symtab);
      }

   static dump_one(stab)
      fb_cell **stab;

      {
         int i;
         fb_cell *c;

         for (i = 0; i < MAXSYM; i++){
            if (stab[i] != NULL)
               printf("hash location %d\n", i);
            for (c = stab[i]; c != NULL; c = c->c_chain)
               printf("   ...%s\n", c->c_sval);
            }
      }
