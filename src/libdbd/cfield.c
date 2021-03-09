/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: cfield.c,v 9.0 2001/01/09 02:56:38 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Choosefield_sid[] = "@(#) $Id: cfield.c,v 9.0 2001/01/09 02:56:38 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static long seektop = 0;
static long seekbot = 0;
static char *FMT3 = "%3d> %-10s %c %6d";
static char *FMT4 = " [%s] ";

#define CHOOSEFIELD_HELP "choosefield.hlp"
extern long cdb_seekstack[];
extern short int cdb_topstack;
static int choicepages;

#if FB_PROTOTYPES
static r_choosefield(char *buf);
static fchoice(void);
#else
static r_choosefield();
static fchoice();
#endif /* FB_PROTOTYPES */

/* 
 * choosefield - allowing choosing of fb_field by choice mechanism
 */
 
   fb_choosefield(buf)
      char *buf;
      
      {
         int st;
	 
         fb_move(2, 1);
	 fb_clrtobot();
	 fb_scrstat("Choose Field");

         fb_cx_set_seekpoint(0);
         seektop = seekbot = 0;
         fb_initstack();
	 buf[0] = NULL;
	 st = r_choosefield(buf);
         fb_cx_pop_env();
	 return(st);
      }

/*
 * r_choosefield - choosefield mechanism - the driver
 */

   static r_choosefield(buf)
      char *buf;
      
      {
         int st, c_st = 0, j;
	 char inp[FB_MAXLINE];

         if (cdb_db->nfields > cdb_t_lines - 5)	/* 5 is the top 3 and bot 2 */
            choicepages = 2;
         else
            choicepages = 1;
         /* 6=Skip, dbd style */
         fb_cx_push_env("6E", CX_CHOOSEFIELD_SELECT, NIL);
         /* test here whether D and dU belong */
         if (choicepages > 1)
            fb_cx_add_buttons("dU");
         fb_cx_add_buttons("H");
         fb_cx_add_buttons("c");			/* ol_cfield stuff */	
	 c_st = fchoice();
	 for (st = FB_ERROR; st == FB_ERROR ;){
	    for (;;){
	       fb_infoline();
               fb_move(cdb_t_lines, 1);
	       fb_printw("Enter Field # :");
               if (choicepages <= 1)
                  buf[0] = NULL;
               else
                  strcpy(buf, "<CTL>-F=Next, <CTL>-B=Prev, ");
               strcat(buf, "-=END, <CTL>-X=Skip");
               fb_scrhlp(buf);
               st = fb_input(-cdb_t_lines, 17, 4, 0,
                  FB_POS_NUM, inp, FB_ECHO, -FB_OKEND, FB_CONFIRM);
               if (st == FB_ABORT || st == FB_END || st == FB_DEFAULT)
                  return(st);

               if (st == FB_AOK){
                  fb_trim(inp);
                  j = atoi(inp) - 1;
                  if (j >= 0 && j < cdb_db->nfields){
                     strcpy(buf, cdb_db->kp[j]->id);
                     break;
                     }
                  }
               else if (st == FB_QHELP)
                  fb_fhelp(CHOOSEFIELD_HELP);
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
	    }
	 return(st);
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
         for (i = 4; i <= cdb_t_lines-2 && j < cdb_db->nfields; i++, j++){
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
