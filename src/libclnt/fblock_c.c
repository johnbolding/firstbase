/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: fblock_c.c,v 9.0 2001/01/09 02:56:33 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Fblockd_clnt_sid[] = "@(#) $Id: fblock_c.c,v 9.0 2001/01/09 02:56:33 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fbserver.h>
#include <sys/time.h>

/* Default timeout can be changed using clnt_control() */
static struct timeval FB_TIMEOUT = { 25, 0 };

fb_varvec *
fblockd_1(argp, clnt)
	fb_varvec *argp;
	CLIENT *clnt;
{
	static fb_varvec res;

        fb_clear_vec(&res);
	if (clnt_call(clnt, FBLOCKD, fb_xdr_varvec, (char *) argp,
              fb_xdr_varvec, (char *) &res, FB_TIMEOUT) != RPC_SUCCESS) {
	   return (NULL);
	}
	return (&res);

}

#endif /* RPC */
