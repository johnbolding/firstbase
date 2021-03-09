/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbclean.c,v 9.2 2002/07/24 22:01:55 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbclean_sid[] = "@(#) $Id: dbclean.c,v 9.2 2002/07/24 22:01:55 john Exp $";
#endif

#define FREADWRITE 	(00444|00222)		/* read/write by all */

/*
 *  dbclean - is used to regenerate a fb_database map and rebuild
 *            the fb_database with no fb_free list gaps nor fragmented space.
 *            as long as a SEQF and some kind of header exist
 *            dbclean can recover a dbase, restoring reccnt, delcnt.
 */
 
#include <fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fb_vars.h>

#if !FB_PROTOTYPES
static void sclean();
static void makemap();
static void skipbyte();
static void goodrec();
static int nextread();
static int nextwrite();
static int wflush();
static void nextfp();
/*static void usage();*/
#else /* FB_PROTOTYPES */
extern main(int, char **);
static void sclean(char *dname);
static void makemap(int, int, int);
static void skipbyte(int);
static void goodrec(int, int, int, int);
static int nextread(int, char *);
static int nextwrite(int, char *);
static int wflush(int);
static void nextfp(void);
/*static void usage(void);*/
#endif /* FB_PROTOTYPES */

extern short int cdb_secure;
extern long cdb_headsize;
extern char cdb_FREEC;
extern char cdb_FILLC;
extern char cdb_EOREC;
extern char cdb_FILLE;
extern char *cdb_S_FILLC;
extern char *cdb_S_EOREC;
extern char *cdb_S_FILLE;

#define BLKSIZE	10240

char buffer[BLKSIZE + 1];		/* for buffered read */
char wbuffer[BLKSIZE + 1];		/* for buffered write */
int curp = -1;				/* current pointer into buffer */
int maxp = 0;				/* maximum file pointer */
int extrap = 0;				/* extra bytes at block boundry */
int truefields = 0;			/* for counting true # of fields */
long min_rlen = FB_MAPREC_SIZE + 2;
int w_curp = 0;				/* current pointer into wbuffer */

char tempfile[] = ".DBCLN_XXXXXX";	/* file for new dbase (newfd) */
char recnum[20];			/* temp space */
char tbuf[FB_MAXLINE];			/* temporary buffer */
long rpos;				/* for tracking new rpos */
long nrec;				/* for tracking record number */
long ndel;				/* for tracking fb_delete count */
fb_database *hp;			/* pointer to current database */
fb_field *fp = NULL;			/* field pointer of current field */
int nfp = 0;				/* true array offset into kp */
extern short int cdb_allow_dirty;	/* kludge to allow dirty dbase */
char ddname[FB_MAXNAME];		/* for -dd named objects */
char startdb[50];

/*
 * main - mainline logic for dbclean
 */

   main(argc, argv)
   int argc;
   char *argv[];
      {
	 char dname[FB_MAXNAME];
	 int j;
	 
         (void) Dbclean_sid;

         fb_getargs(argc, argv, FB_NODB);
         if (cdb_secure)
            strcpy(startdb, "000000000000000000000000000000");
         else
            strcpy(startdb, "00000000000000000000");
	 ddname[0] = NULL;
	 if ((j = fb_testargs(argc, argv, "-dd")) > 0)
	    strcpy(ddname, argv[j + 1]);
	 if (fb_testargs(argc, argv, "-f") > 0)
	    cdb_allow_dirty = 1;
	 for (j = 1; j < argc; j++){
	    if (!(argv[j][0] == '-')){
	       fb_rootname(dname, argv[j]);
	       sclean(dname);
	       }
	    else if (equal(argv[j], "-dd"))
	       j++;
	    }
         fb_ender();
      }
      
