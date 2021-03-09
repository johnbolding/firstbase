/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: putfree.c,v 9.0 2001/01/09 02:57:00 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Putfree_sid[] = "@(#) $Id: putfree.c,v 9.0 2001/01/09 02:57:00 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern char cdb_FREEC;
extern char cdb_FILLC;
extern char cdb_FILLE;

extern short int cdb_dbase_byteorder;
extern short int cdb_cpu_byteorder;

/* 
 *  putfree - update the free list pointer in the dbase map.
 *	store the current freep in the new rpos area.
 *      and make freep = rpos.
 *	(ie, push free area onto front of free list).
 */
 
   fb_putfree(fd, mfd, rpos, rlen, hp)
      long rpos, rlen;
      register int fd, mfd;
      fb_database *hp;
   
      {
         long freep, size, count = 0, val;
         char *buf, *p;

         buf = p = hp->arec;
	 /* seek to freep and read it */
	 if (lseek(mfd, (long) (FB_SLONG), 0) < 0L){
	    fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }
	 if (read(mfd, (char *) &freep, FB_SLONG) != FB_SLONG){
	    fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }
         if (cdb_dbase_byteorder != cdb_cpu_byteorder)
            M_32_SWAP(freep);
	 
	 /* seek to dbase free space */
	 if (lseek(fd, rpos, 0) < 0L){
	    fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_DATA], NIL);
	    return(FB_ERROR);
	    }

	 *p++ = cdb_FREEC; count++;		/* write free marker ^F */

	 /* write len of this area */
         val = rlen;
         if (cdb_dbase_byteorder != cdb_cpu_byteorder)
            M_32_SWAP(val);
         memcpy(p, (char *) &val, FB_SLONG);
         p += FB_SLONG; count += FB_SLONG;

	 /* write freep link */
         val = freep;
         if (cdb_dbase_byteorder != cdb_cpu_byteorder)
            M_32_SWAP(val);
         memcpy(p, (char *) &val, FB_SLONG);
         p += FB_SLONG; count += FB_SLONG;

	 /* fill the hole */
	 for (size = FB_SLONG * 2 + 2; size < rlen; size++){
            *p++ = cdb_FILLC; count++;
            }
         *p++ = cdb_FILLE; count++;

         /* actual write of count bytes */
	 if (write(fd, buf, (unsigned) count) != count){
            fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_DATA], NIL);
	    return(FB_ERROR);
	    }

	 /* seek to map freep */
	 if (lseek(mfd, (long) (FB_SLONG), 0) < 0L){
	    fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }

	 /* write new freep */
         if (cdb_dbase_byteorder != cdb_cpu_byteorder)
            M_32_SWAP(rpos);
	 if (write(mfd, (char *) &rpos, FB_SLONG) != FB_SLONG){
	    fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }

	 return(FB_AOK);
      }

