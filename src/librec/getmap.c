/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getmap.c,v 9.0 2001/01/09 02:56:58 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getmap_sid[] = "@(#) $Id: getmap.c,v 9.0 2001/01/09 02:56:58 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_dbase_byteorder;
extern short int cdb_cpu_byteorder;

/* 
 *  getmap - get map information - avail, freep. also rpos,rlen if n > 0.
 */
 
   fb_getmap(mfd, n, avail, freep, rpos, rlen)
      long *avail, *freep, *rpos, *rlen;
      long n;
      int mfd;
   
      {
         long fend, ns;
         char buf[FB_SLONG+FB_SLONG+3];
         int map_rlen = FB_SLONG + FB_SLONG;
	 
	 if (lseek(mfd, 0L, 0) < 0L){
	    fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }
	 if (read(mfd, buf, (unsigned) map_rlen) != map_rlen){
	    fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }
         memcpy((char *) avail, buf, FB_SLONG);
         memcpy((char *) freep, buf + FB_SLONG, FB_SLONG);
         if (cdb_dbase_byteorder != cdb_cpu_byteorder){
            M_32_SWAP((*avail));
            M_32_SWAP((*freep));
            }
	 if (n <= 0L)
	    return(FB_AOK);
	 if ((fend = lseek(mfd, 0L, 2)) < 0L){
	    fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }
	 ns = n * (long) FB_MAPREC_SIZE;
	 if (ns < fend){
	    if (lseek(mfd, ns, 0) < 0L){
	       fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], NIL);
	       return(FB_ERROR);
	       }
            if (read(mfd, buf, (unsigned) map_rlen) != map_rlen){
               fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_MAP], NIL);
               return(FB_ERROR);
               }
            memcpy((char *) rpos, buf, FB_SLONG);
            memcpy((char *) rlen, buf + FB_SLONG, FB_SLONG);
            if (cdb_dbase_byteorder != cdb_cpu_byteorder){
               M_32_SWAP((*rpos));
               M_32_SWAP((*rlen));
               }
	    }
	 else{
	    *rpos = 0L;
	    *rlen = 0L;
	    }
	 return(FB_AOK);
      }
