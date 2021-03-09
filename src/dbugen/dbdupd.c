/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbdupd.c,v 9.1 2001/02/16 19:48:39 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)dbdupd.c	8.2 05 Apr 1996 FB";
#endif

/* 
 *  dbdupd.c - screen editor for update dictionary (idictu) file
 */

#include <fb.h>
#include <fb_vars.h>
#include <dbd.h>

extern short int cdb_locklevel;
extern short int cdb_choosefield;
extern char *cdb_T_DELETE;
extern short int cdb_askgen;
extern char *cdb_UGEN;

#define HLP_DBDUPD	"dbdupd.hlp"

static int dot = 0;			/* current fb_field */
static int lastitem = 0;		/* last list item */

static fb_upd **itemlist = NULL;	/* for list of items */

static int col[8] = {1, 6, 18, 22, 30};	/* edit columns (screen) */

static fb_database *hp;

#define SHOWP 4				/* ending of shown screen columns */


static edit_idictu();
static fillscreen();
static escreen();
static void fdelete();
static void finsert();
static fadder();
static eitem();
static check();
static fput();
static newdictu();
static headers();

/* 
 *  dbdupd - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
      
      {
         int st, i;
	 char idictu[FB_MAXNAME];

         cdb_locklevel = -1;
         fb_getargs(argc, argv, FB_ALLOCDB);
	 hp = cdb_db;
	 fb_initdbd(hp);
         if (fb_getd_dict(hp) == FB_ERROR)
            fb_xerror(FB_BAD_DICT, hp->ddict, NIL);            
         else
            for (i = 0; i < hp->nfields; i++)
               fb_nounders(cdb_keymap[i]);
	 fb_scrhdr(hp, "Define Updates");
         headers();
	 sprintf(idictu, "%su", hp->idict);	/* idictu file for updates */
         st = edit_idictu(idictu);
         if (st == FB_END){
            newdictu(idictu);
	    if (cdb_askgen)
	       fb_chainto("Generate Updates", cdb_UGEN, argv);
	    }
         fb_ender();
      }

