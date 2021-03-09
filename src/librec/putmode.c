/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: putmode.c,v 9.0 2001/01/09 02:57:01 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Putmode_sid[] = "@(#) $Id: putmode.c,v 9.0 2001/01/09 02:57:01 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char *FMT1 = "%04d%03d%s";

/*
 *  putmode - put record mode (record 24: 33 chars) into file fd.
 *	format 24-27 == uid, 28-30 = gid, 31-33 = mode
 */
 
   fb_putmode(fd, uid, gid, mode)
      register int fd;
      int uid, gid;
      char *mode;
   
      {
	 char buf[22];

	 if (lseek(fd, FB_MODESTART, 0) < 0L){
	    fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_INDEX], NIL);
	    return(FB_ERROR);
	    }
	 if (uid < 0 || gid < 0){
	    fb_lerror(FB_MESSAGE, SYSMSG[S_ACCESSDENIED], NIL);
	    return(FB_ERROR);
	    }
	 sprintf(buf, FMT1, uid, gid, mode);
	 if (write(fd, buf, 10) != 10){
	    fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_INDEX], NIL);
	    return(FB_ERROR);
	    }
	 return(FB_AOK);
      }
