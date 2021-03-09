/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbcheck.c,v 9.1 2001/01/16 02:46:47 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbcheck_sid[] = "@(#) $Id: dbcheck.c,v 9.1 2001/01/16 02:46:47 john Exp $";
extern char Serial_sid[];
#endif

/*
 *  dbcheck - dbcheck is used to check the integrity of a 
 *          FirstBase fb_database (dbase), and its map (dmap).
 *          checks include:
 *		- tracing of all fields of records
 *		- tracing of the space on the fb_free list
 *		- attempts to account for all bytes of a fb_database file
 *		- checks for overlapping addresses in the database
 *		  (this is done by writing to disk, sorting, and reading...
 *		   could be done in memory...but who knows how big the
 *		   database might be! note that both the free list and map
 *		   are used to account for all byte spans)
 */
   
#include <fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fb_vars.h>

#if !FB_PROTOTYPES
static void scheck();
static void chkcon();
static int trace();
static int B_process();
static int B_trace();
static int tracefree();
void dbcheck_onintr();
static void usage();
#else /* FB_PROTOTYPES */
extern main(int, char **);
static void scheck(char *);
static void chkcon(void);
static int trace(fb_database *hp);
static int B_process(fb_database *);
static int B_trace(fb_database *);
static int tracefree(fb_database *);
void dbcheck_onintr(int);
static void usage(void);
#endif /* FB_PROTOTYPES */

extern long serial_number;
extern short int cdb_allow_links;
extern short int cdb_interrupt;
extern short int cdb_locklevel;
extern long cdb_headsize;
extern char cdb_EOREC;
extern short cdb_allow_dirty;		/* kludge to allow dirty dbase */

#define STDERR 		stdout

long usedbytes = 0L, 
     fragbytes = 0L, 
     freebytes = 0L,
     ndel = 0L;
long min_rlen = FB_MAPREC_SIZE + 2;
int vflag, 		/* verbose flag */
    cflag, 		/* connectivity flag */
    lflag,		/* fb_field length check flag */
    pflag, 		/* percent in use flag */
    tflag,		/* trace flag for verboisty on fields */
    qflag,		/* quick flag - skips record reads and connectivity */
    Bflag,		/* Blocking flag */
    binary_flag = 0,    /* binary fields flag */
    errflag = 0,	/* error flag */
    m_errflag = 0,	/* map error flag */
    g_errflag = 0;	/* general error flag */
struct stat sbuf;
char tempfile[] = ".DBCHK_XXXXXX";
static char *MSG_INT = "\nInterrupt\n";
int truefields = 0;	/* for counting true # of fields */
char ddname[FB_MAXNAME];	/* for -dd ddict option */

int tfd;		/* temp file descriptor */
int reclen;

extern short int cdb_opendb_level;
extern int cdb_blockread; /* block characters read - blockeach */
extern short int cdb_dbase_byteorder;
extern short int cdb_cpu_byteorder;

   main(argc, argv)
   int argc;
   char *argv[];
      {
         int j, n = 0, k;
	 char dname[FB_MAXNAME];

         (void) Dbcheck_sid;
	 cdb_batchmode = 1;
         cdb_allow_links = 0;
         fb_getargs(argc, argv, FB_NODB);
	 if (cdb_interrupt)
            fb_allow_int();
	 qflag = 1;
	 ddname[0] = NULL;
         if (cdb_locklevel == 2)
            cdb_locklevel = 1;
         for (j = 1; j < argc; j++){
	    if (argv[j][0] == '-')
	       for (k = 1; argv[j][k] != NULL; k++){
		  switch(argv[j][k]){
		     case 'c': cflag = 1; qflag = 0; break;
		     case 'l': lflag = 1; qflag = 0; break;
		     case 'p': pflag = 1; qflag = 0; break;
		     case 't': tflag = 1;
		     case 'v': lflag = vflag = pflag = cflag = 1;
			       qflag = 0;
			       break;
		     case 'd':
		        if (argv[j][k+1] == 'd' && argv[j][k+2] == 0 && 
			      k == 1){
			   strcpy(ddname, argv[++j]);
			   k = -1;
			   break;
			   }
		     case 'b': break ;
		     case 'B': Bflag = 1;
                               break;
		     default: usage();
		     }
		  if (k < 1)
		     break;
		  }
	    }
	 for (n = 0, j = 1; j < argc; j++){
	    if (j != 1)
	       fprintf(STDERR, "\n");
	    if (!(argv[j][0] == '-') && strchr(argv[j], '=') == 0){
	       fb_rootname(dname, argv[j]);
	       scheck(dname);
	       n++;
	       }
	    else {
	       if (equal(argv[j], "-dd"))
	          j++;
	       }
	    }
	 if (n == 0)
	    usage();
	 fprintf(STDERR, "\n\n<>\n");
         fb_exit(0);
      }

