/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbvf_v.h,v 9.0 2001/01/09 02:56:10 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* global variables for dbvform go here */

#include <dbvform.h>
#include <fb_vars.h>
#include <vdict_v.h>

fb_database *hp;			/* to point to the global database */

fscreen vfscr;

int p_lines;			/* page length */
int p_cols;			/* page width */
int formfeed_flag = 0;		/* formfeed flag */
