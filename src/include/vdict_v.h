/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: vdict_v.h,v 9.0 2001/01/09 02:56:18 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* global variables for vdict items go here */

char vdict[FB_MAXNAME];

fb_page *phead = NULL,		/* page head and tail */ 
     *ptail = NULL,
     *pcur  = NULL;		/* current page */

fb_node *ncur  = NULL;		/* current node */

fb_node *dot;			/* current field location */

