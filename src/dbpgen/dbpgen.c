/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbpgen.c,v 9.2 2001/02/16 19:44:55 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)dbpgen.c	8.7 04 Apr 1996 FB";
#endif

/*
 *  dbpgen.c - printout generator (uses idictp)
 */

#include <fb.h>
#include <fb_vars.h>
#include <pgen.h>

extern short int cdb_interrupt;
extern short int cdb_screenprint;
extern short int cdb_secure;
extern char *cdb_coname;

static int pwidth = 0;			/* pagewidth */
static int detail = 1;			/* default = print detail  */

fb_database *hp;				/* used to tie to db */
static pgen();
static breakline();
static mktotal();
static char *blanks();
static mkdash();
static fformat();
static void headers();

static int short firsttime = 1;
static char pline[FB_PRINTLINE + 1];
static char pline1[FB_PRINTLINE + 1];
static long n;
static char hdr1[FB_PRINTLINE + 1], hdr2[FB_PRINTLINE + 1];
static char crec[FB_RECORDPTR+1], *fld, *tmp, *p;
static char bline[FB_PRINTLINE + 1];
static long rec;
static int i, j, k, len, hsize, bflag = -1, maxbrk, max, one_pgen();
static struct totlist *tp, *tempp, *temphead;
struct totlist *fb_copylist();
static struct brklist *bp;
static struct totlist *thead;
static struct brklist *bhead;
static FILE *fs = NULL;
FILE *initprint();
static int fmt;

/* 
 *  assume that only a max of FB_MAXKEEP fields will be printed at any
 *  one time.
 *
 *  keyprint is an array of all the fields to use to generate printout from.
 */
 
static fb_field *keyprint[FB_MAXKEEP] = { NULL };
static fb_field **pp;

char blankline[FB_PRINTLINE] = {""};  		/* for blanks() */
extern int pagelength, formlength, formfeed;
static char title[FB_PRINTLINE];

/*
 *  dbpgen - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         char filen[FB_MAXNAME];
         int csize[3], lc, pflag = 0;

	 if (fb_testargs(argc, argv, "-") > 0){
	    cdb_batchmode = 1;
	    fs = stdout;
	    }
	 if ((lc = fb_testargs(argc, argv, "-p")) > 0){
            formlength = atoi(argv[lc + 1]);
            pagelength = formlength - 6;
            pflag = 1;
            }
	 if (fb_testargs(argc, argv, "-f"))
            formfeed = 1;
         fb_getargs(argc, argv, FB_ALLOCDB);
	 if (cdb_batchmode)
	    fb_allow_int();		/* allows instant piping */	
	 hp = cdb_db;
	 fb_opendb(cdb_db, READ, FB_WITHINDEX, FB_MUST_INDEX);
         fb_scrhdr(cdb_db, "Parsing"); fb_infoline();
         initprint(cdb_keymap, keyprint, cdb_db, 
                        title, filen, cdb_db->idict, &fmt, csize);
	 if (fs == NULL)
	    fs = fb_mustfopen(filen ,"w");
         else if (fs == stdout && pflag == 0){
	    pagelength = 23;
	    formlength = 24;
            }
         pwidth = csize[0];
         if (pwidth < 0){
	    pwidth = -pwidth;
	    detail = 0;
	    }
	 inittotals(keyprint, &thead, &bhead);
	 if (!cdb_batchmode && !cdb_yesmode){
            lc = pr_trace(keyprint, title, filen, fmt, pwidth, detail);
	    ftrace(thead, bhead, lc);
	    if (fb_mustbe('y',"If accurate, enter 'y' to continue :",
	                cdb_t_lines, 1)==FB_ERROR)
               fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    fb_move(4, 1), fb_clrtobot(), fb_infoline();
	    }
	 if (cdb_interrupt)
	    fb_allow_int();
         pp = keyprint;
	 if (pgen(fs, thead, bhead) == FB_ERROR)
            fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	 fb_closedb(cdb_db);
	 if (fs != stdout)
            fclose(fs);
	 if (!cdb_batchmode && cdb_screenprint)
	    fb_screenprint(filen);
	 fb_ender();
      }

