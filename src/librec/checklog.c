/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: checklog.c,v 9.0 2001/01/09 02:56:56 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "%W% %G% FB";
#endif

#include <fb.h>
#include <fb_ext.h>

#define CKLEN	3

static char *CKBUF = "@C\n";

extern short int cdb_reclog;

/*
 * checklog - check point the log by writing a check point record to log
 */

   fb_checklog(hp)
      fb_database *hp;

      {
         if (cdb_reclog){
            if (lseek(hp->logfd, 0L, 2) < 0)
               fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_DATA], SYSMSG[S_BAD_LOG]);
            if (write(hp->logfd, CKBUF, CKLEN) != CKLEN)
               fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_DATA], SYSMSG[S_BAD_LOG]);
            }
      }
