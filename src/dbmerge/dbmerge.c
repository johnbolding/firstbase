/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbmerge.c,v 9.1 2001/02/16 19:44:23 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)dbmerge.c	8.5 04 Apr 1996 FB";
#endif

/*
 * dbmerge.c - provides an interface between nroff (etc) and
 *   the cdb system. field_name tokens on the fb_input stream are replaced
 *   before being written to the output stream by the fields
 *   of a single record. the fb_input stream is rewound, and
 *   the process started again with a new record (the next in the index).
 *
 *   this is the cdb version of mail-merge systems.
 */

#include <dbdmrg_v.h>

#define SCREENLIMIT 15
#define BEGINSYMBOL	"_BEGIN_"
#define ENDSYMBOL	"_END_"

static char inpname[FB_MAXNAME] = {""};	/* input file name */

int counter;				/* record counter */
int outfd;				/* output file descriptor */
int ncount = 0;				/* -n ncount records only flag */
int Bflag = 0;				/* -B blocking factor flag */

fb_aline *a_begin = NULL;
fb_aline *a_end = NULL;

extern short int cdb_mergecols;
extern short int cdb_interrupt;
extern short int cdb_screenprint;

static initm();
static mparse();
static merge();
static mkpart();
static checkbegin();

/*
 *  merge - main driver
 */
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         char *buf, filen[FB_MAXNAME];

	 if (fb_testargs(argc, argv, "-") > 0)
	    cdb_batchmode = 1;
         fb_getargs(argc, argv, FB_ALLOCDB);
	 hp = cdb_db;
	 if (fb_testargs(argc, argv, "-i") > 0)
	    fb_opendb(cdb_db, READ, FB_WITHINDEX, FB_MUST_INDEX);
         else
	    fb_opendb(cdb_db, READ, FB_NOINDEX, FB_MUST_INDEX);
         fb_scrhdr(cdb_db, "Parsing"); fb_infoline();
         initm(argc, argv, filen);
         buf = (char *) fb_malloc(MAX(hp->recsiz,hp->irecsiz)+10);
	 if (!cdb_yesmode && !cdb_batchmode)
            mparse(filen);
	 if (!cdb_yesmode && !cdb_batchmode){
            if (fb_mustbe('y',
	             "If accurate, enter 'y' to continue:",
                     cdb_t_lines, 1) == FB_ERROR)
               fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    fb_move(3, 1), fb_clrtobot(), fb_infoline();
	    }
	 if (cdb_interrupt)
	    fb_allow_int();
         if (merge(buf, filen) == FB_ERROR)
            fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	 fb_closedb(cdb_db);
	 if (!cdb_batchmode && cdb_screenprint)
	    fb_screenprint(filen);
         fb_ender();
      }
      