/*
 * scheck - simple check on a single database
 */
   static void scheck(f)
      char *f;
      
      {
         fb_database *hp;
	 int j, st;
	 char tname[FB_MAXNAME];
 
         usedbytes = cdb_headsize;
         fragbytes = 0L;
         freebytes = 0L;
	 truefields = 0;
         ndel = 0L;
	 hp = fb_dballoc();
	 fb_initlock(0, hp);
         cdb_allow_dirty = 1;
	 fb_dbargs(f, NIL, hp);
	 if (ddname[0] != NULL){
	    sprintf(tname, SYSMSG[S_FMT_2S], ddname, SYSMSG[S_EXT_DDICT]);
	    fb_mkstr(&(hp->ddict), tname);
	    }
	 fb_opendb(hp, READ, FB_NOINDEX, 0);
	 st = fb_lock_head(hp);
	 if (st == FB_ERROR)
	    fb_xerror(FB_READ_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
	 for (j = 0; j < hp->nfields; j++){
	    if (hp->kp[j]->type != FB_FORMULA && hp->kp[j]->dflink == NULL &&
                  hp->kp[j]->type != FB_LINK)
	       truefields++;
            /* with binary_flag, then NULL counting is turned off */
	    if (hp->kp[j]->type == FB_BINARY)
               binary_flag = 1;
            }
	 truefields++;		/* for the deletion fields */
	 if (qflag || Bflag)
	    cflag = 0;
	 if (cflag){
	    close(mkstemp(tempfile));
	    tfd = fb_mustopen(tempfile, WRITE);
            fb_w_init(1, tfd, -1);
	    }
         if (!Bflag){
            if (trace(hp) == FB_ERROR){
               fprintf(STDERR, "\n *** Fatal Trace ERROR dbase %s\n", f);
               g_errflag++;
	       fb_unlock_head(hp);
               return;
               }
            }
         else{
            /* Bflag invoked */
            if (B_process(hp) == FB_ERROR){
               fprintf(STDERR, "\n *** Fatal BLOCK Trace ERROR dbase %s\n", f);
               g_errflag++;
	       fb_unlock_head(hp);
               return;
               }
            }
	 if (tracefree(hp) == FB_ERROR){
	    fprintf(STDERR, "\n *** Fatal TraceFree ERROR dbase %s\n", f);
	    g_errflag++;
	    fb_unlock_head(hp);
	    return;
	    }
	 if (cflag){
	    if (m_errflag > 0){
	       fprintf(STDERR, 
	          "--> Map Entry Errors Found!! Connectivity NOT Checked.\n");
	       unlink(tempfile);
	       }
	    else
	       chkcon();
	    }
	 if (pflag){
	    stat(hp->dbase, &sbuf);
	    fprintf(STDERR, "--- bytes in use:   %10ld bytes\n", usedbytes);
	    fprintf(STDERR, "--- bytes free:     %10ld bytes\n", freebytes);
	    fprintf(STDERR, "--- bytes fragment: %10ld bytes\n", fragbytes);
	    fprintf(STDERR, "                    ----------\n");
	    fprintf(STDERR, "--- bytes in dbase: %10ld bytes\n", 
	       (long) sbuf.st_size);
	    fprintf(STDERR, "--- percent in use: %10.2f%% \n", 
	       (float) usedbytes / (float) sbuf.st_size * 100.0);
	    if (ndel > 0){
	       fprintf(STDERR, "!!! deleted bytes : %10ld\n", ndel);
	       fprintf(STDERR, "!!! del percent   : %10.2f%%\n", 
	          (float) ndel / (float) sbuf.st_size  * 100);
	       }
	    }
	 if (m_errflag > 0){
	    fprintf(STDERR, "\n--> Mapping Errors.\n");
            fprintf(STDERR,
               "    Use dbemit(1)/dbload(1), dbrestor(8) or dbclean(1).\n");
            }
	 if (errflag > 0 || g_errflag > 0){
	    fprintf(STDERR, "\n--> Data Errors Found.\n");
            fprintf(STDERR,
               "    Use dbemit(1)/dbload(1), dbrestor(8) or dbclean(1).\n");
            }
	 if (m_errflag == 0 && errflag == 0 && g_errflag == 0)
	    fprintf(STDERR,
	       "\n*** Check of database complete with no errors.\n");
	 fb_unlock_head(hp);
	 fb_closedb(hp);
         cdb_opendb_level = 0;
      }

/*
 *  chkcon - check map address connectivity
 *           file already on disk, named tempfile.
 */
 
   static void chkcon()

      {

         char com[100], tbuf[100];
	 long rpos, rlen, gpos, rec;
	 
         fprintf(STDERR, "*** checking connectivity ***\n");
         fb_wflush(1);
         fb_w_end(1);
	 sprintf(com, "sort -n -o %s %s", tempfile, tempfile);
	 fb_system(com, FB_WITHROOT);
	 if (cdb_interrupt)
	    fb_allow_int();
	 tfd = fb_mustopen(tempfile, READ);
         fb_r_init(tfd);
	 for (gpos = (long) cdb_headsize; ; gpos += rlen ){
            if (fb_nextline(tbuf, 0) == 0)
               break;
	    sscanf(tbuf, "%ld %ld %ld", &rpos, &rlen, &rec);
	    if (gpos != rpos){
	       fprintf(STDERR, 
	          "--> alignment error: gpos=%ld, rpos=%ld, rlen=%ld, ",
		  gpos, rpos, rlen);
	       fprintf(STDERR, "rec=%ld\n", rec);
	       errflag++;
	       }
	    }
	 fb_r_end();
	 if (errflag > 0)
	    fprintf(STDERR, "NOTE: Diagnostic File \"%s\" NOT Deleted\n",
	       tempfile);
	 else
	    unlink(tempfile);
      }

/*
 *  trace - trace out the values of a record
 */
 
   trace(hp)
      fb_database *hp;
   
      {
	 register int i, len;
	 int status, tchars, nullcount;
	 long avail, freep, rpos, rlen;
	 char *buf, tbuf[100];
         long n;
   
         reclen = hp->recsiz + 1;	/* recsiz + cdb_EOREC marker */
	 status = fb_getmap(hp->mfd, 0L, &avail, &freep, (long) NULL,
            (long) NULL);
	 if (status < 0){
	    fprintf(STDERR, "cannot getmap from trace() status = %d", status);
	    return(FB_ERROR);
	    }
	 status = fb_gethead(hp);
	 if (status == FB_ERROR){
	    fprintf(STDERR, "cannot gethead in trace\n");
	    return(FB_ERROR);
	    }
	 fprintf(STDERR, "*** FirstBase %s -- ", VERSION);
	 fprintf(STDERR, "Copyright by FirstBase Software\n");
	 fprintf(STDERR, "*** FirstBase Database File system Check - %s:\n",
	    hp->dbase);
	 fprintf(STDERR, "*** (Database identification number  %d)\n", 
	    fb_getseq(hp->fd));
	 fprintf(STDERR, "*** reccnt = %ld, dirty = %c, delcnt = %ld, ", 
	    hp->reccnt, hp->dirty, hp->delcnt);
	 fprintf(STDERR, "availp = %ld, freep = %ld\n", avail, freep);
	 if (hp->dirty == CHAR_1){
	    fprintf(STDERR, "--> WARNING: Database MUST be restored.\n");
	    fprintf(STDERR, 
	       "             Use dbrestor(8) or dbclean(1) to recover.\n");
	    g_errflag++;
	    }
	 fprintf(STDERR, "*** recsiz = %d\n", hp->recsiz);
	 fflush(STDERR);
	 if (qflag)
	    return(FB_AOK);
	 fprintf(STDERR, "*** checking data         *** \n");
	 fflush(STDERR);
	 for (n = 1; n <= hp->reccnt; n++ ){
	    status = fb_getmap(hp->mfd, n, &avail, &freep, &rpos, &rlen);
	    if (status == FB_ERROR){
	       fprintf(STDERR, "--> getmap error: record %ld\n", n);
	       return(FB_ERROR);
	       }
	    if (rpos < cdb_headsize){
	       fprintf(STDERR, "--> map entry error: record %ld\n", n);
	       m_errflag++;
	       continue;
	       }
	    if (vflag)
	       fprintf(STDERR, "	(%ld): rpos = %ld, rlen = %ld ", 
		  n, rpos, rlen);
	    if (cflag){
               sprintf(tbuf, "%ld %ld %ld\n", rpos, rlen, n);
               fb_nextwrite(0, tbuf);
               }
	    if (rlen < min_rlen || (rlen > reclen && reclen > min_rlen)){
	       if (vflag)
	          fprintf(STDERR, "\n");
	       fprintf(STDERR, "--> record length error: record %ld ", n);
	       fprintf(STDERR, "(expected >= %ld && <= %d, got %ld)\n", 
	          min_rlen, reclen, rlen);
	       g_errflag++;
	       continue;
	       }
	    if ((tchars = fb_getrec(n, hp)) == FB_ERROR){
	       if (vflag)
	          fprintf(STDERR, "\n");
	       fprintf(STDERR, "--> getrec error: record %ld\n", n);
	       g_errflag++;
	       continue;			/* get next record */
	       }
	    usedbytes += tchars;		/* add in only pure data */
	    fragbytes += (rlen - (long) tchars);
	    if (hp->orec[tchars-1] != cdb_EOREC){
	       fprintf(STDERR, "--> no end-of-record marker rec=%ld\n", n);
	       continue;
	       }
            if (!binary_flag){
               for (buf = hp->orec, nullcount = 0; *buf != cdb_EOREC; buf++)
                  if (*buf == NULL)
                     nullcount++;
               if (nullcount != truefields){
                  if (vflag)
                     fprintf(STDERR, "\n");
                  fprintf(STDERR, "--> wrong field count: record %ld ", n);
                  fprintf(STDERR, "(expected %d, got %d)\n", 
                     truefields, nullcount);
                  g_errflag++;
                  continue;
                  }
               }
	    for (i = 0; i < hp->nfields; i++)
	       if (cdb_kp[i]->type != FB_FORMULA && cdb_kp[i]->dflink == NULL
                     && cdb_kp[i]->type != FB_LINK
                     && cdb_kp[i]->type != FB_BINARY){
	          /* skip those nasty virtual fields */
		  if (lflag)
		     if ((len = strlen(cdb_kp[i]->fld)) > cdb_kp[i]->size){
		        if (vflag)
			   fprintf(STDERR, "\n");
			fprintf(STDERR, 
			   "--> field length error, record %ld: fld=%d", 
			   n, i+1);
			fprintf(STDERR, " expected <= %d, got %d\n",
			   cdb_kp[i]->size, len);
			g_errflag++;
			}
		  if (tflag)
		     fprintf(STDERR, "%s:", cdb_kp[i]->fld);
		  }
	    if (!qflag && tflag)
	       fprintf(STDERR, "\n");
	    if (FB_ISDELETED(hp)){
	       if (vflag)
		  fprintf(STDERR, " * deleted * "); 
	       ndel += rlen;
	       }
	    if (!qflag && (vflag || tflag))
	       fprintf(STDERR, "\n");
	    }
	 return(FB_AOK);
      }

/*
 * B_process - main loop for Block tracing.
 */

   static int B_process(hp)
      fb_database *hp;

      {
	 int status;
	 long avail, freep;
   
         reclen = hp->recsiz + 1;	/* recsiz + cdb_EOREC marker */
	 status = fb_getmap(hp->mfd, 0L, &avail, &freep, (long) NULL,
            (long) NULL);
	 if (status < 0){
	    fprintf(STDERR, "cannot getmap from trace() status = %d", status);
	    return(FB_ERROR);
	    }
	 status = fb_gethead(hp);
	 fprintf(STDERR, 
	    "*** FirstBase Serial Number: Copyright by FirstBase Software\n");
	 fprintf(STDERR, "*** FirstBase Database File system Check - %s:\n",
	    hp->dbase);
	 fprintf(STDERR, "*** (Database identification number  %d)\n", 
	    fb_getseq(hp->fd));
	 fprintf(STDERR, "*** reccnt = %ld, dirty = %c, delcnt = %ld, ", 
	    hp->reccnt, hp->dirty, hp->delcnt);
	 fprintf(STDERR, "availp = %ld, freep = %ld\n", avail, freep);
	 if (hp->dirty == CHAR_1){
	    fprintf(STDERR, "--> WARNING: Database MUST be restored.\n");
	    fprintf(STDERR, 
	       "             Use dbrestor(8) or dbclean(1) to recover.\n");
	    g_errflag++;
	    }
	 fprintf(STDERR, "*** recsiz = %d\n", hp->recsiz);
	 fflush(STDERR);
	 if (qflag)
	    return(FB_AOK);
         fprintf(STDERR,
            "*** WARNING: With -B, map connectivity is NOT traced.\n");
	 fprintf(STDERR, "*** checking data         *** \n");
	 fflush(STDERR);
         status = fb_blockeach(hp, B_trace);
         return(status);
      }

/*
 *  B_trace - Block trace out the values of a record - No Map Reading!
 */
 
   static int B_trace(hp)
      fb_database *hp;
   
      {
	 register int i, len;
	 int tchars, nullcount;
	 long rlen, n;
	 char *buf;

         rlen = cdb_blockread;
         n = hp->rec;
         tchars = cdb_blockread;
         usedbytes += tchars;			/* add in only pure data */
         fragbytes += (rlen - (long) tchars);
         if (hp->orec[tchars-1] != cdb_EOREC){
            fprintf(STDERR, "--> no end-of-record marker rec=%ld\n", n);
            g_errflag++;
            return(FB_AOK);
            }
         if (!binary_flag){
            for (buf = hp->orec, nullcount = 0; *buf != cdb_EOREC; buf++)
               if (*buf == NULL)
                  nullcount++;
            if (nullcount != truefields){
               if (vflag)
                  fprintf(STDERR, "\n");
               fprintf(STDERR, "--> wrong field count: record %ld ", n);
               fprintf(STDERR, "(expected %d, got %d)\n", 
                  truefields, nullcount);
               g_errflag++;
               return(FB_AOK);
               }
            }
         for (i = 0; i < hp->nfields; i++)
            if (cdb_kp[i]->type != FB_FORMULA && cdb_kp[i]->dflink == NULL
                  && cdb_kp[i]->type != FB_LINK
                  && cdb_kp[i]->type != FB_BINARY){
               /* skip those nasty virtual fields */
               if (lflag)
                  if ((len = strlen(cdb_kp[i]->fld)) > cdb_kp[i]->size){
                     if (vflag)
                        fprintf(STDERR, "\n");
                     fprintf(STDERR, 
                        "--> field length error, record %ld: fld=%d", 
                        n, i+1);
                     fprintf(STDERR, " expected <= %d, got %d\n",
                        cdb_kp[i]->size, len);
                     g_errflag++;
                     }
               if (tflag)
                  fprintf(STDERR, "%s:", cdb_kp[i]->fld);
               }
         if (!qflag && tflag)
            fprintf(STDERR, "\n");
         if (FB_ISDELETED(hp)){
            if (vflag)
               fprintf(STDERR, " * deleted * "); 
            ndel += rlen;
            }
         if (!qflag && (vflag || tflag))
            fprintf(STDERR, "\n");
	 return(FB_AOK);
      }

/*
 *  tracefree - trace the fb_free list
 */
 
   static int tracefree(hp)
      fb_database *hp;
   
      {
	 long avail, mfreep, freep, rlen, lastp, llen, slen;
         char tbuf[100];
   
	 fprintf(STDERR, "*** checking free list    ***\n");
	 if (fb_getmap(hp->mfd,0L,&avail,&mfreep,(long)NULL,(long)NULL)==FB_ERROR){
#if DEBUG
	    fprintf(STDERR, "error (tracefree): returned from getmap()\n");
#endif /* DEBUG */
	    }
	 llen = 0L;
	 slen = 9999999;			/* should be maxlong */
	 freep = mfreep;
	 while (freep != 0L){
	    lastp = freep;
	    if (lseek(hp->fd, freep+1L, 0L) < 0){ /* seek past fb_free marker */
#if DEBUG
	       fprintf(STDERR, "tracefree: bad lseek to %ld\n", freep);
#endif /* DEBUG */
	       return(FB_ERROR);
	       }
	    if (read(hp->fd, (char *) &rlen, FB_SLONG) != FB_SLONG){
#if DEBUG
	       fprintf(STDERR, "tracefree: bad read (fd:rlen)\n");
#endif /* DEBUG */
	       return(FB_ERROR);
	       }
            if (cdb_dbase_byteorder != cdb_cpu_byteorder)
               M_32_SWAP(rlen);
	    if (rlen > llen)
	       llen = rlen;			/* save max for reporting */
	    if (rlen < slen)
	       slen = rlen;			/* save min for reporting */
	    if (read(hp->fd, (char *) &freep, FB_SLONG) != FB_SLONG){
#if DEBUG
	       fprintf(STDERR, "tracefree: bad read (fd:freep)\n");
#endif /* DEBUG */
	       return(FB_ERROR);
	       }
            if (cdb_dbase_byteorder != cdb_cpu_byteorder)
               M_32_SWAP(freep);
	    freebytes += rlen;
	    if (vflag)
	       fprintf(STDERR, 
		  "@freep = %10ld, f-len = %10ld, tot-bytes = %10ld\n",
		  lastp, rlen, freebytes);
	    if (cflag){
	       sprintf(tbuf, "%ld %ld -1\n", lastp, rlen);
               fb_nextwrite(0, tbuf);
               }
	    }
	 if (vflag && mfreep != 0L)
	    fprintf(STDERR, "(*) longest/shortest free are %ld %ld\n", 
	       llen, slen);
	 return(FB_AOK);
      }

/*
 * private onintr routine to enable unlinking of temp file
 */

   void dbcheck_onintr(disp)
      int disp;

      {
         (void) disp;

	 unlink(tempfile);
	 fflush(stdout);
	 fprintf(stderr, MSG_INT);
	 fflush(stderr), 
	 fb_exit(2); 
      }

/*
 *  usage - for dbcheck
 */
   static void usage()
      {
         fprintf(STDERR,
	    "usage: dbcheck [-dd ddict] [-{clptv}] dbase [dbase ...]\n");
	 fb_exit(1);
      }
