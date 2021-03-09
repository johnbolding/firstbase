/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: putmap.c,v 9.0 2001/01/09 02:57:01 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Putmap_sid[] = "@(#) $Id: putmap.c,v 9.0 2001/01/09 02:57:01 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_dbase_byteorder;
extern short int cdb_cpu_byteorder;

/* 
 *  put map information - avail, freep, rpos, and rlen.
 */
 
   fb_putmap(mfd, n, avail, rpos, rlen)
      long avail, rpos, rlen, n;
      register int mfd;
   
      {
         char buf[FB_SLONG+FB_SLONG+3];
         int map_rlen = FB_SLONG+FB_SLONG;

	 if (lseek(mfd, 0L, 0) < 0L){		/* seek to begining of map */
	    fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }
	    					/* write avail blindly! */
         if (cdb_dbase_byteorder != cdb_cpu_byteorder)
            M_32_SWAP(avail);
	 if (write(mfd, (char *) &avail, FB_SLONG) !=FB_SLONG){
	    fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }
	 if (rpos > 0){				/* install {rpos,rlen} */
	    if (lseek(mfd, n * (long) FB_MAPREC_SIZE, 0) < 0L){
	       fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], NIL);
	       return(FB_ERROR);
	       }
            
            if (cdb_dbase_byteorder != cdb_cpu_byteorder){
               M_32_SWAP(rpos);
               M_32_SWAP(rlen);
               }
            memcpy(buf, (char *) &rpos, FB_SLONG);
            memcpy(buf + FB_SLONG, (char *) &rlen, FB_SLONG);
            if (write(mfd, buf, (unsigned) map_rlen) != map_rlen){
	       fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_MAP], NIL);
               return(FB_ERROR);
               }
	    }
	 return(FB_AOK);
      }
