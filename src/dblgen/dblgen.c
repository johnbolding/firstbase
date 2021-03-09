/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dblgen.c,v 9.1 2001/02/16 19:43:22 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)dblgen.c	8.3 11/26/93 FB";
#endif

/*
 *  dblgen - generate labels (uses idictl)
 */

#include <fb.h>
#include <fb_vars.h>

#define LABEL_SIZE 55
#define MAXLABEL 250

extern short int cdb_interrupt;
extern short int cdb_screenprint;
extern short int cdb_secure;

static int curlbl = 0;			/* to mark current label point */
static int justify = 0;			/* set to right justify header line */
static int asize = 0;			/* default label size */

static lgen();
static maklab();
static void dump();
static one_lgen();
static usage();

/*
 *  surely there will not be more that FB_MAXKEEP (1000) fields used
 *  for a label. lets hope not, anyway...
 *
 *  keyprint is the list of fields in the label.
 *     each line has a label pointer slot of NULL to terminate the
 *     list of fields for that line. there may be a max of 5
 *     such lines.
 */
 
fb_field *keyprint[FB_MAXKEEP] = { NULL };
fb_field **pp;
fb_database *hp;

int nostat_label = 0;
int headlines = 0;
int taillines = 0;

char *pline[5], crec[FB_RECORDPTR+1], fld[MAXLABEL];
long rec, n, atol();
FILE *fs, *initlabel();
int fmt;

/*
 *  dblgen - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         char filen[FB_MAXNAME];
         int p;

	 asize = LABEL_SIZE;
         fb_getargs(argc, argv, FB_ALLOCDB);
	 if (fb_testargs(argc, argv, "-n"))
	    nostat_label = 1;
	 if ((p = fb_testargs(argc, argv, "-H")) > 0)
	    headlines = atoi(argv[p + 1]);
	 if ((p = fb_testargs(argc, argv, "-T")) > 0)
	    taillines = atoi(argv[p + 1]);
	 hp = cdb_db;
	 fb_opendb(cdb_db, READ, FB_WITHINDEX, FB_MUST_INDEX);
         fb_scrhdr(cdb_db, "Parsing"); fb_infoline();
         fs = initlabel(cdb_keymap, keyprint, hp, 
                        filen, argv, hp->idict, &fmt, &asize);
	 if (fs == NULL){
	    sprintf(filen, "%sl", hp->idict);
	    fb_xerror(FB_BAD_DICT, filen, NIL);
	    }
	 if (!cdb_batchmode && !cdb_yesmode){
            ltrace(keyprint, filen, fmt, asize);
            if (fb_mustbe('y',
	          "If accurate, enter 'y' to continue:",
                  cdb_t_lines, 1) == FB_ERROR)
               fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    fb_move(4, 1), fb_clrtobot(), fb_infoline();
	    }
	 if (fmt < 0){
	    fmt = -fmt;
	    justify = 1;
	    }
	 if (fmt * asize >= MAXLABEL)
	    fb_xerror(FB_MESSAGE, SYSMSG[S_TOO_LONG], NIL);
	 if (cdb_interrupt)
	    fb_allow_int();
         pp = keyprint;
         if (lgen() == FB_ERROR)
            fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	 fb_closedb(cdb_db);
	 fclose(fs);
	 if (!cdb_batchmode && cdb_screenprint)
	    fb_screenprint(filen);
         fb_ender();
      }
                  
/* 
 *  lgen - generate labels according to list in pp 
 */
 
   static lgen()

      {
         int i, one_lgen();

         n = 0L;
	 if (!cdb_batchmode){
            fb_scrstat("Printing");
	    FB_XWAIT();
	    fb_gcounter(n);
	    }
	 if (cdb_yesmode)
	    sleep(1);				/* some are too fast */
	 for (i = 0; i < 5; i++){
	    pline[i] = (char *) fb_malloc(MAXLABEL+1);
	    pline[i][0] = NULL;
	    }
         fb_forxeach(hp, one_lgen);
	 if (nostat_label == 0){
	    sprintf(fld, " %ld labels printed", n);
	    strcat(pline[0], fld);
	    curlbl++;
	    }
	 if (curlbl > 0){
	    curlbl = fmt - 1;	/* force dump */
	    dump(pline, fmt, fs);
	    }
	 return(FB_AOK);
      }

   static one_lgen(hp)
      fb_database *hp;

      {
         maklab(pline, pp);
         dump(pline, fmt, fs);
         ++n;
         if (!cdb_batchmode)
            fb_gcounter(n);
         return(FB_AOK);
      }

/* 
 *  maklab - make a label. fb_put into plines 
 */
 
   static maklab(pline, pp)
      char *pline[];
      fb_field *pp[];

      {
         char fld[MAXLABEL], pfld[MAXLABEL], *fb_trim(), tmp[MAXLABEL];
	 int i, p, j, head;

         strcpy(pfld, " ");
	 if (pp[0] == NULL)
	    head = 0;
	 else
	    head = 1;
	 for (p = 0, i = 0; ; i++){
	    if (pp[i] == NULL){
	       if (head == 1 && p == 0 && justify == 1){ /* justify header  */
		  sprintf(fld, "%100s", " ");
		  j = asize - strlen(pfld) - 1;
		  if (j < 0)
		     j = 0;
		  fld[j] = NULL;
		  strcat(fld, pfld);
		  }
	       else{				/* normal case */
	          fb_rmlead(pfld);
		  sprintf(fld, "%-100s", pfld);
	          fb_rmlead(fld);
		  fld[asize] = NULL;
		  }
	       strcpy(pfld, fld);
	       if (strlen(pfld) > 0)
		  strcat(pline[p++], pfld); 
	       pfld[0] = fld[0] = NULL;
	       continue;
	       }
	    else if (pp[i]->size == 0)
	       break;
	    if (pp[i]->size > 0){
	       fb_fetch(pp[i], fld, hp);
	       fb_formfield(tmp, fld, pp[i]->type, pp[i]->size);
	       fb_rmlead(fb_trim(tmp));
	       strcpy(fld, tmp);
	       }
	    else
	       strcpy(fld, pp[i]->id);
	    strcat(pfld, fld);
	    }
	 sprintf(fld, "%-100s", " ");
	 fld[asize] = NULL;
	 for (; p < 5; p++)
	    strcat(pline[p], fld);
	 }

/* 
 *  dump - dump plines is fmt labels already made 
 */
 
   static void dump(pline, fmt, fs)
      char *pline[];
      int fmt;
      FILE *fs;

      {
	 char *fb_trim();
	 int i;

	 if (++curlbl < fmt)
	    return;
	 for (i = 0; i < headlines; i++)
	    fprintf(fs, "\n");
	 for (i = 0; i < 5; i++){
	    fprintf(fs, "%s\n", fb_trim(pline[i]));
	    pline[i][0] = NULL;
	    }
	 for (i = 0; i < taillines; i++)
	    fprintf(fs, "\n");
	 curlbl = 0;
	 fprintf(fs, "\n");
      }

/* 
 *  usage message 
 */
 
   static usage()
      {
         fb_xerror(FB_MESSAGE,
            "usage: dblgen [-d dbase] [-i index] [-b] [-y]", NIL);
      }
