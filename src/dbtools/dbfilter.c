/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbfilter.c,v 9.2 2001/02/16 19:47:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbfilter_sid[] = "@(#) $Id: dbfilter.c,v 9.2 2001/02/16 19:47:51 john Exp $";
#endif

/* 
 *  dbfilter.c - target is a standard Cdb index based on a text
 *	file. Look up the lines of the loadfile, generate an
 *	index entry for these. Report on those not found.
 */

#include <fb.h>
#include <fb_vars.h>

#if !FB_PROTOTYPES
extern main();
static int filter();
static int getindex();
static int nextindex();
static FILE *finit();
void usage();
#else /* FB_PROTOTYPES */
extern main(int, char **);
static int filter(FILE *);
static int getindex(char *, fb_database *);
static int nextindex(char *, fb_database *);
static FILE *finit(int, char **);
void usage(void);
#endif /* FB_PROTOTYPES */

static int vflag = 0;			/* verbose flag */
static int tflag = 0;			/* trace flag */
static int sflag = 0;			/* sflag - sort flag */
static int aflag = 0;			/* aflag - all matches flag */
static int column = 0;			/* column cut off for matching */
static int ksize = 0;			/* strlen of key */
static char filter_fb_basename[FB_MAXNAME];
static char filter_idx[FB_MAXNAME];
static char filter_idict[FB_MAXNAME];
static int idx_fd;
static int idict_fd;

#define FILTER_BASENAME	"filter"
#define FILTER_LOADNAME	"FILTER"


