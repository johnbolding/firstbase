/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: panic.h,v 9.0 2001/01/09 02:56:16 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/*
 *  this is used to be able to panic out of a program at two places:
 *     - for input() stuff, 5 ^F's and PANIC_PWD will end.
 *     - for HIT ANY KEY with a ':' on the end, use ^B and PANIC_PWD
 */

#define PANIC 1
#define PANIC_THRESH 5			/* panic threshhold */
#define PANIC_PWD "wizzardshit"		/* panic password */

#if PANIC
static int panic_count = 0;		/* for counting panic signals */
static char panic_area[80] = {NULL};	/* for passwd */
#endif
