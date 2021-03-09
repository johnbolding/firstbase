/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: lserve_c.c,v 9.0 2001/01/09 02:56:35 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Lserver_clnt_sid[] = "@(#) $Id: lserve_c.c,v 9.0 2001/01/09 02:56:35 john Exp $";
#endif

#if RPC

/* the little stub here was created by rpcgen ... i added the rest */

#include <fb.h>
#include <rpc/rpc.h>
#include <sys/time.h>
#include <fblserve.h>

/* Default timeout can be changed using clnt_control() */
static struct timeval FB_TIMEOUT = { 25, 0 };

char **
fb_lserver_1(argp, clnt)
	char **argp;
	CLIENT *clnt;
{
	static char *res;

	res = NULL;
	if (clnt_call(clnt, CDBSERVER, xdr_wrapstring, (char *) argp,
	      xdr_wrapstring, (char *) &res, FB_TIMEOUT) != RPC_SUCCESS) {
	   return (NULL);
	}
	return (&res);
}
#endif /* RPC */
