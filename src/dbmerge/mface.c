/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mface.c,v 9.0 2001/01/09 02:55:41 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Mface_sid[] = "@(#) $Id: mface.c,v 9.0 2001/01/09 02:55:41 john Exp $";
#endif

#include <dbdmrg_e.h>

#include <setjmp.h>
#include <signal.h>
static jmp_buf jmp_env;
static RETSIGTYPE sigalrm();

static int sub_state;
static char command;
static char ncount = 5;
char iname[FB_MAXNAME];			/* index name - init in initmrg */
extern char filen[];			/* file name of buffer */

#define SUB_WINDOW_SIZE		10
#define INDEX_ROW		16
#define NCOUNT_ROW		18
#define READY_ROW		20
#define DISPLAY_COL		20
#define INPUT_COL		40

#define TEMPDIR			"./"
#define TEMPFILE		".cdb_XXXXXX"
static char *SCREEN_MSG =
   "Use FULL screen/print options? y=Yes, <RETURN>=simple display? ";

static sub_edit();
static input_sub();
static display_sub();
static clear_sub();
static checkarrow();
static merge_request();
static pagefile();

/*
 * mface - merge interface procedure for dbdmrg(1)
 */

   mface()
      {
         int st;

         fb_scrstat("Merge Level");
         fb_move(cdb_t_lines, 1); fb_clrtobot();
         st = edit_mface();
         /* clean up and return */
         mrg_display();
         fb_refresh();
         return(st);
      }

/*
 * edit_mface - edit the interface options
 */

   edit_mface()

      {
         int i, j, start_row, top_row;

         sub_state = 1;
         start_row = 12;
         j = start_row + 10;
         for (i = start_row; i < j; i++){
            fb_move(i, 1); fb_clrtoeol();
            }
         top_row = start_row + 1;
         fb_move(start_row, 1);
         fb_reverse("----------------------------------------");
         fb_move(start_row, 41);
         fb_reverse("----------------------------------------");
         return(sub_edit(top_row));
      }

/*
 * sub_edit - edit a token object main loop
 */

   static sub_edit(trow)
      int trow;

      {
         int st;

         for (;;){
            st = 0;
            clear_sub(trow);
            display_sub(trow, 0, 0);
            display_sub(trow, sub_state, 1);
            fb_refresh();
            command = NULL;
	    setjmp(jmp_env);
	    signal(SIGALRM, sigalrm);
            if (command == '\033')
               break;
	    read(0, &command, 1);
            if (command == '-' || command == 'q')
               break;
            switch(command){
               case 'h':
                  st = FB_ERROR;
                  command = '\033';		/* force exit */
                  break;
	       case '\012':
	       case '\015':
                  if (sub_state == 3){
                     merge_request(trow);
                     st = FB_ERROR;
                     command = '\033';		/* force exit */
                     break;
                     }
                  /* else drop thru to next clause */
               /* j ^N w - cursor down */
	       case 'j': case 'w':
	       case '\016':
                  if (++sub_state > 3)
                     sub_state = 1;
                  break;
               /* kh ^P - cursor up */
	       case 'k': case 'b':
	       case '\020':
                  if (--sub_state <= 0)
                     sub_state = 3;
                  break;
	       case '\033':			/* FB_ESCAPE */
	          st = checkarrow();
	          break;
               case 'i': case 'a':
               case 'o': case 'O':
               case 'r': case 'R':
               case 'c': case 'C':
               case 'l': case '@':
                  st = input_sub();
                  break;
	       default:
	          st = FB_ERROR;
		  break;
               }
            if (command == '\033' && st == FB_ERROR)
               break;
	    if (st == FB_ERROR){
	       fb_bell();
               fb_refresh();
               }
            }
         return(st);
      }

/*
 * input_sub - interface to fb_input information into mface fields
 */

   static input_sub()

      {
         int st;
         char buf[FB_MAXLINE];

         if (sub_state == 1){
            fb_scrhlp("Select index to use, default is Sequential Order");
            st = fb_input(INDEX_ROW, INPUT_COL, 40, 0, FB_ALPHA, buf,
               -FB_ECHO, FB_OKEND, FB_CONFIRM);
            if (st == FB_AOK){
               fb_trim(buf);
               strcpy(iname, buf);
               }
            else if (st == FB_DEFAULT)
               iname[0] = NULL;
            }
         else if (sub_state == 2){
            /* get number of records to process */
            fb_scrhlp("Enter number of records to process, 0 for ALL");
            st = fb_input(NCOUNT_ROW, INPUT_COL, 5, 0, FB_POS_NUM, buf,
               -FB_ECHO, FB_OKEND, FB_CONFIRM);
            if (st == FB_AOK)
               ncount = atoi(buf);
            }
         fb_move(cdb_t_lines, 1); fb_clrtoeol();
         return(FB_AOK);
      }

/*
 * display_sub - display the entire token unit
 */

   static display_sub(trow, state, rflag)
      int trow, state, rflag;

      {

         char buf[10], dname[FB_MAXNAME];

         if (state == 0){
            fb_move(trow + 1, DISPLAY_COL);
            fb_prints("Merge Database: ");
            fb_basename(dname, cdb_db->dbase);
            fb_move(trow + 1, INPUT_COL);
            fb_prints(dname);
            }
         if (state == 0 || state == 1){
            fb_move(INDEX_ROW, DISPLAY_COL);
            fb_prints("Index Name:");
            fb_move(INDEX_ROW, INPUT_COL);
            if (iname[0] != NULL){
               if (rflag)
                  fb_reverse(iname);
               else
                  fb_prints(iname);
               }
            else{
               if (rflag)
                  fb_reverse("Sequential Order");
               else
                  fb_prints("Sequential Order");
               }
            }

         if (state == 0 || state == 2){
            fb_move(NCOUNT_ROW, DISPLAY_COL);
            fb_prints("# of Records:");
            fb_move(READY_ROW, DISPLAY_COL);
            fb_prints("Run merge request?");

            fb_move(NCOUNT_ROW, INPUT_COL);
            if (ncount <= 0)
               sprintf(buf, "ALL");
            else
               sprintf(buf, "%d", ncount);
            if (rflag)
               fb_reverse(buf);
            else
               fb_prints(buf);
            }

         if (state == 0 || state == 3){
            fb_move(READY_ROW, INPUT_COL);
            if (rflag)
               fb_reverse("y");
            else
               fb_prints("y");
            }
      }

