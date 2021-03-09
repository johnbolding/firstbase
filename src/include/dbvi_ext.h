/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbvi_ext.h,v 9.0 2001/01/09 02:56:10 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* external variables for dbvcalc go here */

#include <dbvi.h>
#include <fb_ext.h>
#include <vdict_e.h>

extern fb_database *hp;		/* to point to the global database */

extern char
     com[],			/* command storage area */
     msg[],			/* message for scrhlp and scrstat */
     mode;			/* for autoadd mode */

extern short
     scanner,			/* set to 1 if dbscan, else 0 */
     st,			/* status marker */
     def,			/* type of default ? */
     autodef;			/* for auto default feature */
     
extern long 
     rec,			/* record marker */
     oldrec,			/* last record */
     irec[],			/* to mark index record levels */
     ibase[];			/* to mark index base levels */
     
/* additions added for the dbvcalc part of the tool */

extern char cdict[];

extern column **gcolumn;		/* array of columns */

extern column *col_mhead, *col_mtail;
extern column *col_phead, *col_ptail;
extern column *col_current;
extern crec *crec_mhead, *crec_mtail;
extern crec *crec_phead, *crec_ptail;
extern crec *crec_current;
extern int    ncolumns;			/* maximum columns in calc view */
extern int    ncur_column;
extern int    ncur_crec;
extern int    c_bufsize;		/* size of a crec buffer */
extern int    calc_row;			/* first calc row */

extern int    modified;			/* modified flag */
extern int    clear_lastline;		/* kludge to clear error messages */

extern char cdb_KU[];
extern char cdb_KD[];
extern char cdb_KL[];
extern char cdb_KR[];

extern int fullsize, halfsize;
extern column *col_leftcorn;
extern crec *crec_leftcorn;
