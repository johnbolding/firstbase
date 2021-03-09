/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getby.c,v 9.2 2001/02/16 19:43:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getby_sid[] = "@(#) $Id: getby.c,v 9.2 2001/02/16 19:43:51 john Exp $";
#endif

/* 
 *  getby - 'get by' library for dbdind
 */

#include <fb.h>
#include <fb_ext.h>
#define GETBY_HELP "getby.hlp"
extern  long cdb_seekstack[];
extern short int cdb_topstack;
static long seektop = 0;
static long seekbot = 0;
static int choicepages;
static int cur_p;				/* current array p for by[] */
static char *FMT3 = "%3d> %-10s %c %6d";
static char *FMT4 = " [%s] ";
static char cur_line[FB_MAXLINE];

static enterby();
static showby();
static fchoice();

/* 
 *  getby - do sortby fields for dbdind 
 */
 
   getby(by)
      fb_field *by[];
      
      {
	 if (by[0] == NULL)
	    by[0] = cdb_kp[0];
	 for (;;){
	    fb_move(2, 1); fb_clrtobot(); fb_infoline();
	    fb_move(2, 5); fb_printw("Selected 'Sort By' Fields");
	    fb_move(3, 5); fb_printw("=========================");
	    showby(by);
            if (fb_mustbe('y', "Any Change (y=Yes, <RETURN>=No):",
                  cdb_t_lines, 1) != FB_AOK)
               break;
	    fb_move(2, 5); fb_clrtoeol();
	    fb_move(3, 5); fb_clrtoeol();
	    enterby(by);
	    }
	 fb_move(2, 1); fb_clrtobot();
      }

/* 
 *  enterby - gather all elements to print in the index. mere integers. 
 */
 
   static enterby(by)
      fb_field *by[];
      
      {
         int st, c_st, p;
         char line[FB_MAXLINE], buf[FB_MAXLINE];
         extern long fb_pop();
	 
         fb_cx_set_seekpoint(0);
         seektop = seekbot = 0;
         fb_initstack();
         for (p = 0; p < FB_MAXBY; p++)		/* NULL the by structure */
            by[p] = NULL;
         cur_p = 0;
         cur_line[0] = NULL;
         if (cdb_db->nfields > cdb_t_lines - 5)		/* 5 is the top 3 and bot 2 */
            choicepages = 2;
         else
            choicepages = 1;
         fb_cx_push_env("E", CX_IGEN_SORTBY_SELECT, NIL);
         /* test here whether D and dU belong */
         if (choicepages > 1)
            fb_cx_add_buttons("dU");
         fb_cx_add_buttons("Hc");			/* ol_cfield stuff */	
	 c_st = fchoice();
         for (;;){
            if (cur_line[0] != NULL){
               sprintf(buf, "Current Sort By Selections: %s", cur_line);
               fb_move(2, cdb_t_cols - strlen(buf) - 1);
               fb_printw(buf);
               }
            fb_infoline();
            fb_move(cdb_t_lines, 1); 
            fb_clrtoeol();
            fb_printw("Enter SortBy #'s:");
            strcpy(buf, "-=End");
            if (choicepages > 1)
               strcat(buf, ", <CTL>-F=Next Page, <CTL>-B=Prev Page");
            fb_scrhlp(buf);
            st = fb_input(-cdb_t_lines, 19, 15, 0, FB_ALPHA, line, FB_ECHO,
               -FB_OKEND, FB_CONFIRM);
            if (st == FB_AOK){
               add_getby(by, line);
               }
            else if (st == FB_ABORT){
               st = FB_ABORT;
               break;
               }
            else if (st == FB_END || st == FB_DEFAULT){
               st = FB_END;
               break;
               }
            else if (st == FB_QHELP)
               fb_fhelp(GETBY_HELP);
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
            c_st = fchoice();
            }
         fb_cx_pop_env();
         if (cur_p == 0){
            fb_screrr("No valid fields entered: Field_1 assumed...");
            by[0] = cdb_kp[0];
            }
	 return(st);
      }

/*
 * add_getby - add the fields the numbers represent from line onto the by array
 */

   add_getby(by, line)
      fb_field *by[];
      char *line;

      {
         int i, j;
         char word[FB_MAXLINE];

         for (i = 1; (i = fb_getword(line, i, word)) != 0; ){
            if (cur_p >= FB_MAXBY)
               break;
            j = atoi(word) - 1;
            if (j < 0 || j >= cdb_db->nfields)
               fb_serror(FB_MESSAGE, "Can't find field", word);
            else if (cdb_kp[j]->type == FB_FORMULA ||
                  cdb_kp[j]->type == FB_LINK ||
                  cdb_kp[j]->type == FB_BINARY)
               fb_screrr("No sorting by FORMULA, LINK, or BINARY.");
            else if (cdb_kp[j]->size > 300)
               fb_screrr("Cannot Use fields with NEWLINES for index.");
            else{
               by[cur_p++] = cdb_kp[j];
               strcat(cur_line, word);
               strcat(cur_line, " ");
               }
            }
      }

/* 
 *  showby - show the existing by fields
 */
 
   static showby(by)
      fb_field *by[];
      
      {
         int i;
	 
         fb_move(4, 1); 
         for (i = 0; i < FB_MAXBY; i++)
            if (by[i] != NULL)
               fb_printw("          (by: %s)\n", (by[i])->id);
      }
	    
/*
 * fchoice - display the fields
 */
 
    static fchoice()
      
      {
         register int i, j;
         fb_field *f;

         fb_move(3, 1); fb_clrtobot();
         j = seektop;
         fb_cx_set_seekpoint(seektop);
         for (i = 4; i <= 22 && j < cdb_db->nfields; i++, j++){
            fb_move(i, 15);
            f = cdb_db->kp[j];
            fb_printw(FMT3, j+1, f->id, f->type, f->size);
            if (f->idefault != NULL)
               fb_printw(FMT4, f->idefault);
            if (f->type == FB_FORMULA || f->type == FB_LINK)
               fb_printw(FB_FSTRING, f->idefault);
            }
         seekbot = j;
         if (j >= cdb_db->nfields)
            return(EOF);
         else
            return(FB_AOK);
      }
