/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: mdict_v.h,v 9.0 2001/01/09 02:56:16 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* global variables for mdict items go here */

fb_mpage *mphead = NULL,	/* page head and tail */ 
     *mptail = NULL,
     *mpcur  = NULL;		/* current page */

fb_token *token_cur = NULL;	/* current token */

int linewidth;			/* widht of the line storage */
int base_left = 0;		/* left edge of display window */
int base_right = 80;		/* right edge of display window */
int base_length;		/* length of display line */
int base_top = 3;		/* top row of base display */
int base_bottom;		/* bottom row of base display */

int leftcorn;			/* global left corner */
fb_aline *atop;			/* global top line */

fb_mpage *killpage;		/* kill page */
fb_aline *killbuffer;		/* kill buffer */
fb_mpage *copypage;		/* copy page */
fb_aline *copybuffer;		/* copy buffer */


