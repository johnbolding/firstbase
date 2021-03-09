/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: scrstat.c,v 9.1 2001/02/05 18:21:29 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Scrstat_sid[] = "@(#) $Id: scrstat.c,v 9.1 2001/02/05 18:21:29 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_scr_status;
extern short int cdb_scr_status_mask;
static char *FMT =  "Status: %s";
static char *FMT1 = "%s";

/* 
 *  print status message on screen to constant location.
 */
 
   void fb_scrstat(p)
      char *p;
      
      {
         char t1[FB_MAXLINE], line[FB_MAXLINE];
	 
	 if (!cdb_batchmode){
	    if (cdb_scr_status){
               if (!cdb_scr_status_mask){
	          sprintf(t1, FMT,  p);
                  fb_pad(line, t1, 25);
                  fb_move(1, cdb_t_cols - 24), fb_reverse(line);
                  }
               else{
	          sprintf(t1, FMT1,  p);
                  fb_pad(line, t1, 17);
                  fb_move(1, cdb_t_cols - 16), fb_reverse(line);
                  }
	       }
	    fb_move(1, cdb_t_cols);
	    }
      }
