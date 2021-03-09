/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbjoin.c,v 9.1 2001/02/16 19:47:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbjoin_sid[] = "@(#) $Id: dbjoin.c,v 9.1 2001/02/16 19:47:51 john Exp $";
#endif

#include <fb.h>
#include <fb_vars.h>

#if !FB_PROTOTYPES
static finit();
static int join();
static int readit();
static int joinrecs();
static void sput();
static void checkids();
static void joindicts();
static void jtrace();
#else /* FB_PROTOTYPES */
extern main(int, char **);
static finit(int, char **);
static int join(int);
static int readit(char *, fb_database *hs, fb_field **, char *, int);
static int joinrecs(int, int);
static void sput(int, fb_database *, fb_field *);
static void checkids(void);
static void joindicts(void);
static void jtrace(void);
#endif /* FB_PROTOTYPES */

extern char *cdb_DCDB;
extern char *cdb_DIDX;
extern short int cdb_secure;
extern char *cdb_S_EOREC;

/* 
 *  algorithm: let normal init procedure pull dbase1 into the master
 *  header, hp, and its associated cdb_keymap, cdb_kp.
 *  then, make hj point to header (&master) and redo hp to point
 *  to a newly allocated header to be used for dbase2.
 *  reset the file names (dbase, dmap!, ddict, index, idict) and 
 *  re-initialize.
 *  the index pointer array, ip, is handled similarly, swapped with
 *  the dbase1 area, ij. 
 *  the fb_database keymaps are reached through their respective header
 *  structres (hp->kp, and hj->kp).
 *
 *  to join, we are using two databases and two indexes.
 *  read one from each index, if index values do not match,
 *  either skip least valued one or pad and write (if -a).
 *  this demands that the indexes be SORTED. (standard Cdb sort is ascending).
 *
 *  if they match, output the common field (ie the indexed one) and
 *  then all OTHER fields from dbase1's record followed by all other
 *  fields from dbase2's record.
 *
 *  usage: dbjoin [-a] [-d dbase] [-i index] [-d2 dbase2] [-i2 index2]
 */

/*
 *  dbase1 variables and data structures 
 */
      
static fb_field *ij[FB_MAXIDXS] = { NULL };	/* index fields of dbase1 */
static fb_database *hj = NULL;		/* header of dbase1 */
static fb_database *hp = NULL;
static char dataname[FB_MAXNAME] = {""};

/*
 *  dbase2 stuff is the normal cdb_kp, ip, and hp found in fb_vars.h,
 *  although dbase1 stuff is pushed through these first...
 */

static int aflag         = 0;		/* for -a flag */
static char *abuf	 = NULL;	/* standard dbase1 buffer area */
static char *bbuf	 = NULL;	/* standard dbase2 buffer area */
static char dbase1[FB_MAXNAME] = {""};
static char dmap1[FB_MAXNAME] = {""};
static char index1[FB_MAXNAME] = {""};
static char outname[FB_MAXNAME]= {""};	/* output file name */
static char perm[FB_MAXNAME];
static char *FMT1 = "%04d%03d%s";	/* secure bits */

extern char *cdb_DBCLEAN;

/* 
 *  dbjoin main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         int fd;
	 char cmd[FB_MAXLINE], msg[FB_MAXLINE];

         (void) Dbjoin_sid;
         fb_getargs(argc, argv, FB_ALLOCDB);
	 hp = cdb_db;
	 fb_opendb(cdb_db, READ, FB_WITHINDEX, FB_MUST_INDEX);
         fb_scrhdr(cdb_db, "Open Files"); fb_infoline();
	 fd = finit(argc, argv);
         fb_w_init(1, fd, -1);
	 if (!cdb_yesmode && !cdb_batchmode){
            jtrace();			/* everything it needs is global */
	    if (fb_mustbe('y',"If accurate, enter 'y' to continue: ",
	           cdb_t_lines, 1) == FB_ERROR)
               fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    fb_move(3, 1), fb_clrtobot(), fb_infoline();
	    }
         if (join(fd) == FB_ERROR)
            fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	 close(fd);
	 joindicts();
	 fb_closedb(hp);
	 fb_closedb(hj);
	 if (!cdb_batchmode){
	    fb_move(4,1), fb_clrtobot();
	    fb_scrstat("Cleaning");
	    sprintf(msg, "Cleaning %s", outname);
	    fb_fmessage(msg);
	    }
	 sprintf(cmd, "%s -b %s", cdb_DBCLEAN, outname);
	 fb_system(cmd, FB_WITHROOT);
	 fb_ender();
      }

