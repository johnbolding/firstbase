/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: atof.c,v 9.0 2001/01/09 02:56:18 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Atof_sid[] = "@(#) $Id: atof.c,v 9.0 2001/01/09 02:56:18 john Exp $";
#endif

#include <fb.h>

/*
 * ATOF - get around buggy SCO atof
  */

#undef atof
double ATOF(s)
   char *s;

   {
      double atof();

      while (*s && (*s == ' ' || *s == '\t'))
         s++;
      if (*s == 'n' || *s == 'N')
         return((double) 0);
      return(atof(s));
   }
