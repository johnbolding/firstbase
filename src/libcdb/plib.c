/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: plib.c,v 9.0 2001/01/09 02:56:28 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Plib_sid[] = "@(#) $Id: plib.c,v 9.0 2001/01/09 02:56:28 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <mdict.h>
#include <mdict_e.h>

/*
 * (merge) page library routines
 */

/*
 * make_mpage - create a page structure (page), zero it out, return it.
 */

   fb_mpage *fb_make_mpage()
      {
         fb_mpage *p;
	 
	 p = (fb_mpage *) fb_malloc(sizeof(fb_mpage));
         p->mp_num = 0;			/* page number */
         p->mp_row = 0;
         p->mp_col = 0;
         p->mp_leftcorn = 0;
         p->mp_rightcorn = 0;
         p->mp_ahead = NULL;		/* top aline of page */
         p->mp_atail = NULL;		/* bottom aline of page */
         p->mp_atop = NULL;		/* dispay top of this page */
         p->mp_abot = NULL;		/* dispay top of this page */
         p->mp_acur = NULL;		/* current line for this page */
         p->mp_next = NULL;		/* next page */
         p->mp_prev = NULL;		/* previous page */
	 return(p);
      }

/*
 * freepage - free a page record.
 */

   fb_freepage(p)
      fb_mpage *p;

      {
         fb_freealist(p->mp_ahead);
	 fb_free((char *) p);
      }

/*
 * insert_page - insert page record o in front of page ap.
 *	if ap == NULL, then append beyond tail.
 */

   fb_insert_page(p, ap)
      fb_mpage *p, *ap;
      
      {
         if (ap == NULL){		/* append beyond tail */
	    if (mptail == NULL)
	       mptail = mphead = p;
	    else{
	       mptail->mp_next = p;
	       p->mp_prev = mptail;
	       p->mp_next = NULL;
	       mptail = p;
	       }
	    }
	 else if (ap == mphead){		/* insert before head */
	    p->mp_prev = NULL;
	    p->mp_next = mphead;
	    mphead->mp_prev = p;
	    mphead = p;
	    }
	 else{				/* normal case */
	    p->mp_next = ap;
	    p->mp_prev = ap->mp_prev;
	    ap->mp_prev->mp_next = p;
	    ap->mp_prev = p;
	    }
      }

