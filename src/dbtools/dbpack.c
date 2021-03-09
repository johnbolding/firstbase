/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbpack.c,v 9.1 2001/02/16 19:47:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)dbpack.c	8.3 06/18/00 FB";
#endif

/*
 *  dbpack.c - (dbundo.c) pack or undo deleteions from an entire
 *     Cdb fb_database.
 */

#include <fb.h>
#include <fb_vars.h>

extern char *cdb_pgm;
extern short int cdb_secure;
extern char *cdb_DBCLEAN;

static char tempname[] = {".pckXXXXXX"};	/* for temp file name */
static char fname[FB_MAXNAME];			/* for filename */
static int pack;
static void spack();
static packemin();

/*
 *  dbpack - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         char dname[FB_MAXNAME];
	 int j;

         fb_getargs(argc, argv, FB_NODB);
	 pack = (strcmp(cdb_pgm, "dbpack") == 0) ? 1 : 0;
	 for (j = 1; j < argc; j++)
	    if (!(argv[j][0] == '-')){
	       fb_rootname(dname, argv[j]);
	       spack(dname);
	       }
	 fb_ender();
      }

/*
 *  spack - simple pack for one fb_database.
 */

   static void spack(dname)
      char *dname;
      
      {
 	 fb_database *hp;
	 char msg[FB_MAXLINE];
	 
	 hp = fb_dballoc();
	 fb_dbargs(dname, NIL, hp);
         if (access(hp->dbase, 0) != 0)
            return;
	 fb_opendb(hp, READWRITE, FB_NOINDEX, 0);
	 if (!cdb_batchmode){
            fb_scrhdr(hp, NIL); fb_scrtime(hp);
            (pack) ? fb_scrstat("Packing") : fb_scrstat("Undoing");
	    }
         if (cdb_secure && fb_getuid() != 0)
            fb_xerror(FB_MESSAGE, "Must be FirstBase Root (UID=0) to Pack/Undo",NIL);
	 if (hp->delcnt == 0L){
	    if (cdb_batchmode || cdb_yesmode)
	       return;
	    if (fb_mustbe('y',
	          "Delete Count is 0! Still want to do this (y/n) ?",
	           cdb_t_lines, 1) == FB_ERROR)
	       return;
	    }
	 if (!cdb_batchmode && !cdb_yesmode){
	    if (pack)
	       strcpy(msg, "Ready to PACK? Are You SURE (y/n)?");
	    else
	       strcpy(msg, "Ready to UNDO? Are You SURE (y/n)?");
	    if (fb_mustbe('y', msg, cdb_t_lines, 1) == FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
	 packemin(hp);
	 if (1){ /* if dbundo does work in place, only pack needs cleaning */
	 	 /* for now, always do cleaning -- makes indexes obsolete */
	    fb_rootname(fname, hp->dbase);
	    if (!cdb_batchmode){
	       fb_move(4,1), fb_clrtobot();
	       fb_scrstat("Cleaning");
	       sprintf(msg, "Cleaning %s", fname);
	       fb_fmessage(msg);
	       }
	    sprintf(msg, "%s -b %s", cdb_DBCLEAN, fname);
	    fb_system(msg, FB_WITHROOT);
	    }
         fb_closedb(hp);
      }
      
/* 
 *  packemin - pack or undo a fb_database 
 */
 
   static packemin(hp)
      fb_database *hp;
      
      {
         long rec, n;
	 int fd, size;
	 char p[FB_SEQSIZE+2];

         n = 0L;
	 if (!cdb_batchmode){
	    FB_XWAIT();
	    fb_gcounter(n);
	    }
	 close(mkstemp(tempname));
	 fd = fb_mustopen(tempname, READWRITE);
	 if (pack)				/* pack demands new dbase */
	    fb_putseq(fd);
	 else{					/* post undo indexes are ok */
	    sprintf(p, "%04d", fb_getseq(hp->fd));
	    lseek(fd, FB_SEQSTART, 0);
	    write(fd, p, FB_SEQSIZE);
	    }
         fb_puthead(fd, 0L, 0L);
         if (cdb_secure)
            if (fb_putmode(fd, fb_getuid(), fb_getgid(), "666") == FB_ERROR)
               fb_xerror(FB_IO_ERROR, hp->dbase, NIL);
	 for (rec = 1L; rec <= hp->reccnt; rec++){
	    if ((size = fb_getrec(rec, hp)) == FB_ERROR)
	       fb_xerror(FB_FATAL_GETREC, hp->dbase, (char *) &rec);
	    if (FB_ISDELETED(hp)){
	       if (pack)
	          continue;
	       else
	          hp->kp[hp->nfields]->fld[0] = FB_BLANK;
	       }
	    if (write(fd, hp->orec, size) != size)
	       fb_xerror(FB_WRITE_ERROR, cdb_pgm, hp->dbase);
	    ++n;
	    if (!cdb_batchmode)
	       fb_gcounter(n);
	    }
	 if (fb_puthead(fd, n, 0L) == FB_ERROR)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
	 close(fd);
	 unlink(hp->dbase);
	 if (link(tempname,hp->dbase) < 0)
	    if (fb_copyfile(tempname,hp->dbase) < 0)
	       fb_xerror(FB_EXEC_FAILURE, "Could not link/copy: ", tempname);
	 unlink(tempname);
      }

/*  
 *  usage message 
 */
 
   usage()
      {
         fb_xerror(FB_MESSAGE, "usage: dbpack/dbundo [-b] -[y] dbase", NIL);
      }
