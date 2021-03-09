/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: choosval.c,v 9.1 2001/02/16 18:57:06 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)choosval.c	8.5 04 Aug 1997 FB";
#endif

#include <fb.h>
#include <fb_ext.h>

#define C_STRING	"$STRING"

extern char *cdb_help;

#if FB_PROTOTYPES
static fchoice(char *f);
static choiceselect(char *f, char *inp, char *line);
#else /* FB_PROTOTYPES */
static fchoice();
static choiceselect();
#endif /* FB_PROTOTYPES */

/* 
 * chooseval - routine called for choosing a string value.
 *   idea here is to display a list of choices.
 *   user picks one, choiceval returns associated string. and
 *   the choice file allows the choice of an optional override.
 *
 *   type is a flag for cx_ use -- 1=IGEN_VAL1, 2=IGEN_VAL2, 3=IGEN_AND
 */
 
   fb_chooseval(f, buf, mlen, type)
      char *f, *buf;
      int mlen, type;
      
      {
         char inp[FB_MAXLINE], fname[FB_MAXNAME], status[FB_MAXNAME];
	 int st, tcol;

         fname[0] = NULL;
         if (access(f, 0) != 0)
            strcpy(fname, cdb_help);
         strcat(fname, f);
	 fb_scrstat("Choose Value");
	 if (fchoice(fname) == FB_ERROR)
	    return(FB_ERROR);
	 inp[0] = NULL;
         fb_cx_set_seekfile(fname);
         fb_cx_set_seekpoint(0);
         status[0] = NULL;
         switch(type){
            case 1: strcpy(status, CX_IGEN_VAL1_SELECT); break;
            case 2: strcpy(status, CX_IGEN_VAL2_SELECT); break;
            case 3: strcpy(status, CX_IGEN_AND_SELECT); break;
            }
         fb_cx_push_env("6", status, NIL);
         fb_cx_add_buttons("c");			/* ol_cfield stuff */	
	 for (st = FB_ERROR; st == FB_ERROR ;){
	    fb_infoline();
	    fb_fmessage(SYSMSG[S_CHOICE_MSG1]);
            tcol = strlen(SYSMSG[S_CHOICE_MSG1]) + 1;
            fb_scrhlp(SYSMSG[S_SELECT_CHOICES]);
	    st = fb_input(cdb_t_lines, -tcol, -10, 0, FB_ALPHA, inp, FB_ECHO,
               FB_OKEND, FB_CONFIRM);
            if (st == FB_END)
               st = FB_ABORT;
	    fb_trim(inp);
	    if (st == FB_AOK){
	       if ((st = choiceselect(fname, inp, buf)) == FB_ERROR)
	          fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
	       else if (st == FB_AOK && equal(buf, C_STRING)){
	          st = fb_getfilename(buf, "Enter String: ", NIL);
		  }
	       }
	    }
	 if ((int) strlen(buf) > mlen)
	    buf[mlen] = NULL;
         fb_cx_pop_env();
	 return(st);
      }

/*
 *  choiceselect - selects a line from a choice file. 
 *     format of choice file is: choice-word meaning-word [comment-word ..]
 *     where words are runs of non white space.
 */

   static choiceselect(f, inp, line)
      char *f;
      char *inp, *line;
      
      {
         int j, st;
	 char inbuf[FB_MAXLINE], word[FB_MAXLINE];
	 FILE *fs;
	 
	 st = FB_ERROR;
         if ((fs = fopen(f, FB_F_READ)) == NULL){
	    fb_serror(FB_CANT_OPEN, f, NIL);
	    return(FB_ERROR);
	    }
	 for (; fgets(inbuf, FB_MAXLINE, fs) != NULL ;)
	    if (inbuf[0] != '#')
	       if ((j=fb_getword(inbuf, 1, word)) != 0 && equal(inp, word)){
		  fb_getword(inbuf, j, line);
		  fb_underscore(line, 0);
		  st = FB_AOK;
		  break;
		  }
	 fclose(fs);
	 return(st);
      }

/*
 * fchoice - display a choice file
 */
 
   static fchoice(f)
      char *f;
      
      {
         FILE *fs;
	 register int i, lc;
	 int eof, j, t, pt[4], k, length = 0;
	 char line[FB_MAXLINE], c, ms[30], word[FB_MAXLINE];
	 char title[FB_MAXLINE];
	 
         if ((fs = fopen(f, FB_F_READ)) == NULL){
	    fb_serror(FB_CANT_OPEN, f, NIL);
	    return(FB_ERROR);
	    }
         fb_move(2, 1); fb_clrtobot();
	 title[0] = NULL;
	 for (lc = 0; fgets(line, FB_MAXLINE, fs) != NULL; lc++){
	    if (line[0] == '#'){
	       if (lc == 0)
		  strcpy(title, line + 1);
	       continue;
	       }
	    for (t = k = 1; (k = fb_getword(line, k, word)) != 0; ++t){
	       if (t == 2)
	          length = MAX(length, (int) strlen(word));
	       }
	    }
	 rewind(fs);
	 pt[1] = 10; pt[2] = 20; 
	 			/* 7 is the strlen of 'meaning' */
	 pt[3] = pt[2] + MAX(7, length) + 1;
	 sprintf(line, "Available choices for %s", title);
	 fb_move(4, 40 - (int) (strlen(line) / 2));
	 fb_stand(line);
	 fb_move(6,pt[1]); fb_printw(SYSMSG[S_CHOICE]);
	 fb_move(6,pt[2]); fb_printw(SYSMSG[S_MEANING]);
	 fb_move(6,pt[3]); fb_printw(SYSMSG[S_COMMENT]);
	 for (j = 0;;){
	    for (eof = 0, i = 8; i <= 22; i++){
	       if (fgets(line, FB_MAXLINE, fs) == NULL){
	          eof = 1;
		  break;
		  }
	       if (line[0] == '#'){
	          if (j != 0){
		     line[0] = CHAR_BLANK;
		     fb_move(i, 1);
		     fb_printw(FB_FSTRING, line);
		     }
		  else
		     i--;
		  }
	       else{
		  for (k=t=1;t <= 2 && (k=fb_getword(line, k, word)) != 0;++t){
		     fb_move(i, pt[t]);
		     if (t == 1)
			fb_stand(word);
		     else
			fb_force(fb_underscore(word, 0));
		     }
		  fb_move(i, pt[t]);
		  line[k + cdb_t_cols - pt[3]] = NULL;
		  fb_printw(FB_FSTRING, line+k);
		  }
	       if (lc == ++j){
	          eof = 1;
		  break;
		  }
	       }
	    if (eof == 1){
	       break;
	       }
	    else{
	       sprintf(ms, SYSMSG[S_DISPLAY], j, lc);
	       fb_scrhlp(ms);
	       fb_fmessage(SYSMSG[S_SPACE_BAR]);
               read(0, &c, 1);
	       if (c != FB_BLANK)
	          break;
	       fb_move(8, 1), fb_clrtobot();
	       }
	    }
	 fclose(fs);
	 return(FB_AOK);
      }
