/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: tlib.c,v 9.0 2001/01/09 02:56:31 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Tlib_sid[] = "@(#) $Id: tlib.c,v 9.0 2001/01/09 02:56:31 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <mdict.h>
#include <mdict_e.h>

/*
 * token library routines
 */

/*
 * maketoken - create a token structure (token), zero it out, return it.
 */

   fb_token *fb_maketoken()

      {
         fb_token *t;
	 
	 t = (fb_token *) fb_malloc(sizeof(fb_token));
         t->t_field = NULL;
         t->t_width = 0;
         t->t_next = NULL;			/* next token */
         t->t_prev = NULL;			/* previous token */

         t->t_text = NULL;
         t->t_sub1 = 0;
         t->t_sub2 = 0;
         t->t_reverse = 0;
         t->t_readonly = 0;
         t->t_type = 0;

	 return(t);
      }

/*
 * freetokens - free a token list.
 */

   fb_freetokens(t)
      fb_token *t;

      {
         fb_token *nt;

         for (; t != NULL; t = nt){
            nt = t->t_next;
            if (t->t_text != NULL)
               fb_free(t->t_text);
            fb_free((char *) t);
            }
      }

/*
 * copytlist - copy the token list t into the line a
 */

   fb_copytlist(a, t)
      fb_aline *a;
      fb_token *t;

      {
         fb_token *nt = NULL;

         for (; t != NULL; t = t->t_next){
            nt = fb_maketoken();
            *nt = *t;		/* structure assignment, except for text */
            if (t->t_text != NULL)
               fb_mkstr(&(nt->t_text), t->t_text);
            fb_insert_token(nt, (fb_token *) NULL, a);
            }
      }

/*
 * insert_token - insert token in the line in front of at
 *	if at == NULL, then append beyond tail.
 */

   fb_insert_token(t, at, a)
      fb_token *t, *at;
      fb_aline *a;
      
      {
         if (at == NULL){		/* append beyond tail */
	    if (a->a_ttail == NULL)
	       a->a_ttail = a->a_thead = t;
	    else{
	       a->a_ttail->t_next = t;
	       t->t_prev = a->a_ttail;
	       t->t_next = NULL;
	       a->a_ttail = t;
	       }
	    }
	 else if (at == a->a_thead){	/* insert before ahead */
	    t->t_prev = NULL;
	    t->t_next = a->a_thead;
	    a->a_thead->t_prev = t;
	    a->a_thead = t;
	    }
	 else{				/* normal case */
	    t->t_next = at;
	    t->t_prev = at->t_prev;
	    at->t_prev->t_next = t;
	    at->t_prev = t;
	    }
      }

/*
 * delete_token - delete token in the line a.
 */

   fb_delete_token(t, a)
      fb_token *t;
      fb_aline *a;
      
      {
         if (a->a_thead == t)
            a->a_thead = t->t_next;
         if (a->a_ttail == t)
            a->a_ttail = t->t_prev;
         if (t->t_prev != NULL)
            t->t_prev->t_next = t->t_next;
         if (t->t_next != NULL)
            t->t_next->t_prev = t->t_prev;
         if (t->t_text != NULL)
            fb_free(t->t_text);
         fb_free((char *) t);
      }
