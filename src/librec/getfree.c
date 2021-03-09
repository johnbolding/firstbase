/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getfree.c,v 9.0 2001/01/09 02:56:58 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getfree_sid[] = "@(#) $Id: getfree.c,v 9.0 2001/01/09 02:56:58 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_dbase_byteorder;
extern short int cdb_cpu_byteorder;

/* 
 *  getfree - return the first available node that will allow size bytes.
 *  	search the free list for the first fit.
 */
 
   long fb_getfree(fd, mfd, size, rlen)
      register int fd, mfd;
      long *rlen, size;
   
      {
         long avail, freep, lastp, nextp, curp, val;
         char buf[FB_SLONG+FB_SLONG+3];
         int map_rlen = FB_SLONG + FB_SLONG;
	 
	 if (lseek(mfd, 0L, 0) < 0L){	/* seek to BOF and read avail,freep */
            fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }
	 if (read(mfd, buf, (unsigned) map_rlen) != map_rlen){
	    fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }
         memcpy((char *) &avail, buf, FB_SLONG);
         memcpy((char *) &freep, buf + FB_SLONG, FB_SLONG);
         if (cdb_dbase_byteorder != cdb_cpu_byteorder){
            M_32_SWAP(avail);
            M_32_SWAP(freep);
            }

	 /* search the free list. first fit wins */
	 for(lastp = 0, nextp = freep; nextp != 0L; ){
	    curp = nextp;			/* save curp */
	    					/* seek past FREE marker */
	    if (lseek(fd, (long) (nextp + 1L), 0) < 0L){
               fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_DATA], NIL);
	       return(FB_ERROR);
	       }
	       					/* rlen is a long ptr */
            if (read(fd, buf, (unsigned) map_rlen) != map_rlen){
               fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_DATA], NIL);
               return(FB_ERROR);
               }
            memcpy((char *) rlen, buf, FB_SLONG);
            memcpy((char *) &nextp, buf + FB_SLONG, FB_SLONG);
            if (cdb_dbase_byteorder != cdb_cpu_byteorder){
               M_32_SWAP((*rlen));
               M_32_SWAP(nextp);
               }

	    if (size <= *rlen){		/* found it. rah rah */
	       if (lastp == 0){			/* mfd freep is it */
	          if (lseek(mfd, (long) FB_SLONG, 0) < 0L){
                     fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], NIL);
		     return(FB_ERROR);
		     }
                  val = nextp;
                  if (cdb_dbase_byteorder != cdb_cpu_byteorder)
                     M_32_SWAP(val);
		  if (write(mfd, (char *) &val, FB_SLONG) != FB_SLONG){
                     fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_MAP], NIL);
		     return(FB_ERROR);
		     }
	          }
	       else{				/* middle of list */
	       					/* seek to lastps nextp */
	          if (lseek(fd, lastp + 1L + (long) FB_SLONG, 0) < 0L){
                     fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], NIL);
		     return(FB_ERROR);
		     }
		     				/* write this ps nextp */
                  val = nextp;
                  if (cdb_dbase_byteorder != cdb_cpu_byteorder)
                     M_32_SWAP(val);
		  if (write(fd, (char *) &val, FB_SLONG) != FB_SLONG){
                     fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_MAP], NIL);
		     return(FB_ERROR);
		     }
	          }
	       return(curp);
	       }
	    else				/* save curp as lastp */
	       lastp = curp;
	    }
	 /* if nothing else, return avail */
	 *rlen = 0L;
	 return(avail);
      }