/*
 *  sclean - simple clean of a single fb_database argument
 */      

   static void sclean(dname)
      char *dname;
      
      {
         int newfd, j;
	 char msg[FB_MAXLINE], tname[FB_MAXNAME];
	 int headlen;
	 
	 headlen = cdb_headsize - FB_HEADSTART;
	 hp = fb_dballoc();
 	 fb_dbargs(dname, NIL, hp);
         truefields = 0;
	 if (ddname[0] != NULL){
	    sprintf(tname, SYSMSG[S_FMT_2S], ddname, SYSMSG[S_EXT_DDICT]);
	    fb_mkstr(&(hp->ddict), tname);
	    }
         if (fb_getd_dict(hp) == FB_ERROR)
            fb_xerror(FB_BAD_DICT, hp->ddict, NIL);
         hp->fd = fb_mustopen(hp->dbase, READWRITE);
         if (fb_gethead(hp) == FB_ERROR)
	    fb_xerror(FB_READ_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
	 fb_scrhdr(hp, "Check Files"); fb_infoline();
	 if (!cdb_batchmode && !cdb_yesmode){
	    fb_move(4, 1);
	    fb_printw("%s%s!!!\n%s\n%s\n",
	       "Make sure no one else is using ", hp->dbase,
	       "After cleaning database, all non-auto indexes",
	       "for that database will require regeneration.");
	    sprintf(msg, "Ready to Clean %s (y/<no>) ?", hp->dbase);
	    if (fb_mustbe('y', msg, cdb_t_lines, 1) != FB_AOK)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
	 for (j = 0; j < hp->nfields; j++){
	    if (hp->kp[j]->type != FB_FORMULA && hp->kp[j]->type != FB_LINK)
	       truefields++;
	    if (hp->kp[j]->aid != NULL){	/* abuse msg buffer */
	       sprintf(msg, "%s.idict", hp->kp[j]->aid->autoname);
	       unlink(msg);
	       }
	    }
	 truefields++;			/* for the fb_delete fb_field */
	 close(creat(hp->dmap, 0666));
	 hp->mfd = fb_mustopen(hp->dmap, READWRITE);
	 if (fb_bootmap(hp->mfd) == FB_ERROR)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_MAP], hp->dmap);
	 close(mkstemp(tempfile));
	 newfd = fb_mustopen(tempfile, WRITE);
	 fb_putseq(newfd);	/* put a new sequence here - map is new */
	 if (lseek(newfd, FB_HEADSTART, 0) < 0L)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
	 if (write(newfd, startdb, (unsigned) headlen) != headlen)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
	 makemap(hp->fd, newfd, hp->mfd);
         if (fb_puthead(newfd, nrec, ndel) == FB_ERROR)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
         if (cdb_secure)
            if (fb_putmode(newfd, fb_getuid(), fb_getgid(), "666") == FB_ERROR)
	       fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
	 if (unlink(hp->dbase) < 0)
	    fb_xerror(FB_EXEC_FAILURE, "Could not unlink: ", hp->dbase);
	 if (link(tempfile, hp->dbase) < 0)
	    if (fb_copyfile(tempfile, hp->dbase) < 0)
	       fb_xerror(FB_EXEC_FAILURE, "Could not link/copy: ", tempfile);
         chmod(hp->dbase, FREADWRITE);
	 if (unlink(tempfile) < 0)
	    fb_xerror(FB_EXEC_FAILURE, "Could not unlink: ", tempfile);
	 if (ddname[0] == NULL)
	    sprintf(msg, "dbregen -d %s -b -B", hp->dbase);
	 else
	    sprintf(msg, "dbregen -dd %s -d %s -b -B", ddname, hp->dbase);
	 fb_closedb(hp);
	 fb_refresh();
	 fb_scrstat(SYSMSG[S_AUTOREGEN]); fb_refresh();
	 fb_system(msg, FB_WITHROOT);
      }

