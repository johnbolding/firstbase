/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbvi_v.h,v 9.0 2001/01/09 02:56:10 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* global variables for dbvcalc go here */

#include <dbvi.h>
#include <fb_vars.h>
#include <vdict_v.h>

fb_database *hp;		/* to point to the global database */

char
     com[FB_MAXLINE] = { NULL },/* command storage area */
     msg[FB_MAXLINE] = { NULL },/* message for scrhlp and scrstat */
     mode = NULL;		/* for autoadd mode */
     
short          
     scanner = 0,		/* set to 1 if dbview, else 0 */
     st = 0,			/* status marker */
     def = 0,			/* type of default ? */
     autodef = 0;		/* for auto default feature */

long 
     rec = 0L,			/* record marker */
     oldrec = 0L,		/* last record */
     irec[FB_MAXIDXS] = { 0L },	/* to mark index record levels */
     ibase[FB_MAXIDXS] = { 0L };/* to mark index base levels */

/* additions added for the dbvcalc part of the tool */

char cdict[FB_MAXNAME];

column **gcolumn;		/* array of columns */

column *col_mhead, *col_mtail;
column *col_phead, *col_ptail;
column *col_current;
crec *crec_mhead, *crec_mtail;
crec *crec_phead, *crec_ptail;
crec *crec_current;

int    ncolumns = 0;		/* maximum columns in calc view */
int    ncur_column = 0;
int    ncur_crec = 0;
int    c_bufsize = 0;		/* size of a crec buffer */
int    calc_row;		/* first calc row */

int    modified = 0;		/* modified flag */
int    clear_lastline = 0;	/* kludge to clear error messages */

int fullsize, halfsize;
column *col_leftcorn;
crec *crec_leftcorn;

