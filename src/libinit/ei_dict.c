/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: ei_dict.c,v 9.0 2001/01/09 02:56:46 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Emptyi_idict_sid[] = "@(#) $Id: ei_dict.c,v 9.0 2001/01/09 02:56:46 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/* 
 *  emptyi_dict - load an empty index dictionary. define everything.
 */
 
   fb_emptyi_dict(dhp)
      fb_database *dhp;

      {
         dhp->ifd = -1;
         dhp->ihfd = -1;
	 dhp->irecsiz = -1;
	 if (dhp->ip != NULL)
	    fb_free((char *) dhp->ip);
	 cdb_ip = cdb_keyindx = dhp->ip =
            (fb_field **) fb_malloc(2 * sizeof(fb_field *));
         dhp->ip[0] = dhp->kp[0];	/* simulate first fb_field as index */
         dhp->ifields = 2;
         dhp->bsmax = 0L;
         dhp->bsend = 0L;
	 fb_mkstr(&(dhp->dindex), SYSMSG[S_NO_INDEX]);

         if (dhp->b_seq != NULL){
            dhp->b_seq->bs_head = 0;
            dhp->b_seq->bs_tail = 0;
            }
         if (dhp->b_idx != NULL)
            dhp->b_idx->bi_root = 0;
      }
