/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: fbserv_c.c,v 9.0 2001/01/09 02:56:34 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Fbserver_clnt_sid[] = "@(#) $Id: fbserv_c.c,v 9.0 2001/01/09 02:56:34 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>
#include <sys/time.h>

extern short int cdb_locktime;

/* Default timeout can be changed using clnt_control() */
static struct timeval FB_TIMEOUT = { 60, 0 };

fb_varvec *
fb_server_1(argp, clnt)
	fb_varvec *argp;
	CLIENT *clnt;
{
	static fb_varvec res;

        fb_clear_vec(&res);
        FB_TIMEOUT.tv_sec = cdb_locktime + 10;

	if (clnt_call(clnt, FBSERVER, fb_xdr_varvec, (char *) argp,
              fb_xdr_varvec, (char *) &res, FB_TIMEOUT) != RPC_SUCCESS) {
	   return (NULL);
	}
	return (&res);
}

#endif /* RPC */
