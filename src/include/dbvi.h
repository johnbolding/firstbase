/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbvi.h,v 9.0 2001/01/09 02:56:10 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* global definitions for dbvcalc go here */

#include <fb.h>
#include <vdict.h>

/* additions added for the dbvcalc part of the tool */

/*
 * the idea is to keep a double link list of crecs.
 * size and shape of structures is calculated on the fly.
 */

typedef struct s_crec crec;	/* cell records */
struct s_crec {
   char **c_cell;		/* individual cell ptrs */
   char *c_buf;			/* space for cells */
   crec *c_next;		/* ptrs to next and prev for link lists */
   crec *c_prev;
   };

typedef struct p_column column;	/* pillars or columns definitions */
struct p_column {
   fb_field *p_field;		/* field ptr for this column */
   int    p_ioloc;		/* input/ouput location */
   char  *p_label;		/* column label */
   int    p_width;		/* total column width */
   int    p_array;		/* array location of this column */
   column *p_prev;
   column *p_next;
   };

#define FIRST_IOLOC	7
