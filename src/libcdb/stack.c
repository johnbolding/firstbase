/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: stack.c,v 9.0 2001/01/09 02:56:30 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Stack_sid[] = "@(#) $Id: stack.c,v 9.0 2001/01/09 02:56:30 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/*
 * stack routines - used to push and store page numbers, seek file locales, etc.
 */

#define MAXSTACK 200
long cdb_seekstack[MAXSTACK];
short int cdb_topstack;

/*
 * push and pop routines for keeping track of pages
 */

   fb_push(v)
      long v;

      {
         cdb_seekstack[cdb_topstack] = v;
         if (++cdb_topstack >= MAXSTACK){
            fb_serror(FB_MESSAGE, "Too many stacked values - push", NIL);
            cdb_topstack = 0;
            }
      }

   long fb_pop()
      {
         if (--cdb_topstack < 0){
            fb_serror(FB_MESSAGE, "Too few stacked values - pop", NIL);
            cdb_topstack = 0;
            }
         return(cdb_seekstack[cdb_topstack]);
      }

   fb_initstack()
      {
         cdb_topstack = 0;
      }
