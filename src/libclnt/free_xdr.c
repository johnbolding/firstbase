/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: free_xdr.c,v 9.0 2001/01/09 02:56:34 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Free_xdr_sid[] = "@(#) $Id: free_xdr.c,v 9.0 2001/01/09 02:56:34 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

/*
 * fb_free_xdr_vec - fb_free up the nfs xdr allocated storage
 */

   fb_free_xdr_vec(vp)
      fb_varvec *vp;

      {
         if (vp == NULL)
            return;
         xdr_free(fb_xdr_varvec, (char *) vp);
      }

#endif /* RPC */
