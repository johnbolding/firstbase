/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: ixalloc.c,v 9.0 2001/01/09 02:56:48 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Ixalloc_sid[] = "@(#) $Id: ixalloc.c,v 9.0 2001/01/09 02:56:48 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/*
 *  ixalloc - create a fb_autoindex space and null out all the nasties.
 */

   fb_autoindex *fb_ixalloc()
      
      {
         fb_autoindex *ix;
	 
         ix = (fb_autoindex *) fb_malloc(sizeof(fb_autoindex));
         ix->autoname = NULL;
         ix->dup_fld = NULL;
         ix->afd = -1;
         ix->hfd = -1;
         ix->uniq = -1;
         ix->ix_tree = 0;
         ix->ix_seq = NULL;
         ix->ix_seqtmp = NULL;
         ix->ix_idx = NULL;
         ix->ix_idxtmp = NULL;
         ix->ix_ip = NULL;
         ix->ix_bsmax = 0;
         ix->ix_bsend = 0;
         ix->ix_ifields = 0;
         ix->ix_key_fld = NULL;
	 return(ix);
      }