/*
 * clear_sub - clear the entire sub token unit
 */

   static clear_sub(trow)
     int trow;

      {
         int j;

         j = trow + SUB_WINDOW_SIZE;
         for (; trow < j; trow++){
            fb_move(trow, 1);
            fb_clrtoeol();
            }
      }

   static RETSIGTYPE sigalrm()
      {
         command = '\033';
	 longjmp(jmp_env, 1);
      }

   static checkarrow()

      {
	 int size, i, st = FB_ERROR;
	 char buf[10], *p;

         if ((size = strlen(cdb_KU)) == 0)
	    return(FB_ERROR);
	 p = buf;
	 *p++ = '\033';
         alarm(1);
	 for (i = 1; i < size; i++){
	    read(0, p, 1);
	    if (*p == '\033')
	       break;
	    p++;
	    }
	 *p = NULL;
	 alarm(0);
	 if (equal(buf, cdb_KL))
            st = FB_ERROR;		/* exit out of it */
         else if (equal(buf, cdb_KR))
            st = input_sub();
         else if (equal(buf, cdb_KU)){
            if (--sub_state <= 0)
               sub_state = 3;
            st = FB_AOK;
            }
         else if (equal(buf, cdb_KD
         )){
            if (++sub_state > 3)
               sub_state = 1;
            st = FB_AOK;
            }
	 return(st);
      }

/*
 * merge_request - set up the command, run it to a temp file
 *	fb_page the temp file into the window, delete the file
 *
 *	should do somthing like screenprint
 */

   static merge_request(trow)
      int trow;

      {
         char scommand[FB_MAXLINE], buf1[FB_MAXLINE], buf2[FB_MAXLINE];
         char output[FB_MAXNAME];

         if (modified){
            writefile();
            modified = 0;
            }
         sprintf(output, "%s%s", TEMPDIR, TEMPFILE);
         close(mkstemp(output));
         clear_sub(trow);
         fb_move(trow + 4, DISPLAY_COL + 5);
         fb_prints("Executing merge request ...");
         fb_move(trow + 6, 1);		/* leave cursor for outside errors */
         fb_refresh();
         buf1[0] = buf2[0] = NULL;
         if (iname[0] != NULL)
            sprintf(buf1, "-i %s", iname);
         if (ncount > 0)
            sprintf(buf2, "-n %d", ncount);
         sprintf(scommand, "dbmerge -b -w %d -d %s %s %s %s %s",
            linewidth, cdb_db->dbase, buf1, buf2, filen, output);
         fb_system(scommand, FB_NOROOT);
         if (fb_mustbe(CHAR_y, SCREEN_MSG, cdb_t_lines, 1) == FB_AOK){
            sprintf(scommand, "screenprint %s", output);
            fb_system(scommand, FB_NOROOT);
            fb_scrhdr(cdb_db, NIL);
            mrg_display();
            }
         else{
            pagefile(output, trow);
            }
         unlink(output);
      }

/* 
 *  pagefile - page a file to the screen...using only lines trow...cdb_t_lines-2
 *	and no touch of the timeline.
 */
 
   static pagefile(f, trow)
      char *f;
      int trow;
      
      {
         FILE *fs;
	 register int i, lc;
	 int eof, j;
	 char line[FB_MAXLINE], c, ms[30];
	 
         if (f == NULL || *f == NULL ||
	       (fs = fopen(f, FB_F_READ)) == NULL){
	    fb_serror(FB_CANT_OPEN, f, NIL);
	    return(FB_ERROR);
	    }
	 for (lc = 0; fgets(line, FB_MAXLINE, fs) != NULL; lc++)
	    ;
	 rewind(fs);
	 for (j = 0;;){
	    fb_move(trow, 1); fb_clrtobot(); fb_refresh();
	    for (eof = 0, i = trow; i <= cdb_t_lines-2; i++){
	       if (fgets(line, FB_MAXLINE, fs) == NULL){
	          eof = 1;
		  break;
		  }
	       line[strlen(line) - 1] = NULL;
	       fb_move(i, 1);
	       fb_printw(FB_FSTRING, line);
	       if (lc == ++j){
	          eof = 1;
		  break;
		  }
	       }
	    if (eof == 1){
	       fb_screrr(SYSMSG[S_END_FILE]);
	       break;
	       }
	    else{
	       fb_infoline();
	       sprintf(ms, SYSMSG[S_DISPLAY], j, lc);
	       fb_scrhlp(ms);
	       fb_move(cdb_t_lines, 1);
	       fb_printw(SYSMSG[S_SPACE_BAR]);
	       fb_refresh();
	       for (;;){
                  read(0, &c, 1);
		  if (c == FB_REDRAW1 || c == FB_REDRAW2)
		     fb_redraw();
		  else
		     break;
		  }
	       fb_move(cdb_t_lines, 1); fb_clrtoeol();
	       if (c != FB_BLANK)
	          break;
	       }
	    }
	 fclose(fs);
	 return(FB_AOK);
      }
