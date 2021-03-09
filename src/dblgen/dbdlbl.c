/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbdlbl.c,v 9.1 2001/02/16 19:43:22 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)dbdlbl.c	8.2 11/26/93 FB";
#endif

/* 
 *  dbdlbl.c - screen editor for label dict file : idictl 
 */

#include <fb.h>
#include <fb_vars.h>
#include <dbd.h>

#define HLP_DBDLBL 	"dbdlbl.hlp"

#define COLUMN 15			/* column position of fb_input */
#define FORMAT 4			/* default format "up" */
#define ASIZE 54			/* standard label size at 12cps */
#define SLPRT "cdblbl"			/* standard base output file name */

extern short int cdb_locklevel;
extern short int cdb_askgen;
extern char *cdb_LGEN;

static char filen[FB_MAXNAME] = {""};	/* name of label dictionary */
static char rcfile[FB_MAXNAME] = {""};	/* name of output label file */
static char justify = 'n';		/* left justify header? flag */

static int fmt = 0;			/* format 1-4 up */
static int asize = 0;			/* label size in characters (cpi) */

static int anychange();

struct lab {				/* for a each lab line */
   char aid[FB_SCREENFIELD+1];
   };

static fb_database *hp;

static ed_labrc();
static fput();
static void check();
static iskey();
static fget();
static output();
static headers();

/* 
 * dbdlbl - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
      
      {
	 cdb_locklevel = -1;
         fb_getargs(argc, argv, FB_ALLOCDB);
	 hp = cdb_db;
	 fb_initdbd(hp);
         if (fb_getd_dict(hp) == FB_ERROR)
            fb_xerror(FB_BAD_DICT, hp->ddict, NIL);
	 fb_scrhdr(hp, "Define Labels");
         if (ed_labrc(argv) != FB_END)
	    fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	 if (cdb_askgen)
	    fb_chainto("Generate Labels", cdb_LGEN, argv);
	 fb_ender();
      }
      
/* 
 *  ed_labrc - edit a label dictionary file 
 */
 
   static ed_labrc(argv)
      char *argv[];

      {
	 fb_field *keyprint[FB_MAXKEEP];
	 struct lab labels[5];
	 char abuf[FB_MAXLINE];
	 int i, p, st;

         initlabel(cdb_keymap, keyprint, hp,
                        filen, argv, hp->idict, &fmt, &asize);
	 headers();
	 sprintf(rcfile, "%sl", hp->idict);
	 if (fmt == 0){				/* no dictl file */
	    strcpy(filen, SLPRT);
	    fmt = FORMAT;
	    asize = ASIZE;
	    for (i = 0; i <= 4; i++)
	       labels[i].aid[0] = NULL;
	    for (i = 0; i <= 8; i++)
	       fput(labels, i);
	    for (i = 1; i <= 4; i++)
	       if (fget(labels, i) == FB_END)
		  break;
	    }
	 else{
	    if (fmt < 0){
	       fmt = -fmt;
	       justify = 'n';
	       }
	    else
	       justify = 'y';
	    for (p = 0, i = 0; ; p++){
	       abuf[0] = NULL;
	       while (keyprint[i] != NULL)
		  strcat(abuf, keyprint[i++]->id);
	       if (p >= 5)
		  fb_xerror(FB_MESSAGE, "too many label lines", NIL);
	       strcpy(labels[p].aid, abuf);
	       ++i;
	       if (keyprint[i] != NULL && keyprint[i]->size == 0)
		  break;
	       }
	    for (i = 0; i <= 8; fput(labels, i++))  /* 1-4=lines;5+=other */
	       ;
	    }
	 st = anychange(labels);
	 if (st == FB_END)
	    output(labels);
	 return(st);
      }

/* 
 *  fput - fb_put an entire label line to a constant screen location 
 */
 
   static fput(k, i)
      struct lab k[];
      int i;
      
      {
         int row;
         
	 check(k, i);
         row = (i + 1) * 2 + 3;
	 if (i >= 5)
	    row++;
         fb_move(row, COLUMN);
	 if (i >= 0 && i <= 5)
	    fb_printw("%2d> ", i);
	 else
	    fb_printw("    ");
         switch (i){
            case 0:
	       if (justify == 'n')
	          fb_printw("%50s", k[i].aid);
	       else 
	          fb_printw("%-50s", k[i].aid);
	       break;
            case 1: case 2: case 3: case 4: 
	       fb_printw("%-50s", k[i].aid); break;
            case 5: 
	       fb_printw("Output to: ---> %s", filen); break;
            case 6: 
	       fb_printw("Label Format:   %d up", fmt); break;
            case 7: 
	       fb_printw("Label Size:     %d char/label", asize); break;
            case 8: 
	       fb_printw("Left Justify 0? %c ", justify); break;
            }
      }

