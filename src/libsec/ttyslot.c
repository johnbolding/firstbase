/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: ttyslot.c,v 9.2 2001/11/18 18:10:19 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Ttyslot_sid[] = "@(#) $Id: ttyslot.c,v 9.2 2001/11/18 18:10:19 john Exp $";
#endif

#include <fb.h>
#include <dbutmp.h>
#if !HAVE_TTYSLOT
#include <utmp.h>
extern struct utmp *getutent();
#endif

   int TTYSLOT()
      {
#if !HAVE_TTYSLOT
         struct utmp *u;
         char *p, *q;
         int slot;

         setutent();
         p = ttyname(2);
         q = strrchr(p, '/');
         if (q != 0)
            q++;
         else
            q = p;
         for (slot = 0;;slot++){
            u = getutent();
            if (u == (struct utmp *) NULL){
               slot = -1;
               break;
               }
            if (strcmp(q, u->ut_line) == 0)
               break;
            }
         endutent();
         return(slot);
#else
         return(ttyslot());
#endif
      }
