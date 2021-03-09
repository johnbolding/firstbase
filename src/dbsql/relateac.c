/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: relateac.c,v 9.0 2001/01/09 02:55:50 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Relatewalk_sid[] = "@(#) $Id: relateac.c,v 9.0 2001/01/09 02:55:50 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * relateeach - foreach relation record, load all records and call function.
 *	relations are 0 based: 0 .. r->r_reccnt - 1
 */

   relateeach(r, f)
      relation *r;
      int (*f)();

      {
         long relrec;

         for (relrec = 0; relrec < r->r_reccnt; relrec++){
            getrec_relation(relrec, r);
            getrec_loadrel(r);
            if (((*f)(r)) == FB_ERROR)
               return(FB_ERROR);
            }
         return(FB_AOK);
      }
