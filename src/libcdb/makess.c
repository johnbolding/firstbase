/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: makess.c,v 9.0 2001/01/09 02:56:27 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Makess_sid[] = "@(#) $Id: makess.c,v 9.0 2001/01/09 02:56:27 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short cdb_datedisplay;

static char *local_bfld = NULL;

/*
 * makess - make a search string from the input string and type.
 */
 
   fb_makess(buf, type, size)
      char *buf, type;
      int size;
      
      {
         char buf1[12], *p;

         p = cdb_bfld;
         if (p == NULL){
            if (local_bfld == NULL)
               local_bfld = (char *) fb_malloc(1024);
            p = local_bfld;
            }
         strcpy(p, buf);
	 if (FB_OFNUMERIC(type)){
	    fb_formfield(buf, p, type, size);
            /* special case - fed a NULL to formfield so pad to size */
            if (buf[0] == NULL)
	       fb_pad(buf, p, size);
            }
	 else if (type == FB_DATE && size == 6){
            if (cdb_datedisplay == 8)
               fb_endate(buf);
            else{
               fb_longdate(buf1, buf);
               strcpy(buf, buf1);
               fb_long_endate(buf);
               }
            }
	 else
	    fb_pad(buf, p, size);
         return(FB_AOK);
      }
