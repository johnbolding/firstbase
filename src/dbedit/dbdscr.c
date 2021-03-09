/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbdscr.c,v 9.1 2001/02/16 19:41:42 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)dbdscr.c	8.3 05 Apr 1996 FB";
#endif

/* 
 *  dbdscr.c - screen editor for screen dictionary (sdict) file
 */

#include <fb.h>
#include <fb_vars.h>
#include <dbd.h>

static int dot = 0;			/* current fb_field */
static int lastitem = 0;		/* last list item */

static fb_upd **itemlist = NULL;	/* for list of items */

static int col[8] = {1, 6, 18, 22, 30};	/* edit columns (screen) */

static fb_database *hp;
static char sdict[FB_MAXNAME];		/* for sdict filename */

static char *HLP_DBDSCR = "dbdscr.hlp";

#define SHOWP 2				/* ending of shown screen columns */

#if FB_PROTOTYPES
static edit_sdict();
static fillscreen();
static escreen();
static void fdelete();
static void finsert();
static fadder();
static eitem();
static fput();
static new_sdict();
static headers();
static usage();
static read_sdict(char *fname, fb_upd **itemlist);
#else /* FB_PROTOTYPES */
static edit_sdict();
static fillscreen();
static escreen();
static void fdelete();
static void finsert();
static fadder();
static eitem();
static fput();
static new_sdict();
static headers();
static usage();
static read_sdict();
#endif /* FB_PROTOTYPES */

extern short int cdb_locklevel;
extern short int cdb_askgen;

extern char *cdb_EDIT;

/* 
 *  dbdscr - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
      
      {
         int st, i, p;

	 cdb_locklevel = -1;
         fb_getargs(argc, argv, FB_ALLOCDB);
	 hp = cdb_db;
	 fb_initdbd(hp);
         if (fb_getd_dict(hp) == FB_ERROR)
            fb_xerror(FB_BAD_DICT, hp->ddict, NIL);            
         else
            for (i = 0; i < hp->nfields; i++)
               fb_nounders(cdb_keymap[i]);
	 fb_scrhdr(hp, "Define Screen");
         headers();
	 fb_rootname(sdict, hp->dbase);
	 if ((p = fb_testargs(argc, argv, "-s")) > 0){
	    if (++p >= argc)
	       usage();
	    if (!equal(argv[p], "-"))	/* get around the -s - menu call */
	       fb_rootname(sdict, argv[p]);
	    }
	 strcat(sdict, ".sdict");
         st = edit_sdict(sdict);
         if (st == FB_END){
            new_sdict(sdict);
	    if (cdb_askgen)
	       fb_chainto("Run Dbedit", cdb_EDIT, argv);
	    }
         fb_ender();
      }

/* 
 *  edit_sdict - edit an dbase.sdict or allow creation of one 
 */
 
   static edit_sdict(p)
      char *p;
      
      {
         int st, i;

	 itemlist =
	     (fb_upd **) fb_malloc((hp->nfields+1) * sizeof(fb_upd *));
	 			/* allocate enough ptrs for all fields */
	 if ((lastitem = read_sdict(p, itemlist)) == FB_ERROR)
	    lastitem = 0;
	 for (i = lastitem + 1; i < hp->nfields ; i++)
	    itemlist[i] = NULL;
         dot = 1;
         fillscreen();
         while ((st = escreen()) != FB_END)
            if (st == FB_ABORT)
               break;
            else if (st >= dot && st <= dot + 9 && st <= lastitem){
               fb_cx_push_env("6D", CX_KEY_SELECT, NIL);
               eitem(st, 0);
               fb_cx_pop_env();
               }
            else if (st >= 1 && st <= lastitem){
               dot = fb_onpage(st);
               fillscreen();
               }
            else
               fb_screrr("Invalid Input");
         return(st);
      }