/* 
 *  finit - initialize dbase to join (with index). set aflag.
 *     also, do all re-allocation for system since first dbase may not
 *     be the maximum size of the two.
 */
 
   static finit(argc, argv)
      int argc;
      char *argv[];
      
      {
         int i, flen, argp;
	 char dname[FB_MAXNAME], iname[FB_MAXNAME];
         char str[FB_MAXNAME], fname[FB_MAXNAME];
	 
         aflag = 0;
	 if (fb_testargs(argc, argv, "-a") > 0)
	    aflag = 1;
	 hj = hp;		/* set dbase1 fb_database pointer */
	 for (i = 0; i <= (hp->ifields - 1); i++){	/* save ip into ij */
	    ij[i] = cdb_ip[i];
	    cdb_ip[i] = NULL;
	    }
	 flen = cdb_fieldlength;
	 hp = fb_dballoc();			/* set hp to join dbase2 */
	 
	 /* 
	  *  now all important stuff is saved so we can read in 
	  *  a new fb_database dictionary, et al.
	  */
	 
	 if ((argp = fb_testargs(argc, argv, "-d2")) > 0)
	    fb_rootname(dname, argv[argp + 1]);
	 else
	    dname[0] = NULL;
	 for (;;){
	    if (dname[0] == NULL)
	       if (fb_getfilename(dname, "Join DataBase Name: ",
                     cdb_DCDB) == FB_ERROR)
	          fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    if (!equal(dname, fb_basename(fname, hj->dbase)))
	       break;
	    if (argp == 0)
	       fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], "non-distinct dbases");
	    else
	       fb_xerror(FB_MESSAGE, SYSMSG[S_ILLEGAL], "non-distinct dbases");
            dname[0] = NULL;
	    }
	 
	 /* now do the index */
	 if ((argp = fb_testargs(argc, argv, "-i2")) > 0)
	    fb_rootname(iname, argv[argp + 1]);
	 else
	    iname[0] = NULL;
	 for (;;){
	    if (iname[0] == NULL)
	       if (fb_getfilename(iname, "Join Index Name: ",
                     cdb_DIDX) == FB_ERROR)
	          fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    if (!equal(iname, fb_basename(fname, hj->dindex)))
	       break;
	    if (argp == 0)
	       fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL],"non-distinct indexes");
	    else
	       fb_xerror(FB_MESSAGE, SYSMSG[S_ILLEGAL],"non-distinct indexes");
            iname[0] = NULL;
	    }

	 fb_basename(dbase1, hj->dbase);	/* save orig file names */
	 fb_basename(dmap1, hj->dmap);		/* not needed with new hp..*/
	 fb_basename(index1, hj->dindex);
	 fb_dbargs(dname, iname, hp);
	 fb_opendb(hp, READ, FB_WITHINDEX, FB_MUST_INDEX);
	 fb_infoline();
	 
         checkids();		/* check for unique ids, types of indexes */
	 
         /* 
	  *  get and check output file
	  */
	 
	 if ((argp = fb_testargs(argc, argv, "-o")) > 0)
	    fb_rootname(outname, argv[argp + 1]);
	 else
	    outname[0] = NULL;
         fb_move(2, 1); fb_clrtobot(); fb_infoline(); fb_refresh();
	 for (;;){
	    if (outname[0] == NULL)
	       if (fb_getfilename(outname, "Join Output File: ","join")
                     == FB_ERROR)
	          fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    sprintf(fname, "%s.cdb", outname);
	    strcpy(dataname, fname);
	    if (strcmp(outname, hp->dbase) == 0 || 
	        strcmp(outname, dbase1) == 0){
	       if (argp == 0)
	          fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], "IO collision");
	       else
	          fb_xerror(FB_MESSAGE, SYSMSG[S_ILLEGAL], "IO collision");
	       }
	    else
	       break;
	    }
	 if (!cdb_batchmode && !cdb_yesmode && access(fname, 0) != FB_ERROR){
	    sprintf(str, 
	       "Permission to OVERWRITE %s (y = yes, <cr> = no)? ", fname);
	    if (fb_mustbe('y', str, 15, 10) == FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
	 close(creat(fname, 0666));
	    
	 /* 
	  * redo storage allocation: two databases need to be in use
	  * at the same time, with eveything intact
	  */
	 
	 i = MAX(hj->irecsiz, hp->irecsiz) + 10;
	 abuf = (char *) fb_malloc((unsigned) i);
	 bbuf = (char *) fb_malloc((unsigned) i);
	 
	 /* reset cdb_afld/cdb_bfld to longest cdb_fieldlength */
	 fb_free(cdb_afld);
	 fb_free(cdb_bfld);
	 cdb_afld =
            (char *) fb_malloc((unsigned) MAX(cdb_fieldlength, flen) + 1);
	 cdb_bfld =
            (char *) fb_malloc((unsigned) MAX(cdb_fieldlength, flen) + 1);
	 if (!cdb_batchmode){
	    fb_basename(dname, hp->dbase);
	    fb_basename(iname, hp->dindex);
            sprintf(str, "%s/%s", dname, iname);
	    fb_scrstat(str);
	    }
	 return(fb_mustopen(fname, 2));
      }

/*
 *  join - join datafiles using the globals kj,ij,hj and cdb_kp,ip,hp.
 *     fb_put the output into fd.
 */

   static int join(fd)
      int fd;

      {
         long n;
	 int eof1, eof2, st;

         n = 0L;
	 if (!cdb_batchmode){
            FB_XWAIT();
	    fb_gcounter(n);
	    fb_scrstat("Joining");
	    }
	 if (cdb_yesmode)
	    sleep(1);				/* some are too fast */
	 fb_putseq(fd);
	 fb_puthead(fd, 0L, 0L);
         if (cdb_secure){
            if (fb_putmode(fd, fb_getuid(), fb_getgid(), "666") == FB_ERROR)
               fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_HEADER], NIL);
            sprintf(perm, FMT1, fb_getuid(), fb_getgid(), "666");
            }
	 abuf[0] = NULL; bbuf[0] = NULL;
         fb_x_init(2, hj->ifd, hp->ifd);
         for(eof1 = eof2 = 0; (eof1 != EOF || eof2 != EOF); ){
	    if (eof1 != EOF && abuf[0] == NULL)
	       eof1 = readit(abuf, hj, ij, cdb_afld, 0);
	    if (eof2 != EOF && bbuf[0] == NULL)
	       eof2 = readit(bbuf, hp, cdb_ip, cdb_bfld, 1);
	    if (eof1 == EOF && eof2 == EOF)
	       break;

            /*
	     * now we have hp and hs loaded with records
	     * abuf and bbuf are used for local storage, and as flags.
	     * and cdb_afld and cdb_bfld are loaded with the index values
	     * used to determine if the records can be laminated (joined)
	     * together -- till cgen do they part.
	     * note, they are not fb_put together if there is no match...
	     * (unless -a specified)
	     */
	     
	    if (eof1 != EOF && eof2 == EOF)
	       st = joinrecs(fd, -1);			/* do abuf */
	    else if (eof1 == EOF && eof2 != EOF)
	       st = joinrecs(fd, 1);			/* do bbuf */
	    else 
	       st = joinrecs(fd, strcmp(cdb_afld, cdb_bfld));/* maybe both */
	    if (st == FB_AOK){
	       ++n;
	       if (!cdb_batchmode)
	          fb_gcounter(n);
	       }
	    }
         fb_wflush(1);
	 if (fb_puthead(fd, n, 0L) == FB_ERROR)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_HEADER], NIL);
	 return(FB_AOK);
      }

