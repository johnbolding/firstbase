/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mustopen.c,v 9.0 2001/01/09 02:56:27 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Mustopen_sid[] = "@(#) $Id: mustopen.c,v 9.0 2001/01/09 02:56:27 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_returnerror;

/* 
 *  mustopen - open file or die trying. mode:0=read,1=write,2=both
 */
 
   fb_mustopen(file, mode)
      char *file;
      int mode;
   
      {
	 int fd;
   
	 if ((fd = open(file, mode)) < 0){
            if (!cdb_returnerror){
               if (mode == 0)
                  fb_xerror(FB_CANT_OPEN, file, NIL);
               else
                  fb_xerror(FB_CANT_CREATE, file, NIL);
               }
	    }
	 return(fd);
      }
