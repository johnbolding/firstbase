/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: puthead.c,v 9.0 2001/01/09 02:57:00 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Puthead_sid[] = "@(#) $Id: puthead.c,v 9.0 2001/01/09 02:57:00 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char *FMT1 = "%010ld";

/*
 *  puthead - put header record (record 4: 24 chars) into file fd.
 *	new format 4-13 == rec_count, 14 = dirty bit, 15-23 = delcnt
 */
 
   fb_puthead(fd, reccnt, delcnt)
      long reccnt, delcnt;
      register int fd;
   
      {
	 char buf[22], *p;
	 int wsize;

	 if (lseek(fd, FB_HEADSTART, 0) < 0L){
	    fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_INDEX], NIL);
	    return(FB_ERROR);
	    }
	 if (delcnt < 0)
	    delcnt = 0;
	 sprintf(buf, FMT1, reccnt);
	 if (write(fd, buf, 10) != 10){
	    fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_INDEX], NIL);
	    return(FB_ERROR);
	    }
	 sprintf(buf, FMT1, delcnt);
	 p = buf; wsize = 10;
	 if (reccnt != 0 || delcnt != 0){
	    if (lseek(fd, 1L, 1) < 0L){	/* skip dirty bit */
	       fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_INDEX], NIL);
	       return(FB_ERROR);
	       }
	    p = buf + 1;
	    wsize = 9;
	    }
	 if (write(fd, p, (unsigned) wsize) != wsize){
	    fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_INDEX], NIL);
	    return(FB_ERROR);
	    }
	 return(FB_AOK);
      }
