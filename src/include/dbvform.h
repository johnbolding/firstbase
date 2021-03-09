/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbvform.h,v 9.0 2001/01/09 02:56:10 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* global definitions for dbvform go here */

#include <fb.h>
#include <vdict.h>

#define F_PROW        66
#define F_PCOL  132

struct s_fscreen {
   char line[F_PROW][F_PCOL];
   int  s_ccount[F_PROW + 1];
   int s_x, s_y;
   };

typedef struct s_fscreen fscreen;
