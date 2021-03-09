/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbdmrg_v.h,v 9.0 2001/01/09 02:56:08 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* global variables for dbdmrg go here */

#include <dbdmrg.h>
#include <fb_vars.h>
#include <mdict_v.h>

fb_database *hp;		/* to point to the global database */

char
     msg[FB_MAXLINE] = { NULL },/* message for scrhlp and scrstat */
     mode = NULL;		/* for autoadd mode */

int clear_lastline = 0;		/* kludge to clear error messages */
int modified;
int st;
int lastline;			/* mark the last line of the screen */