/* 
 * readit - read in a single record and index.
 *   store 1st index field into sfld, record stored in hs(header) per normal.
 *   if reached eof, return EOF...
 */

   static int readit(rbuf, hs, is, sfld, ch)
      fb_database *hs;
      fb_field **is;
      char *sfld, *rbuf;
      int ch;

      {
         char crec[FB_RECORDPTR+1];
	 long rec;
         int cpoint, max;

         cpoint = hs->irecsiz - 11;
         max = hs->irecsiz + 2;
	 for (;;){
	    if (fb_x_nextline(ch, rbuf, max) == 0)
	       return(EOF);
	    fb_ffetch(is[((hs->ifields) - 1)], crec, rbuf, hs);
	    rec = atol(crec);
            strncpy(sfld, rbuf, cpoint);
            sfld[cpoint] = NULL;
	    fb_trim(sfld);
	    if (rec < 1L || rec > hs->reccnt)
	       continue;		/* ignore autoindex deletions */
	    if (fb_getrec(rec, hs) == FB_ERROR)
	       fb_xerror(FB_FATAL_GETREC, hs->dbase, (char *) &rec );
            if (cdb_secure && fb_record_permission(hs, READ) == FB_ERROR)
               continue;
	    if (hs->kp[hs->nfields]->fld[0] != '*')
	       break;
	    /* else it must be deleted, so get next one */
	    }
	 return(FB_AOK);
      }

