/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbdcnv.c,v 9.1 2001/02/16 19:44:55 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)dbdcnv.c	8.2 05 Apr 1996 FB";
#endif

/* 
 *  dbdcnv.c - screen editor for conversion (idictc) file
 */

#include <fb.h>
#include <fb_vars.h>
#include <dbd.h>
#include <pgen.h>

#define HLP_DBDCNV	"dbdcnv.hlp"

extern short int cdb_locklevel;
extern short int cdb_askgen;
extern char *cdb_CGEN;

static int col[8] = {1, 6, 20, 35, 50, 60, 70};

static int dot = 0;			/* current fb_field */
static int lastitem = 0;		/* last list item */

static fb_field *itemlist[FB_MAXKEEP] = { NULL };
static int csize[FB_MAXKEEP] = { 0 };
static char fname[FB_MAXNAME] = {""};	/* output file name */

fb_database *hp;

#define SHOWP 3				/* ending of shown screen columns */

static edit_idictc();
static fillscreen();
static escreen();
static void fdelete();
static void finsert();
static fadder();
static eitem();
static gettitle();
static changetitle();
static fput();
static newdictc();
static read_idictp();
static headers();

/* 
 * dbdcnv - main driver for dbdcnv
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
      
      {
         int st, i;
	 char idictc[FB_MAXNAME];

	 cdb_locklevel = -1;
         fb_getargs(argc, argv, FB_ALLOCDB);
	 hp = cdb_db;
	 fb_initdbd(hp);
         if (fb_getd_dict(hp) == FB_ERROR)
            fb_xerror(FB_BAD_DICT, hp->ddict, NIL);
         else
            for (i = 0; i < (hp->nfields); i++)
               fb_nounders(cdb_keymap[i]);
	 fb_scrhdr(hp, "Define Conversion");
         headers();
	 sprintf(idictc, "%sc", hp->idict);
         st = edit_idictc(idictc);
         if (st == FB_END){
	    gettitle();
            newdictc(idictc);
	    if (cdb_askgen)
	       fb_chainto("Generate Conversion", cdb_CGEN, argv);
	    }
         fb_ender();
      }

/* 
 *  edit_idictc - edit an index.idictc or allow creation of one.
 */
 
   static edit_idictc(p)
      char *p;
      
      {
         int st, i;
	 FILE *fs;

         fb_basename(fname, DCNV);
         if ((fs = fopen(p, "r")) != NULL)
	    read_idictp(fs);
	 for (i = lastitem + 1; i < FB_MAXKEEP; i++)
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
 *  fillscreen - fill a screen with dictionary items.
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
 *  escreen - set up a screen, accept commands for screen control.
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
                     fb_screrr("Conversion Dictionary File Unchanged");
		     return(st);
		     }
		  break;
               case ADDMODE:
                  fadder();
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
	          if (lastitem > 0 && lastitem < FB_MAXKEEP)
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
	          fb_fhelp(HLP_DBDCNV);
		  headers();
		  break;
	       case MINFOMODE:
	          gettitle();
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
 *  fdelete - fb_delete a dictionary item 
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
	 if (csize[n] < 0)
	    fb_free((char *) itemlist[n]);
         for (; n < lastitem; n++){
            itemlist[n] = itemlist[n + 1];
	    csize[n] = csize[n + 1];
	    }
         lastitem--;
      }

/* 
 *  finsert - insert a defaulted dict item before a given item.
 */
 
   static void finsert()            
      {
         int st, n, rn;

	 fb_fmessage("Insert before what number (#,-) ? ");
         st = fb_input(cdb_t_lines, 36, 3, 0, FB_INTEGER, (char *) &n,
            FB_ECHO,FB_OKEND,FB_CONFIRM);
         if (st == FB_END || st == FB_ABORT)
            return;
	 else if (n < 1 || n > lastitem)
	    return;
	 rn = n;
         for (n = lastitem; n >= rn; n--){		/* go backwards */
            itemlist[n+1] = itemlist[n];
	    csize[n+1] = csize[n];
	    }
	 itemlist[rn] = cdb_kp[0];			/* default to first fb_field */
         lastitem++;
      }

