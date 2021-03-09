/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: initu.c,v 9.0 2001/01/09 02:55:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Initu_sid[] = "@(#) $Id: initu.c,v 9.0 2001/01/09 02:55:57 john Exp $";
#endif

/*
 *  initu.c - library for dbugen and dbdupd
 */

#include <fb.h>
#include <fb_ext.h>
#include <dbd.h>

/* 
 *  read_idictu - read in an existing idictu file for editing 
 */
 
   read_idictu(fname, itemlist)
      char *fname;
      fb_upd **itemlist;
      
      {
         char line[FB_MAXLINE], word[FB_MAXLINE];
	 fb_upd *up;
	 fb_field *fp;
	 int p, q;
	 FILE *fs;
	 
	 if ((fs = fopen(fname, "r")) == NULL)
	    return(FB_ERROR);
	 q = 0;
	 while (fgets(line, FB_MAXLINE, fs) != NULL){
	    if (line[0] == '%')
	       break;
	    q++;
	    p = fb_getword(line, 1, word);
	    fb_underscore(word, 0);
	    if ((fp = fb_findfield(word, cdb_db)) == NULL)
	       fb_xerror(FB_BAD_DICT, word, NIL);
	    up = (fb_upd *) fb_malloc(sizeof(fb_upd));
	    up->fp = fp;
	    p = fb_getword(line, p, word);
	    fb_underscore(word, 0);	/* replace underscores with blanks */
	    up->fv = (char *) fb_malloc(strlen(word)+1);
	    strcpy(up->fv, word);
	    itemlist[q] = up;
	    }
	 if (line[0] != '%')
	    fb_xerror(FB_BAD_DICT, "no terminator", NIL);
	 fclose(fs);
	 return(q);
      }
