/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbdprt.c,v 9.1 2001/02/16 19:44:55 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)dbdprt.c	8.1 8/19/93 FB";
#endif

/* 
 *  dbdprt.c - screen editor for printout (idictp) file 
 */

#include <fb.h>
#include <fb_vars.h>
#include <dbd.h>
#include <pgen.h>

#define HLP_DBDPRT	"dbdprt.hlp"

extern short int cdb_locklevel;
extern short int cdb_pgencols;
extern short int cdb_askgen;
extern short int cdb_choosefield;
extern char *cdb_PGEN;
extern char *cdb_T_TOTAL;
extern char *cdb_T_BREAK;
extern char *cdb_T_SBREAK;

static int col[8] = {1, 6, 20, 35, 50, 60, 70};
static int dot = 0;			/* current fb_field */
static int lastitem = 0;		/* last list item */
static int detail = 1;			/* to mark detail of report */

struct item {				/* editable items */
   fb_field *fp;
   char tot, brk;
   int lev;
   } ;

struct tv {				/* editable title structure */
   char tname[FB_MAXLINE];
   char fname[FB_MAXNAME];
   int sp;
   int psize;
   } ;

/*
 *  Assumption is that not more than FB_MAXKEEP fields will be printed.
 *    this is realistic since FB_MAXKEEP is 1000.
 *
 *  itemlist is a list of pointers to editable items.
 *  info is a tv structure that holds title info. it is edited also.
 */
       
static struct item *itemlist[FB_MAXKEEP] = { NULL };
static struct tv info = { "", "cdbprt", 1, 80 };	/* defaults */
static fb_database *hp;

static edit_idictp();
static fillscreen();
static escreen();
static void fdelete();
static void finsert();
static fadder();
static newitem();
static eitem();
static gettitle();
static changetitle();
static fput();
static wput();
static newdictp();
static read_idictp();
static void headers();