/*
 *  makemap - read the fb_database (fd) byte-by-byte, outputting
 *            good records to the new fb_database (nfd) and recording
 *            their spots in the fb_database map (mfd).
 */
 
   static void makemap(fd, newfd, mfd)
      int fd, newfd, mfd;
       
      {
         char c;
	 int n, freeval;
	 
	 nrec = ndel = 0L;
	 if (!cdb_batchmode){ 
	    fb_move(4, 1); fb_clrtobot();
	    fb_scrstat("Cleaning");
	    FB_XWAIT();
	    fb_gcounter(0L);
	    fb_infoline();
	    }
	 if (cdb_yesmode)
	    sleep(1);				/* some are too fast */
	 if (lseek(fd, (long) cdb_headsize, 0) < 0L)
	    fb_xerror(FB_SEEK_ERROR, hp->dbase, NIL);
	 rpos = (long) cdb_headsize;
	 freeval = FB_SLONG*2;
	 for (;;){
	    if ((n = nextread(fd, &c)) == 0)
	       break;
	    else if (n != 1)
	       fb_xerror(FB_READ_ERROR, hp->dbase, NIL);
	    switch(c){
	       case '\006':			/* cdb_FREEC - freelist area */
	          curp += freeval;
	          if (curp > maxp)
		     extrap = curp - maxp;
		  break;
	       case '\030':			/* cdb_FILLC - skip region */
	          skipbyte(fd); break;
	       case '\005':			/* cdb_EOREC - no - op */
	       case '\031':			/* cdb_FILLE - no - op */
	          break;
	       default:				/* must be good */
	          ++nrec;
		  if (!cdb_batchmode){
	             fb_gcounter(nrec);
		     }
	          goodrec((int) c, fd, newfd, mfd);
		  break;
	       }
	    }
	 wflush(newfd);
	    				/* rpos is new avail */
	 if (fb_putmap(mfd, 0L, rpos, 0L, 0L) == FB_ERROR)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_MAP], NIL);
      }
      
/*
 *  skipbyte - all cdb_FILLC's to cdb_EOREC or FILLE
 */

   static void skipbyte(fd)
      int fd;
      
      {
         int n;
	 char c;
	 
	 /* checking for eorec is paranoia since all fillc's end in fille */
         for(c = NULL ; c != cdb_FILLE && c != cdb_EOREC ; ){
	    if ((n = nextread(fd, &c)) == 0)
	       break;
	    else if (n != 1)
	       fb_xerror(FB_READ_ERROR, hp->dbase, NIL);
	    }
      }

