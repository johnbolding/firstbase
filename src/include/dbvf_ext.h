/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbvf_ext.h,v 9.0 2001/01/09 02:56:09 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* external variables for dbvform go here */

#include <dbvform.h>
#include <fb_ext.h>
#include <vdict_e.h>

extern fb_database *hp;		/* to point to the global database */

extern fscreen vfscr;

extern int p_lines;		/* page length */
extern int p_cols;		/* page width */
extern int formfeed_flag;	/* formfeed flag */
