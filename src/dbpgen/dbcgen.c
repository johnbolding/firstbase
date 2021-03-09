/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbcgen.c,v 9.1 2001/02/16 19:44:55 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)dbcgen.c	8.1 8/19/93 FB";
#endif

/* 
 *  dbcgen - conversion generator. convert a Cdb fb_database to
 *    a newly structured dbase with image/expanded/truncated fields.
 */
 
#include <fb.h>
#include <fb_vars.h>

/*
 *  assumption here is that no more than FB_MAXKEEP fields will be converted
 *  at any one time. this is needed since any number of new fields can be 
 *  used...perhaps i just got lazy. hopefully, 1000 is enough.
 *
 *  keyprint is the list of new fields.
 *  csize (matches one for one). its value determines the
 *     status of the converted fb_field. < 0 = new.
 *     positive sizes are compared to determine whether 
 *     old fb_field is image/truncated/expanded
 */

extern short int cdb_secure;
extern char *cdb_DBCLEAN;
extern char *cdb_S_EOREC;

static char *FMT1 = "%04d%03d%s";		/* secure bits */
char perm[FB_MAXNAME];

static fb_field *keyprint[FB_MAXKEEP] = { NULL };
static fb_field **pp;
static int csize[FB_MAXKEEP] = { 0 };
fb_database *hp;
static char *fld;
static long wcount;				/* write counter */
static Bflag = 0;				/* Blocking flag */

static cgen();
static condict();

/*  
 *  dbcgen main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         char title[FB_MAXLINE], filen[FB_MAXNAME], source[FB_MAXNAME], 
	      dest[FB_MAXNAME], msg[FB_MAXLINE];
         FILE *initprint();
	 int fd;

         fb_getargs(argc, argv, FB_ALLOCDB);
	 hp = cdb_db;
	 if (fb_testargs(argc, argv, "-B") > 0)
	    Bflag = 1;
	 fb_opendb(cdb_db, READ, FB_WITHINDEX, FB_MUST_INDEX);
	 fb_scrhdr(cdb_db, "Parsing"), fb_infoline();
         initprint(cdb_keymap, keyprint, hp, title, filen,
	       hp->idict, (int *) 0, csize);
	 close(creat(filen, 0666));		
	 fd = fb_mustopen(filen, 2);
	 fb_rootname(dest, filen);
	 fb_rootname(source, hp->dbase);
	 if (equal(source, dest))
	    fb_xerror(FB_MESSAGE, SYSMSG[S_ILLEGAL], "same files");
	 if (!cdb_batchmode && !cdb_yesmode){
            ctrace(keyprint, filen, csize);
	    if (fb_mustbe('y',"If accurate, enter 'y' to continue: ",
	          cdb_t_lines, 1) == FB_ERROR)
               fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    fb_move(4, 1); fb_clrtobot(); fb_infoline();
	    }
         pp = keyprint;
         if (cgen(fd, filen) == FB_ERROR)
            fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	 fb_closedb(cdb_db);
	 if (!cdb_batchmode){
	    fb_scrstat("Cleaning");
	    fb_move(4,1), fb_clrtobot();
	    sprintf(msg, "Cleaning %s", dest);
	    fb_fmessage(msg);
	    }
	 sprintf(title, "%s -b %s", cdb_DBCLEAN, dest); /* borrow title */
	 fb_system(title, FB_WITHROOT);
	 fb_ender();
      }
      
/* 
 *  cgen - generate converted data base according to list in pp 
 */
 
   static cgen(fd, filen)
      char filen[];
      int fd;
      
      {
         char fname[FB_MAXNAME];
         int gen_one();

         wcount = 0L;
         if (!cdb_batchmode){
            fb_scrstat("Converting");
	    FB_XWAIT();
	    fb_gcounter(wcount);
	    }
	 if (cdb_yesmode)
	    sleep(1);				/* some are too fast */
         fld = cdb_afld;				/* set local to global */

	 fb_putseq(fd);
	 fb_puthead(fd, 0L, 0L);
         if (cdb_secure){
            if (fb_putmode(fd, fb_getuid(), fb_getgid(), "666") == FB_ERROR)
               fb_xerror(FB_IO_ERROR, filen, NIL);
	    sprintf(perm, FMT1, fb_getuid(), fb_getgid(), "666");
            }
         fb_w_init(1, fd, -1);
         if (Bflag == 0)
	    fb_forxeach(hp, gen_one);
         else
	    fb_blockeach(hp, gen_one);
         fb_wflush(1);
         if (fb_puthead(fd, wcount, 0L) == FB_ERROR)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
	 close(fd);
         fb_basename(fname, filen);
         strcat(fname, ".ddict");
         condict(fname);
	 return(FB_AOK);
      }

/*
 * gen_one - generate exactly one -- called from a foreach style loop
 */

   gen_one(hp)
      fb_database *hp;

      {
         int k, nc;

         for (k = 0; (pp[k]->size) != 0; k++){
            if (pp[k]->type != FB_FORMULA && pp[k]->dflink == NULL){
               fld[0] = NULL;
               if (csize[k] >= 0){
                  strcpy(fld, pp[k]->fld); /* used to allow truncation */
                  if (pp[k]->type != FB_BINARY){
                     if (csize[k] < pp[k]->size)
                        fld[csize[k]] = NULL;
                     fb_trim(fld);
                     }
                  }
               else{
                  if (pp[k]->type == FB_BINARY)
                     for (nc = pp[k]->size; nc > 0; nc--)
                        fb_w_write(0, "\000");
                  }
               if (pp[k]->type != FB_BINARY){
                  fb_nextwrite(0, fld);
                  fb_w_write(0, "\000");
                  }
               else if (csize[k] >= 0)
                  fb_w_writen(0, fld, pp[k]->size);
               }
            }
         fb_nextwrite(0, " ");      	/* deletion place holder */
         if (cdb_secure)
            fb_nextwrite(0, perm);      /* permissions */
         fb_w_write(0, "\000");
         fb_w_write(0, cdb_S_EOREC);	/* end of record marker */
         ++wcount;
         if (!cdb_batchmode){
            fb_gcounter(wcount);
            }
      }

/* 
 *  condict - output the dict to the converted dbase dictionary file
 */
 
   static condict(f)
      char *f;
      
      {
         int i;
         FILE *fs, *fb_mustfopen();
   
         fs = fb_mustfopen(f, "w");
         for (i = 0; pp[i]->size != 0; i++){
            fb_unders(pp[i]);
	    if (csize[i] < 0)
	       csize[i] = -csize[i];
	    pp[i]->size = csize[i];
	    fb_sdict(pp[i], fs, 0);
            }
         fclose(fs);
      }
   
/* usage message */
   usage()
      {
         fb_xerror(FB_MESSAGE,"usage: dbcgen [-b] [-y] [-d dbase] [-i index]", NIL);
      }
