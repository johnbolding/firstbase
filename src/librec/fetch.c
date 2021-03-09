/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: fetch.c,v 9.0 2001/01/09 02:56:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Fetch_sid[] = "@(#) $Id: fetch.c,v 9.0 2001/01/09 02:56:57 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_use_rpc;

/* 
 * fetch -  fetch the fb_field described by k from the buffer into s.
 *	this allows walking on the fb_field without disturbing data.
 *
 */
 
   fb_fetch(k, s, dp)
      fb_field *k;
      char *s;
      fb_database *dp;
   
      {
         fb_link *ak;

#if RPC
         if (cdb_use_rpc)
            return(fb_fetch_clnt(k, s, dp));
#endif /* RPC */
         if (k->type != FB_FORMULA && k->type != FB_BINARY)
	    strcpy(s, k->fld);
         else if (k->type == FB_BINARY)
            memcpy(s, k->fld, (unsigned) k->size);
	 else if (k->dflink == NULL)
	    fb_getformula(k, k->idefault, s, 0, dp);
	 else{ /* must be a FB_FORMULA link */
	    ak = k->dflink;
	    fb_getformula(ak->f_ffp, ak->f_ffp->idefault, ak->f_fld, 
	       0, ak->f_dp);
	    ak->f_tfp->fld = ak->f_fld;		/* same as k->fld == ... */
	    strcpy(s, ak->f_tfp->fld);
	    }
         return(FB_AOK);
      }
