/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getxhead.c,v 9.0 2001/01/09 02:56:58 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getxhead_sid[] = "@(#) $Id: getxhead.c,v 9.0 2001/01/09 02:56:58 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_dbase_byteorder;
extern short int cdb_cpu_byteorder;

/*
 * getxhead - get two FB_SLONG values from the start of the header.
 */
 
   fb_getxhead(fd, v1, v2)
      int fd;
      long *v1, *v2;

      {
         char buf[FB_SLONG+FB_SLONG+3];
         int rlen = FB_SLONG + FB_SLONG + 1;	/* 1 to get past newline */;

	 if (lseek(fd, (long) FB_HEADSTART, 0) < 0L){
	    fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_INDEX], NIL);
	    return(FB_ERROR);
	    }
	 if (read(fd, buf, (unsigned) rlen) != rlen){
	    fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_INDEX], NIL);
	    return(FB_ERROR);
	    }
         memcpy((char *) v1, buf, FB_SLONG);
         memcpy((char *) v2, buf + FB_SLONG, FB_SLONG);
         if (cdb_dbase_byteorder != cdb_cpu_byteorder){
            M_32_SWAP((*v1));
            M_32_SWAP((*v2));
            }
         /*
          * did do a seek here to get past newline.
          * now, newline is read at same time into buf above.
          * lseek(fd, 1L, 1);
          */
	 return(FB_AOK);
      }

/*
 * skipxhead - skip past the index header
 */
 
   fb_skipxhead(fd)
      int fd;

      {
         long rlen = FB_SLONG + FB_SLONG + 1 + FB_HEADSTART;
         					/* 1 to get past newline */;

	 if (lseek(fd, rlen, 0) < 0L){
	    fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_INDEX], NIL);
	    return(FB_ERROR);
	    }
	 return(FB_AOK);
      }
