/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbve_v.h,v 9.0 2001/01/09 02:56:09 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* global variables for dbvedit and dbvemit go here */

#include <dbve.h>
#include <fb_vars.h>
#include <vdict_v.h>

fb_database *hp;		/* to point to the global database */

char
     com[FB_MAXLINE] = { NULL },/* command storage area */
     msg[FB_MAXLINE] = { NULL },/* message for scrhlp and scrstat */
     mode = NULL;		/* for autoadd mode */
     
short          
     globaladd = 0,		/* add mode only flag -- from arg line */
     scanner = 0,		/* set to 1 if dbview, else 0 */
     st = 0,			/* status marker */
     pindx = 0,			/* level of index */
     simple_pindx = 0,		/* simple fld pointer when no index used */
     def = 0,			/* type of default ? */
     autodef = 0;		/* for auto default feature */

long 
     rec = 0L,			/* record marker */
     oldrec = 0L,		/* last record */
     irec[FB_MAXIDXS] = { 0L },	/* to mark index record levels */
     ibase[FB_MAXIDXS] = { 0L };/* to mark index base levels */
