/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: rjustify.c,v 9.0 2001/01/09 02:56:29 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Rjustify_sid[] = "@(#) $Id: rjustify.c,v 9.0 2001/01/09 02:56:29 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/* 
 *  rjustify - right justify t of type dollar/float/[Nn]umeric into s.
 *	pad or truncate s to size. return s. for display/lookup only.
 */
 
   char *fb_rjustify(s, t, size, type)
      char s[], t[];
      int size, type;
      
      {
         char line[120], *fb_trim(char *s), *fb_rmlead(char *s);
         register int i, j, k;
	 int sz;
         
	 if (type)
            t = fb_trim(fb_rmlead(t));
	 sz = strlen(t);
	 if (sz > size){
	    if (type == FB_DOLLARS)
	       size--;
	    t[size] = NULL;
	    sz = size;
	    }
         i = sz - 1;
	 if (type == FB_DOLLARS)
	    i--;			/* provides room for decimal */
         if (i <= 0){
	    j = i = 0;
	    if (type == FB_DOLLARS)
	       line[j++] = CHAR_0;	/* special case "", "n", "nn" */
	    }
	 else{
	    for (j = 0; j < i; j++)
	       line[j] = t[j];
	    }
         k = i;
         if (type == FB_DOLLARS){
            line[j++] = CHAR_DOT;
	    if (t[k] == NULL){		/* special case - "" */
	       line[j++] = CHAR_0;
	       line[j++] = CHAR_0;
	       }
	    else if (t[k + 1] == NULL)	/* special case - "n" */
	       line[j++] = CHAR_0;
	    }
         for (; t[k] != NULL; line[j++] = t[k++])
            ;
         line[j] = NULL;
         for (k = 0, i = strlen(line); i < size; i++)
            s[k++] = CHAR_BLANK;
         s[k] = NULL;
         return(strcat(s, line));
      }