/* 
 *  fadder - go into auto add mode.
 */
 
   static fadder()
      {
         int st;

         dot = fb_onpage(lastitem);
	 if ((lastitem + 1) < (dot + 10))
            fillscreen();
         for (;lastitem < FB_MAXKEEP;){
            if (++lastitem >= dot + 10){
               dot += 10;
               fb_move(4, 1); fb_clrtobot(); fb_infoline();
               }
            fb_cx_push_env("6D", CX_KEY_SELECT, NIL);
            st = eitem(lastitem, 1);
            fb_cx_pop_env();
            if (st == FB_END){
	       lastitem--;
               break;
	       }
            }
      }

/* 
 *  eitem - edit an entire item.
 */
 
   static eitem(it, new)
      int it, new;

      {
         int row, st, itemp, i;
         char buf[FB_MAXLINE], *fb_trim(), temp[FB_MAXLINE];
	 fb_field *fp = NULL;

         fb_move(cdb_t_lines, 1); fb_clrtoeol();
         if ((row = (it % 10)) == 0)
            row = 10;
         row = row * 2 + 2;
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
	       strcpy(temp, fp->id);
	       fb_move(row, col[1]); fb_printw("%s", temp);
	       st = FB_AOK;
	       }
            if (st == FB_ABORT)
               break;
            else if (st == FB_AOK){
               fb_trim(temp);
	       for (i = 1; i <= lastitem; i++){
	          if (i != it)
		     if (equal(temp, itemlist[i]->id))
		        break;
		  }
	       if (i <= lastitem){
	          fb_screrr("That Field Id already used");
		  continue;
		  }
	       fp = fb_findfield(temp, hp);
	       }
	    if (itemlist[it] != NULL){
	       if (csize[it] < 0)		/* clean out what was there */
	          fb_free((char *) itemlist[it]);
	       itemlist[it] = NULL;
	       csize[it] = 0;
	       }
	    if (fp == NULL){
	       for (i = 0; temp[i]; i++){
	          if ((temp[i] >= 'A' && temp[i] <= 'Z') ||
		      (temp[i] >= 'a' && temp[i] <= 'z') ||
		      (temp[i] >= '0' && temp[i] <= '9') ||
		      (temp[i] == FB_BLANK))
		     continue;
		  else{
		     fb_screrr("FieldName must be Alphanumeric!");
		     break;
		     }
		  }
	       if (temp[i])
	          continue;
	       fb_fmessage("NEW field!");
	       fp = (fb_field *) fb_malloc(sizeof(fb_field));
	       fp->id = (char *) fb_malloc(strlen(temp) + 1);
	       strcpy(fp->id, temp);
	       fp->size = 10;
	       fp->type = 'a';
	       csize[it] = -10;	/* so type will prompt */
	       }
	    else{
	       fb_fmessage("Existing field.");
	       csize[it] = fp->size;
	       }
            itemlist[it] = fp;
	    break;
            }
         fput(it, 1);
	 fput(it, 3);
	 
         /* 
	  *  get type -- only if new, meaning csize < 0
	  */

	 for(;;){
	    if (csize[it] > 0)		/* no chaning types if fb_field exists */
	       break;
	    fb_fmessage("Enter type [aAcdfFnN$] for NEW field");
            st = fb_input(row, col[2], 1, 0, FB_ALPHA, buf, FB_ECHO, FB_NOEND, FB_CONFIRM);
            if (st == FB_DEFAULT || (st == FB_ABORT && new == 1))
               buf[0] = 'a';
            else if (st == FB_ABORT)
               break;
            switch(buf[0]){
	       case I_FORMULA:
	       case I_LINK:
               case I_ALPHA: case I_NUMERIC: case I_DATE: case I_DOLLARS:
               case I_STRICTALPHA: case I_FLOAT:
	       case I_CHOICE: case I_POS_NUM: case I_UPPERCASE:
	       case I_CCHOICE:
	       case I_BINARY:
	       case I_EXCHOICE:
                  st = FB_AOK;
                  break;
	       case I_TIME: 
	       case I_YESNO:
	          fb_screrr("Unimplemented Type");
		  st = FB_ERROR;
		  break;
               default:
                  fb_screrr("Invalid Type: use [aAcdfFnNty$]");
                  continue;
               }
            if (st == FB_AOK){
               itemlist[it]->type = buf[0];
               break;
               }
            }
	 fput(it, 2);
	 
        /* 
	 *  size
	 */
	 
         for(;;){
	    if (itemlist[it]->type ==FB_DATE){
	       if (csize[it] < 0)
	          fp->size = csize[it] = -6;	/* new date fb_field */
	       break;
	       }
	    fb_fmessage("Enter Size, <RET>=Default, <CTL>-X=Skip");
            st = fb_input(row, col[3], 6, 0,FB_INTEGER,(char *) &itemp,
	          FB_ECHO,FB_NOEND,FB_CONFIRM);
	    if (st == FB_DEFAULT || st == FB_ABORT){
	       if (csize[it] < 0)
	          itemp = -csize[it];	/* new - make pos so -itemp works */
	       else
	          itemp = csize[it];
	       }
	    else if (itemp < 0)
	       continue;
	    else if (itemp >= FB_NUMLENGTH && 
	          itemlist[it]->type != FB_ALPHA && 
	          itemlist[it]->type != FB_BINARY && 
	          itemlist[it]->type != FB_STRICTALPHA &&
		  itemlist[it]->type != FB_CHOICE){
		fb_screrr("Must be less than 16 to behave numerically");
		continue;
		}
            if (csize[it] < 0)
	       csize[it] = -itemp;
	    else
	       csize[it] = itemp;
            break;
            }
         fput(it, 3);
	 
	 /*
	  * finally return.
	  */
	  
         return(FB_AOK);
      }

