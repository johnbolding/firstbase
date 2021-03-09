/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbigen.c,v 9.1 2001/02/16 19:43:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbigen_sid[] = "@(#) $Id: dbigen.c,v 9.1 2001/02/16 19:43:51 john Exp $";
#endif

/*
 *  dbigen.c - index generator (uses idicti)
 */

#include <fb.h>
#include <fb_vars.h>
#include <igen.h>
#include <sys/types.h>
#include <sys/stat.h>

static char *MSG1 = "Index is Current and Up to Date!";

fb_database *hp;

/*
 *  dbigen - main driver
 */

fb_field *by[FB_MAXBY];			/* sortby fields */
struct self *top;			/* top of the select tree */
int rflag = 0;				/* report sorted by flag */
int cflag = 0;				/* case-insensative flag */
int fflag = 0;				/* force flag - uncond. regen */
int nflag = 0;				/* newline to blank mapping flag */
int Bflag = 0;				/* Blocking factor flag */
int btree = 0;				/* gen a Btree index when done */

static getindex();

   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         struct self *stree();
         char gidict[FB_MAXNAME], gindex[FB_MAXNAME], bindex[FB_MAXNAME];
	 int i;
	 struct stat dbuf, xbuf, ibuf;
   
         fb_getargs(argc, argv, FB_ALLOCDB);
	 hp = cdb_db;
	 if (fb_testargs(argc, argv, "-f") > 0)
	    fflag = 1;
	 /* if the index is current, do not bother with igen */
	 dbuf.st_mtime = xbuf.st_mtime = ibuf.st_mtime = 0;
	 sprintf(gidict, "%si", hp->idict);	/* borrow the gidict area */
	 if (!fflag && stat(hp->dbase, &dbuf) == 0	&&
	     stat(hp->dindex, &xbuf) == 0	&&
	     stat(gidict, &ibuf) == 0 		&&
	     access(hp->idict, 0) == 0){	/* files must exist */
	    if (xbuf.st_mtime > ibuf.st_mtime &&
		xbuf.st_mtime > dbuf.st_mtime)
	       if (fb_checktotals(hp) == FB_AOK){
		  if (!cdb_batchmode && !cdb_yesmode)
		     fb_serror(FB_MESSAGE, MSG1, NIL);
		  fb_ender();			/* rah rah - up to date */
		  }
	    }
	 
	 /* deep sigh. run it. */
	 if (fb_testargs(argc, argv, "-B") > 0)
	    Bflag = 1;
	 strcpy(gidict, hp->idict);
	 strcpy(gindex, hp->dindex);
	 fb_basename(bindex, hp->dindex);
	 fb_opendb(cdb_db, READ, FB_NOINDEX, 0);
	 fb_scrhdr(hp, bindex); fb_infoline();
	 for (i = 0; i < hp->nfields; i++)
	    fb_nounders(cdb_keymap[i]);
         if ((top = stree(cdb_keymap, hp, by)) == NULL){
	    sprintf(gidict, "%si", hp->idict);
	    fb_xerror(FB_BAD_DICT, gidict, NIL);
	    }
	 if (getindex(argc, argv, gindex) == FB_ABORT)
	    fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	 if (!cdb_batchmode && !cdb_yesmode){
            itrace(top, by, 0);
	    fb_infoline();
            if (fb_mustbe('y',"If accurate, enter 'y' to continue:",
                  cdb_t_lines, 1) != FB_AOK)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    fb_move(4, 1), fb_clrtobot(), fb_infoline();
	    }
	 if (!cdb_batchmode){
	    if (!cflag)
               fb_scrstat("Selecting");
	    else
               fb_scrstat("Selecting-CaseIns");
	    FB_XWAIT();
	    fb_gcounter(0L);
	    }
	 if (cdb_yesmode)
	    sleep(1);				/* some are too fast */
         if (mkidx(gidict, gindex) == FB_ERROR)
            fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	 fb_closedb(cdb_db);
         fb_ender();
      }

/* 
 *  getindex - get index from which the generated index will made from 
 *    the default is to use the actual fb_database itself 
 */
 
   static getindex(argc, argv, gindex)
      char *gindex, *argv[];
      int argc;
      
      {
         char file[FB_MAXNAME], err[FB_MAXNAME];
	 int st, xflag;
	 
	 hp->dindex = hp->idict = NULL;
	 file[0] = NULL;
	 if (fb_testargs(argc, argv, "-r") > 0)
	    rflag = 1;
	 if (fb_testargs(argc, argv, "-c") > 0)
	    cflag = 1;
	 if (fb_testargs(argc, argv, "-n") > 0)
	    nflag = 1;
	 if ((xflag = fb_testargs(argc, argv, "-x")) > 0){
	    if (argc > xflag)
	       strcpy(file, argv[xflag + 1]);
	    else if (cdb_batchmode || cdb_yesmode)	/* cannot be -x as last arg */
	       usage();
	    }
	 if (fb_testargs(argc, argv, "-btree") > 0)
	    btree = 1;
	 if (!xflag)				/* if no -x no ask */
	    return(FB_AOK);
	 for (;;){
	    if (!cdb_batchmode && !xflag){
	       fb_move(4, 1); fb_clrtobot();
	       fb_move(10,10);
	       fb_printw("Enter Name of EXISTING Index to Work From...");
	       fb_move(11,10);
	       fb_printw("Or <RETURN> to Force Use of Entire DataBase:");
	       fb_infoline();
	       fb_scrhlp(SYSMSG[S_CHOICE_MSG1]);
	       st = fb_input(13, 10, 44, 0, FB_ALPHA, file, FB_ECHO, FB_NOEND, FB_CONFIRM);
	       fb_scrhlp(NIL);
	       if (st == FB_DEFAULT || st == FB_ABORT)
		  return(st);
	       }
	    fb_trim(file);
            fb_dbargs(NIL, file, hp);
	    if (equal(hp->dindex, gindex)){
	       if (cdb_batchmode)
	          usage();
	       fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], SYSMSG[S_INVALID]);
	       }
	    else if (fb_geti_dict(READWRITE, hp) == FB_ERROR)
	       fb_xerror(FB_BAD_DICT, hp->idict, NIL);
	    else if (fb_geti_data(READWRITE, hp) == FB_ERROR)
	       fb_xerror(FB_BAD_DATA, hp->dindex, NIL);
	    break;
	    }
	 if (!cdb_batchmode && !cdb_yesmode){
	    sprintf(err, "Using EXISTING index %s to generate %s",
	       hp->dindex,gindex);
	    fb_serror(FB_MESSAGE, err, NIL);
	    }
	 return(FB_AOK);
      }

/* 
 * usage message 
 */

   usage()
      {
         fb_xerror(FB_MESSAGE, "usage: dbigen [-d dbase] [-i index]",
	    "[-y] [-b [-x xref_index]]");
      }
