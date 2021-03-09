/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: fgetrec.c,v 9.0 2001/01/09 02:56:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Fgetrec_sid[] = "@(#) $Id: fgetrec.c,v 9.0 2001/01/09 02:56:57 john Exp $";
#endif

#include <fb.h>

/* 
 *  fgetrec - fixed length getrec: get logical record n from fd into buf
 *	taking into account the header bytes.
 */
 
   fb_fgetrec(n, fd, size, buf, header)
      long n;
      register int size, header, fd;
      char *buf;
   
      {
	 if (lseek(fd, (n - 1L) * (long) (size) + (long) (header), 0) < 0)
	    return(FB_ERROR);
	 if (read(fd, buf, (unsigned) size) != size)
	    return(FB_ERROR);
	 *(buf + size) = NULL;		/* allows scanning of buf */
	 return(FB_AOK);
      }

/* 
 *  fputrec - fixed length putrec: put logical record n from buf to fd
 *	taking into account the header bytes.
 */
 
   fb_fputrec(n, fd, size, buf, header)
      long n;
      register int size, header, fd;
      char *buf;
   
      {
	 if (lseek(fd, (n - 1L) * (long) (size) + (long) (header), 0) < 0)
	    return(FB_ERROR);
	 if (write(fd, buf, (unsigned) size) != size)
	    return(FB_ERROR);
	 return(FB_AOK);
      }
