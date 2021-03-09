/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getco.c,v 9.1 2001/01/16 02:46:54 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getco_sid[] = "@(#) $Id: getco.c,v 9.1 2001/01/16 02:46:54 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char *sys_coname = NULL;
extern short int cdb_scr_status_mask;
extern char *cdb_CN;
extern char *cdb_coname;

/* 
 *  get company name. searchpath = fb_dirname of dbase, PATH, /usr/lib/cdb
 *	company name is more like the screen title.
 */
 
   void fb_getco(hp)
      fb_database *hp;
      
      {
         FILE *fs;
         char cname[FB_MAXNAME+1], *p;
         
         cname[0] = NULL;
	 if (hp != NULL && hp->dbase != NULL){
            if (*(fb_dirname(cname, hp->dbase)))
	       sprintf(cname, SYSMSG[S_FMT_SSLASHS], cname, cdb_CN);
	    else
	       strcpy(cname, cdb_CN);
	    }
         fs = NULL;
         if (cname[0] == NULL || (fs = fopen(cname, FB_F_READ)) == NULL){
	    fb_pathname(cname, cdb_CN);
	    if (cname[0] == NULL || (fs = fopen(cname, FB_F_READ)) == NULL){
               if (sys_coname == NULL)
                  fb_homefile(&sys_coname, cdb_CN);
	       if ((fs = fopen(sys_coname, FB_F_READ)) == NULL){
		  cdb_coname = NIL;
		  return;
		  }
               }
	    }
         /* this used to depend on FB_CN_LEN - but scrhdr was rewritten */
	 fgets(cname, FB_MAXNAME, fs);
	 if ((p = strchr(cname, FB_NEWLINE)) != 0)
	    *p = NULL;
         fclose(fs);
	 fb_mkstr(&cdb_coname, cname);
      }