/*
 *  dbfilter - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         char str[FB_MAXLINE];
         FILE *fs;


         (void) Dbfilter_sid;

	 if (fb_testargs(argc, argv, "-") > 0)
            cdb_batchmode = 1;	/* force cdb_batchmode if using stdio */
         fb_getargs(argc, argv, FB_ALLOCDB);
	 fb_opendb(cdb_db, READ, FB_WITHINDEX, FB_MUST_INDEX);
	 fb_allow_int();		/* allows instant piping */	
         fb_scrhdr(cdb_db, "Open Files"); fb_infoline();
	 fs = finit(argc, argv);
	 if (!cdb_batchmode && !cdb_yesmode){
	    sprintf(str, "Ok to Generate Filter %s (y/n) ?",
               filter_fb_basename);
	    if (fb_mustbe('y', str, cdb_t_lines, 1) == FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
         if (filter(fs) == FB_ERROR)
	    fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	 fclose(fs);
	 fb_closedb(cdb_db);
	 fb_ender();
      }

/* 
 *  filter - filter all lines from a load file (fs) into a Cdb index
 */
 
   static int filter(fs)
      FILE *fs;
      
      {
         long n = 0L;
         char line[FB_MAXLINE], *p, key[FB_MAXLINE], msg[FB_MAXLINE];
         char com[FB_MAXLINE];

         if (!cdb_batchmode){
	    FB_XWAIT();
	    fb_gcounter(n);
	    }
         /* init target filter index */

	 for (;;){
            if (fgets(line, FB_MAXLINE, fs) == NULL)
               break;
            if ((p = strchr(line, FB_NEWLINE)) != 0)
               *p = NULL;
            line[column] = NULL;
            strcpy(key, line);
            if (FB_OFNUMERIC(cdb_db->ip[0]->type))
	       fb_makess(key, cdb_db->ip[0]->type, cdb_db->ip[0]->size);
            ksize = strlen(key);
            if (tflag && cdb_batchmode)
               fb_serror(FB_MESSAGE, key, NIL);
            if (getindex(key, cdb_db) != FB_AOK){
               fb_serror(FB_MESSAGE, "Could not find: ", key);
               continue;
               }

            for (;;){
               n++;
               if (!cdb_batchmode)
                  fb_gcounter(n);
               fb_nextwrite(0, cdb_db->irec);	/* enter into filter index */
               if (!aflag)
                  break;
               /* aflag is set here, so find the next match */
               if (nextindex(key, cdb_db) != FB_AOK)
                  break;
               }
	    }
	 fb_wflush(1);
         close(idx_fd);
	 fb_putxhead(idict_fd, n, n);
         close(idict_fd);

         if (sflag){
            if (!cdb_batchmode){
               fb_scrstat("Sorting");
               fb_move(4,1), fb_clrtobot();
               sprintf(msg, "Sorting %s", filter_fb_basename);
               fb_fmessage(msg);
               }
            sprintf(com, "sort -o %s %s", filter_idx, filter_idx);
            fb_system(com, FB_WITHROOT);
            }
	 if ((!cdb_batchmode && !cdb_yesmode)){
	    if (!cdb_batchmode)
	       fb_fmessage(NIL);
	    sprintf(com, "There were %ld Records Selected", n);
	    fb_serror(FB_MESSAGE, com, NIL);
	    }
         return(FB_AOK);
      }

/*
 *  getindex - try and get an index record loaded
 *	this is fb_getxrec(3) without the getrec tied in.
 */

   static getindex(s, dp)
      char *s;
      fb_database *dp;
      
      {
         long rec;
	 int st = FB_ERROR;
         char *p;
	 
         /* b_tree handled if needed */
         if (dp->b_tree){
            rec = fb_btree_search(s, dp->b_idx, dp->b_seq);
            if (rec > 0 && rec <= dp->reccnt)
               st = FB_AOK;
            p = fb_key_ptr(dp->b_seq);
            strcpy(dp->irec, p);
            dp->irec[dp->irecsiz-1] = FB_NEWLINE;
            }
         /* standard FB index */
         else{
            rec = fb_megasearch(dp->ifd, s, 0, 1, dp->bsend, dp->bsmax,
               dp->irecsiz, 1, dp->irec);
            if (rec > 0 && rec <= dp->bsmax){
               rec = atol((char *) 
                  (dp->irec + dp->irecsiz - 11));	/* FB_RECORDPTR + 1 */
               if (rec > 0 && rec <= dp->reccnt)
                  st = FB_AOK;
               }
            }
	 return(st);
      }

/*
 *  nextindex - force the next indexed record to be loaded.
 */

   static int nextindex(s, dp)
      char *s;
      fb_database *dp;
      
      {
         long rec;
	 int st = FB_ERROR;
         fb_bseq *bs;
         char *p;
	 
         if (dp->b_tree){				/* btree path */
            bs = dp->b_seq;
            /*
             * the current node has to be re-read in case a key was deleted
             * by some other process.
             */
            /*
            if (bs->bs_curkey >= 1 && bs->bs_curkey <= 3)
               fb_seq_getrec(bs->bs_recno, bs);
            */
            for (;;){
               if (bs->bs_curkey >= 3){
                  if (bs->bs_next == 0)
                     return(st);
                  fb_seq_getrec(bs->bs_next, bs);
                  bs->bs_curkey = 0;
                  }
               bs->bs_curkey++;
               rec = fb_key_eval(bs);
               if (rec == 0)
                  bs->bs_curkey = 4;
               else if (rec > 0L && rec <= dp->reccnt)	/* ignore rec of 0 */
                  break;
               }
            p = fb_key_ptr(bs);
            if (strncmp(p, s, ksize) == 0){
               strcpy(dp->irec, p);
               dp->irec[dp->irecsiz-1] = FB_NEWLINE;
               st = FB_AOK;
               }
            }
         else {
            while (read(dp->ifd, dp->irec, (unsigned) dp->irecsiz) ==
                  dp->irecsiz){
               dp->bsrec += 1L;
               rec = atol((char *) (dp->irec + dp->irecsiz - 11));
               if (rec > 0L && rec <= dp->reccnt){	/* ignore rec of 0 */
                  if (strncmp(s, dp->irec, ksize) == 0)
                     st = FB_AOK;
                  break;
                  }
               }
            }
	  return(st);
      }

/*
 *  finit - intialize --- merely check for existance.
 */
 
   static FILE *finit(argc, argv)
      int argc;
      char *argv[];
      
      {
         char str[FB_MAXLINE], fname[FB_MAXNAME], seq[FB_SEQSIZE + 1];
         char buf[FB_MAXLINE];
	 FILE *fs;
         long bsmax, bsorg;
	 int i;
	 
	 fname[0] = NULL;
         strcpy(filter_fb_basename, FILTER_BASENAME);
         
	 for (i = 1; i < argc; i++)
	    if (argv[i][0] == '-'){		/* skip dbase stuff */
	       if (argv[i][1] == 'd' || argv[i][1] == 'i')
	          i++;
	       else if (argv[i][1] == 't')
                  tflag = 1;
	       else if (argv[i][1] == 'a')
                  aflag = 1;
	       else if (argv[i][1] == 'v')
                  vflag = 1;
	       else if (argv[i][1] == 's')
                  sflag = 1;
	       else if (argv[i][1] == 'c'){	/* get column indicator */
                  if (++i >= argc)
                     usage();
	          column = atoi(argv[i]);
                  if (column >= FB_MAXLINE)
                     fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE],
                        "Column Wrong");
                  }
	       else if (argv[i][1] == 'f'){	/* get filter name */
                  if (++i >= argc)
                     usage();
	          strcpy(filter_fb_basename, argv[i]);
                  }
	       }
	    else{
	       strcpy(fname, argv[i]);
	       break;
	       }
         if (column == 0)
            column = cdb_db->irecsiz;
         /* build target filter (index) names */
         sprintf(filter_idx,
               SYSMSG[S_FMT_2S], filter_fb_basename, SYSMSG[S_EXT_IDX]);
         sprintf(filter_idict,
               SYSMSG[S_FMT_2S], filter_fb_basename, SYSMSG[S_EXT_IDICT]);
	 if (equal(filter_idx, cdb_db->dindex))
	    fb_xerror(FB_ABORT_ERROR,
               "Filter_index MUST be distinct from index", NIL);
	 if (access(filter_idx, 0) != -1 && !cdb_batchmode && !cdb_yesmode){
	    sprintf(str, 
	       "Permission to OVERWRITE filter `%s' (y = yes, <cr> = no)? ",
               filter_fb_basename);
	    if (fb_mustbe('y', str, 15, 10) == FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
         
	 if (fname[0] == NULL){		/* requires full path name */
            if (cdb_batchmode == 0){
               strcpy(str, "FILTER");
               if (fb_getfilename(fname, "Filter Lookup File: ", str) ==
                     FB_ERROR)
                  fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
               }
	    }
         if (fname[0] != NULL){
            sprintf(str, "from %s", fname);
            fb_scrstat(str);
            fs = fb_mustfopen(fname, "r");
            }
         else
            fs = stdin;

         /* lastly, create the new filter (index) files */
         close(creat(filter_idx, 0666));
         close(creat(filter_idict, 0666));
         if ((idict_fd = open(filter_idict, READWRITE)) < 0)
            fb_xerror(FB_CANT_OPEN, filter_idict, NIL);
         sprintf(seq, SYSMSG[S_FMT_04D], fb_getseq(cdb_db->fd));
         lseek(idict_fd, FB_SEQSTART, 0);		/* write SEQF */
         write(idict_fd, seq, FB_SEQSIZE);
         fb_putxhead(idict_fd, 0L, 0L);		/* positions fds also */
         fb_r_init(cdb_db->ihfd);
         fb_getxhead(cdb_db->ihfd, &bsmax, &bsorg);
         for(; fb_nextline(buf, FB_MAXLINE) != 0;){
            if (buf[0] == '%')
               break;
            strcat(buf, "\n");
            write(idict_fd, buf, strlen(buf));
            }
         close(cdb_db->ihfd);
         close(idict_fd);

	 idict_fd = open(filter_idict, READWRITE);
	 if ((idx_fd = open(filter_idx, READWRITE)) < 0)
	    fb_xerror(FB_CANT_OPEN, filter_idx, NIL);

	 fb_w_init(1, idx_fd, -1);

         return(fs);
      }

/* 
 *  usage message 
 */
 
   void usage()
      {
         fb_xerror(FB_MESSAGE,
	    "usage: dbfilter [-d dbase] [-i index] [filter options] file",NIL);
      }