/*  
 * joinrecs - join two recs together (to fd) depending on status.
 *    status of -1,0,1 = left side, pure join, right side.
 *    if a buffer is used, set it to NULL for future use.
 *    NOTE: must be careful to write the null byte between fields,
 *          as well as the eorec symbol.
 */
 
   static int joinrecs(fd, status)
      int status, fd;
      
      {
	 int i;

	 if (status < 0){			/* fb_put dbase1 stuff */
	    if (!aflag){
	       abuf[0] = NULL;
	       return(FB_ERROR);
	       }
	    fb_nextwrite(0, cdb_afld);		/* unique to dbase1 (a) */
	    fb_w_writen(0, "\000", 1);		/* end-of-fb_field marker */
	    sput(fd, hj, ij[0]);
	    for (i = 1; i < hp->nfields; i++)
	       fb_w_writen(0, "\000", 1);	/* fb_put place holders */
	    abuf[0] = NULL;
	    }
	  else if (status > 0){			/* fb_put dbase2 stuff */
	    if (!aflag){
	       bbuf[0] = NULL;
	       return(FB_ERROR);
	       }
	    fb_nextwrite(0, cdb_bfld);		/* unique to dbase2 (a) */
	    fb_w_writen(0, "\000", 1);		/* end-of-fb_field marker */
	    for (i = 1; i < hj->nfields; i++)
	       fb_w_writen(0, "\000", 1);	/* fb_put place holders */
	    sput(fd, hp, cdb_ip[0]);
	    bbuf[0] = NULL;
	    }
	 else {					/* indexes match: put both */
	    fb_nextwrite(0, cdb_afld);			/* common */
	    fb_w_writen(0, "\000", 1);		/* end-of-fb_field marker */
	    sput(fd, hj, ij[0]);
	    sput(fd, hp, cdb_ip[0]);
	    abuf[0] = bbuf[0] = NULL;
	    }
	 fb_nextwrite(0, " ");
         if (cdb_secure)
 	    fb_nextwrite(0, perm);		/* permissions */
	 fb_w_writen(0, "\000", 1);		/* end-of-fb_field marker */
	 fb_w_writen(0, cdb_S_EOREC, 1);	/* fb_put eorec */
	 return(FB_AOK);
      }