/* 
 *  fillscreen - fill a screen with items
 */
 
   static fillscreen()
      {
         int j, i, p;
         
         fb_move(4, 1); fb_clrtobot();
         j = dot + 9;
         for (i = dot; i <= j && i <= lastitem; i++)
            for (p = 0; p <= SHOWP; ++p)
               fput(i, p);
	 fb_infoline();
         fb_cx_set_viewpage(dot);
      }
            
/* 
 *  escreen - set up a screen, accept commands for screen control 
 */
 
   static escreen()
      {
         int st;
	 char com[6];
         
         for (;;){
            fb_cx_push_env("E1", CX_DBD_SELECT, NIL);
            if (lastitem > 10)
               fb_cx_add_buttons("Ge");
            fb_cx_add_buttons("XH");
            st = fb_anychange(com);
            fb_cx_pop_env();
            switch (st) {
               case FB_ABORT:
	          if (fb_mustbe('y', "Really Quit? (y/n) ",
                        cdb_t_lines, 1) == FB_AOK){
                     fb_screrr("Screen Dictionary File Unchanged");
		     return(st);
		     }
		  break;
               case ADDMODE:
	          if (lastitem < hp->nfields)
                     fadder();
		  else
		     continue;
                  break;
               case DELMODE:
	          if (lastitem > 0)
                     fdelete();
		  else
		     continue;
                  break;
               case FB_END:
                  return(st);
	       case HELPMODE:
	          fb_move(2, 1); fb_clrtobot();
		  fb_help(com, hp);
		  fb_move(4,1); fb_clrtobot();
		  headers();
		  break;
	       case INSERTMODE:
	          if (lastitem > 0 && lastitem < hp->nfields)
	             finsert();
		  else
		     continue;
		  break;
               case PAGEF:
                  dot = dot + 10;
                  if (dot > lastitem)
                     dot = 1;
                  break;
               case PAGEB:
                  dot = dot - 10;
                  if (dot < 1)
                     dot = fb_onpage(lastitem);
                  break;
	       case FB_QHELP: 
	          fb_move(2, 1); fb_clrtobot();
	          fb_fhelp(HLP_DBDSCR);
		  headers();
		  break;
	       case FB_AOK:
                  return(atoi(com));
               default:
                  return(st);
               }
            fillscreen();
            }
      }

/* 
 *  fdelete - delete an update item 
 */
 
   static void fdelete()
      {
         int st, n;
         
         fb_move(cdb_t_lines, 1);
         fb_clrtoeol();
         fb_printw("Delete what number ? (#,-) ");
         st = fb_input(cdb_t_lines, 28, 3, 0, FB_INTEGER, (char *) &n,
            FB_ECHO,FB_OKEND,FB_CONFIRM);
         if (st == FB_END || st == FB_ABORT)
            return;
	 else if (n < 1 || n > lastitem)
	    return;
	 if (itemlist[n] != NULL)
	    fb_free((char *) itemlist[n]);
         for (; n < lastitem; n++)
            itemlist[n] = itemlist[n + 1];
         lastitem--;
      }

/* 
 *  finsert - insert a defaulted item before a given item 
 */
 
   static void finsert()            
      {
         int st, n, rn;

	 fb_fmessage("Insert before what number (#,-) ? ");
         st = fb_input(cdb_t_lines, 36, 3, 0, FB_INTEGER, (char *) &n, FB_ECHO,
            FB_OKEND, FB_CONFIRM);
         if (st == FB_END || st == FB_ABORT)
            return;
	 else if (n < 1 || n > lastitem)
	    return;
	 rn = n;
         for (n = lastitem; n >= rn; n--)		/* go backwards */
            itemlist[n+1] = itemlist[n];
	 itemlist[rn] = (fb_upd *) fb_malloc(sizeof(fb_upd));
	 itemlist[rn]->fp = NULL;		/* default to first fb_field */
	 itemlist[rn]->fv = NULL;
         lastitem++;
      }

