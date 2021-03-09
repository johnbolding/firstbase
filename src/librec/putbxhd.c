/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: putbxhd.c,v 9.0 2001/01/09 02:57:00 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Putbxhead_sid[] = "@(#) $Id: putbxhd.c,v 9.0 2001/01/09 02:57:00 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_dbase_byteorder;
extern short int cdb_cpu_byteorder;

 /*
 * putbxhead.c - put 3 FB_SLONG values and FB_NEWLINE at FB_HEADSTART of fd.
 *	used as an btree+ index header.
 */
 
   fb_putbxhead (fd, v1, v2, v3, v4)
      int fd;
      long v1, v2, v3, v4;
      
      {

         char buf[FB_SLONG+FB_SLONG+FB_SLONG+FB_SLONG+3];
         int rlen = FB_SLONG + FB_SLONG + FB_SLONG + FB_SLONG+ 1;

	 if (lseek(fd, FB_HEADSTART, 0) < 0L){
	    fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_INDEX], NIL);
	    return(FB_ERROR);
	    }
         if (cdb_dbase_byteorder != cdb_cpu_byteorder){
            M_32_SWAP(v1);
            M_32_SWAP(v2);
            M_32_SWAP(v3);
            M_32_SWAP(v4);
            }
         memcpy(buf, (char *) &v1, FB_SLONG);
         memcpy(buf + FB_SLONG, (char *) &v2, FB_SLONG);
         memcpy(buf + FB_SLONG + FB_SLONG, (char *) &v3, FB_SLONG);
         memcpy(buf + FB_SLONG + FB_SLONG + FB_SLONG, (char *) &v4, FB_SLONG);
         memcpy(buf + rlen - 1, SYSMSG[S_STRING_NEWLINE], 1);
	 if (write(fd, buf, (unsigned) rlen) != rlen){
	    fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_HEADER], NIL);
	    return(FB_ERROR);
	    }
	 return(FB_AOK);
      }
