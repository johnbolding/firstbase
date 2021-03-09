/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: secure.h,v 9.0 2001/01/09 02:56:17 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/*
 * secure permission settings
 *	use x |= T_VAL to turn on, x &= ~T_VAL to turn off.
 */

#define PO_READ		0004		/* other read */
#define PO_WRITE 	0002		/* other write */
#define PG_READ 	0040		/* group read */
#define PG_WRITE	0020		/* group write */
#define PU_READ		0400		/* user read */
#define PU_WRITE	0200		/* user write */

#define isperm_o_read(x) 	(((x & PO_READ) == 0) ? 0 : 1)
#define isperm_o_write(x) 	(((x & PO_WRITE) == 0) ? 0 : 1)
#define isperm_g_read(x) 	(((x & PG_READ) == 0) ? 0 : 1)
#define isperm_g_write(x)	(((x & PG_WRITE) == 0) ? 0 : 1)
#define isperm_u_read(x)	(((x & PU_READ) == 0) ? 0 : 1)
#define isperm_u_write(x) 	(((x & PU_WRITE) == 0) ? 0 : 1)
