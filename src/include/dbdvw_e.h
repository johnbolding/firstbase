/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbdvw_e.h,v 9.0 2001/01/09 02:56:08 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* external variables for dbdmrg go here */

#include <dbdview.h>
#include <fb_ext.h>
#include <vdict_ext.h>
#include <mdict_ext.h>

extern fb_database *hp;		/* to point to the global database */

extern char
     msg[],			/* message for scrhlp and scrstat */
     mode;			/* for autoadd mode */

extern char cdb_KU[];
extern char cdb_KD[];
extern char cdb_KL[];
extern char cdb_KR[];

extern int clear_lastline;	/* kludge to clear error messages */
extern int modified;
extern int st;
extern int lastline;		/* mark the last line of the screen */
