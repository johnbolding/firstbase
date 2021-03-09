/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: bootmap.c,v 9.0 2001/01/09 02:56:55 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Bootmap_sid[] = "@(#) $Id: bootmap.c,v 9.0 2001/01/09 02:56:55 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern long cdb_headsize;
extern short int cdb_dbase_byteorder;
extern short int cdb_cpu_byteorder;

/* 
 *  boot the fb_database map with cdb_headsize for avail, and 0 for freep.
 */
 
   fb_bootmap(mfd)
      register int mfd;
   
      {
         long avail, freep;
	 
         avail = (long) (cdb_headsize); freep = 0L;
	 if (lseek(mfd, 0L, 0) < 0L){
            fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }
         if (cdb_dbase_byteorder != cdb_cpu_byteorder){
            M_32_SWAP(avail);
            M_32_SWAP(freep);
            }
	 if (write(mfd, (char *) (&avail), FB_SLONG) != FB_SLONG){
            fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }
	 if (write(mfd, (char *) (&freep), FB_SLONG) != FB_SLONG){
            fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }
	 return(FB_AOK);
      }