/* 
 *  dbdprt - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
      
      {
         int st, i;
	 char idictp[FB_MAXNAME];

	 cdb_locklevel = -1;
         fb_getargs(argc, argv, FB_ALLOCDB);
	 info.psize = cdb_pgencols;
	 hp = cdb_db;
	 fb_initdbd(hp);
         if (fb_getd_dict(hp) == FB_ERROR)
            fb_xerror(FB_BAD_DICT, hp->ddict, NIL);
         else
            for (i = 0; i < hp->nfields; i++)
               fb_nounders(cdb_keymap[i]);
	 fb_scrhdr(hp, "Define Printout");
         headers();
	 sprintf(idictp, "%sp", hp->idict);
         st = edit_idictp(idictp);
         if (st == FB_END){
	    gettitle();
            newdictp(idictp);
	    if (cdb_askgen)
	       fb_chainto("Generate Printout", cdb_PGEN, argv);
	    }
         fb_ender();
      }

/* 
 *  edit_idictp - edit an index.idictp or allow creation of one
 */
 
   static edit_idictp(p)
      char *p;
      
      {
         int st;
	 FILE *fs;

         if ((fs = fopen(p, "r")) != NULL)
	    read_idictp(fs);
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
 *  fillscreen - fill a screen with fields 
 */
 
   static fillscreen()
      {
         int j, i, p;
         
         fb_move(4, 1); fb_clrtobot();
         j = dot + 9;
         for (i = dot; i <= j && i <= lastitem; i++)
            for (p = 0; p <= 6; ++p)
               fput(i, p);
	 wput(dot);		/* fb_put widths */
	 fb_infoline();
         fb_cx_set_viewpage(dot);
      }
            
/* 
 * escreen - set up a screen, accept commands for screen control 
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
                     fb_screrr("Printout Dictionary File Unchanged");
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
	          fb_fhelp(HLP_DBDPRT);
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
 *  fdelete - fb_delete a fb_field 
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
	 fb_free((char *) itemlist[n]);
         for (; n < lastitem; n++)
            itemlist[n] = itemlist[n + 1];
         lastitem--;
      }

/* 
 *  finsert - insert a defaulted fb_field before a given fb_field 
 */
 
   static void finsert()
      {
         int st, n, rn;
	 struct item *ip;

	 fb_fmessage("Insert before what number (#,-) ? ");
         st = fb_input(cdb_t_lines, 36, 3, 0, FB_INTEGER, (char *) &n,
            FB_ECHO, FB_OKEND, FB_CONFIRM);
         if (st == FB_END || st == FB_ABORT)
            return;
	 else if (n < 1 || n > lastitem)
	    return;
         ip = (struct item *) fb_malloc(sizeof(struct item));
	 newitem(ip);
	 ip->fp = cdb_kp[0];		/* default to first fb_field */
	 rn = n;
         for (n = lastitem; n >= rn; n--)		/* go backwards */
            itemlist[n+1] = itemlist[n];
	 itemlist[rn] = ip;
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
         for (;lastitem < FB_MAXKEEP;){
            itemlist[++lastitem] = 
	    	(struct item *) fb_malloc(sizeof(struct item));
            if (lastitem >= dot + 10){
               dot += 10;
               fb_move(4, 1); fb_clrtobot(); fb_infoline();
               }
	    newitem(itemlist[lastitem]);
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
 *  newitem - make sure new item is well defined 
 */
 
   static newitem(ip)
      struct item *ip;
         {
	    ip->fp = NULL;	/* fb_field pointer default is number one */
	    ip->tot = FB_BLANK;
	    ip->brk = FB_BLANK;
	    ip->lev = -1;
	 }
	 
/*
 *  eitem - edit an entire item 
 */
 
   static eitem(it, new)
      int it, new;

      {
         int row, i, st, itemp;
         char buf[FB_MAXLINE], *fb_trim(), temp[FB_MAXLINE], msg[FB_MAXLINE];
	 char typ;
	 fb_field *f;

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
	       headers();
	       }
            if (st == FB_END && new == 1){
               fb_move(row, 1); 
               fb_clrtoeol();
               return(FB_END);
               }
            else if (st == FB_END)
               st = FB_DEFAULT;
            if (st == FB_DEFAULT || (st == FB_ABORT && new == 1)){
	       if ((it - 1) < hp->nfields)
	          itemlist[it]->fp = cdb_kp[it - 1];
	       else
	          itemlist[it]->fp = cdb_kp[0];
	       break;
	       }
            else if (st == FB_ABORT)
               break;
            else if (st == FB_AOK)
               fb_trim(temp);
	    if ((f = fb_findfield(temp, hp)) != NULL){
	       itemlist[it]->fp = f;
	       break;
	       }
            fb_screrr("Field Name not in current DataBase");
            }
	 if (cdb_choosefield){
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
	 wput(it);		/* fb_put widths */
	 
	 /*
	  *  get tot  && brk 
	  */
	  
         typ = itemlist[it]->fp->type;
	 if (FB_OFNUMERIC(typ) || typ == FB_FORMULA)	/* formulas can total also */
	    i = 4;	/* get total and break */
	 else {
	    i = 5;	/* just get break */
	    itemlist[it]->tot = FB_BLANK;
	    }
         for (; i <= 5; i++){			/* 4=total 5=break columns */
	    for(;;){
	       if (i == 4)
                  strcpy(msg, "y=Yes, <RETURN>=No, <CTL>-X=Skip");
	       else
                  strcpy(msg, "y=Yes, s=Simple, <RETURN>=No, <CTL>-X=Skip");
	       fb_fmessage(msg);
	       st = fb_input(row,col[i],3,0,FB_ALPHA,buf,FB_ECHO,FB_NOEND,FB_CONFIRM);
	       if (st == FB_DEFAULT || (st == FB_ABORT && new == 1))
		  buf[0] = FB_BLANK;
	       else if (st == FB_ABORT)
		  break;
	       else if (buf[0] == 'n')
	          buf[0] = FB_BLANK;
	       if (i == 4 && buf[0] != FB_BLANK && buf[0] != 'y')
		  continue;
	       if (i == 5 && buf[0] != FB_BLANK && 
	             buf[0] != 'y' && buf[0] != 's')
		  continue;
	       if (i == 4)
	          itemlist[it]->tot = buf[0];
	       else
	          itemlist[it]->brk = buf[0];
	       break;
	       }
	    fput(it, i);
	    }
	    
	 /*
	  *  get lev 
	  */
	  
         for(;;){
	    if (itemlist[it]->brk != 'y' && itemlist[it]->brk != 's')
	       break;
	    fb_fmessage("Enter Level #, <RET>, or <CTL>-X=Skip");
            st = fb_input(row, col[6], 2, 0,FB_INTEGER,(char *) &itemp,
	          FB_ECHO,FB_NOEND,FB_CONFIRM);
	    if (st == FB_DEFAULT || (st == FB_ABORT && new == 1))
	       itemp = 1;
            else if (st == FB_ABORT)
               break;
	    else if (itemp <= 0)
	       continue;
            itemlist[it]->lev = itemp;
            break;
            }
         fput(it, 6);
	 
	 /*
	  *  finally return
	  */
	 
         return(FB_AOK);
      }

/* 
 *  gettitle - get the information on printout: 
 *    spacing, title, width, output name 
 */
       
   static gettitle()
      {
         fb_move(2, 1); fb_clrtobot(); fb_infoline();
	 fb_move(4, 5); fb_printw("Miscellaneous Information");
	 fb_move(10,5); fb_clrtoeol(); fb_printw("Title:");
	 fb_move(12,5); fb_clrtoeol(); fb_printw("Output File:");
	 fb_move(14,5); fb_clrtoeol(); fb_printw("Spacing:");
	 fb_move(16,5); fb_clrtoeol(); fb_printw("Columns/Page:");
	 fb_move(18,5); fb_clrtoeol(); fb_printw("Print Detail? ");
	 fb_move(10,20); fb_printw(FB_FSTRING, info.tname);
	 fb_move(12,20); fb_printw(FB_FSTRING, info.fname);
	 fb_move(14,20); fb_printw("%d", info.sp);
	 fb_move(16,20); fb_printw("%d", info.psize);
	 fb_move(18,20); 
	 if (detail)
	    fb_printw("Yes");
	 else
	    fb_printw("No");
	 while (changetitle() != FB_END)
	    ;
      }

/* 
 *  changetitle - ask about changing title, accept fb_input for change 
 */
 
   static changetitle()
      {
         char temp[FB_MAXLINE];
	 int itemp, st;
	 
         if (fb_mustbe('y', "Any Information Change (y/<RET>)",
               cdb_t_lines, 1) != FB_AOK)
	    return(FB_END);
	 fb_fmessage("Enter value, <CTL>-X=Skip, <RET>=Default");
	 
         /* 
	  *  get tname 
	  */
	  
	 st = fb_input(10,20,59,0,FB_ALPHA,temp, FB_ECHO, FB_NOEND, FB_CONFIRM);
	 if (st == FB_DEFAULT || st == FB_AOK)
	    strcpy(info.tname, temp);
	 fb_move(10,20); fb_clrtoeol(); fb_printw(FB_FSTRING, info.tname);
	 
	 /* 
	  *  get fname 
	  */
	  
	 st = fb_input(12,20,59,0,FB_ALPHA,temp, FB_ECHO, FB_NOEND, FB_CONFIRM);
	 if (st != FB_ABORT){
	    if (st == FB_DEFAULT)
	       fb_rootname(temp, DPRT);		/* DPRT is set for initpr */
	    strcpy(info.fname, fb_trim(temp));
	    }
	 fb_move(12,20); fb_clrtoeol(); fb_printw(FB_FSTRING, info.fname);
	 
	 /* 
	  *  get sp 
	  */
	  
	 st = fb_input(14,20,2,0,FB_INTEGER, (char *) &itemp,
	    FB_ECHO, FB_NOEND, FB_CONFIRM);
	 if (st != FB_ABORT){
	    if (st == FB_DEFAULT)
	       itemp = 1;
	    info.sp = itemp;
	    }
	 fb_move(14,20); fb_clrtoeol(); fb_printw("%d", info.sp);
	 
	 /* 
	  *  get psize 
	  */
	  
	 st = fb_input(16,20,3,0,FB_INTEGER, (char *) &itemp,
	 FB_ECHO, FB_NOEND, FB_CONFIRM);
	 if (st != FB_ABORT){
	    if (st == FB_DEFAULT)
	       itemp = cdb_pgencols;
	    info.psize = itemp;
	    }
	 fb_move(16,20); fb_clrtoeol(); fb_printw("%d", info.psize);

	 /*
	  *  get detail 
	  */
	 
	 fb_move(18,20); fb_clrtoeol();
         if (fb_mustbe('n', "<RET>=yes, n=no", 18, 20) == FB_AOK)
	    detail = 0;
	 else
	    detail = 1;
	 fb_move(18,20); fb_clrtoeol();
	 if (detail)
	    fb_printw("Yes");
	 else
	    fb_printw("No");
	    
	 /*
	  *  finally return
	  */
	  
	 return(FB_NOEND);		/* not FB_END */
      }

/* 
 *  fput - fb_put an entire fb_field to a constant screen location 
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
            case 1: fb_printw("%-10s", itemlist[i]->fp->id); break;
	    case 2: case 3:	/* these are formulated on the fly */
	       break;
            case 4: 
	       if (itemlist[i]->tot == 'y')
	          fb_printw("yes");
	       else
	          fb_printw("   ");
	       break;
            case 5:
	       if (itemlist[i]->brk == 'y')
	          fb_printw("yes    ");
	       else if (itemlist[i]->brk == 's')
	          fb_printw("yes (s)");
	       else
	          fb_printw("       ");
	       break;
            case 6: 
               if ((itemlist[i]->brk == 'y' || itemlist[i]->brk == 's') &&
	            itemlist[i]->lev != -1)
                  fb_printw("%2d", itemlist[i]->lev);
	       else
	          fb_printw("  ");
               break;
            }
      }

