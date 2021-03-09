/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: choice.c,v 9.2 2001/02/16 19:15:28 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Choiceinput_sid[] = "@(#) $Id: choice.c,v 9.2 2001/02/16 19:15:28 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <vdict.h>

#include <sys/types.h>
#include <sys/stat.h>

static FILE *cfs;		/* the choice file itself */
extern fb_cnode *chead, *cptr;
char *choice_attribute = "|";
static char *CHOICE_INPUT = "$INPUT";
static char *CHOICE_INPUT_STAR = "$INPUT*";
static char *STAR = " *";
static char *MSG_CHOICE_INPUT = "Enter Full Value: ";
static char *MSG_ADD_CHOICE = "'%s' Not Found. Enter Full Value: ";
static char *MSG_HELP = "<CTL>-X=Cancel";
extern char *cdb_prompt_choicemsg;	/* prompt for choice message */
extern char *cdb_dbvedit_cho_ploc;	/* prompt location for choice level */
extern short int cdb_dbvedit_cho_pilength; /* length, dbvedit, choice level */
extern short int cdb_record_level;	/* record level toggle switch */
extern short int scanner;

static long seektop = 0;
static long seekbot = 0;

extern long cdb_seekstack[];
extern short int cdb_topstack;
static int choicepages;

static int r_choiceinput(fb_field *k, char *line, int new, int row, int col,
   char *inp, char *label, int e_size, int okend, char *helpfile);
static int choiceselect(fb_field *f, char *inp, char *line, int e_size);
static int fchoice(fb_field *f, char *label, int lastrow);
static int movefs(FILE *fs, char *label);
static int addchoice(char *label, char *value, fb_field *f);
static int getinchoice(char *p, char *msg, char *def, int size);

extern long fb_pop(void);
extern char *fb_underscore(char *p, int rep_blank);

extern char *cdb_T_PREV;
extern char *cdb_T_NPREV;