/* 
 *  pgen - generate printout according to list in pp 
 */
 
   static pgen(fs, thead, bhead)
      struct totlist *thead;
      struct brklist *bhead;
      FILE *fs;
      
      {

         fld = cdb_afld; tmp = cdb_bfld; 	/* set locals to globals */
         n = 0L;
	 if (!cdb_batchmode){
	    fb_scrstat("Printing");
	    FB_XWAIT();
	    fb_gcounter(n);
	    }
	 if (cdb_yesmode)
	    sleep(1);				/* some are too fast */
         headers(pp, hdr1, hdr2);
	 for (maxbrk = 0, bp = bhead; bp != NULL; bp = bp->blink)
	    if (bp->blevel > maxbrk)
	       maxbrk = bp->blevel;
	 temphead = fb_copylist(thead);
	 pline1[0] = NULL;
         fb_r_init(hp->ifd);
         max = hp->irecsiz + 2;

         /* meat of printout production is done here */
         fb_forxeach(hp, one_pgen);

         breakline(thead, bhead, bline, pp, 1,
            fs, title, hdr1, hdr2, maxbrk, n); /* force break 1 */
         print(NIL, fs, title, hdr1, hdr2, NIL, NIL, cdb_coname, pwidth);
         if (testpage() < 2){
            print(NIL,fs,title,hdr1,hdr2,NIL, NIL, cdb_coname, pwidth);
            print(NIL,fs,title,hdr1,hdr2,NIL, NIL, cdb_coname, pwidth);
            }
         mktotal(pline, pline1, thead, thead, pp, 0);
         if (pline1[0] != NULL)
            p = pline1;
         else
            p = pline;
         mkdash(bline, p, '-');
         print(bline, fs, title, hdr1, hdr2,NIL,NIL,cdb_coname,pwidth);
         mkdash(bline, p, '=');
         if (pline1[0] != NULL)
            print(pline1,fs,title,hdr1, hdr2, NIL, NIL, cdb_coname, pwidth);
         print(pline, fs, title, hdr1, hdr2, NIL, NIL, cdb_coname, pwidth);
         print(bline, fs, title, hdr1, hdr2,NIL,NIL,cdb_coname,pwidth);
         sprintf(pline, " %ld records printed", n);
         print(NIL, fs, title, hdr1, hdr2, NIL, NIL, cdb_coname, pwidth);
         print(pline, fs, title, hdr1, hdr2, NIL, NIL, cdb_coname, pwidth);
         return(FB_AOK);
      }

   static one_pgen(hp)
      fb_database *hp;

      {
         if (firsttime){
            firsttime = 0;
            for (bp = bhead; bp != NULL; bp = bp->blink){
               fb_fetch(bp->bfld, fld, hp);
               fb_mkstr(&(bp->bval), fld);
               }
            }
         pline[0] = FB_BLANK; pline[1] = NULL; 
         len = 1;
         tp = thead; tempp = temphead;
         for (k = 0; ; k++){
            if ((pp[k]->size) == 0)
               break;
            if (pp[k]->type !=FB_DATE){
               if ((j = strlen(pp[k]->id)) <= (pp[k]->size))
                  hsize = (pp[k]->size);
               else
                  hsize = j;
               }
            else{
               if ((j = strlen(pp[k]->id)) <= (pp[k]->size + 2))
                  hsize = (pp[k]->size + 2);
               else
                  hsize = j;
               }
            len += hsize + 1;
            if (len > pwidth){
               strcpy(pline1, pline);
               pline[0] = '@'; pline[1] = NULL;
               len = hsize + 1;
               }
            fb_fetch(pp[k], fld, hp);
                                          /* check for break line */
            for (bp = bhead; bp != NULL; bp = bp->blink){
               if (bp->bfld == pp[k] && (bp->bval == NULL ||
                     strcmp(fld, bp->bval) != 0)){
                  if (bp->bval != NULL && 
                        (bflag == 0 || bp->blevel < bflag))
                     bflag = bp->blevel;
                  break;
                  }
               }
            if (tp != NULL && tp->tfld == pp[k]){	/* a totals line */
               tempp->tval = atof(fld);
               tp = tp->tlink;
               tempp = tempp->tlink;
               }
            if (FB_OFNUMERIC(pp[k]->type) || pp[k]->type == FB_FORMULA){
               if (pp[k]->type != FB_POS_NUM)
                  fformat(tmp, (double) atof(fld), pp[k]);
               else
                  strcpy(tmp, fld);
               fb_rjustify(fld, tmp, hsize, 0);
               }
            else if (pp[k]->type ==FB_DATE && fld[0] != NULL){
               sprintf(tmp, "%c%c/%c%c/%c%c", fld[0], fld[1], fld[2],
                  fld[3], fld[4], fld[5]);
               strcpy(fld, tmp);
               }
            else if (pp[k]->size > FB_LONGFIELD)
               for (p = fld; *p; p++)
                  if (*p == '\n')
                     *p = FB_BLANK;
            strcat(pline, fld);
            strcat(pline, " ");
            for(j = strlen(pline); j < len; j++)
               strcat(pline, " ");
            }
         if (bflag > 0)		/* then already been by once */
            breakline(thead, bhead, bline, pp, bflag,
                  fs, title, hdr1, hdr2, maxbrk, n);
         bflag = 0;
         if (pline1[0] != NULL){
            if (detail)
               print(pline1,fs,title,hdr1,hdr2,NIL,NIL,cdb_coname,pwidth);
            pline1[0] = NULL;
            }
         if (detail){
            print(pline,fs,title,hdr1,hdr2,NIL,NIL,cdb_coname,pwidth);
            for (i = fmt; i > 1; i--)
               print(NIL,fs,title,hdr1,hdr2,NIL,NIL,cdb_coname,pwidth);
            }
         n++;
         if (!cdb_batchmode){
            fb_gcounter(n);
            }
         for (tp=thead, tempp = temphead; tp != NULL && tempp != NULL;){
            tp->tval += tempp->tval;
            tp = tp->tlink;
            tempp = tempp->tlink;
            }
         return(FB_AOK);
      }

