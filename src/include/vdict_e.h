/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: vdict_e.h,v 9.0 2001/01/09 02:56:17 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* external variables for vdict items go here */

extern char vdict[];

extern fb_page *phead, *ptail;	/* page head and tail */
extern fb_page *pcur;
extern fb_node *ncur;

extern fb_node *dot;		/* current location of index pointer */
