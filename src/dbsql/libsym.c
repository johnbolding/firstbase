/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: libsym.c,v 9.0 2001/01/09 02:55:48 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Libsym_sid[] = "@(#) $Id: libsym.c,v 9.0 2001/01/09 02:55:48 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * libsym - the symbol table library
 */

static hash();

/*
 * makesymtab - make a cell symbol table, return it
 */

   cell **makesymtab()
      {
         int i;
         cell **cp;

         cp = (cell **) fb_malloc(MAXSYM * sizeof(cell *));
         for (i = 0; i < MAXSYM; i++)
            cp[i] = NULL;
         return(cp);
      }

/*
 * expunge_sumtab - remove a cell symbol table
 */

   expunge_symtab(stab)
      cell *stab[];

      {
         int i;
         cell *c, *nc;

         for (i = 0; i < MAXSYM; i++){
            c = stab[i];
            for (; c != NULL; c = nc){
               nc = c->c_chain;
               fb_free(c->c_sval);
               fb_free((char *) c);
               }
            }
         fb_free((char *) stab);
      }

/*
 * lookup - lookup a symbol, return its cell or 0
 */

   cell *lookup(s)
      char *s;

      {
         cell *u_lookup();

         return(u_lookup(s, symtab));
      }

/*
 * sym_install - install a symbol
 */

   cell *sym_install(s)
      char *s;

      {
         cell *u_install();

         return(u_install(s, symtab));
      }

/*
 * sinstall - symbol lookup/install
 */

   cell *sinstall(s)
      char *s;

      {
         cell *c, *u_install(), *u_lookup();

         if ((c = u_lookup(s, symtab)) == (cell *) NULL)
            c = u_install(s, symtab);
         return(c);
      }

/*
 * u_lookup - lookup a symbol, return its cell or 0
 */

   cell *u_lookup(s, stab)
      char *s;
      cell *stab[];

      {
         int i;
         cell *c;

         i = hash(s);
         for (c = stab[i]; c != NULL; c = c->c_chain)
            if (equal(c->c_sval, s))
               return(c);
         return((cell *) NULL);
      }

/*
 * u_install - install a symbol
 */

   cell *u_install(s, stab)
      char *s;
      cell *stab[];

      {
         cell *c, *nc, *s_makecell();
         int i;

         c = s_makecell();
         fb_mkstr(&(c->c_sval), s);
         i = hash(s);
         nc = stab[i];
         stab[i] = c;
         c->c_chain = nc;
         return(c);
      }

/*
 * s_makecell - make a cell without linking to others. for symtab use.
 */

   cell *s_makecell()
      {
         cell *c;

         c = (cell *) fb_malloc(sizeof(cell));
         c->c_sval = NULL;
         c->c_chain = NULL;
         return(c);
      }

/*
* makecell - make a cell
*/

   cell *makecell()
      {
         cell *c, *s_makecell();

         c = s_makecell();
         c->c_chain = c_ghead;
         c_ghead = c;
         return(c);
      }

/*
 * gcollect_cell - garbage collection for symbol table cells
 */

   gcollect_cell()
      {
         cell *c, *nc;

         for (c = c_ghead; c != NULL; c = nc){
            nc = c->c_chain;
            fb_free(c->c_sval);
            fb_free((char *) c);
            }
         c_ghead = NULL;
      }

/*
 * hash - provide a simple hash function
 */

   static hash(s)
      register  char *s;
      {
         register int hashval;

         for (hashval = 0; *s != '\0'; )
             hashval += (unsigned) *s++;
         return(hashval % MAXSYM);
      }

#if DEBUG
   dump_symtab()
      {
         int i;
         cell *c;

         for (i = 0; i < MAXSYM; i++){
            if (symtab[i] != NULL)
               fprintf(stderr, "hash location %d\n", i);
            for (c = symtab[i]; c != NULL; c = c->c_chain)
               fprintf(stderr, "   ...%s\n", c->c_sval);
            }
      }
#endif /* DEBUG */
