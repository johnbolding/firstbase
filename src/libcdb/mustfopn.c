/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mustfopn.c,v 9.0 2001/01/09 02:56:27 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Mustfopen_sid[] = "@(#) $Id: mustfopn.c,v 9.0 2001/01/09 02:56:27 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_returnerror;

/* 
 *  mustfopen - open a file STREAM or die trying.
 */
 
   FILE *fb_mustfopen(file, mode)
      char *file, *mode;
   
      {
	 FILE *fs;
   
	 if ((fs = fopen(file, mode)) == 0){
            if (!cdb_returnerror){
               if (*mode != CHAR_w)
                  fb_xerror(FB_CANT_OPEN, file, NIL);
               else
                  fb_xerror(FB_CANT_CREATE, file, NIL);
               }
	    }
	 return((FILE *) fs);
      }