/* 
 * choiceinput - routine called from edit_field in dbedit.c 
 *   idea here is to display a list of choices.
 *   user picks one, choiceinput returns associated string.
 *
 *   multi-value fields implemented to simulate mutli menu selections.
 *   e_size is used only to limit input entered.
 */
 
   fb_choiceinput(k, line, new, row, col, e_size, okend)
      fb_field *k;
      char *line;
      int new, row, col, e_size, okend;
      
      {
         char inp[FB_MAXLINE], label[FB_MAXLINE], helpfile[FB_MAXNAME], *p;
         int st;
	 
         if (k->help == NULL || *k->help == NULL ||
	       (cfs = fopen(k->help, FB_F_READ)) == NULL){
	    fb_serror(FB_CANT_OPEN, k->help, NIL);
	    return(FB_ERROR);
	    }
         fb_cx_set_seekfile(k->help);
         fb_cx_set_seekpoint(0);
         seektop = seekbot = 0;
         fb_initstack();
	 fb_initcomment(cfs, helpfile);
	 fb_initcnode();
	 label[0] = NULL;
	 st = r_choiceinput(k,line,new,row,col,inp,label,e_size,
	    okend,helpfile);
	 if (st == FB_AOK){
	    sprintf(label, "$%s", inp);
	    fb_addcnode(line);
	    while (movefs(cfs, label)){
	       st = r_choiceinput(k,line,new,row,col,inp,label,e_size,
	          okend,helpfile);
	       if (st != FB_AOK)
	          break;
	       sprintf(label, "$%s", inp);
	       fb_addcnode(line);
	       }
	    }
	 if (st == FB_AOK){
	    cptr = chead;
	    strcpy(line, cptr->c_meaning);
	    for (cptr = chead->c_next; cptr; cptr = cptr->c_next) {
	       if (strlen(line) + strlen(cptr->c_meaning) + 1 < k->size){
		  strcat(line, choice_attribute);
		  /* replace any attribute characters with a dash */
		  for (p = cptr->c_meaning; *p; p++)
		     if (*p == *choice_attribute)
		        *p = CHAR_MINUS;
		  strcat(line, cptr->c_meaning);
		  }
	       }
	    }
	 fb_freecnode();
	 fb_freeccom();
	 fclose(cfs);
	 return(st);
      }

   static r_choiceinput(k,line,new,row,col,inp,label,e_size, okend, helpfile)
      fb_field *k;
      char *line, *label, *inp, *helpfile;
      int new, row, col, e_size, okend;
      
      {
	 int minc = 0, maxc, icol, ploc_row, ploc_col, t_minc, silentchoice;
         int st, c_st = 0;
	 char *cp, buf[FB_MAXLINE], cx_status[FB_MAXLINE];

         choicepages = -1;
         if (k->type == FB_CHOICE){
            silentchoice = 0;
	    ploc_row = cdb_t_lines;
	    ploc_col = 2;
	    if (cdb_dbvedit_cho_ploc != NULL){
	       strcpy(buf, cdb_dbvedit_cho_ploc);
	       cp = strchr(buf, ',');
	       *cp = NULL;
	       ploc_row = atoi(buf);
	       ploc_col = atoi(cp + 1);
	       }

            row = ploc_row;	/* allow HELP signal */
	    icol = (strlen(cdb_prompt_choicemsg) + ploc_col);
	    maxc = -cdb_dbvedit_cho_pilength;
	    c_st = fchoice(k, label, row - 2);
            strcpy(cx_status, CX_CHOICE_SELECT);
	    }
	 else{			/* silent choice (C) OR window choice */
            fb_move(cdb_t_lines, 1); fb_clrtoeol();
	    ploc_row = cdb_t_lines;
	    ploc_col = 2;
            fb_move(ploc_row, ploc_col); fb_clrtoeol();
            silentchoice = 1;
	    maxc = -(k->size);
	    icol = col;
            strcpy(cx_status, CX_SILENT_CHOICE_SELECT);
	    }
	 if (scanner == 1){
	    FB_PAUSE();
	    return(FB_ERROR);
	    }
	 inp[0] = NULL;
         /* always allow FB_DSIGNAL now */
	 icol = -icol;
	 if (k->idefault == NULL || strlen(k->idefault) == 0)
	    minc = 1;
	 else if (equal(k->idefault, cdb_T_PREV) && k->prev == NULL)
	    minc = 1;
	 else if (equal(k->idefault, cdb_T_NPREV) && k->prev == NULL)
	    minc = 1;
	 if (!cdb_record_level)
	    t_minc = 0;
	 else
	    t_minc = minc;
         /* test here whether D and dU belong */
         fb_cx_push_env("EXNP", cx_status, NIL); 	/* fld level */
         if (choicepages > 1)
            fb_cx_add_buttons("dU");
         if (t_minc == 0)
            fb_cx_add_buttons("D");
         if (helpfile[0] != NULL)
            fb_cx_add_buttons("H");
         if (label[0] == NULL)
            fb_cx_add_buttons("4");
	 for (st = FB_ERROR; st == FB_ERROR ;){
	    for (;;){
	       fb_infoline();
               fb_move(ploc_row, ploc_col);
               fb_printw(cdb_prompt_choicemsg);
               if (choicepages <= 1)
                  buf[0] = NULL;
               else
                  strcpy(buf, "<CTL>-F=Next Page, <CTL>-B=Prev Page, ");
               strcat(buf, "<CTL>-X=Abort");
               fb_scrhlp(buf);
               st = fb_input(-row, icol, maxc, t_minc,
                  FB_ALPHA, inp, FB_ECHO, -FB_OKEND, FB_CONFIRM);

               /*
                * this code is intended to make more keystrokes
                * work in more places, more transparent
                * - make ^D work like FB_END for non new records
                * - make FB_END be some FB_QSIGNAL interpreted outside
                */
               if (st == FB_END && okend != FB_OKEND)
                  st = FB_QSIGNAL;
               if (st == FB_DSIGNAL && !new)
                  st = FB_END;

	       if (!cdb_record_level && st == FB_DEFAULT)
		  if ((k->fld != NULL && strlen(k->fld)>0) || minc)
		     st = FB_ESIGNAL;

/*
*              removing this enables the FB_DSIGNAL to filter back up
*
*	       if (st == FB_DSIGNAL){
*		  if (!minc)
*		     st = FB_DEFAULT;
*		  else
*		     st = FB_ESIGNAL;
*		  }
*/
               if (st == FB_QHELP){
                  if (helpfile[0] != NULL)
                     fb_fhelp(helpfile);
                  else
                     fb_fhelp(k->help);
                  }
               else if (st == FB_PAGEUP || st == FB_BSIGNAL){
                  if (cdb_topstack != 0)
                     seektop = fb_pop();
                  }
               else if (st == FB_PAGEDOWN || st == FB_FSIGNAL){
                  if (c_st == EOF){
                     cdb_topstack = 0;
                     seektop = cdb_seekstack[cdb_topstack];
                     }
                  else{
                     fb_push(seektop);
                     seektop = seekbot;
                     }
                  }
               else
                  break;
               if (!silentchoice)
	          c_st = fchoice(k, label, row - 2);
               else
                  fb_local_display(1);
	       }
	    fb_trim(inp);
	    if (st == FB_AOK){
               if ((st = choiceselect(k, inp, line, e_size)) == FB_ERROR)
                 if (k->choiceflag != CHAR_A)
                    fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
	       }
	    }
         fb_cx_pop_env();
	 return(st);
      }

