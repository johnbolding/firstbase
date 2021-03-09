/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbregen.c,v 9.1 2001/01/16 02:46:48 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbregen_sid[] = "@(#) $Id: dbregen.c,v 9.1 2001/01/16 02:46:48 john Exp $";
#endif

#include <fb.h>
#include <fb_vars.h>

char ddname[FB_MAXNAME];			/* for -dd named objects */
int forceflag = 0;
static char pass_env[FB_MAXLINE];
extern short int cdb_autobtree;

#if !FB_PROTOTYPES
static regen_btree();
static test_for_btree();
static int autoregen();
static int regen();
static void get_env();
/*static void usage();*/
#else /* FB_PROTOTYPES */
static int regen_btree(char *, char *, char *, char *, char *);
static int test_for_btree(char *, char *);
static int test_for_btree(char *, char *);
static int autoregen(fb_database *, char *);
static int regen(char *, char *, int);
static void get_env(char *, int, char **);
extern int main(int, char **);
/*static void usage(void);*/
#endif /* FB_PROTOTYPES */

/*
 *  dbregen - generate all the autoindexes indicated.
 *     either system to dbigen , or just sort if its an autoindex.
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         int j;
	 char flags[10], buf[FB_MAXLINE];

         (void) Dbregen_sid;
         fb_getargs(argc, argv, FB_ALLOCDB);
         get_env(pass_env, argc, argv);
         
	 ddname[0] = NULL;
	 if ((j = fb_testargs(argc, argv, "-dd")) > 0)
	    strcpy(ddname, argv[j + 1]);
         if (fb_getd_dict(cdb_db) == FB_ERROR)
            fb_xerror(FB_BAD_DICT, cdb_db->ddict, NIL);
	 flags[0] = NULL;
	 if (!cdb_batchmode){
            fb_scrhdr(cdb_db, "ReGenerate AutoIdx");
	    fb_infoline();
	    fb_refresh();
	    }
	 else
	    strcat(flags, "-b ");
	 if (cdb_yesmode)
	    strcat(flags, "-y ");
	 if (fb_testargs(argc, argv, "-B") > 0)
	    strcat(flags, "-B ");
	 if (fb_testargs(argc, argv, "-f") > 0){
	    strcat(flags, "-f ");
            forceflag = 1;
            }
	 if (fb_testargs(argc, argv, "-n") == 0){
	    for (j = 0; j < cdb_db->nfields; j++)
	       if (cdb_db->kp[j]->aid != NULL){
		  if (regen(cdb_db->kp[j]->aid->autoname, flags, 0)==FB_ERROR)
		     if (!cdb_batchmode){
			sprintf(buf,"Cannot regenerate autoindex %s", 
			   cdb_db->kp[j]->aid->autoname);
			fb_serror(FB_MESSAGE, buf, NIL);
			}
		     }
	    }
	 for (j = 1; j < argc; j++)
	    if (equal("-d", argv[j]) || equal("-i", argv[j]))
	       j++;
	    else if (!(equal("-b", argv[j])) && 
	             !(equal("-y", argv[j])) &&
	             !(equal("-f", argv[j])) &&
		     !(equal("-n", argv[j])))
	       regen(argv[j], flags, 1);
         autoregen(cdb_db, flags);
         fb_ender();
      }

/*
 *  regen - regenerate the index for fname. if igen=1, force dbigen.
 */

   static int regen(fname, flags, igen)
      char *fname, *flags;
      int igen;
      
      {
         char buf[FB_MAXNAME], fidx[FB_MAXNAME], fidict[FB_MAXNAME];
         char fidicti[FB_MAXNAME], rname[FB_MAXNAME];
	 int fd;
	 long bsmax = 0, bsend = 0;

         if (forceflag)
            igen = 1;

	 fb_rootname(rname, fname);
	 sprintf(fidicti, "%s.idicti", fname);
	 sprintf(fidict, "%s.idict", fname);
	 sprintf(fidx, "%s.idx", fname);

         if (cdb_autobtree || test_for_btree(fname, fidicti))
            return(regen_btree(fname, rname, fidicti, fidict, flags));
	 
	 if (access(fidx, 0) == 0 && access(fidict, 0) == 0){
	    if ((fd = open(fidict, 2)) < 0)
	       return(FB_ERROR);
	    FB_CHECKERROR(fb_getxhead(fd, &bsmax, &bsend))
	    close(fd);
	    if (bsmax > bsend)		/* force a resort only */
	       igen = 0;
            else if (!igen && bsmax == bsend)
	       return(FB_ERROR);
	    }
	 /* if either idx or idict file is gone, regenerate with igen */
	 if (igen || (access(fidx, 0) != 0 || access(fidict, 0) != 0)){
	    sprintf(buf, "%s.idicti", rname);
	    if (access(buf, 0) == 0){
	       fb_scrstat(rname);
	       fb_refresh();
	       if (ddname[0] == NULL)
	          sprintf(buf, "dbigen -d %s -i %s %s",cdb_db->dbase,
                     rname, flags);
	       else
	          sprintf(buf, "dbigen -dd %s -d %s -i %s %s",
		     ddname, cdb_db->dbase, rname, flags);
               strcat(buf, pass_env); 
	       fb_system(buf, FB_WITHROOT);
	       fb_scrhdr(cdb_db, NIL); fb_infoline();
	       fb_refresh();
	       }
	    }
	 else {		/* else redo the header and sort the index (idx) */
	    if (!cdb_batchmode){
	       fb_basename(buf, fidx);
	       strcat(buf, ":Auto");
               fb_scrstat(buf);
	       fb_refresh();
	       }
	    if ((fd = open(fidict, 2)) < 0)
	       return(FB_ERROR);
	       
	    /* no checking of SEQF here - assume creation was ok. */
	    FB_CHECKERROR(fb_getxhead(fd, &bsmax, &bsend))
	    FB_CHECKERROR(fb_putxhead(fd, bsmax, bsmax))
	    close(fd);
	    sprintf(buf, "sort -o %s %s", fidx, fidx);
	    fb_system(buf, FB_NOROOT);
	    }
	 return(FB_AOK);
      }