/* 
 *  check - check lines of label for constants -- mark them with '^'
 */
 
   static void check(k, i)
      struct lab k[];
      int i;

      {
	 char line[FB_MAXLINE], word[FB_MAXLINE], out[FB_MAXLINE], *fb_trim();
	 int j, p, undef, row;

	 if (i < 0 || i > 4)
	    return;
         row = (i + 1) * 2 + 4;
	 undef = 0;
	 out[0] = NULL;
	 line[0] = NULL;
	 sprintf(line, "%50s", "");
	 for (p = 1, j = 1; (j = fb_gettoken(k[i].aid, j, word, '_')) != 0; ){
	    if (iskey(word) != 0){
	       line[p-1] = '^';
	       undef = 1;
	       }
	    if (strlen(word) > FB_TITLESIZE)	/* in lgen, words are */
	       word[FB_TITLESIZE] = NULL;		/* stored in fields */
	    strcat(out, word);
	    p = strlen(out) + 1;
	    }
	 strcpy(k[i].aid, out);
	 fb_move(row, 1);
	 if (undef == 0)
	    fb_printw("%13s", " ");
	 else
	    fb_printw("undefined -->");	/* len = 13 */
	 if (i == 0 && (justify == 'n' || justify == 'N')){
	    p = strlen(out);
	    line[p] = NULL;
	    sprintf(out, "%50s", line);
	    }
	 else
	    strcpy(out, line);
	 fb_move(row, COLUMN + 4);
	 fb_printw(FB_FSTRING, out);
      }

/* 
 *  iskey - determine if word is a fb_field key name or ignorrable (ie white) 
 */
 
   static iskey(word)
      char *word;

      {
	 int i;

         if ((word[0] >= 'A' && word[0] <= 'Z') ||
	     (word[0] >= 'a' && word[0] <= 'z') ||
	     (word[0] >= '0' && word[0] <= '9')){
	    for (i = 0; i < hp->nfields; i++)
	       if (strcmp(word, cdb_kp[i]->id) == 0)
		  return(0);
	    return(1);
	    }
	 else
	    return(0);		/* not worth noting: ignore NON-FB_ALPHA */
      }

/* 
 * fget - get an entire label line from a constant screen location 
 */
 
   static fget(k, i)
      struct lab k[];
      int i;
      
      {
         int row, st, num, xend;
	 char abuf[FB_MAXLINE], *fb_trim();
         
         row = (i + 1) * 2 + 3;
	 if (i >= 5)
	    row++;
	 fb_fmessage("Enter Label Line, <CTL>-X = abort");
         switch (i){
            case 0: case 1: case 2: case 3: case 4: 
	       if (i != 1)
		  xend = FB_NOEND;
	       else
		  xend = FB_OKEND;
	       st = fb_input(row, COLUMN + 4, FB_SCREENFIELD, 0,
	          FB_ALPHA, abuf, FB_ECHO, xend, FB_CONFIRM);
	       if (st == FB_END){
		  fput(k, 1);
		  return(FB_END);
		  }
	       if (st == FB_DEFAULT)
		  k[i].aid[0] = NULL;
	       else if (st == FB_AOK)
		  strcpy(k[i].aid, fb_trim(abuf));
	       break;
            case 5:
	       /* 
	        *  get file name 
		*/
	       st = fb_input(row, COLUMN+20 , FB_TITLESIZE, 0, FB_ALPHA,
	          abuf, FB_ECHO, FB_NOEND, FB_CONFIRM);
	       if (st == FB_DEFAULT)
		  strcpy(filen, SLPRT);
	       else if (st == FB_AOK){
		  strcpy(filen, fb_trim(abuf));
		  }
	       fput(k, i++);
	       
	       /* 
	        *  get label format 
		*/
		
	       row += 2;
	       st = fb_input(row, COLUMN+20 , 1, 0, FB_INTEGER,
	          (char *) &num, FB_ECHO, FB_NOEND, FB_CONFIRM);
	       if (st == FB_DEFAULT || (st == FB_AOK && (num < 1 || num > 4)))
		  fmt = 4;
	       else if (st == FB_AOK)
		  fmt = num;
	       fput(k, i++);
	       
	       /* 
	        *  get label size  -- char/label 
		*/
		
	       row += 2;
	       st = fb_input(row, COLUMN+20 , 2, 0, FB_INTEGER,
	          (char *) &num, FB_ECHO, FB_NOEND, FB_CONFIRM);
	       if (st == FB_DEFAULT || (st == FB_AOK && (num < 1 || num > 220)))
		  asize = ASIZE;
	       else if (st == FB_AOK)
		  asize = num;
	       fput(k, i++);

	       /* 
	        *  justify header flag 
		*/
		
	       row += 2;
	       st = fb_input(row, COLUMN+20 , 1, 0, FB_ALPHA,
	          abuf, FB_ECHO, FB_NOEND, FB_CONFIRM);
	       if (st == FB_DEFAULT)
		  justify = 'n';
	       else if (st == FB_AOK){
	          if (justify != 'y' && justify != 'n' &&
	              justify != 'Y' && justify != 'N')
		     abuf[0] = 'n';
		  justify = abuf[0];
		  }
	       if (st != FB_ABORT)
	          fput(k, 0);
	       break;
            }
	 fput(k, i);
	 
	 /* 
	  *  finally return
	  */
	  
	 return(FB_AOK);
      }
            