/* 
 *  breakline - format a breakline into bline -- adjust break values 
 */
 
   static breakline(thead, bhead, bline, pp, bflag,
                        fs, title, hdr1, hdr2, maxbrk, nc)
      struct totlist *thead;
      struct brklist *bhead;
      char *bline, *title, *hdr1, *hdr2;
      fb_field *pp[];
      int bflag, maxbrk;
      long nc;
      FILE *fs;
      
      {
	 struct brklist *bp;
	 char bline1[FB_PRINTLINE+1], dline[FB_PRINTLINE+1], *p;
	 short t;

	 for (bp = bhead; bp != NULL && bflag <= maxbrk; ){
	    if (bp->blevel == maxbrk){
	       /* save the current break value */
	       fb_formfield(cdb_bfld, bp->bval, bp->bfld->type,
                  strlen(bp->bval));
	       fb_rmlead(fb_trim(cdb_bfld));
	       strcpy(cdb_afld, "* ");
	       for (t = 1; t <= bp->blevel; t++)
	          strcat(cdb_afld, "*");
	       sprintf(cdb_afld, "%s (value:%s) ", cdb_afld, cdb_bfld);
	       
	       /* store away the next next break value */
	       fb_fetch(bp->bfld, cdb_bfld, hp);
	       fb_mkstr(&(bp->bval), cdb_bfld);
	       
	       /* note no error checking for min length < pwidth. */
	       sprintf(bline,"%s*%s* (level:%02d[%-10s]/count:%04ld) *",
	          cdb_afld, blanks(pwidth - 39 - strlen(cdb_afld)), 
		  maxbrk, bp->bfld->id, nc - (long) bp->bcount);
	       bp->bcount = nc;
	       if (testpage() < 2){
	          print(NIL,fs,title,hdr1,hdr2,NIL, NIL, cdb_coname, pwidth);
	          print(NIL,fs,title,hdr1,hdr2,NIL, NIL, cdb_coname, pwidth);
		  }
	       if (!bp->simple_flag){		/* allows simple break */
	          if (detail)
	             print(NIL,fs,title,hdr1,hdr2,NIL, NIL, cdb_coname, pwidth);
		  print(bline, fs, title, hdr1, hdr2,
			NIL, NIL, cdb_coname, pwidth);
		  }
	       mktotal(bline, bline1, bp->btlist, thead, pp, 1);
	       if (bp->simple_flag){
	          if (bline1[0] != NULL)
	             p = bline1;
		  else
		     p = bline;
		  mkdash(dline, p, '-');
		  print(dline, fs, title, hdr1, hdr2,NIL,NIL,cdb_coname,pwidth);
		  }
	       if (bline1[0] != NULL)
		  print(bline1,fs,title,hdr1,hdr2,NIL,NIL,cdb_coname,pwidth);
	       print(bline, fs, title, hdr1, hdr2, NIL, NIL, cdb_coname, pwidth);
	       maxbrk--;
	       bp = bhead;
	       }
	    else
	       bp = bp->blink;
	    }
	 if (detail)
	    print(NIL, fs,title,hdr1,hdr2, NIL, NIL, cdb_coname, pwidth);
      }

/* 
 *  mktotal - make total list --- adjust brklist if breaker == 1
 */
 
   static mktotal(bline, bline1, tp, thead, pp, breaker)
      char *bline, *bline1;
      struct totlist *tp, *thead;
      fb_field *pp[];
      int breaker;
      
      {
         int len, k, j, hsize;
	 char *fld;
	 double fc;
	 
	 fld = cdb_afld;		/* set local to global */
	 bline[0] = '*'; bline[1] = NULL;
	 bline1[0] = NULL;
	 len = 1;
	 for (k = 0 ; ; k++){
	    if ((pp[k]->size) == 0)
	       break;
	    if (pp[k]->type !=FB_DATE){
	       if ((j = strlen(pp[k]->id)) <= (pp[k]->size))
		  hsize = (pp[k]->size);
	       else
		  hsize = j;
	       }
	    else{
	       if ((j = strlen(pp[k]->id)) <= (pp[k]->size + 2))
		  hsize = (pp[k]->size + 2);
	       else
		  hsize = j;
	       }
	    len += hsize + 1;
	    if (len > pwidth){
	       strcpy(bline1, bline);
	       bline[0] = '@'; bline[1] = NULL;
	       len = hsize + 1;
	       }
	    if (tp != NULL && tp->tfld == pp[k]){
	       if (breaker)
	          fc = thead->tval - tp->tval;
	       else
	          fc = tp->tval;
	       fformat(fld, fc, tp->tfld);
	       strcat(bline, blanks(hsize - strlen(fld)));
	       strcat(bline, fld);
	       strcat(bline, " ");
	       if (breaker)
	          tp->tval = thead->tval;
	       thead = thead->tlink; 
	       tp = tp->tlink;;
	       }
	    else{
	       for(j = strlen(bline); j < len; j++)
	          strcat(bline, " ");
	       }
            }
      }