/* 
 *  edit_idictu - edit an index.idictu or allow creation of one 
 */
 
   static edit_idictu(p)
      char *p;
      
      {
         int st, i;

	 itemlist =
	     (fb_upd **) fb_malloc((hp->nfields+1) * sizeof(fb_upd *));
	 			/* allocate enough ptrs for all fields */
	 if ((lastitem = read_idictu(p, itemlist)) == FB_ERROR)
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
                     fb_screrr("Update Dictionary File Unchanged");
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
	          fb_fhelp(HLP_DBDUPD);
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
 *  fdelete - fb_delete an update item 
 */
 
   static void fdelete()
      {
         int st, n;
         
         fb_move(cdb_t_lines, 1);
         fb_clrtoeol();
         fb_printw("Delete what number ? (#,-) ");
         st = fb_input(cdb_t_lines, 28, 3, 0, FB_INTEGER, (char *) &n,
            FB_ECHO, FB_OKEND, FB_CONFIRM);
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
         st = fb_input(cdb_t_lines, 36, 3, 0, FB_INTEGER, (char *) &n,
            FB_ECHO, FB_OKEND, FB_CONFIRM);
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
	    if (!cdb_choosefield){
	       if (new)
		  fb_fmessage("Enter FieldName, -=End");
	       else
		  fb_fmessage("Enter FieldName, <CTL>-X=Skip");
	       st = fb_input(row, col[1], FB_TITLESIZE, 0, FB_ALPHA, 
		    temp, FB_ECHO, FB_OKEND, FB_CONFIRM);
	       }
	    else{
	       st = fb_choosefield(temp);
	       }
            if (st == FB_END && new == 1){
               fb_move(row, 1); 
               fb_clrtoeol();
	       if (cdb_choosefield)
	          headers();
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
	    if (fp->type == FB_FORMULA || fp->type == FB_LINK){
	       fb_screrr("Not here silly. Use dbdbas to change formulas/links.");
	       continue;
	       }
	    for(i = 1; i <= lastitem; i++)
	       if (itemlist[i]->fp == fp)
	          break;
	    if (i <= lastitem){
	       fb_screrr("That field is already in this update dictionary!");
	       continue;
	       }
            itemlist[it]->fp = fp;
	    break;
            }
	 if (cdb_choosefield){
	    headers();
	    if (!new){
	       fillscreen();
	       }
	    else{				/* must be new */
	       lastitem--;
	       fillscreen();
	       lastitem++;
	       fput(it, 0);	/* puts number on screen */
	       }
	    }
         fput(it, 1);
	 fput(it, 2);		/* type is always constant */
	 fput(it, 3);		/* size is always constant */
	 
	 /* 
	  *  get updates 
	  */
	  
         for(;;){
	    if (FB_OFNUMERIC(itemlist[it]->fp->type))
	       fb_fmessage("Enter Update (numeric/formula), <CTL>-X=Skip");
	    else
	       fb_fmessage("Enter Update (alphanumeric), <CTL>-X=Skip");
            st = fb_input(row, col[4], 49, 0,FB_ALPHA,temp,FB_ECHO,FB_NOEND,FB_CONFIRM);
            if (st == FB_ABORT && new != 1)
               strcpy(temp, itemlist[it]->fv);
	    else if (st == FB_DEFAULT || st == FB_ABORT)
	       strcpy(temp, " ");
	    if (check(fb_trim(temp), itemlist[it]->fp) != FB_AOK)
	       continue;
	    if (itemlist[it]->fv != NULL)
	       fb_free(itemlist[it]->fv);
	    itemlist[it]->fv = (char *) fb_malloc(strlen(temp)+1);
	    strcpy(itemlist[it]->fv, temp);
            break;
            }
         fput(it, 4);
	 
	 /*
	  * finally return
	  */

         return(FB_AOK);
      }

/* 
 *  check - check the type against the buffer for validity: 
 *    allows formulas for numeric fields 
 */
 
   static check(p, k)
      fb_field *k;
      char *p;
      
      {
         if (!(FB_OFNUMERIC(k->type))){
	    if (!equal(cdb_T_DELETE, p) && strlen(p) > k->size)
	       p[k->size] = NULL;
            return(FB_AOK);
	    }
	 return(fb_getformula(k, p, NIL, 1, hp));
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
	       if (itemlist[i]->fp != NULL)
	          fb_printw(FB_FCHAR, itemlist[i]->fp->type);
	       break;
	    case 3: 
	       if (itemlist[i]->fp != NULL)
                  fb_printw("%6d", itemlist[i]->fp->size);
	       break;
	    case 4: 
	       fb_clrtoeol();
	       if (itemlist[i]->fv != NULL)
	          fb_printw(FB_FSTRING, itemlist[i]->fv); 
	       break;
            }
      }

/* 
 *  newdictu - printout a new dictionary list of itemlist 
 */
 
   static newdictu(p)
      char *p;
      
      {
         int i;
	 FILE *fs, *fb_mustfopen();
	 
	 fs = fb_mustfopen(p, "w");
	 for (i = 1; i <= lastitem; i++)
	    if (itemlist[i]->fp != NULL){
	       fb_trim(itemlist[i]->fv);
	       fb_underscore(itemlist[i]->fp->id, 1); /* fb_put back underscores */
	       fb_underscore(itemlist[i]->fv, 1);     /* insert underscores */
	       fprintf(fs, "%s %s\n",
	             itemlist[i]->fp->id, itemlist[i]->fv);
	       }
	 fprintf(fs, "%%\n");
	 fclose(fs);
      }
      
/* 
 *  headers - print header at top of screen 
 */
 
   static headers()
      {
	 fb_scrstat("Define Updates");
         fb_move(2, 1); fb_clrtoeol();
	 fb_move(3, 1); fb_clrtoeol();
         fb_move(2,col[1]);   fb_printw("Field");
         fb_move(3,col[1]);   fb_printw("=====");
	 fb_move(2,col[2]-1); fb_printw("Type");
	 fb_move(3,col[2]-1); fb_printw("====");
	 fb_move(2,col[3]+2); fb_printw("Size");
	 fb_move(3,col[3]+2); fb_printw("====");
	 fb_move(2,col[4]);   fb_printw("Update String");
	 fb_move(3,col[4]);   fb_printw("=============");
      }
      
/* 
 *  usage message 
 */
 
   usage()
      {
         fb_xerror(FB_MESSAGE, "usage: dbdupd [-d dbase] [-i index]", NIL);
      }