/* 
 *  gettitle - get the information on conversion: output file name
 */
 
   static gettitle()
      {
         fb_move(2, 1); fb_clrtobot(); fb_printw("Miscellaneous Information");
	 fb_move(12,5); fb_clrtoeol(); fb_printw("Output File:");
	 fb_move(12,20); fb_printw(FB_FSTRING, fname);
	 while (changetitle() != FB_END)
	    ;
      }

/* 
 *  changetitle - ask about changing title, accept fb_input for change 
 */
 
   static changetitle()
      {
         char temp[FB_MAXLINE], dname[FB_MAXNAME];
	 int st;
	 
         if (fb_mustbe('y', "Any Information Change (y/<RET>)",
               cdb_t_lines, 1) != FB_AOK)
	    return(FB_END);
	 fb_fmessage("Enter value, <CTL>-X=Skip, <RET>=Default");
	 fb_basename(dname, cdb_db->dbase);
	 
	 /*
	  *  get fname 
	  */
	 for (;;){
	    st = fb_input(12,20,59,0,FB_ALPHA,temp, FB_ECHO, FB_NOEND, FB_CONFIRM);
	    if (st != FB_ABORT){
	       if (st == FB_DEFAULT)
		  fb_basename(temp, DCNV);
	       strcpy(fname, fb_trim(temp));
	       }
	    fb_move(12,20); fb_clrtoeol(); fb_printw(FB_FSTRING, fname);
	    if (!equal(fname, dname))
	       break;
	    fb_screrr("Target and source databases must be distinct.");
	    }
	 return(FB_NOEND);		/* not FB_END */
      }