/* 
 *  fadder - go into auto add mode 
 */
 
   static fadder()
      {
         int st;

         dot = fb_onpage(lastitem);
	 if ((lastitem + 1) < (dot + 10))
            fillscreen();
         for (;lastitem < hp->nfields;){
	    itemlist[++lastitem] = (fb_upd *) fb_malloc(sizeof(fb_upd));
	    itemlist[lastitem]->fp = NULL;
	    itemlist[lastitem]->fv = NULL;
            if (lastitem >= dot + 10){
               dot += 10;
               fb_move(4, 1); fb_clrtobot(); fb_infoline();
               }
            fb_cx_push_env("6D", CX_KEY_SELECT, NIL);
            st = eitem(lastitem, 1);
            fb_cx_pop_env();
            if (st == FB_END){
	       fb_free((char *) itemlist[lastitem--]);
               break;
	       }
            }
      }

/* 
 *  eitem - edit an entire item 
 */
 
   static eitem(it, new)
      int it, new;

      {
         int row, i, st;
         char *fb_trim(), temp[FB_MAXLINE];
	 fb_field *fp = NULL;

         fb_move(cdb_t_lines, 1); fb_clrtoeol();
         if ((row = (it % 10)) == 0)
            row = 10;
         row = row * 2 + 2;
	 if (itemlist[it] == NULL)
	    fb_xerror(FB_BAD_DICT, hp->idict, "bad itemlist");
         if (new == 1)
            fput(it, 0);

         /* 
	  *  get id 
	  */
	  
         for (;;){
	    if (new)
               fb_fmessage("Enter FieldName, -=End");
	    else
               fb_fmessage("Enter FieldName, <CTL>-X=Skip");
            st = fb_input(row, col[1], FB_TITLESIZE, 0, FB_ALPHA, 
                 temp, FB_ECHO, FB_OKEND, FB_CONFIRM);
            if (st == FB_END && new == 1){
               fb_move(row, 1); 
               fb_clrtoeol();
               return(FB_END);
               }
            else if (st == FB_END)
               st = FB_DEFAULT;
            if (st == FB_DEFAULT || (st == FB_ABORT && new == 1)){
	       if ((it - 1) < hp->nfields)
	          fp = cdb_kp[it - 1];
	       else
	          fp = cdb_kp[0];
	       }
            else if (st == FB_ABORT)
               break;
            else if (st == FB_AOK){
               fb_trim(temp);
	       fp = fb_findfield(temp, hp);
	       }
	    if (fp == NULL){
	       fb_screrr("Cannot locate that field. Sorry.");
	       continue;
	       }
	    else if (it == 1 && (fp->type == FB_FORMULA || fp->type==FB_LINK)){
	       fb_screrr("Must use a non-formula/link field as first field!");
	       continue;
	       }
	    for(i = 1; i <= lastitem; i++)
	       if (itemlist[i]->fp == fp)
	          break;
	    if (i <= lastitem){
	       fb_screrr("That field is already in this screen dictionary!");
	       continue;
	       }
            itemlist[it]->fp = fp;
	    break;
            }
         fput(it, 1);
	 
	 /* 
	  *  get override default 
	  */
	  
         for(;;){
	    if (itemlist[it]->fp->type == FB_FORMULA ||
	        itemlist[it]->fp->type == FB_LINK)
	       break;
	    if (new)
	       fb_fmessage("Enter Default Value");
	    else
	       fb_fmessage("Enter Default Value, <CTL>-X=Skip");
            st = fb_input(row, col[2], 15, 0, FB_ALPHA,temp, FB_ECHO,
               FB_NOEND, FB_CONFIRM);
            if (st == FB_ABORT && new != 1)
               strcpy(temp, itemlist[it]->fv);
	    else if (st == FB_DEFAULT || st == FB_ABORT)
	       strcpy(temp, " ");
	    else if (st == FB_AOK){
	       fb_trim(temp);
	       if (temp[0] == NULL)
	          strcpy(temp, "_");
	       fb_underscore(temp, 1);
	       }
	    fb_mkstr(&(itemlist[it]->fv), temp);
            break;
            }
         fput(it, 2);
         return(FB_AOK);
      }

