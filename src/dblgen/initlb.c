/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: initlb.c,v 9.0 2001/01/09 02:55:39 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Initlb_sid[] = "@(#) $Id: initlb.c,v 9.0 2001/01/09 02:55:39 john Exp $";
#endif

/*
 *  initlb.c - library used by dbdlbl and dblgen
 */

#include <fb.h>
#include <fb_ext.h>

static char outname[FB_MAXNAME] = {""};		/* program name */

#define LABEL_UP 4
#define GENERATOR "dblgen"
#define LPRT 	"cdblbl.lbl"			/* default labels output */

static getnums();

/* 
 *  inititialize Cdb label template using cdb_keymap built in initialize 
 */
 
   FILE *initlabel(cdb_kp, pp, hp, filen, argv, iname, sflag, asize)
      fb_field *cdb_kp[], *pp[];
      fb_database *hp;
      char *argv[], *filen, iname[];
      int *sflag, *asize;
   
      {
	 int p, i, j, gen;
	 FILE *fs, *fb_mustfopen(), *prtfs;
	 char id[FB_MAXLINE], ifile[FB_MAXNAME], word[FB_TITLESIZE+1], err[FB_MAXLINE];
   
	 fb_basename(ifile, argv[0]);
	 if (strcmp(ifile, GENERATOR) == 0)
	    gen = 1;
	 else
	    gen = 0;
	 strcpy(outname, ifile);
	 outname[0] = 'D';
	 prtfs = NULL;
	 sprintf(ifile, "%sl", iname);
	 if ((fs = fopen(ifile, "r")) == NULL)
	    return(NULL);
	 for (p = 0; p < FB_MAXKEEP; ){
	    if (fgets(id, FB_MAXLINE, fs) == NULL)
	       break;
	    id[strlen(id)-1] = NULL;		/* cast out newline */
	    if (id[0] == '%'){
	       getnums(id+1, sflag, asize); 	/* get number up/lab size */
	       if (fgets(ifile, FB_MAXNAME, fs) == NULL)
		  sprintf(filen, FB_FSTRING, LPRT);
	       else{
		  ifile[strlen(ifile)-1] = NULL; /* ditto */
		  sprintf(filen, FB_FSTRING, ifile);
		  if (gen == 1)
		     strcat(filen, ".lbl");
		  }
	       if (!cdb_batchmode && !cdb_yesmode && 
	             gen == 1 && access(filen, 0) != -1){
		  sprintf(err, 
		     "Permission to OVERWRITE %s (y = yes, <cr> = no)? ",
		        filen);
		  fb_bell();
		  if (fb_mustbe('y', err, 15, 10) == FB_ERROR)
		     fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
		  }
	       if (gen == 1)
		  prtfs = fb_mustfopen(filen, "w");
	       break;
	       }
	    for (j = 1; (j = fb_gettoken(id, j, word, '_')) != 0; ){
	       for (i = 0; i < hp->nfields; i++)
		  if (strcmp(word, cdb_kp[i]->id) == 0){
		     pp[p++] = cdb_kp[i];
		     break;
		     }
	       if (i >= hp->nfields){
		  pp[p] = (fb_field *) fb_malloc(sizeof(fb_field));
		  pp[p]->size = -1;
		  pp[p]->id = NULL;
		  fb_mkstr(&(pp[p]->id), word);
		  p++;
		  }
	       }
	    pp[p++] = NULL;
	    }
	 if (p >= FB_MAXKEEP) /* Only1000 fields Max per label at one time */
	    fb_xerror(FB_ABORT_ERROR, SYSMSG[S_ILLEGAL], SYSMSG[S_TOO_LONG]);
	 fclose(fs);
	 pp[p] = (fb_field *) fb_malloc(sizeof(fb_field));
	 pp[p]->size = 0;
	 pp[p]->id = NULL;
	 return(prtfs);
      }

/* 
 *  getnums - get the number up and the label size 
 */
 
   static getnums(id, sflag, asize)
      int *sflag, *asize;
      char *id;

      {
	 int i;
	 char word[FB_MAXLINE];

	 i = fb_getword(id, 1, word);
	 if (i == 0)
	    *sflag = LABEL_UP;		/* default is */
	 else{
	    *sflag = atoi(word);  	/* for number 'up' */
	    i = fb_getword(id, i, word);
	    if (i != 0)
	       *asize = atoi(word);	/* for label size */
	    }
      }
   
/* 
 *  ltrace - trace out all the elements asked for on the printout
 */
 
   ltrace(pp, outf, fmt, sz)
      fb_field *pp[];
      char *outf;
      int fmt, sz;
      
      {
	 int p, rj;

         fb_move(4, 1);
	 if (fmt < 0){
	    fmt = -fmt;
	    rj = 1;
	    }
	 else
	    rj = 0;
         fb_printw("%s: output to --> %s: %d up : label size = %d\n\n", 
	       outname, outf, fmt, sz);
	 if (rj == 1 && pp[0] != NULL)
	    fb_printw("<right justify...> ");
	 for (p = 0;;){
	    while (pp[p] != NULL)
	       fb_printw(FB_FSTRING, pp[p++]->id);
	    fb_printw("\n");
            p++;
	    if (pp[p] != NULL && pp[p]->size == 0)
	       break;
            }
         
      }