/* 
 *  wput - fb_put out widths from p on down of fields on fb_page 
 */
 
   static wput(p)
      int p;
      
      {
         int j, i, row, len, w, plines;
         
	 plines = 1;
	 for (len = 1, i = 1; i < p && i <= lastitem; i++){
	    w = MAX(itemlist[i]->fp->size, strlen(itemlist[i]->fp->id));
	    if (len + w + 1 > info.psize){
	       plines++;
	       len = 0;
	       }
	    len += (w + 1);
	    }
         j = dot + 9;
         for (i = p; i <= j && i <= lastitem; i++){
	    if ((row = (i % 10)) == 0)
	       row = 10;
	    row = row * 2 + 2;
            fb_move(row, col[2]);
	    w = MAX(itemlist[i]->fp->size, strlen(itemlist[i]->fp->id));
	    fb_printw("%3d", w);
	    if (len + w + 1 > info.psize){
	       plines++;
	       len = 0;
	       }
	    len += (w + 1);
	    fb_move(row, col[3]);
	    fb_printw("%3d ", len);
	    if (plines <= 2)
	       fb_printw("(%d)", plines);
	    else
	       fb_printw("(TOO LONG)");
	    }
      }

/* 
 *  newdictp - printout a new dictionary list of itemlist 
 */
 
   static newdictp(p)
      char *p;
      
      {
         int i;
	 struct item *it;
	 FILE *fs, *fb_mustfopen();
	 
	 for (i = 0; i < hp->nfields; i++)
	    fb_unders(cdb_keymap[i]);
	 fs = fb_mustfopen(p, "w");
	 for (i = 1; i <= lastitem; i++){
	    it = itemlist[i];
	    fprintf(fs, "%s ", it->fp->id);
	    if (it->tot == 'y')
	       fprintf(fs, "%s ", cdb_T_TOTAL);
	    if (it->brk == 'y'){
	       fprintf(fs, "%s ", cdb_T_BREAK);
	       if (it->lev != -1)
	          fprintf(fs, "%d ", it->lev);
	       }
	    else if (it->brk == 's'){
	       fprintf(fs, "%s ", cdb_T_SBREAK);
	       if (it->lev != -1)
	          fprintf(fs, "%d ", it->lev);
	       }
	    fprintf(fs, "\n");
	    }
	 if (!detail)
	    info.psize = -info.psize;
	 fprintf(fs, "%%%d %d\n%s\n", info.sp, info.psize, fb_trim(info.tname));
	 fb_trim(info.fname);
	 if (strlen(info.fname) > 0)
	    fprintf(fs, "%s\n", info.fname);
	 fclose(fs);
      }
      
