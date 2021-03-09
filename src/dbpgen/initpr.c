/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: initpr.c,v 9.0 2001/01/09 02:55:43 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Initpr_sid[] = "@(#) $Id: initpr.c,v 9.0 2001/01/09 02:55:43 john Exp $";
#endif

/*
 *  initpr.c - initialization library for dbcgen and dbpgen
 */

#include <fb.h>
#include <fb_ext.h>
#include <pgen.h>

extern char *cdb_pgm;

/* 
 *  initprint - initialize Cdb pgen/cgen  keys using cdb_keymap 
 *     return file stream of output file.
 *     if printer == 1, then its pgen else cgen calling.
 */
 
   FILE *initprint(cdb_kp, pp, hp, title, filen, iname, sflag, csize)
      fb_field *cdb_kp[], *pp[];
      fb_database *hp;
      char *title, *filen, iname[];
      int *sflag, csize[];
   
      {
	 int p, i, lp, printer;
	 FILE *fs, *fb_mustfopen();
	 char id[FB_TITLESIZE+1], ifile[FB_MAXNAME], line[FB_MAXLINE];
         char err[FB_MAXLINE], *fb_trim();
         fb_field *fb_makefield();
   
	 if ((equal(cdb_pgm, "dbpgen")) || (equal(cdb_pgm, "dbdprt"))){
	    printer = 1;
	    sprintf(ifile, "%sp", iname);		/* idictp file */
	    }
	 else{
	    printer = 0;
	    sprintf(ifile, "%sc", iname);		/* idictc file */
	    }
	 if ((fs = fopen(ifile, "r")) == NULL)
	    fb_xerror(FB_BAD_DICT, ifile, NIL);
	 for (p = 0; p < FB_MAXKEEP;){
	    if (fgets(line, FB_MAXLINE, fs) == NULL)
	       break;
	    lp = fb_getword(line, 1, id);
	    if (id[0] == '%'){
	       if (printer == 1){
		  *sflag = 2;		/* default spacing = 2 */
		  csize[0] = 132;	/* default fb_page width */
		  if (isdigit(id[1]))
		     if ((*sflag = atoi(id+1)) < 0)
			*sflag = 2;
		  lp = fb_getword(line, lp, id);
		  if ((csize[0] = atoi(id)) == 0)  /* < 0 == detail flag */
		     csize[0] = 132;
		  if (fgets(title, FB_MAXLINE, fs) == NULL)
		     title[0] = NULL;
		  else
		     title[strlen(title)-1] = NULL; /* ditto */
		  }
	       if (fgets(ifile, FB_MAXNAME, fs) == NULL){
		  sprintf(filen, FB_FSTRING, DPRT);
		  if (printer == 0)
		     sprintf(filen, FB_FSTRING, DCNV);
		  }
	       else{
		  ifile[strlen(ifile)-1] = NULL; /* remove newline */
		  sprintf(filen, FB_FSTRING, fb_trim(ifile));
		  if (printer == 1)
		     strcat(filen, ".prt");
		  else
		     strcat(filen, ".cdb");
		  }
	       if (!cdb_batchmode && !cdb_yesmode && access(filen, 0) != FB_ERROR){
		  sprintf(err, 
		     "Permission to OVERWRITE %s (y = yes, <cr> = no)? ", 
		        filen);
		  if (fb_mustbe('y', err, 15, 10) == FB_ERROR)
		     fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
		  }
	       break;
	       }
	    for (i = 0; i < hp->nfields; i++)
	       if (strcmp(id, cdb_kp[i]->id) == 0){
		  pp[p] = cdb_kp[i];
		  if (printer == 0){			/* cgen stuff */
		     lp = fb_getword(line, lp, id);
		     lp = fb_getword(line, lp, id);
		     if (id[0] == NULL)
			csize[p] = cdb_kp[i]->size;
		     else if ((csize[p] = atoi(id)) <= 0)
			csize[p] = 10;
		     }
		  p++;
		  break;
		  }
	    if (printer == 1 && i >= hp->nfields)
	       fb_xerror(FB_BAD_DICT, hp->idict, id);
	    else if (i >= hp->nfields){
	       pp[p] = fb_makefield();
	       pp[p]->id = (char *) fb_malloc(strlen(id) + 1);
	       strcpy(pp[p]->id, id);
	       lp = fb_getword(line, lp, id);
	       if ((pp[p]->type = id[0]) == NULL)
		  pp[p]->type = 'a';		/* default if no type */
	       lp = fb_getword(line, lp, id);
	       if ((pp[p]->size = atoi(id)) == 0)
		  pp[p]->size = 10;		/* default if no size */
	       csize[p] = -(pp[p]->size);
	       p++;
	       }
	    }
	 if (p >= FB_MAXKEEP)
	    fb_xerror(FB_ABORT_ERROR, SYSMSG[S_ILLEGAL], NIL);
	 fclose(fs);
	 pp[p] = (fb_field *) fb_malloc(sizeof(fb_field));
	 pp[p]->size = 0;
	 pp[p]->id = NULL; pp[p]->aid = NULL;	/* paranoia */
      }
   
/* 
 *  pr_trace - trace out all the elements asked for on the printout 
 */
 
   pr_trace(pp, title, outf, fmt, pwidth, detail)
      fb_field **pp;
      char *title, *outf;
      int fmt, pwidth, detail;
      
      {
         int lc;
	 
         fb_move(4, 1);
         fb_printw("%s: %s --> %s\n        (spacing: %d; page width: %d; ", 
	    cdb_pgm, title, outf, fmt, pwidth);
	 if (detail)
	    fb_printw("all detail)\n\n");
	 else
	    fb_printw("no detail)\n\n");
	 lc = 2;
         while (((*pp)->size) != 0){
            fb_printw("%-15s %c %2d\n",(*pp)->id,(*pp)->type,(*pp)->size);
            pp++;
	    bump(&lc);
            }
         fb_printw("\n");
	 bump(&lc);
	 return(lc);
      }
      
/* 
 *  ctrace - trace out all the elements asked for on the conversion 
 */
 
   ctrace(pp, outf, csize)
      fb_field *pp[];
      int csize[];
      char *outf;
      
      {
         int j, k, i;
	 
         fb_move(4, 1);
         fb_printw("%s: --> %s", cdb_pgm, outf);
	 for (i = 0; pp[i]->size != 0; ){
	    if (i != 0)
	       FB_PAUSE();
	    fb_move(6, 1); fb_clrtobot();
	    for (k = 6; k <= 20 && pp[i]->size != 0; k++, i++){
	       fb_printw("%3d) %-15s %c", i+1, pp[i]->id,pp[i]->type);
               if (pp[i]->dflink != NULL)
                  fb_prints("L");
               else
                  fb_prints(" ");
	       if (csize[i] >= 0)
		  j = csize[i];
	       else
		  j = -csize[i];
	       fb_printw("%5d", j);
	       if (csize[i] < 0)
		  fb_printw("...new");
	       else if (csize[i] > pp[i]->size)
		  fb_printw("...expanded");
	       else if (csize[i] < pp[i]->size)
		  fb_printw("...truncated");
	       fb_printw("\n");
	       }
	    fb_infoline();
            }
      }