/*
 *  goodrec - process a good record
 */

   static void goodrec(ic, fd, newfd, mfd)
      int ic;
      int fd, newfd, mfd;
      
      {
         long rlen, flen;
	 int n, nfields, wlen, fmsg = 0;
         char c;

         c = (char) ic;
	 rlen = 0;
	 nfields = 0;
	 nfp = 0;
	 flen = 0;
	 nextfp();
	 for (;;){
            while (fp->type == FB_BINARY){	/* skip any FB_BINARY fields */
               for (wlen = fp->size; wlen > 0; wlen--){
                  /* skip binary section of data */
                  if (nextwrite(newfd, (char *) &c) != 1)
                     fb_xerror(FB_WRITE_ERROR, hp->dbase, NIL);
                  if ((n = nextread(fd, (char *) &c)) == 0){
                     fb_serror(FB_MESSAGE, "Warning: Unexpected EOF.", NIL);
                     return;
                     }
                  rlen++;
                  }
               nfields++;
               if (nfields < truefields){
                  nfp++;
                  nextfp();
                  }
               flen = fmsg = 0;
               }
	    if (c == '\000'){		/* NULL == fb_field seperator */
	       nfields++;
	       if (nfields < truefields){
		  nfp++;
		  nextfp();
		  }
	       flen = 0;
	       fmsg = 0;
	       }
	    else if (c == cdb_EOREC)
	       break;
	    if (c != '\000')
	       flen++;
	    if (c != '\000' && fp != NULL && flen > fp->size){
	       if (!fmsg){
		  sprintf(tbuf, "Truncating Field %s of record %ld.", 
		     fp->id, nrec);
		  fb_serror(FB_MESSAGE, tbuf, NIL);
		  fmsg = 1;
		  }
	       }
	    else{
	       if (nextwrite(newfd, (char *) &c) != 1)
		  fb_xerror(FB_WRITE_ERROR, hp->dbase, NIL);
	       rlen++; 
	       if (nfp == hp->nfields && c == CHAR_STAR)
	          ndel++;
	       }
	    if ((n = nextread(fd, (char *) &c)) == 0){
	       fb_serror(FB_MESSAGE, "Warning: Unexpected EOF.", NIL);
	       break;
	       }
	    else if (n != 1)
	       fb_xerror(FB_READ_ERROR, hp->dbase, NIL);
	    }
	 if (nfields < truefields){	/* too few fields -- add some */
	    for (nfields++; nfields < truefields; nfields++, rlen++)
	       if (nextwrite(newfd, "\000") != 1)
		  fb_xerror(FB_WRITE_ERROR, hp->dbase, NIL);
	    /* write fb_delete fb_field -- use * to mark this record deleted */
	    nextwrite(newfd, "*");
	    if (nextwrite(newfd, "\000") != 1)
	       fb_xerror(FB_WRITE_ERROR, hp->dbase, NIL);
	    rlen += 2;
	    sprintf(recnum, "%ld", nrec);
	    fb_serror(FB_MESSAGE, "Too Few Fields. Deleting Record ", recnum);
	    ndel++;
	    }
	 if (nextwrite(newfd, cdb_S_EOREC) != 1)
	    fb_xerror(FB_WRITE_ERROR, hp->dbase, NIL);
	 rlen++;
	 if (rlen < min_rlen){			/* fill hole if needed */
	    while (++rlen < min_rlen)
	       nextwrite(newfd, cdb_S_FILLC);
	    nextwrite(newfd, cdb_S_FILLE);		/* rec[rlen] = fille */
	    }
	 if (fb_putmap(mfd, nrec, 0L, rpos, rlen) == FB_ERROR)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_MAP], NIL);
	 rpos += rlen;
      }

/*
 * nextread - semi buffered read - read a block if needed.
 */
   static int nextread(fd, buf)
      int fd;
      char *buf;
      
      {
         if (curp < 0 || curp >= maxp){
	    maxp = read(fd, buffer, (unsigned) BLKSIZE);
	    if (maxp <= 0)
	       return(0);
	    curp = 0 + extrap;
	    extrap = 0;
	    }
	 *buf = buffer[curp++];
         return(1);
      }
 
/*
 * nextwrite - semi buffered write - write a block if needed.
 */
   static nextwrite(fd, buf)
      int fd;
      char *buf;
      
      {
         if (w_curp < 0 || w_curp >= BLKSIZE){
	    if (write(fd, wbuffer, (unsigned) BLKSIZE) != BLKSIZE)
	       return(0);
	    w_curp = 0;
	    }
	 wbuffer[w_curp++] = *buf;
         return(1);
      }

/*
 * wflush - flush anything left in the wbuffer
 */
   static wflush(fd)
      int fd;
      
      {
         if (w_curp > 0){
	    if (write(fd, wbuffer, (unsigned) w_curp) != w_curp)
	       return(0);
	    w_curp = 0;
	    }
         return(1);
      }

/*
 * nextfp - get the next valid fb_field descriptor and assign to fp
 */

   static void nextfp()
      {
	 fp = NULL;
         for (; nfp <= hp->nfields; nfp++){
	    if (hp->kp[nfp]->type != FB_FORMULA &&
                  hp->kp[nfp]->type != FB_LINK){
	       fp = hp->kp[nfp];
	       break;
	       }
	    }
	 if (nfp > hp->nfields)
	    fb_serror(FB_MESSAGE, "Can't set next field.", NIL);
      }

/*
 *  usage - for dbclean
 */
/*
*   static void usage()
*      {
*         fb_xerror(FB_MESSAGE,
*	    "usage: dbclean [-dd ddict] [-b] [-y] dbase [dbase ...]", NIL);
*      }
*/