/* 
 *  initm - initialize the merge process - open fb_input, check output... 
 */
 
   static initm(argc, argv, filen)
      int argc;
      char *argv[], *filen;
      
      {
         int i;
	 char err[FB_MAXLINE];
	 
	 filen[0] = NULL;
         linewidth = cdb_mergecols;
	 for (i = 1; i < argc; i++){
	    if (argv[i][0] == '-' && argv[i][1] == 'w'){
               i++;
               linewidth = atoi(argv[i]);
               }
	    else if (argv[i][0] == '-' && argv[i][1] == 'n'){
               i++;
               ncount = atoi(argv[i]);
               }
	    else if (argv[i][0] == '-' && argv[i][1] != NULL){
               switch(argv[i][1]){
                  case 'B': Bflag = 1;
                  case 'b':
                  case 'y':
                     break;
                  default:
                     i++;
                  }
	       }
	    else if (inpname[0] == NULL){
	       strcpy(inpname, argv[i]);
	       }
	    else if (filen[0] == NULL)
	       strcpy(filen, argv[i]);
	    }
         if (linewidth == 0)
            linewidth = 80;
	 if (cdb_batchmode && (strlen(filen) == 0 || strlen(inpname) == 0))
	    usage();
	 if (inpname[0] == NULL){
	    if (fb_getfilename(inpname, "Input File Name: ", "merge.in")==FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
	 if (filen[0] == NULL)
	    if (fb_getfilename(filen, "Output File Name: ", "merge.out")==FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
         if (strcmp(filen, inpname) == 0)
	    fb_xerror(FB_MESSAGE, SYSMSG[S_ILLEGAL], "Output/Input same file");
	 if (!cdb_batchmode && !cdb_yesmode)
	    if (access(filen, 0) != -1){
	       fb_move(4, 1);
	       fb_bell();
	       sprintf(err, 
		  "Permission to OVERWRITE %s (y=yes, <anyother>=no)? ",
		   filen);
	       if (fb_mustbe('y', err, 15, 10) == FB_ERROR)
		  fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	       }
         fb_readmrg(inpname);
	 mpcur = mphead;
         checkbegin();
         mpcur->mp_acur = mpcur->mp_ahead;
         mpcur->mp_atop = mpcur->mp_ahead;
      }
	 
/* 
 *  mparse - display list of tokens
 */
 
   static mparse(filen)
      char *filen;
      
      {
         int i, row;
         char *p, tbuf[FB_MAXLINE];
         fb_aline *a;
         fb_token *t;
	 
	 row = 1;
         fb_move(3,1);
         fb_printw("Input: %s   Output: %s", inpname, filen);
         fb_move(5, 1);
         fb_printw("Tokens: \n");
	 for (a = mpcur->mp_ahead; a != NULL; a = a->a_next){
            for (p = a->a_text, t = a->a_thead; *p; p++){
               if (*p == CHAR_DOLLAR && t != NULL){
                  if (t->t_field != NULL)
                     sprintf(tbuf, "$%s", t->t_field->id);
                  else
                     sprintf(tbuf, "$");
                  sprintf(tbuf, "%-10s[%2d]", tbuf, t->t_width);
                  /* display it */
                  if (row > SCREENLIMIT){
                     if (row <= SCREENLIMIT+1)
                        FB_PAUSE();
                     fb_move(6,1);
                     fb_clrtobot();
                     fb_infoline();
                     fb_move(6, 1);
                     row = 1;
                     }
	          fb_printw("   ... %s\n", tbuf);
	          row++;

                  /* move on to next token */
                  t = t->t_next;
                  }
               }
	    }
      }

/* 
 *  merge - merge an fb_input file across an indexed fb_database 
 */
 
   static merge(buf, filen)
      char *buf, filen[];
      
      {
         int mkpart();
         fb_aline *a;

         counter = 0L;
	 if (!cdb_batchmode){
            fb_scrstat("Merging");
	    FB_XWAIT();
	    fb_gcounter(counter);
	    }
	 if (equal(filen, "-"))
	    outfd = 1;
	 else{
	    close(creat(filen, 0666));
	    outfd = fb_mustopen(filen, WRITE);
            }
         /* fb_trim any white space out */
	 for (a = mpcur->mp_ahead; a != NULL; a = a->a_next)
            fb_trim(a->a_text);
	 for (a = a_begin; a != NULL; a = a->a_next)
            fb_trim(a->a_text);
	 for (a = a_end; a != NULL; a = a->a_next)
            fb_trim(a->a_text);
         /* initialize the output file */
         fb_w_init(1, outfd, 0);
	 /* print any BEGINSYMBOL stuff  */
	 for (a = a_begin; a != NULL; a = a->a_next){
            fb_nextwrite(0, a->a_text);
            fb_nextwrite(0, "\n");
            }
	 if (hp->dindex == NULL || hp->dindex[0] == NULL || hp->ihfd < 0){
            if (Bflag == 0)
               fb_foreach(hp, mkpart);
            else
               fb_blockeach(hp, mkpart);
            }
	 else
            fb_forxeach(hp, mkpart);
	 /* print any ENDSYMBOL stuff  */
	 for (a = a_end; a != NULL; a = a->a_next){
            fb_nextwrite(0, a->a_text);
            fb_nextwrite(0, "\n");
            }
         fb_wflush(1);
	 if (outfd != 1)
	    close(outfd);
         return(FB_AOK);
      }

/* 
 *  mkpart - make part of output for this buf (record) 
 */
 
   static mkpart(hp)
      fb_database *hp;
      
      {
         char *fld, *tmp, *p, *fb_formdollar(), *fb_rmlead();
         fb_aline *a;
         fb_token *t;
         fb_field *f;
         int width;
	 
         ++counter;
         /* test whether to force drop out of foreach/forxeach loop */
         if (ncount > 0 && counter > ncount)
            return(FB_ERROR);
         if (!cdb_batchmode)
            fb_gcounter(counter);
	 fld = cdb_afld; tmp = cdb_bfld;	/* set locals to globals */
         /* set current aline to place beyond BEGINSYMBOL */
	 for (a = mpcur->mp_ahead; a != NULL; a = a->a_next){
            for (p = a->a_text, t = a->a_thead; *p; p++){
               if (*p == CHAR_DOLLAR && t != NULL){
                  if (t->t_field != NULL){
                     f = t->t_field;
                     fb_fetch(f, fld, hp);
                     if ((width = t->t_width) == 0)
                        width = f->size;
                     fb_formfield(tmp, fld, f->type, width);
                     if (t->t_width > 0 && strlen(tmp) != t->t_width)
                        fb_pad(fld, tmp, t->t_width);
                     else
                        strcpy(fld, tmp);
                     }
                  else
                     strcpy(fld, "$");
                  fb_nextwrite(0, fld);
                  t = t->t_next;
                  }
               else
                  fb_w_write(0, p);
               }
            fb_w_write(0, "\n");
	    }
         return(FB_AOK);
       }

/*
 * checkbegin - check for BEGINSYMBOL and ENDSYMBOL stuff
 *       remove these sections from the list, use alone as text.
 */

   static checkbegin()

      {
         fb_aline *a;
         char *p;

	 for (a = mpcur->mp_ahead; a != NULL; a = a->a_next){
            p = a->a_text;
            if (strncmp(p, BEGINSYMBOL, 7) == 0){
               if (a_begin == NULL)
                  a_begin = a->a_next;
               else{
                  /* looking at last BEGINSYMBOL */
                  mpcur->mp_ahead = a->a_next;          /* patch new head */
                  a->a_next->a_prev = NULL;
                  a->a_prev->a_next = NULL;           /* patch begin list */
                  }
               }
            else if (strncmp(p, ENDSYMBOL, 5) == 0){
               if (a_end == NULL){
                  /* looking at first ENDSYMBOL */
                  a_end = a->a_next;
                  mpcur->mp_atail = a->a_prev;
                  a->a_prev->a_next = NULL;           /* patch tail of list */
                  a->a_prev = NULL;
                  }
               else{
                  /* looking at last ENDSYMBOL */
                  a->a_prev->a_next = NULL;           /* patch end list */
                  }
               }
            }
      }

/* 
 *  usage message 
 */
 
   usage()
      {
         fb_xerror(FB_MESSAGE, 
	    "usage: dbmerge [-d dbase] [-i index] [-b] [-y] input {output|-}",
	    NIL);
      }
