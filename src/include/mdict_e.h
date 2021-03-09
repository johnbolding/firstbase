/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: mdict_e.h,v 9.0 2001/01/09 02:56:15 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* external variables for merge dict items go here */

extern fb_mpage *mphead, *mptail;/* page head and tail */
extern fb_mpage *mpcur;

extern fb_token *token_cur;	/* current token */

extern int linewidth;		/* width of the line storage */
extern int base_left;		/* left edge of display window */
extern int base_right;		/* right edge of display window */
extern int base_length;		/* length of display line */
extern int base_top;		/* top row of base display */
extern int base_bottom;		/* bottom row of base display */

extern int leftcorn;		/* global left corner */

extern fb_aline *atop;

extern fb_mpage *killpage;	/* kill page */
extern fb_aline *killbuffer;	/* kill buffer */
extern fb_mpage *copypage;	/* copy page */
extern fb_aline *copybuffer;	/* copy buffer */