/* 
 *  sput - simple fb_put of buf depending on header/ks/is variables.
 *         fb_put all fields except the one sf points to.
 *         in all cases, write the null byte also.
 */
 
   static void sput(fd, hs, sf)
      int fd;
      fb_database *hs;
      fb_field *sf;
      
      {
         int i;

         for (i = 0; i < hs->nfields; i++){
	    if (sf != hs->kp[i] && 
	          hs->kp[i]->dflink == NULL && hs->kp[i]->type != FB_FORMULA){
               if (hs->kp[i]->type != FB_BINARY){
                  fb_nextwrite(0, hs->kp[i]->fld);
                  fb_w_writen(0, "\000", 1);
                  }
               else
                  fb_w_writen(0, hs->kp[i]->fld, hs->kp[i]->size);
	       }
	    }
      }

/* 
 *  checkids - check that all names in kj and cdb_kp are unique between
 *    them. allow the first indexed fb_field to be the same name.
 */
 
    static void checkids()
       {
          int j, k;
	  
          for (j = 0; j < hj->nfields; j++)
	     if (hj->kp[j] != ij[0])
		for (k = 0; k < hp->nfields; k++)
		   if (equal(hp->kp[k]->id, hj->kp[j]->id))
		      fb_xerror(FB_MESSAGE, 
		         "Duplicate field name: ", hp->kp[k]->id);
          for (k = 0; k < hp->nfields; k++)
	     if (hp->kp[k] != cdb_ip[0])
		for (j = 0; j < hj->nfields; j++)
		   if (equal(hj->kp[j]->id, hp->kp[k]->id))
		      fb_xerror(FB_MESSAGE, 
		         "Duplicate field name: ", hp->kp[j]->id);
       }

/* 
 *  joindicts - output the join of the dictionaries --
 *    first the common field, then all dbase1, then dbase2.
 */
 
   static void joindicts()
      {
         FILE *fs;
	 char fname[FB_MAXNAME];
	 int i;
	 
	 sprintf(fname, "%s.ddict", outname);
	 fs = fb_mustfopen(fname, "w");
	 fb_sdict(ij[0], fs, 0);
	 for (i = 0; i < hj->nfields; i++)
	    if (hj->kp[i] != ij[0])
	       fb_sdict(hj->kp[i], fs, 0);
	 for (i = 0; i < hp->nfields; i++)
	    if (hp->kp[i] != cdb_ip[0])
	       fb_sdict(hp->kp[i], fs, 0);
	 fclose(fs);
      }

/* 
 *  jtrace - trace the fields to be joined...
 */
 
   static void jtrace()
      {
         int j, k, row;
	 
	 fb_move(3,10);
	 fb_printw(" Output File:    %s", outname);
	 fb_move(4, 10);
	 fb_printw(" Join On Fields: %s (%c %d) <--> %s (%c %d)",
	    ij[0]->id, ij[0]->type, ij[0]->size,
	    cdb_ip[0]->id, cdb_ip[0]->type, cdb_ip[0]->size);
	 fb_move(5, 10);
	 fb_printw(" Type of Join:   ");
	 if (aflag == 0)
	    fb_printw("index matches only");
	 else
	    fb_printw("all entries");
	 j = k = 0;
	 for (row = 7; (j < hj->nfields) || (k < hp->nfields); j++,k++,row++){
	    if (row > 22){
	       FB_PAUSE();
	       row = 5;
	       fb_move(row, 1); fb_clrtobot();
	       }
	    if (j < hj->nfields && hj->kp[j] != ij[0]){
	       fb_move(row, 9);
	       fb_printw("(%-10s %c %6d)", hj->kp[j]->id, hj->kp[j]->type, 
	          hj->kp[j]->size);
	       }
	    if (k < hp->nfields && hp->kp[k] != cdb_ip[0]){
	       fb_move(row, 40);
	       fb_printw("(%-10s %c %6d)", hp->kp[k]->id, hp->kp[k]->type, 
	          hp->kp[k]->size);
	       }
	    }
      }

/* 
 *  usage message
 *
 * static usage()
 *    {
 *       fb_xerror(FB_MESSAGE, "usage: dbjoin [-a] [-d dbase] [-i index]",
 *              " [-d2 dbase2] [-i2 index2] [-o output]");
 *    }
 */
