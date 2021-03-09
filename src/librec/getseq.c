/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getseq.c,v 9.0 2001/01/09 02:56:58 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getseq_sid[] = "@(#) $Id: getseq.c,v 9.0 2001/01/09 02:56:58 john Exp $";
#endif

#include <fb.h>

/*
 *  getseq - reads in sequence number id tag on the file fd.
 *	this seq number starts at FB_SEQSTART.
 */

   fb_getseq(fd)
      int fd;
      
      {
	 char p[FB_SEQSIZE+2];
	 
	 lseek(fd, FB_SEQSTART, 0);
	 read(fd, p, FB_SEQSIZE);
	 p[FB_SEQSIZE] = 0;
	 return(atoi(p));
      }
