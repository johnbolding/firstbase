/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: store.c,v 9.0 2001/01/09 02:57:02 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Store_sid[] = "@(#) $Id: store.c,v 9.0 2001/01/09 02:57:02 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_use_rpc;

/* 
 *  store - put fb_field s into hp->arec (alternate record area).
 *	k->loc determines position to put s.
 */
 
   fb_store(k, s, hp)
      fb_field *k;
      register char *s;
      fb_database *hp;
   
      {
	 register char *p;
         int st;
   
#if RPC
         if (cdb_use_rpc){
            st = fb_store_clnt(k, s, hp);
            /*
             * unlike most other RPC hooks, this one drops through
             * so that the field on the client is updated also
             */
            if (st != FB_AOK)
               return(st);
            }
#endif /* RPC */
         if (k->type == FB_FORMULA || k->dflink != NULL ||
               k->type == FB_LINK)
	    return(FB_AOK);
         /* set fb_field pointer to data */
	 k->fld = hp->arec + k->loc;
         if (k->type != FB_BINARY){		/* NULL terminated copy */
            for (p = k->fld; ; s++, p++){
               *p = *s;
               if (!(*s))
                  break;
	       }
            }
         else					/* FB_BINARY byte copy */
            memcpy(k->fld, s, (unsigned) k->size);
         return(FB_AOK);
      }