/*
 *  regen_btree - regenerate the index for fname. if igen=1, force dbigen.
 */

   static int regen_btree(fname, rname, fidicti, fidict, flags)
      char *fname, *rname, *fidicti, *fidict, *flags;
      
      {
         char buf[FB_MAXLINE];

         if (access(fidicti, 0) == 0){
            fb_scrstat(rname);
            fb_refresh();
            if (ddname[0] == NULL)
               sprintf(buf, "dbigen -d %s -i %s %s -btree",
                  cdb_db->dbase, fname, flags);
            else
               sprintf(buf, "dbigen -dd %s -d %s -i %s %s -btree",
                  ddname, cdb_db->dbase, fname, flags);
            strcat(buf, pass_env); 
            fb_system(buf, FB_NOROOT);
            fb_scrhdr(cdb_db, NIL); fb_infoline();
            fb_refresh();
            }
	 return(FB_AOK);
      }

/*
 * test_for_btree - test the idicti and idict for btree status
 *	return either 0 or 1.
 */

   static int test_for_btree(fidicti, fidict)
      char *fidicti, *fidict;

      {
         FILE *fs;
         char line[FB_MAXLINE], lline[FB_MAXLINE];
         long bsmax, bsend;
         int ret = 0, fd;

         /* read all lines of idicti file --- if last is %\n, btree */
         fs = fopen(fidicti, "r");
         if (fs == NULL)
            return(0);

         while (fgets(line, FB_MAXLINE, fs) != NULL)
            strcpy(lline, line);
         fclose(fs);
         if (equal(lline, "%\n"))
            return(1);

         /* read all lines of idict file --- if last is %\n, btree */
         fd = open(fidict, READ);
         if (fd <= 0)
            return(0);
         fb_r_init(fd);
	 if (fb_getxhead(fd, &bsmax, &bsend) == FB_ERROR)
	    return(0);
         for(; fb_nextline(line, FB_MAXLINE) != 0; )
            if (line[0] == '%'){
               ret = 1;
               break;
               }
         close(fd);
         return(ret);
      }

   static int autoregen(db, flags)
      fb_database *db;
      char *flags;

      {
         int fd;
         char fname[FB_MAXLINE], iname[FB_MAXLINE], line[FB_MAXLINE];
         
	 fb_rootname(fname, db->ddict);
         strcat(fname, ".auto");
         if (access(fname, 0) < 0)
            return(FB_AOK);
         fd = open(fname, READ);
         if (fd <= 0){
            fb_serror(FB_CANT_OPEN, fname, NIL);
            return(FB_ERROR);
            }
         fb_r_init(fd);
         for (; fb_nextline(line, FB_MAXLINE) != 0; ){
            fb_getword(line, 1, fname);
	    fb_dirname(iname, db->ddict);
            strcat(iname, fname);
            regen(iname, flags, 0);
            }
         close(fd);
         return(FB_AOK);
      }

   static void get_env(p, argc, argv)
      char *p, *argv[];
      int argc;

      {
         int i;

         strcpy(p, " ");
         for (i = 1; i < argc; i++)
            if (strchr(argv[i], '=') != NULL){
               strcat(p, argv[i]);
               strcat(p, " ");
               }
      }

/* 
 * usage message 
 */
/*
*   static void usage()
*      {
*         fb_xerror(FB_MESSAGE,
*	    "usage: dbregen [-d dbase] [-n] [-b] [-y] [index1 ...]", NIL);
*      }
*/
