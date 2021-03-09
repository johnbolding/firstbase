/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbcat.c,v 9.0 2001/01/09 02:55:53 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbcat_sid[] = "@(#) $Id: dbcat.c,v 9.0 2001/01/09 02:55:53 john Exp $";
#endif

/*  
 * Dbcat - concat databases together (end to end).
 *   read each record of each file and (if not deleted) output
 *   to concat output file. re-adjust the record count.
 */

#include <fb.h>
#include <fb_vars.h>

#define CONCAT_BASE	"concat"	/* dbcat dbase name */

extern char *cdb_DBCLEAN;
extern char *cdb_pgm;
extern short int cdb_secure;

static char ddict[FB_MAXNAME]    = {NULL}; /* for ddict file name */
static char msg[FB_MAXLINE], fname[FB_MAXNAME];

extern short int cdb_opendb_level;	/* in opendb - controls initlock */
extern int cdb_blockread;		/* block chars read - blockeach(3) */

int fd;
long wcount;

#if !FB_PROTOTYPES
static int concat();
static long scat();
static int cat_one();
static int finit();
static void dcopy();
static void usage();
#else /* FB_PROTOTYPES */
static int concat(int, char **);
static long scat(char *);
static int cat_one(fb_database *);
static int finit(char *, int, char **);
static void dcopy(char *);
static void usage(void);
extern int main(int, char **);
#endif /* FB_PROTOTYPES */


   main(argc, argv)
      int argc;
      char *argv[];
   
      {

         (void) Dbcat_sid;

         fb_getargs(argc, argv, FB_NODB);	/* just to set system */
         if (argc < 2)
	    usage();
         fb_scrhdr((fb_database *) NULL, "Get Names");
	 if (cdb_yesmode)
	    sleep(1);				/* some are too fast */
         if (concat(argc, argv) == FB_ERROR)
            fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	 if (!cdb_batchmode){
	    fb_move(4,1), fb_clrtobot();
	    fb_scrstat("Cleaning");
	    sprintf(msg, "Cleaning %s", fname);
	    fb_fmessage(msg);
	    }
	 sprintf(msg, "%s -b %s", cdb_DBCLEAN, fname);	/* borrow msg */
	 fb_system(msg, FB_WITHROOT);
	 fb_ender();
      }

/* 
 *  concat - concat all the fb_database together into fd.
 */
 
   static int concat(argc, argv)
      char *argv[];
      int argc;

      {
         long n;
	 int j, count;
	 char dname[FB_MAXNAME], xname[FB_MAXNAME];

	 for (count = 0, j = 1; j < argc; j++)
	    if (equal("-o", argv[j]))
	       j++;
	    else if (!(argv[j][0] == '-'))
	       count++;
	 if (count < 1)
	    usage();
	 else if (count == 1){
	    if (fb_getfilename(xname, "Second Database Name: ", "") == FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
         fd = finit(fname, argc, argv);
	 fb_scrstat(fname);
	 fb_putseq(fd);			/* sequence a new fb_database */
	 fb_puthead(fd, 0L, 0L);
         if (cdb_secure)
            if (fb_putmode(fd, fb_getuid(), fb_getgid(), "666") == FB_ERROR)
               fb_xerror(FB_IO_ERROR, fname, NIL);
         fb_w_init(1, fd, -1);
	 if (!cdb_batchmode && !cdb_yesmode){
	    sprintf(msg, "Ok to Concat FirstBase Databases into %s (y/n)? ",
               fname);
	    if (fb_mustbe('y', msg, 23, 1) == FB_ERROR)
               fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
         n = 0L;
         if (!cdb_batchmode){
	    FB_XWAIT();
	    fb_gcounter(n);
	    }
	 for (j = 1; j < argc; j++)
	    if (equal("-o", argv[j]))
	       j++;
	    else if (!(argv[j][0] == '-')){
	       fb_rootname(dname, argv[j]);
	       n += scat(dname);
	       }
	 if (count == 1){
	    fb_rootname(dname, xname);
	    n += scat(dname);
	    }
         fb_wflush(1);
	 if (fb_puthead(fd, n, 0L) == FB_ERROR)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_HEADER], NIL);
	 close(fd);
	 dcopy(fname);
	 return(FB_AOK);
      }

/* 
 *  scat - cat one file into fd. return number of recs catted.
 */
 
   static long scat(f)
      char *f;
      
      {
	 fb_database *db;
	 
	 db = fb_dballoc();
	 fb_mkstr(&(db->dbase), f);
	 fb_opendb(db, READ, FB_NOINDEX, 0);
	 strcpy(ddict, db->ddict);
	 fb_getco(db);
         fb_scrtime(db);
         wcount = 0;
         fb_blockeach(db, cat_one);
	 fb_closedb(db);
         cdb_opendb_level = 0;
         return(wcount);
      }

/* 
 *  cat_one - cat one record file into fd.
 */
 
   static int cat_one(dp)
      fb_database *dp;

      {
         if (fb_w_writen(0, dp->orec, cdb_blockread) == 0)
            fb_xerror(FB_WRITE_ERROR, cdb_pgm, NIL);
         wcount++;
         if (!cdb_batchmode){
            fb_gcounter(wcount);
            }
         return(FB_AOK);
      }

/* 
 * finit - merely check for existance of fname. get fname if not on
 *    argument line. ask permission to destroy if it exists.
 */
 
   static int finit(fname, argc, argv)
      char *fname, *argv[];
      int argc;
      
      {
         char str[FB_MAXLINE], outname[FB_MAXNAME];
	 int p;
	 
	 if ((p = fb_testargs(argc, argv, "-o")) == 0){
	    if (fb_getfilename(outname, "Output File Name: ", 
	          CONCAT_BASE) == FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
	 else
	    strcpy(outname, argv[p + 1]);
	 fb_rootname(fname, outname);
	 sprintf(outname, "%s.cdb", fname);		/* cdb extension */
	 if (access(outname, 0) != -1 && !cdb_batchmode && !cdb_yesmode){
	    sprintf(str,
	       "Permission to OVERWRITE %s (y = yes, <cr> = no)? ", fname);
	    if (fb_mustbe('y', str, 15, 10) == FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
	 close(creat(outname, 0666));
	 return(fb_mustopen(outname, READWRITE));
      }

/* 
 *  dcopy - copy dictionary from ddict to fname.ddict
 */
 
   static void dcopy(fname)
      char *fname;
      
      {
         FILE *in, *out;
	 char outname[FB_MAXNAME];
	 register int c;
	 
	 sprintf(outname, "%s.ddict", fname);
	 in = fb_mustfopen(ddict, "r");
	 out = fb_mustfopen(outname, "w");
	 while ((c = fgetc(in)) != EOF)
	    fputc((char) c, out);
	 fclose(in);
	 fclose(out);
      }

/* 
 *  usage message
 */
 
   static void usage()
      {
         fb_xerror(FB_MESSAGE, 
	    "usage: dbcat [-b] [-y] [-o output] dbase [dbase ...] ", NIL);
      }