/*
 *  choiceselect - selects a line from a choice file. 
 *     format of choice file is: choice-word meaning-word [comment-word ..]
 *     where words are runs of non white space.
 */

   static choiceselect(f, inp, line, e_size)
      fb_field *f;
      char *inp, *line;
      int e_size;
      
      {
         int j, withstar = 0, st;
	 char inbuf[FB_MAXLINE], word[FB_MAXLINE], buf[FB_MAXLINE];
	 
	 st = FB_ERROR;
         if (cdb_topstack != 0)
            fseek(cfs, cdb_seekstack[0], 0);
         else
            fseek(cfs, seektop, 0);
	 for (; fgets(inbuf, FB_MAXLINE, cfs) != NULL ;)
	    if (inbuf[0] != '#' && inbuf[0] != '"' && inbuf[0] != '$')
	       if ((j=fb_getword(inbuf, 1, word)) != 0 && equal(inp, word)){
		  fb_getword(inbuf, j, line);
		  if (equal(line, CHOICE_INPUT) || 
		      equal(line, CHOICE_INPUT_STAR)){
	             st = getinchoice(buf, MSG_CHOICE_INPUT, NIL, e_size);
		     if (st == FB_AOK){
			if (equal(line, CHOICE_INPUT_STAR))
			   withstar = 1;
		        strcpy(line, buf);
			if (withstar)
			   strcat(line, STAR);
			}
		     else
		        line[0] = NULL;
		     }
		  else
		     st = FB_AOK;
		  if (strlen(line) > f->size)
		     line[f->size] = NULL;
		  fb_underscore(line, 0);
		  break;
		  }
	 if (st == FB_ERROR && f->choiceflag == CHAR_A && chead == NULL){
	    sprintf(inbuf, MSG_ADD_CHOICE, inp);
	    st = getinchoice(buf, inbuf, " ", e_size);
	    if (st == FB_AOK){
	       fb_underscore(inp, 1);
	       fb_underscore(buf, 1);
	       st = addchoice(inp, buf, f);
	       if (st == FB_AOK){
	          fb_underscore(buf, 0);
	          strcpy(line, buf);
		  }
	       }
	    else
	       st = FB_ERROR;		/* in case it was FB_DEFAULT */
	    }
	 return(st);
      }