/* 
 *  read_idictp - read in an existing idctp file for editing 
 */

   static read_idictp(fs)
      FILE *fs;
      
      {
         char line[FB_MAXLINE], word[FB_MAXLINE];
	 struct item *it;
	 int p, c;
	 
	 lastitem = 0;
	 while (fgets(line, FB_MAXLINE, fs) != NULL){
	    if (line[0] == '%')
	       break;
	    it = (struct item *) fb_malloc(sizeof(struct item));
	    newitem(it);
	    itemlist[++lastitem] = it;
	    for (c = 1, p = 1; (p = fb_getword(line, p, word)) != 0; ++c){
	       if (strcmp(word, cdb_T_TOTAL) == 0)
	          it->tot = 'y';
	       else if (strcmp(word, cdb_T_BREAK) == 0)
	          it->brk = 'y';
	       else if (strcmp(word, cdb_T_SBREAK) == 0)
	          it->brk = 's';
	       else if (isdigit(word[0]) && c != 1)
	          it->lev = atoi(word);
	       else {
                  fb_underscore(word, 0);		/* take out _ before search */
	          if ((it->fp = fb_findfield(word, hp)) == NULL)
		     fb_xerror(FB_MESSAGE, word, SYSMSG[S_NOT_FOUND]);
		  }
	       }
	    }
	 if (line[0] != '%')
	    fb_xerror(FB_BAD_DICT, hp->idict, "no terminator");
	 if (isdigit(line[1]))
	    sscanf(line+1, "%d %d", &(info.sp), &(info.psize));
	 if (info.psize < 0){
	    detail = 0;
	    info.psize = -info.psize;
	    }
	 info.tname[0] = NULL;
	 fb_basename(info.fname, DPRT);
	 if (fgets(line, FB_MAXLINE, fs) != NULL){
	    line[strlen(line)-1] = NULL;
	    strcpy(info.tname, fb_trim(line));
	    if (fgets(line, FB_MAXLINE, fs) != NULL){
	       line[strlen(line)-1] = NULL;
	       strcpy(info.fname, fb_trim(line));
	       }
	    }
      }

/* 
 *  headers - print header at top of screen 
 */
 
   static void headers()
      {
	 fb_scrstat("Define Printout");
         fb_move(2, 1); fb_clrtoeol();
         fb_move(3, 1); fb_clrtoeol();
         fb_move(2,col[1]);   fb_printw("Field");
         fb_move(3,col[1]);   fb_printw("=====");
	 fb_move(2,col[2]-4); fb_printw("Field Width");
	 fb_move(3,col[2]-4); fb_printw("===========");
	 fb_move(2,col[3]-1); fb_printw("Line Length");
	 fb_move(3,col[3]-1); fb_printw("===========");
         fb_move(2,col[4]-1); fb_printw("Total?");
         fb_move(3,col[4]-1); fb_printw("======");
         fb_move(2,col[5]-1); fb_printw("Break?");
         fb_move(3,col[5]-1); fb_printw("======");
         fb_move(2,col[6]-1); fb_printw("Level");
         fb_move(3,col[6]-1); fb_printw("=====");
      }
      
/* 
 *  usage message
 *
 *
 * usage()
 *    {
 *       fb_xerror(FB_MESSAGE, "usage: dbdprt [-d dbase] [-i index]", "");
 *    }
 */
