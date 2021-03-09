/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: freeglob.c,v 9.0 2001/01/09 02:56:46 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Fb_free_globals_sid[] = "@(#) $Id: freeglob.c,v 9.0 2001/01/09 02:56:46 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_t_irecsiz;
extern short int cdb_use_rpc;

/*
 * fb_free_globals - frees up all global memory and resets
 *	some global variables used when opening multiple databases.
 */

   fb_free_globals()
      {
#if RPC
         if (cdb_use_rpc && fb_clnt_ping() == FB_AOK)
            fb_free_globals_clnt();
         /*
          * unlike other RPC calls, this one falls through and does
          * a free_globals on the local machine also.
          */
#endif /* RPC */
         fb_free(cdb_afld);
         fb_free(cdb_bfld);
         cdb_afld = NULL;
         cdb_bfld = NULL;
         cdb_fieldlength = 0;
         cdb_t_irecsiz = 0;
         return(FB_AOK);
      }