/* 
 *  anychange - set up anychange screen 
 */
 
   static anychange(labels)
      struct lab labels[];
      
      {
         char inp[6];
         int st, num, i;
         
	 for(;;){
	    fb_fmessage("Any Change (#, <CTL>-H=Help, -=End) ? ");
	    st = fb_input(-cdb_t_lines, 40, 4, 0, FB_ALPHA, inp, FB_ECHO,
               FB_OKEND, FB_CONFIRM);
	    				/* allow FB_QHELP signal */
            if (st == FB_END)
	       break;
	    else if (st == FB_DEFAULT)
	       continue;
            else if (st == FB_ABORT){
	       if (fb_mustbe('y', "Really Quit? (y/n) ",
                  cdb_t_lines, 1) == FB_AOK)
		     break;
		  }
	    else if (st == FB_AOK){
	       if (inp[0] == FB_HELP)
		  fb_help(inp, hp);
	       else {
	          if (isdigit(inp[0])){
		     num = atoi(inp);
		     if (num < 0 || num > 5)
			fb_screrr("Invalid Selection");
		     else
			fget(labels, num);
		     }
		  else 
		     fb_screrr("Invalid Selection");
		  continue;	/* break out so as not to redraw screen */
		  }
	       }
	    else if (st == FB_QHELP)
	       fb_fhelp(HLP_DBDLBL);
	    fb_move(2, 1);
	    fb_clrtobot();
	    headers();
	    for(i = 0; i <= 8; i++)
	       fput(labels, i);
	    }
	 return(st);
      }
      
/* 
 * output - output the label dictionary 
 */
 
   static output(k)
      struct lab k[];

      {
	 int i;
	 FILE *fs, *fb_mustfopen();

	 fs = fb_mustfopen(rcfile, "w");
	 for (i = 0; i < 5; i++)
	    fprintf(fs, "%s\n", k[i].aid);
	 if (justify == 'n' || justify == 'N')
	    fmt = -fmt;
	 fprintf(fs, "%% %d %d\n", fmt, asize);
	 fprintf(fs, "%s\n", filen);
	 fclose(fs);
      }

/* 
 * headers - draw headliner box 
 */
 
   static headers()
      {
	 int i;

	 fb_move(4, COLUMN); 
	 fb_printw("-------------------------------------------------------");
	 fb_move(15, COLUMN); 
	 fb_printw("-------------------------------------------------------");
	 for (i = 4; i <= 15; i++){
	    fb_move(i, COLUMN-1);
	    if (i == 4)
	       fb_printw("/");
	    else if (i == 15)
	       fb_printw("\\");
	    else
	       fb_printw("|");
	    }
	 for (i = 4; i <= 15; i++){
	    fb_move(i, COLUMN+55);
	    if (i == 4)
	       fb_printw("\\");
	    else if (i == 15)
	       fb_printw("/");
	    else
	       fb_printw("|");
	    }
	 fb_infoline();
      }

/* 
 *  usage fb_fmessage 
 */
 
   usage()
      
      {
         fb_xerror(FB_MESSAGE, "usage: dbdlbl [-d dbase] [-i index]", NIL);
      }