/* 
 *  fformat - format a double into either a float, dollar, or integer 
 */
 
   static fformat(p, f, pfld)
      char *p;
      double f;
      fb_field *pfld;
      
      {
         char *c, dollar[FB_MAXLINE], *fb_formdollar();
	 int prec;
	 
         *p = NULL;
         switch(pfld->type){
	    case 'N': 		/* FB_NUMERIC FB_POS_NUM: */
	    case 'n':
	       sprintf(p, "%.0f", f); break;
	    case '$':		/* FB_DOLLARS: */
	       sprintf(dollar, "%.0f", f);
	       fb_formdollar(p, dollar, pfld->size);
	       break;
	    case 'f':		/* FB_FLOAT  */
	    case 'F':		/* FB_FORMULA */
	       c = 0;
	       if (pfld->idefault != NULL)
	          c = strrchr(pfld->idefault, ':');
	       if (c == 0)
	          prec = 6;	/* default precision */
	       else
	          prec = atoi(c+1);
	       sprintf(p, "%.*f", prec, f); break;
	    }
      }
      
/* 
 *  headers - build all header information: open file for output 
 */
 
   static void headers(pp, hdr1, hdr2)
      fb_field *pp[];
      char *hdr1, *hdr2;
      
      {
         int i, j, k, len, hsize;
         char *p;
         
         hdr1[0] = FB_BLANK;
         hdr2[0] = FB_BLANK;
         hdr1[1] = NULL;
         hdr2[1] = NULL;
         p = hdr1;
         len = 1;
         j = 0;
         while ((pp[j]->size) != 0){
	    if (pp[j]->type !=FB_DATE){
	       if ((k = strlen(pp[j]->id)) <= (pp[j]->size))
		  hsize = (pp[j]->size);
	       else
		  hsize = k;
	       }
	    else{
	       if ((k = strlen(pp[j]->id)) <= (pp[j]->size + 2))
		  hsize = (pp[j]->size + 2);
	       else
		  hsize = k;
	       }
            if ((len += hsize + 1) > pwidth){
               if (p == hdr2){
	          /*
	           * fb_serror(FB_MESSAGE, "> two headers/pagewidth specified",NIL);
		   * fb_serror(FB_MESSAGE, "truncation occuring at:", pp[j]->id);
		   */
		  pp[j]->size = 0;
		  return;
		  }
               p = hdr2;
               len = 1 + hsize;
               }
	    if (FB_OFNUMERIC(pp[j]->type) || pp[j]->type == FB_FORMULA){
	       i = len - strlen(pp[j]->id) - 1;
	       for (k = strlen(p); k < i; k++)
		  strcat(p, " ");
	       strcat(p, pp[j++]->id);
	       strcat(p, " ");
	       }
	    else {
	       strcat(p, pp[j++]->id);
	       strcat(p, " ");   
	       for (k = strlen(p); k < len; k++)
		  strcat(p, " ");
	       }
            }
      }

/* 
 *  blanks - return ptr to string of n blanks -- stores in global blankline
 *     (needed since fb_printw fails beyond 128...)
 */
 
   static char *blanks(n)
      int n;
      
      { 
         int i;
	 
         for (i = 0; i < n && i < FB_MAXLINE; i++)
	    blankline[i] = FB_BLANK;
	 blankline[i] = NULL;
	 return(blankline);
      }

/*
 * create a dash line from a totals line
 */

   static mkdash(d, p, c)
      char d[], *p, c;
      
      {
         int k;
	 
	 for(k = 0; *p; k++, p++){
	    if (isdigit(*p) || *p == '.' || *p == ',' || 
	       *p == '(' || *p == ')' )
	       d[k] = c;
	    else
	       d[k] = ' ';
	    }
	 d[k] = NULL;
      }

/* 
 *  usage message 
 */
 
   usage()
      {
         fb_xerror(FB_MESSAGE,"usage: dbpgen [-d dbase] [-i index] [-b] [-y]",NIL);
      }
