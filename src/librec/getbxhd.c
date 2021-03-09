/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getbxhd.c,v 9.0 2001/01/09 02:56:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getbxhead_sid[] = "@(#) $Id: getbxhd.c,v 9.0 2001/01/09 02:56:57 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_dbase_byteorder;
extern short int cdb_cpu_byteorder;

/*
 * getbxhead - get four FB_SLONG values from the start of the header.
 */

   fb_getbxhead(fd, v1, v2, v3, v4)
      int fd;
      long *v1, *v2, *v3, *v4;

      {
         char buf[FB_SLONG+FB_SLONG+FB_SLONG+FB_SLONG+3];
         int rlen = FB_SLONG + FB_SLONG + FB_SLONG + FB_SLONG + 1;
         						/* 1=past newline */;

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
         memcpy((char *) v3, buf + FB_SLONG + FB_SLONG, FB_SLONG);
         memcpy((char *) v4, buf + FB_SLONG + FB_SLONG + FB_SLONG, FB_SLONG);
         if (cdb_dbase_byteorder != cdb_cpu_byteorder){
            M_32_SWAP((*v1));
            M_32_SWAP((*v2));
            M_32_SWAP((*v3));
            M_32_SWAP((*v4));
            }
	 return(FB_AOK);
      }
