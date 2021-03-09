/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbve_ext.h,v 9.0 2001/01/09 02:56:09 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* external variables for dbvi go here */

#include <dbve.h>
#include <fb_ext.h>
#include <vdict_e.h>

extern fb_database *hp;		/* to point to the global database */

extern char
     com[],			/* command storage area */
     msg[],			/* message for scrhlp and scrstat */
     mode;			/* for autoadd mode */

extern short
     globaladd,			/* add mode only flag -- from arg line */
     scanner,			/* set to 1 if dbscan, else 0 */
     st,			/* status marker */
     pindx,			/* level of index */
     simple_pindx,		/* simple fld pointer when no index used */
     def,			/* type of default ? */
     autodef;			/* for auto default feature */
     
extern long 
     rec,			/* record marker */
     oldrec,			/* last record */
     irec[],			/* to mark index record levels */
     ibase[];			/* to mark index base levels */
     
extern fb_field **cdb_sp;	/* screen filter -- for CCOMANDS */
extern short int cdb_sfields;	/* max screen fields */