/* 
 *  fput - fb_put an entire item to a constant screen location 
 */
 
   static fput(i, pos)
      int i, pos;
      
      {
         int row, v;
         
         if ((row = (i % 10)) == 0)
            row = 10;
         row = row * 2 + 2;
         fb_move(row, col[pos]);
         switch (pos){
            case 0: fb_printw("%3d)", i); break;
            case 1: fb_printw("%-10s", itemlist[i]->id); break;
	    case 2: fb_printw(FB_FCHAR, itemlist[i]->type); break;
	    case 3:
	       if (csize[i] < 0)
	          v = -csize[i];
	       else
	          v = csize[i];
	       fb_printw("%6d", v);
	       fb_move(row, col[pos+1]); fb_clrtoeol();
	       if (csize[i] < 0)
		  fb_printw("new");
	       else if (csize[i] > itemlist[i]->size)
		  fb_printw("expanded");
	       else if (csize[i] < itemlist[i]->size)
		  fb_printw("truncated");
	       else
	          fb_printw("<image>");
            }
      }

/* 
 *  newdictc - printout a new dictionary list of itemlist 
 */
 
   static newdictc(p)
      char *p;
      
      {
         int i;
	 fb_field *fp;
	 FILE *fs, *fb_mustfopen();
	 
	 fs = fb_mustfopen(p, "w");
	 for (i = 1; i <= lastitem; i++){
	    fp = itemlist[i];
	    fb_underscore(fp->id, 1);
	    fprintf(fs, "%s %c %d\n", fp->id, fp->type, abs(csize[i]));
	    }
	 fprintf(fs, "%%\n");
	 if (strlen(fb_trim(fname)) > 0)
	    fprintf(fs, "%s\n", fname);
	 fclose(fs);
      }
      
/* 
 *  read_idictp - read in an existing idctp file for editing 
 */
 
   static read_idictp(fs)
      FILE *fs;
      
      {
         char line[FB_MAXLINE], word[FB_MAXLINE];
	 fb_field  *fp;
	 int p, q, v;
	 
	 q = 0;
	 while (fgets(line, FB_MAXLINE, fs) != NULL){
	    if (line[0] == '%')
	       break;
	    q++;
	    p = fb_getword(line, 1, word);
	    fb_underscore(word, 0);
	    if ((fp = fb_findfield(word, hp)) != NULL){
	       itemlist[q] = fp;
	       csize[q] = fp->size;
	       }
	    else {
	       fp = (fb_field *) fb_malloc(sizeof(fb_field));
	       fp->id = (char *) fb_malloc(strlen(word) + 1);
	       fp->size = 10;
	       fp->type = 'a';
	       strcpy(fp->id, word);
	       itemlist[q] = fp;
	       csize[q] = -10;
	       }
	    p = fb_getword(line, p, word);
	    if (csize[q] < 0)
	       itemlist[q]->type = word[0];
	    p = fb_getword(line, p, word);
	    v = atoi(word);
	    if (csize[q] < 0)
	       csize[q] = -v;
	    else
	       csize[q] = v;
	    }
	 lastitem = q;
	 if (line[0] != '%')
	    fb_xerror(FB_BAD_DICT, hp->idict, "no terminator");
         fb_basename(fname, DCNV);
	 if (fgets(line, FB_MAXLINE, fs) != NULL){
	    line[strlen(line)-1] = NULL;
	    strcpy(fname, fb_trim(line));
	    }
      }

/* 
 * headers - print header at top of screen 
 */
 
   static headers()
      {
         fb_move(2, 1); fb_clrtoeol();
	 fb_move(3, 1); fb_clrtoeol();
         fb_move(2,col[1]);   fb_printw("Field");
         fb_move(3,col[1]);   fb_printw("=====");
	 fb_move(2,col[2]-1); fb_printw("Type");
	 fb_move(3,col[2]-1); fb_printw("====");
	 fb_move(2,col[3]+2); fb_printw("Size");
	 fb_move(3,col[3]+2); fb_printw("====");
         fb_move(2,col[4]-3); fb_printw("Conversion Status");
         fb_move(3,col[4]-3); fb_printw("=================");
      }
      
/* 
 *  usage message 
 */
   usage()
      {
         fb_xerror(FB_MESSAGE, "usage: dbdcnv [-d dbase] [-i index]", NIL);
      }
