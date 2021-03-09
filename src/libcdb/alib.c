/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: alib.c,v 9.0 2001/01/09 02:56:24 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Alib_sid[] = "@(#) $Id: alib.c,v 9.0 2001/01/09 02:56:24 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <mdict.h>
#include <mdict_e.h>

/*
 * fb_aline library routines
 */

/*
 * makeline - create a line structure (fb_aline), zero it out, return it.
 * these routines are used by dbdview and dbmerge, as well as libinit/readmrg.c
 */

   fb_aline *fb_makeline()

      {
         fb_aline *a;
	 
	 a = (fb_aline *) fb_malloc((unsigned) sizeof(fb_aline));
         a->a_text = fb_malloc((unsigned) linewidth + 1);
         fb_pad(a->a_text, NIL, linewidth);
         a->a_thead = NULL;			/* tokens in this line */
         a->a_ttail = NULL;			/* tokens in this line */
         a->a_next = NULL;			/* next fb_aline */
         a->a_prev = NULL;			/* previous fb_aline */
	 return(a);
      }

/*
 * freeline - free an allocated line
 */

   fb_freeline(a)
      fb_aline *a;

      {
         fb_free(a->a_text);
         fb_freetokens(a->a_thead);
         fb_free((char *) a);
      }

/*
 * freealist - free an alist
 */

   fb_freealist(a)
      fb_aline *a;

      {
         fb_aline *na;

         for (; a != NULL; a = na){
            na = a->a_next;
            fb_freeline(a);
            }
      }

/*
 * copyalist - copy count items of the alist, link into a page p
 */

   fb_aline *fb_copyalist(a, count, p)
      fb_aline *a;
      int count;
      fb_mpage *p;

      {
         fb_aline *na;

         for (; a != NULL; a = a->a_next){
            na = fb_makeline();
            /* duplicate the token list here */
            fb_copytlist(na, a->a_thead);
            strcpy(na->a_text, a->a_text);
            fb_insert_line(na, (fb_aline *) NULL, p);
            if (count != -1){
               if (--count == 0)
                  break;
               }
            }
         return(p->mp_ahead);
      }

/*
 * insert_line - insert line in the current page in front of aa
 *	if aa == NULL, then append beyond tail.
 */

   fb_insert_line(a, aa, p)
      fb_aline *a, *aa;
      fb_mpage *p;
      
      {
         if (aa == NULL){		/* append beyond tail */
	    if (p->mp_atail == NULL)
	       p->mp_atail = p->mp_ahead = a;
	    else{
	       p->mp_atail->a_next = a;
	       a->a_prev = p->mp_atail;
	       a->a_next = NULL;
	       p->mp_atail = a;
	       }
	    }
	 else if (aa == p->mp_ahead){	/* insert before ahead */
	    a->a_prev = NULL;
	    a->a_next = p->mp_ahead;
	    p->mp_ahead->a_prev = a;
	    p->mp_ahead = a;
	    }
	 else{				/* normal case */
	    a->a_next = aa;
	    a->a_prev = aa->a_prev;
	    aa->a_prev->a_next = a;
	    aa->a_prev = a;
	    }
      }

/*
 * delete_line - delete line in the page p.
 */

   fb_delete_line(a, p)
      fb_aline *a;
      fb_mpage *p;
      
      {
         if (a == p->mp_ahead)
            p->mp_ahead = a->a_next;
         if (a == p->mp_atail)
            p->mp_atail = a->a_prev;
         if (a->a_prev != NULL)
            a->a_prev->a_next = a->a_next;
         if (a->a_next != NULL)
            a->a_next->a_prev = a->a_prev;
         fb_freeline(a);
         if (p->mp_ahead == NULL)
            p->mp_ahead = p->mp_atail;
         if (p->mp_atail == NULL)
            p->mp_atail = p->mp_ahead;
         if (p->mp_ahead == NULL && p->mp_atail == NULL){
            /* empty list -- force a single element */
            a = fb_makeline();
            fb_insert_line(a, NULL, p);
            p->mp_acur = mpcur->mp_ahead;
            p->mp_atop = mpcur->mp_ahead;
            p->mp_col = 1;
            leftcorn = mpcur->mp_leftcorn;
            atop = mpcur->mp_atop;
            }
      }