/* 
 *  fput - fb_put an entire item to a constant screen location 
 */
 
   static fput(i, pos)
      int i, pos;
      
      {
         int row;
         
         if ((row = (i % 10)) == 0)
            row = 10;
         row = row * 2 + 2;
         fb_move(row, col[pos]);
         switch (pos){
            case 0: fb_printw("%3d)", i); break;
            case 1: 
	       if (itemlist[i]->fp != NULL)
	          fb_printw("%-10s", itemlist[i]->fp->id);
	       else
	          fb_printw("<empty>");
	       break;
	    case 2:
	       fb_clrtoeol();
	       if (itemlist[i]->fv != NULL)
	          fb_printw(FB_FSTRING, itemlist[i]->fv); 
	       break;
            }
      }

/* 
 *  new_sdict - printout a new dictionary list of itemlist 
 */
 
   static new_sdict(p)
      char *p;
      
      {
         int i;
	 FILE *fs, *fb_mustfopen();
	 
	 fs = fb_mustfopen(p, "w");
	 for (i = 1; i <= lastitem; i++){
	    if (itemlist[i]->fp != NULL)
	       fprintf(fs, "%s", fb_underscore(itemlist[i]->fp->id, 1));
	    if (itemlist[i]->fv != NULL && 
	          strlen(fb_trim(itemlist[i]->fv)) > 0)
	       fprintf(fs, " -d %s", fb_underscore(itemlist[i]->fv, 1));
	    fprintf(fs, "\n");
	    }
	 fclose(fs);
      }
      
/* 
 *  headers - print header at top of screen 
 */
 
   static headers()
      {
         fb_move(2, 1); fb_clrtoeol();
	 fb_move(3, 1); fb_clrtoeol();
         fb_move(2,col[1]);   fb_printw("Field");
         fb_move(3,col[1]);   fb_printw("=====");
	 fb_scrstat2(sdict);
	 fb_move(2,col[2]); fb_printw("Default");
	 fb_move(3,col[2]); fb_printw("=======");
/*	 fb_move(2,col[3]+2); fb_printw("Size");
*	 fb_move(3,col[3]+2); fb_printw("====");
*	 fb_move(2,col[4]);   fb_printw("Update String");
*	 fb_move(3,col[4]);   fb_printw("=============");
*/	 
      }
      
/* 
 *  usage message 
 */
 
   static usage()
      {
         fb_xerror(FB_MESSAGE, "usage: dbdscr [-d dbase] [-s index]", NIL);
      }

/* 
 *  read_sdict - read in an existing sdict file for editing 
 */
 
   static read_sdict(fname, itemlist)
      char *fname;
      fb_upd **itemlist;
      
      {
         char line[FB_MAXLINE], word[FB_MAXLINE];
	 fb_upd *up;
	 fb_field *fp;
	 int p, q;
	 FILE *fs;
	 
	 if ((fs = fopen(fname, "r")) == NULL)
	    return(FB_ERROR);
	 q = 0;
	 while (fgets(line, FB_MAXLINE, fs) != NULL){
	    if (line[0] == '%')
	       break;
	    q++;
	    p = fb_getword(line, 1, word);
	    fb_underscore(word, 0);
	    if ((fp = fb_findfield(word, cdb_db)) == NULL)
	       fb_xerror(FB_BAD_DICT, word, NIL);
	    up = (fb_upd *) fb_malloc(sizeof(fb_upd));
	    up->fp = fp;
	    up->fv = NULL;
	    itemlist[q] = up;
	    
	    p = fb_getword(line, p, word);
	    if (equal(word, "-d")){
	       p = fb_getword(line, p, word);
	       fb_mkstr(&(up->fv), word);
	       }
	    else
	       fb_mkstr(&(up->fv), " ");
	    }
	 fclose(fs);
	 return(q);
      }
