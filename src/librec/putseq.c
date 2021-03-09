/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: putseq.c,v 9.0 2001/01/09 02:57:01 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Putseq_sid[] = "@(#) $Id: putseq.c,v 9.0 2001/01/09 02:57:01 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char *FMT = "%04d\n";
extern char *cdb_seqfile;

/*
 *   putseq - reads in SEQFILE file, if existant, and bumps
 *	before assigning to current dbase (FB_SEQSIZE bytes@FB_SEQSTART).
 *	writes new number back to SEQFILE.
 *	assumption is that 9999 distinct databases are plenty.
 *
 *	NOTE: SEQFILE is now cdb_seqfile.
 */

   fb_putseq(fd)
      int fd;
      
      {
         int sfd;
	 char p[FB_MAXLINE];
	 int seq;
	 
	 if ((sfd = open(cdb_seqfile, READWRITE)) <= 0){
	    close(creat(cdb_seqfile, 0666));
	    sfd = fb_mustopen(cdb_seqfile, READWRITE);
            if (sfd < 0)
               return(FB_ERROR);
	    seq = 1001;
	    }
	 else {
	    read(sfd, p, FB_SEQSIZE);
            p[FB_SEQSIZE] = NULL;
	    seq = atoi(p) + 1;
	    if (seq > 9999)
	       seq = 1001;
	    }
	 sprintf(p, FMT, seq);
	 lseek(sfd, 0L, 0);
	 write(sfd, p, strlen(p));
	 close(sfd);
	 lseek(fd, FB_SEQSTART, 0);
	 write(fd, p, FB_SEQSIZE);
         return(seq);
      }
