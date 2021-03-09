/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: putlog.c,v 9.0 2001/01/09 02:57:01 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Log_sid[] = "@(#) $Id: putlog.c,v 9.0 2001/01/09 02:57:01 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#define BLKSIZE	1024

static char FMT[20];
static char SEPARATOR = '|';
static char wbuffer[BLKSIZE + 1];
static int w_curp;

#if FB_PROTOTYPES
static logemit(fb_database *hp);
static nextwrite(int fd, int buf);
static wflush(int fd);
#else
static logemit();
static nextwrite();
static wflush();
#endif /* FB_PROTOTYPES */

extern short int cdb_reclog;
extern short int cdb_secure;

/*
 * putlog - store the record in hp in the log.
 */

   void fb_putlog(hp)
      fb_database *hp;

      {
         char buf[FB_MAXLINE];
	 int len;

         if (!cdb_reclog)
	    return;
         if (cdb_secure)
            strcpy(FMT, "@%ld (%s/%s)\n");
         else
            strcpy(FMT, "@%ld\n");
	 if (lseek(hp->logfd, 0L, 2) < 0)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_DATA], SYSMSG[S_BAD_LOG]);
         if (cdb_secure)
	    sprintf(buf, FMT, hp->rec, fb_getlogin(), getlogin());
         else
	    sprintf(buf, FMT, hp->rec);
	 len = strlen(buf);
	 if (write(hp->logfd, buf, (unsigned) len) != len)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_DATA], SYSMSG[S_BAD_LOG]);
	 logemit(hp);
      }

/* 
 *  logemit - emit all the fields of a record to the log
 */

   static logemit(hp)
      fb_database *hp;
      
      {
         int i;
	 char *p;
	 
	 w_curp = 0;
         for (i = 0; i <= hp->nfields; i++){
	    if (hp->kp[i]->type != FB_FORMULA && hp->kp[i]->dflink == NULL &&
                  hp->kp[i]->type != FB_LINK){
	       if (i != 0)
	          nextwrite(hp->logfd, SEPARATOR);
	       p = hp->kp[i]->fld;
	       for ( ; *p; p++){ 		/* escape emb |s */
		  if (*p == FB_NEWLINE){
		     nextwrite(hp->logfd, CHAR_BACKSLASH);
		     nextwrite(hp->logfd, CHAR_n);
		     }
		  else if (*p == SEPARATOR){
		     nextwrite(hp->logfd, CHAR_BACKSLASH);
		     nextwrite(hp->logfd, SEPARATOR);
		     }
		  else 
		     nextwrite(hp->logfd, *p);
		  }
	       }
	    }
         nextwrite(hp->logfd, SEPARATOR);
	 nextwrite(hp->logfd, CHAR_NEWLINE);
	 wflush(hp->logfd);
      }

/*
 * nextwrite - semi buffered write - write a block if needed.
 */
   static nextwrite(fd, buf)
      int fd;
      int buf;
      
      {
         if (w_curp < 0 || w_curp >= BLKSIZE){
	    if (write(fd, wbuffer, BLKSIZE) != BLKSIZE)
	       fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_DATA], SYSMSG[S_BAD_LOG]);
	    w_curp = 0;
	    }
	 wbuffer[w_curp++] = buf;
      }

/*
 * wflush - flush anything left in the wbuffer
 */
   static wflush(fd)
      int fd;
      
      {
         if (w_curp > 0){
	    if (write(fd, wbuffer, (unsigned) w_curp) != w_curp)
	       fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_DATA], SYSMSG[S_BAD_LOG]);
	    w_curp = 0;
	    }
      }
