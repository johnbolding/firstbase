/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: autoigen.c,v 9.0 2001/01/09 02:56:45 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Autoigen_sid[] = "@(#) $Id: autoigen.c,v 9.0 2001/01/09 02:56:45 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <sys/types.h>
#include <sys/stat.h>

static char *FMT1 = "%si";
static char *COMMAND_FMT = "dbigen -b -d %s -i %s";
static char *BIDX = ".bidx";

extern char *cdb_PGEN;
extern char *cdb_LGEN;
extern char *cdb_IGEN;
extern char *cdb_CGEN;
extern char *cdb_UGEN;
extern char *cdb_DBMERGE;
extern char *cdb_DBEMIT;
extern char *cdb_DBJOIN;
extern char *cdb_pgm;

extern short int cdb_makeindex;
extern short int cdb_batchmode;

/*
 *  autoigen - hook to force index generation if needed before data gen.
 *	called from geti_dict.
 */
 
   void fb_autoigen(hdb)
      fb_database *hdb;
      
      {
         char com[FB_MAXLINE], autofile[FB_MAXLINE], aname[FB_MAXLINE];
	 struct stat dbuf, xbuf, ibuf;
	 
	 if (!equal(cdb_pgm, cdb_PGEN) && 
	     !equal(cdb_pgm, cdb_LGEN) &&
	     !equal(cdb_pgm, cdb_IGEN) &&
	     !equal(cdb_pgm, cdb_CGEN) &&
	     !equal(cdb_pgm, cdb_UGEN) &&
	     !equal(cdb_pgm, cdb_DBMERGE) &&
	     !equal(cdb_pgm, cdb_DBEMIT) &&
	     !equal(cdb_pgm, cdb_DBJOIN))
	    return;
	 
         if (!cdb_makeindex)
	    return;
	    
	 /* if the index is current, do not bother with igen */
	 dbuf.st_mtime = xbuf.st_mtime = ibuf.st_mtime = 0;
	 sprintf(com, FMT1, hdb->idict);	/* borrow the com area */
	 if (stat(hdb->dbase, &dbuf) == 0 &&
	     stat(hdb->dindex, &xbuf) == 0 &&
	     stat(com, &ibuf) == 0){		/* files must exist */
	    if (xbuf.st_mtime > ibuf.st_mtime &&
		xbuf.st_mtime > dbuf.st_mtime)
	       if (fb_checktotals(hdb) == FB_AOK)
	          return;			/* rah rah - up to date */
	    }

         /*
          * if the index exists and its a btree, do not regen it
          */
         fb_dirname(autofile, hdb->dbase);
         fb_basename(aname, hdb->dindex);
         strcat(autofile, aname);
	 strcat(autofile, BIDX);
         if (access(autofile, 0) == 0)
            return;

	 strcpy(com, hdb->idict);		/* walk on com buffer */
	 strcat(com, "i");
	 if (access(com, 0) == 0){
	    sprintf(com, COMMAND_FMT, hdb->ddict, hdb->idict);
	    if (!cdb_batchmode){
	       fb_scrhdr(hdb, SYSMSG[S_AUTOREGEN]);
	       fb_infoline();
	       FB_XWAIT();
	       fb_refresh();
	       }
	    fb_system(com, FB_WITHROOT);
	    }
      }

/*
 *  checktotals - check the reccnt vs orgcnt of the index itself.
 */

   fb_checktotals(hdb)
      fb_database *hdb;
      
      {
         int fd;
	 long reccnt, orgcnt;
	 
	 if ((fd = open(hdb->idict, READ)) < 0)
	    return(FB_ERROR);
	 if (fb_getxhead(fd, &reccnt, &orgcnt) != FB_AOK)
	    return(FB_ERROR);
         close(fd);
	 if (orgcnt != reccnt)
	    return(FB_ERROR);
	 return(FB_AOK);
      }