/*
 * fchoice - display a choice file
 */
 
   static fchoice(f, label, lastrow)
      fb_field *f;
      char *label;
      int lastrow;
      
      {
	 register int i, lc;
	 int eof, j, t, pt[5], k, width = 0, cwidth = 0;
	 char line[FB_MAXLINE], word[FB_MAXLINE];
	 
         fb_move(3, 1); fb_clrtobot();
         fseek(cfs, seektop, 0);
         fb_cx_set_seekpoint(seektop);
	 for (lc = 0; fgets(line, FB_MAXLINE, cfs) != NULL;){
	    if (line[0] == '$' || line[0] == FB_NEWLINE || line[0] == NULL)
	       break;
	    if (line[0] != '#' && line[0] != '"'){
	       j = fb_getword(line, 1, word);
	       k = strlen(word);
	       j = fb_getword(line, j, word);
	       k += strlen(word);
	       width = MAX(width, k);
	       j = fb_getword(line, j, word);
	       k = strlen(word);
	       cwidth = MAX(cwidth, k);
	       lc++;
	       }
	    }
	 pt[1] = 5; pt[2] = 15;
	 			
	 pt[3] = pt[2] + MAX(8, width) + 1; /* 8 = length of 'meaning ' */
	 pt[4] = pt[3] + MAX(8, cwidth) + 1; /* 8 = length of 'previous' */
	 sprintf(line, SYSMSG[S_S_BLANK], SYSMSG[S_CHOICE_TITLE]);
	 strcat(line, f->id);
	 if (strlen(label) > 0)
	    sprintf(line, "%s - %s", line, (label + 1));
	 fb_move(4,pt[1]);
	 fb_stand(line);
	 fb_move(6,pt[1]); fb_printw(SYSMSG[S_CHOICE]);
	 fb_move(6,pt[2]); fb_printw(SYSMSG[S_MEANING]);
	 fb_move(6,pt[3]); fb_printw(SYSMSG[S_COMMENT]);
	 /* movefs(cfs, label, seektop); */
         fseek(cfs, seektop, 0);
         j = 0;
         /* display a single page of choices */
         for (eof = 0, i = 8; i <= lastrow; ){
            if (fgets(line, FB_MAXLINE, cfs) == NULL){
               eof = 1;
               break;
               }
            if (line[0] == '$' || line[0] == FB_NEWLINE || line[0] == NULL){
               eof = 1;
               break;
               }
            if (line[0] == '"' || line[0] == '#' || line[0] == '$')
               continue;
            else{
               for (k=t=1;t <= 2 && (k=fb_getword(line, k, word)) != 0; ++t ){
                  fb_move(i, pt[t]);
                  if (t == 1)
                     fb_stand(word);
                  else
                     fb_printw(fb_underscore(word, 0));
                  }
               fb_move(i, pt[t]);
               line[k + cdb_t_cols - pt[3]] = NULL;
               fb_printw(FB_FSTRING, line+k);
               }
            i++;
            if (lc == ++j){
               eof = 1;
               break;
               }
            }
         fb_showcnode(pt[4]);		/* display cnodes -- suggest column */
         fb_showcomment();		/* display cnodes -- suggest column */
         seekbot = ftell(cfs);
         /* for return codes use EOF or FB_AOK */
         if (choicepages == -1){
            if (eof)
               choicepages = 1;
            else
               choicepages = 2;	/* rather, at least 2 */
            }
         if (eof)
            return(EOF);
         else
	    return(FB_AOK);
      }

   static movefs(fs, label)		/* maybe move file pointer to label */
      FILE *fs;
      char *label;
      
      {
	 char inbuf[FB_MAXLINE], word[FB_MAXLINE];

         fseek(fs, seektop, 0);
	 if (strlen(label) > 0)
	    for (; fgets(inbuf, FB_MAXLINE, fs) != NULL ;){
	       fb_getword(inbuf, 1, word);
	       if (equal(word, label)){
                  seektop = ftell(fs);
                  cdb_topstack = 0;
		  return(1);
                  }
	       }
	 return(0);
      }

   static addchoice(label, value, f)	/* physically add a choice */
      char *label, *value;
      fb_field *f;
      
      {
         int fd;
	 char buf[FB_MAXLINE];
	 struct stat sbuf;

         fb_fmessage("Adding Choice ...");
	 if ((fd = open(f->help, READWRITE)) < 0)
	    return(FB_ERROR);
	 fb_s_lock(fd, 1, f->help);
	 fb_sync_fd(fd);
	 fstat(fd, &sbuf);
	 lseek(fd, 0L, 2); 		/* seek to end of file */
	 sprintf(buf, "%s %s\n", label, value);
	 write(fd, buf, strlen(buf));
	 fb_s_unlock(fd, f->help);
	 close(fd);
	 fb_move(cdb_t_lines, 1); fb_clrtoeol();
	 return(FB_AOK);
      }

/*
 * getinchoice - duplicate the fb_getfilename() function with size option
 */

   static getinchoice(p, msg, def, size)
      char *p, *msg, *def;
      int size;
      
      {  
         register int i, st;
	 int msize;
      
	 fb_fmessage(msg);
	 if (strlen(def) == 0){
	    i = 1;
            fb_cx_push_env("X", CX_KEY_SELECT, NIL);
            }
	 else{
	    i = 0;
            fb_cx_push_env("DX", CX_KEY_SELECT, NIL);
            }
 	 msize = strlen(msg);
         fb_scrhlp(MSG_HELP);
	 st = fb_input(cdb_t_lines,msize+1, size, i, FB_ALPHA, p, FB_ECHO,
            FB_NOEND, FB_CONFIRM);
         fb_cx_pop_env();
	 fb_move(cdb_t_lines, 1); fb_clrtoeol(); fb_refresh();
	 if (st == FB_ABORT)
	    return(FB_ERROR);
	 else if (st == FB_DEFAULT)
	    strcpy(p,  def);
	 fb_trim(p);
	 return(st);
      }
