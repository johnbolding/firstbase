/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbdbas.c,v 9.1 2001/02/16 19:41:42 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)dbdbas.c	8.5 05 Apr 1996 FB";
#endif

/* 
 *  dbdbas.c - database dictionary editor (full screen)
 */

#include <fb.h>
#include <fb_vars.h>
#include <dbd.h>

#define HLP_DBDBAS 	"dbdbas.hlp"

extern char *cdb_T_TIME;
extern char *cdb_T_MTIME;
extern char *cdb_T_INCR;
extern char *cdb_T_PREV;
extern char *cdb_T_NPREV;
extern char *cdb_T_USER;
extern char *cdb_T_AUTOINCR;
extern char *cdb_T_PLUS;
extern char *cdb_T_MINUS;
extern char *cdb_T_DATE;

static int col[8] = {1, 6, 17, 19, 26, 43, 68, 75};	/* screen positions */

extern short int cdb_returnerror;

struct item {
   fb_field *fp;						/* fb_field pointer */
   struct item *prev;					/* previous item */
   struct item *next;					/* next item */
   } ;

static struct item 
   *head = NULL,					/* head of list */
   *tail = NULL,					/* tail of list */
   *pitem(),						/* function */
   *makeitem();						/* function */
   
static int dot = 0;					/* current fb_field */
static int restricted = 0;				/* restricted flag */
static fb_database *hp;

static editdict();
static fillscreen();
static escreen();
static void fdelete();
static void finsert();
static fadder();
static newitem();
static efield();
static gethelp();
static getaddchoice();
static void xgetauto();
static void getrange();
static void getmacro();
static editlink();
static fput();
static void insert();
static void ddelete();
static void update();
static headers();
static is_restricted();
static usage();
static tokp();
static tolist();
static initlist();

extern short int cdb_locklevel;
extern short int cdb_max_file_name;
extern short int cdb_returnerror;

/* 
 *  dbdbas - main driver for dbdbas.c
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
      
      {
         int st;

	 cdb_locklevel = -1;
         fb_getargs(argc, argv, FB_ALLOCDB);
	 hp = cdb_db;
	 fb_initdbd(hp);
	 hp->dindex[0] = NULL;
         initlist();
	 if (access(hp->dbase, 0) != FB_ERROR)
            restricted = 1;
         cdb_returnerror = 1;
         if (fb_getd_dict(hp) != FB_ERROR)
	    tolist();
	 fb_scrhdr(hp, "Define Database");
         headers();
         st = editdict();
         if (st == FB_END){
            update(hp->ddict);
	    /* fb_chainto("Run Dbedit", EDIT, argv); */
            }
         fb_ender();
      }
      
/* 
 *  editdict - edit the link list of items.
 *     - this list will later be used to make the ddict file
 */
 
   static editdict()
      {
         int st;

         fillscreen();
         while ((st = escreen()) != FB_END)
            if (st == FB_ABORT)
               break;
            else if (st >= dot && st <= dot + 9 && st <= hp->nfields){
               fb_cx_push_env("6D", CX_KEY_SELECT, NIL);
               efield(st, 0);	/* value returned is ignored. why? */
               fb_cx_pop_env();
               }
            else if (st >= 1 && st <= hp->nfields){
               dot = fb_onpage(st);
               fillscreen();
               }
            else
               fb_screrr("Invalid Input");
         return(st);
      }

/* 
 *  fillscreen - fill a screen with fields.
 */
 
   static fillscreen()
      {
         int j, i, p;
	 struct item *it;
         
         fb_move(4,1); fb_clrtobot();
	 it = pitem(dot);
         j = dot + 9;
         for (i = dot; i <= j && it != tail; i++, it = it->next)
            for (p = 0; p <= 7; ++p)
               fput(it->fp, i, p);
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
            if (hp->nfields > 10)
               fb_cx_add_buttons("Ge");
            fb_cx_add_buttons("XH");
            st = fb_anychange(com);
            fb_cx_pop_env();
            switch (st) {
               case FB_ABORT:
	          if (fb_mustbe('y', "Really Quit? (y/n) ",
                        cdb_t_lines, 1) == FB_AOK){
                     fb_xerror(FB_ABORT_ERROR, 
		        "Database Dictionary File Unchanged", NIL);
		     return(st);
		     }
		  break;
               case ADDMODE:
                  fadder();
                  break;
               case DELMODE:
		  if (hp->nfields > 0)
		     fdelete();
		  else
		     continue;
                  break;
               case FB_END:
                  return(st);
	       case INSERTMODE:
	          if (!is_restricted()){
		     if (hp->nfields > 0)
			finsert();
		     else
			continue;
		     }
		  break;
               case PAGEF:
                  dot = dot + 10;
                  if (dot > hp->nfields)
                     dot = 1;
                  break;
               case PAGEB:
                  dot = dot - 10;
                  if (dot < 1)
                     dot = fb_onpage(hp->nfields);
                  break;
	       case '?':	/* CHAR_QMARK */
	          fb_move(2, 1); fb_clrtobot();
		  tokp();
	          fb_help(com, hp);
		  headers();
		  break;
	       case FB_QHELP:
	          fb_move(2, 1); fb_clrtobot();
	          fb_fhelp(HLP_DBDBAS);
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
 *  fdelete - delete a dictionary entry 
 */
 
   static void fdelete()
      {
         int st, n;
         struct item *p;
	 
         fb_fmessage("Delete what number ? (#,-) ");
         st = fb_input(cdb_t_lines,28,3,0, FB_INTEGER, (char *) &n, FB_ECHO,
            FB_OKEND, FB_CONFIRM);
         if (st == FB_END || st == FB_ABORT)
            return;
	 else if (n < 1 || n > hp->nfields){
	    fb_screrr("delete request out of range");
	    return;
	    }
	 p = pitem(n);
	 if (restricted && p->fp->type != FB_LINK && p->fp->type != FB_FORMULA){
	    is_restricted();
	    return;
	    }
         ddelete(p);
	 hp->nfields--;
	 if (dot > hp->nfields)
	    dot = fb_onpage(hp->nfields);
      }

/* 
 *  finsert - insert a defaulted dict entry before an existing entry.
 */
 
   static void finsert()
      {
         int st, n;
	 struct item *it;

	 fb_fmessage("Insert before what number (#,-) ? ");
         st = fb_input(cdb_t_lines,36,3,0, FB_INTEGER, (char *) &n, FB_ECHO,
            FB_OKEND, FB_CONFIRM);
         if (st == FB_END || st == FB_ABORT)
            return;
	 else if (n < 1 || n > hp->nfields){
	    fb_screrr("add request out of range");
	    return;
	    }
	 it = makeitem();
	 newitem(it);
	 it->fp->id = (char *) fb_malloc(strlen("Inserted") + 1);
	 strcpy(it->fp->id, "Inserted");
	 insert(it, pitem(n));
         hp->nfields++;
      }

/* 
 *  fadder - go into auto add mode.
 */
 
   static fadder()
      {
         struct item *it;
         int st;
	 
         dot = fb_onpage(hp->nfields);
	 if (hp->nfields < (dot + 9))		/* ie, still on this page */
            fillscreen();
         for (;;){
            if (hp->nfields >= dot + 9){
               dot += 10;
               fb_move(4, 1); fb_clrtobot(), fb_infoline();
               }
	    hp->nfields++;
	    it = makeitem();
	    newitem(it);
	    insert(it, tail);
            fb_cx_push_env("6D", CX_KEY_SELECT, NIL);
            st = efield(hp->nfields, 1);
            fb_cx_pop_env();
            if (st == FB_END){
	       ddelete(it);
	       hp->nfields--;
               break;
	       }
            }
      }

/* 
 * newitem - make sure new item and fb_field are well defined.
 */
 
   static newitem(it)
      struct item *it;
      
      {
         fb_autoindex *fb_ixalloc();

	 it->fp = (fb_field *) fb_malloc(sizeof(fb_field));
         it->fp->id = NULL;
	 if (!restricted)
	    it->fp->type = 'a';			/* FB_ALPHA is default */
	 else
	    it->fp->type = 'L';			/* LINT=restricted default */
	 it->fp->size = 10;			/* 10 is default */
	 it->fp->comment = NULL;
	 it->fp->idefault = NULL;
	 it->fp->help = NULL;
	 it->fp->prev = NULL;
	 it->fp->range = NULL;
	 it->fp->a_template = NULL;
	 it->fp->incr = 0;
	 it->fp->comloc = FB_BLANK;
	 it->fp->lock = 'n';
	 it->fp->choiceflag = FB_BLANK;
	 it->fp->dflink = NULL;
	 it->fp->xflink = NULL;
	 it->fp->mode = NULL;
	 it->fp->aid = fb_ixalloc();
	 it->fp->f_macro = NULL;
      }
	 
/* 
 *  efield - edit an entire dictionary item (fb_field).
 */
 
   static efield(fld, new)
      int fld, new;

      {
         int row, i, st, itemp, j, e_link;
         char buf[FB_MAXLINE], *fb_trim(), temp[FB_MAXLINE], msg[FB_MAXLINE];
	 struct item *it, *tt;
	 long ltemp;

	 it = pitem(fld);
         fb_move(cdb_t_lines, 1); fb_clrtoeol();
         if ((row = (fld % 10)) == 0)
            row = 10;
         row = row * 2 + 2;
         if (new == 1)
            fput(it->fp, fld, 0);
         strcpy(msg, "Enter Value, <RET>=Default, <CTL>-X=Skip");
	 
	 /*
	  *  get id (fieldname)
	  */
	  
         for (;;){
	    if (new)
               fb_fmessage("Enter Id, <RET>=Default, -=End");
	    else
               fb_fmessage("Enter Id, <RET>=Default, <CTL>-X=Skip");
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
               strcpy(temp, "Field");
               sprintf(buf, "%d", fld);
               strcat(temp, buf);
               }
            else if (st == FB_ABORT)
               break;
            else if (st == FB_AOK){		/* check for alphanumeric */
	       fb_trim(temp);
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
	       }
            for (tt = head->next; tt != tail; tt = tt->next)
               if (tt->fp != it->fp)
                  if (strcmp(temp, tt->fp->id) == 0)
                     break;
            if (tt == tail){	/* then must be unique id */
	       if (it->fp->id != NULL)
	          fb_free(it->fp->id);
	       it->fp->id = (char *) fb_malloc(strlen(temp) + 1);
               strcpy(it->fp->id, temp);
               break;
	       }
            fb_screrr("Field Id already used");
            }
         fput(it->fp, fld, 1);
	 
         /* 
	  *  get type
	  */
	  
         for(; !restricted || new;){
	    fb_fmessage(msg);
            st = fb_input(row, col[2], 1, 0, FB_ALPHA, buf, FB_ECHO, FB_NOEND, FB_CONFIRM);
            if (st == FB_DEFAULT || (st == FB_ABORT && new == 1)){
	       if (!restricted)
                  buf[0] = 'a';
	       else
                  buf[0] = 'L';
	       }
            else if (st == FB_ABORT)
               break;
	    if (restricted && buf[0] != I_FORMULA && buf[0] != I_LINK){
	       is_restricted();
	       continue;
	       }
            switch(buf[0]){
	       case I_FORMULA:
	       case I_LINK:
	          if (fld == 1){
		     fb_screrr("First field must NOT be a FORMULA/LINK");
		     st = FB_ERROR;
		     break;
		     }
               case I_ALPHA: case I_NUMERIC: case I_DATE: case I_DOLLARS:
               case I_STRICTALPHA: case I_FLOAT:
	       case I_CHOICE: case I_POS_NUM: case I_UPPERCASE:
	       case I_CCHOICE:
	       case I_BINARY:
	       case I_EXCHOICE:
                  st = FB_AOK;
                  break;
	       /*
	        * case I_TIME:
	        * case I_MTIME:
	        * case I_YESNO:
	        *   fb_screrr("Unimplemented Type");
		*   st = FB_ERROR;
		*   break;
		*/
               default:
                  fb_screrr("Invalid Type: use [aAcCdfFnNU$]");
                  st = FB_ERROR;
		  break;
               }
            if (st == FB_AOK){
               it->fp->type = buf[0];
               break;
               }
            }
         fput(it->fp, fld, 2);
	 
	 /*
	  *  get size
	  */
	  
         for( ;!restricted || new || (restricted && it->fp->type == FB_FORMULA);){
	    fb_fmessage(msg);
            if (it->fp->type == FB_DATE){
               it->fp->size = 6;
               break;
               }
            else if (it->fp->type == FB_LINK){
               it->fp->size = 0;
               break;
               }
            st = fb_input(row, col[3], 6, 0, FB_INTEGER,
                 (char *) &itemp, FB_ECHO, FB_NOEND, FB_CONFIRM);
            if (st == FB_DEFAULT || (st == FB_ABORT && new == 1))
               itemp = 10;
            if (st == FB_ABORT)
               break;
            else if (itemp <= 0)
               fb_screrr("Size must be POSITIVE number");
	    else if (itemp >= FB_NUMLENGTH && it->fp->type != FB_ALPHA &&
	                           it->fp->type != FB_STRICTALPHA &&
	                           it->fp->type != FB_UPPERCASE &&
				   it->fp->type != FB_CHOICE)
		fb_screrr("Must be less than 25 to behave numerically");
	    else if (fld == 1 && itemp > FB_SCREENFIELD)	/* first fb_field */
	       fb_screrr("First Field MUST fit on the screen (<=50)");
/*
*	    else if (it->fp->type == FB_CHOICE && itemp > 50)
*	       fb_screrr("Choice Field MUST fit on the screen (<=50)");
*/
            else {
               it->fp->size = itemp;
               break;
               }
            }
         fput(it->fp, fld, 3);
	 
	 /*
	  *  get idefault
	  */
	  
         for(;;){
            e_link = 0;
	    if (it->fp->type == FB_FORMULA){
	       i = 48; j = 0;
	       fb_bell();
	       fb_fmessage("Enter FORMULA...use Field #'s and [+-*/][C#]");
	       }
	    else if (it->fp->type == FB_LINK){
	       i = 48; j = 0;
	       fb_bell();
	       fb_fmessage("Enter LINK Address...Field!index!dbase!Field");
               if (new == 0)
                  e_link = 1;
	       }
	    else{
	       i = 15; j = 0;
	       fb_fmessage(msg);
	       }
            if (e_link){
               st = editlink(it->fp, fld);
	       fb_move(row, col[1]); fb_clrtoeol();
               for (i = 0; i <= 7; ++i)
                  fput(it->fp, fld, i);
               if (st == FB_AOK)
                  strcpy(buf, it->fp->idefault);
               }
            else
               st = fb_input(row, col[4], i, j, FB_ALPHA, buf, FB_ECHO,
                  FB_NOEND, FB_CONFIRM);
            if (st == FB_DEFAULT || (st == FB_ABORT && new == 1)){
	       if (it->fp->type == FB_FORMULA){
	          strcpy(buf, "C0:2");
		  st = FB_AOK;
		  }
	       else if (it->fp->type == FB_LINK){
	          strcpy(buf, "1!index!dbase!2");
		  st = FB_AOK;
		  }
	       else
                  it->fp->idefault = NULL;
	       }
            if (st == FB_AOK){
               if (new == 0 && it->fp->idefault != NULL)
                  fb_free(it->fp->idefault);
               fb_trim(buf);
               if (buf[0] == NULL)
                  strcpy(buf, "_");
	       else if (strcmp(buf, cdb_T_TIME) == 0){
	          it->fp->size = 5;
		  it->fp->type = 'a';
		  fput(it->fp, fld, 2);
		  fput(it->fp, fld, 3);
		  }
	       else if (strcmp(buf, cdb_T_MTIME) == 0){
	          it->fp->size = 8;
		  it->fp->type = 'a';
		  fput(it->fp, fld, 2);
		  fput(it->fp, fld, 3);
		  }
	       else if (strcmp(buf, cdb_T_INCR) == 0){
	          if (it->fp->type != FB_NUMERIC && it->fp->type != FB_POS_NUM){
		     fb_screrr("INCR requires a NUMERIC (n/N) field...");
		     continue;
		     }
		  }
	       if ((strcmp(buf, cdb_T_INCR) == 0) ||
	           (strcmp(buf, cdb_T_TIME) == 0) ||
	           (strcmp(buf, cdb_T_MTIME) == 0) ||
	           (strcmp(buf, cdb_T_PREV) == 0) ||
	           (strcmp(buf, cdb_T_NPREV) == 0) ||
	           (strcmp(buf, cdb_T_USER) == 0) ||
	           (strcmp(buf, cdb_T_AUTOINCR) == 0) ||
	           (strcmp(buf, cdb_T_PLUS) == 0) ||
	           (strcmp(buf, cdb_T_MINUS) == 0) ||
	           (strcmp(buf, cdb_T_DATE) == 0))
	          fb_screrr("** Meta-Default Recognized! **");
	       else if (it->fp->type != FB_FORMULA && 
	                it->fp->type != FB_LINK &&
                        strlen(buf) > it->fp->size){
		  fb_screrr("Default Size must be LESS or EQUAL to SIZE");
		  continue;
		  }
	       else if (it->fp->type == FB_FORMULA){
	          tokp();
	          if (fb_getformula(it->fp, buf, NIL, 1, hp) != FB_AOK)
		     continue;	/* failed formula parsing */
		  }
	       else if (it->fp->type == FB_LINK){
	          if (fb_parselink(buf, temp, temp, temp, temp, &ltemp) !=
                        FB_AOK){
                     fb_screrr("Link Address not in proper form.");
                     fb_free(it->fp->idefault);
                     it->fp->idefault = NULL;
		     continue;  /* failed formula parsing */
                     }
		  }
               it->fp->idefault = (char *) fb_malloc(strlen(buf) + 1);
               strcpy(it->fp->idefault, buf);
               }
	    if (it->fp->type == FB_LINK && it->fp->idefault == NULL)
	       continue;
            break;
            }
	 if (it->fp->type == FB_FORMULA || it->fp->type == FB_LINK){
	    fb_move(row, col[4]);
	    fb_clrtoeol();
	    if (new == 0){			/* ie, not new */
	       fput(it->fp, fld, 5);
	       fput(it->fp, fld, 6);
	       fput(it->fp, fld, 7);
	       }
	    }
         fput(it->fp, fld, 4);
	 
	 /*
	  *  get comment
	  */
	  
         for(;;){
	    fb_fmessage(msg);
	    if (it->fp->size >= FB_SCREENFIELD){
               if (it->fp->comment != NULL && !new)
                  fb_free(it->fp->comment);	/* was commented out -- ? */
	       it->fp->comment = NULL;
	       it->fp->comloc = FB_BLANK;
               fput(it->fp, fld, 5);	/* efectively clears old comments */
	       break;	/* no comment for xlong lines */
	       }
            st = fb_input(row, col[5], cdb_t_lines, 0, FB_ALPHA, buf,
               FB_ECHO, FB_NOEND, FB_CONFIRM);
            if (st == FB_DEFAULT || (st == FB_ABORT && new == 1)){
	       if (it->fp->comment != NULL)
	          fb_free(it->fp->comment);
               it->fp->comment = NULL;
               it->fp->comloc = FB_BLANK;
               fput(it->fp, fld, 6);
               break;
               }
            else if (st == FB_AOK){
               if (it->fp->comment != NULL)
                  fb_free(it->fp->comment); /* was commented out ?? */
               fb_trim(buf);
               if (buf[0] == NULL)
                  strcpy(buf, "_");		/* to show its a blank */
               it->fp->comment = (char *) fb_malloc(strlen(buf) + 1);
               strcpy(it->fp->comment, buf);
               }
	    if (it->fp->size <= FB_SCREENFIELD && it->fp->comment != NULL &&
                  (strlen(it->fp->comment) + it->fp->size > FB_SCREENFIELD))
               fb_screrr("Comment + Size must be LESS than 50");
            else
               break;
            }
         fput(it->fp, fld, 5);	/* fb_put comment */
	    
	 /*
	  *  get comloc (comment location)
	  */
	    
         if (it->fp->comment != NULL){
            for(;;){
	       fb_fmessage(msg);
               st = fb_input(row,col[6],1,0,FB_ALPHA, buf, FB_ECHO, FB_NOEND, FB_CONFIRM);
               if (st == FB_DEFAULT || (st == FB_ABORT && new == 1))
                  it->fp->comloc = 'b';
               else if (st == FB_AOK)
                  it->fp->comloc = buf[0];
               if (it->fp->comloc == 'a' || it->fp->comloc == 'b')
                  break;
               else
                  fb_screrr("Must use 'b' or <cr> for before, 'a' for after");
               }
            fput(it->fp, fld, 6);
            }
	    
	 /* 
	  *  get lock (fb_field editing lock)
	  */

         for(;;){
	    fb_fmessage(msg);
            st = fb_input(row, col[7], 3, 0, FB_ALPHA, buf, FB_ECHO, FB_NOEND, FB_CONFIRM);
            if (st == FB_DEFAULT || (st == FB_ABORT && new == 1))
               buf[0] = 'n';
            else if (st == FB_ABORT)
               break;
            if (buf[0] != 'n' && buf[0] != 'y')
               fb_screrr("Must use 'n' or <cr> for NO lock, 'y' for YES lock");
            else{
               it->fp->lock = buf[0];
               break;
               }
            }
         fput(it->fp, fld, 7);
	 gethelp(it->fp, fld);
	 if (it->fp->type == FB_CHOICE)
	    getaddchoice(it->fp, fld);
	 xgetauto(it->fp, fld);
	 getrange(it->fp, fld);
	 getmacro(it->fp, fld);
	 fb_move(row, col[1]); fb_clrtoeol();
	 for (i = 0; i <= 7; ++i)
	    fput(it->fp, fld, i);
         return(FB_AOK);
      }

/* 
 *  gethelp - show fb_help file for a fb_field and maybe replace it.
 */
 
   static gethelp(fp, fld)
      fb_field *fp;
      int fld;
      
      {
         char buf[FB_MAXNAME];
	 int row, st;
	 
         if ((row = (fld % 10)) == 0)
            row = 10;
         row = row * 2 + 2;
	 for (;;){
	    fb_move(row, col[1]); fb_clrtoeol();
	    if (fp->type == FB_EXCHOICE)
	       fb_printw("Xchoice File: ");
	    else if (fp->type == FB_CHOICE)
	       fb_printw("Choice File: ");
	    else
	       fb_printw("Help File  : ");
	    if (fp->help == NULL)
	       fb_printw("(VOID)");
	    else
	       fb_printw(FB_FSTRING, fp->help);
	    if (fb_mustbe('y',"Change File name? (y=yes, <other>=no)?",
	    		cdb_t_lines, 1) == FB_AOK){
	       st = fb_input(row, 19, 55, 0, FB_ALPHA, buf, FB_ECHO, FB_OKEND, FB_CONFIRM);
	       if (st == FB_ABORT)
	          continue;
	       if (fp->help != NULL)
	          fb_free(fp->help);
	       fb_trim(buf);
	       fb_underscore(buf, 1);
	       if (st == FB_DEFAULT || st == FB_END)
	          fp->help = NULL;
	       else{
		  fp->help = (char *) fb_malloc(strlen(buf) + 1);
		  strcpy(fp->help, buf);
		  }
	       }
	    else
	       break;
	    }
      }
      
/* 
 *  getaddchoice - get addchoice flag.
 */
 
   static getaddchoice(fp, fld)
      fb_field *fp;
      int fld;
      
      {
         char buf[FB_MAXNAME];
	 int row;
	 
         if ((row = (fld % 10)) == 0)
            row = 10;
         row = row * 2 + 2;
	 for (;;){
	    fb_move(row, col[1]); fb_clrtoeol();
	    fb_printw("AddChoice Option Flag:");
	    fb_move(row, 28);
	    if (fp->choiceflag == CHAR_A)
	       fb_printw(" IS Set ");
	    else
	       fb_printw(" NOT Set ");
	    if (fb_mustbe('y',"Change Setting? (y=yes, <other>=no)?",
		  cdb_t_lines, 1) == FB_AOK){
	       if (fp->choiceflag == CHAR_A)
	          fp->choiceflag = FB_BLANK;
	       else
	          fp->choiceflag = CHAR_A;
	       }
	    else
	       break;
	    }
      }

/* 
 *  xgetauto - show autoindex file for a fb_field and maybe replace it.
 *            also prompt for uniqueness.
 */
 
   static void xgetauto(fp, fld)
      fb_field *fp;
      int fld;
      
      {
         char buf[FB_MAXNAME];
	 int row, st;
	 struct item *tt;
	 
	 if (fp->type == FB_FORMULA || fp->type == FB_LINK || fp->type == FB_BINARY ||
               fp->size > 300)
	    return;
         if ((row = (fld % 10)) == 0)
            row = 10;
         row = row * 2 + 2;
	 for (;;){
	    fb_move(row, col[1]); fb_clrtoeol();
	    fb_printw("AutoIndex File: ");
	    if (fp->aid->autoname == NULL)
	       fb_printw("(VOID)");
	    else {
	       fb_printw(FB_FSTRING, fp->aid->autoname);
	       if (fp->aid->uniq > 0)
	          fb_printw(" (UNIQUE) ");
	       else
	          fb_printw(" (NOT UNIQUE) ");
	       }
	    if (fb_mustbe('y',"Change Auto Index Info? (y=yes, <other>=no)?",
	    		cdb_t_lines, 1) == FB_AOK){
	       for (;;){
		  st = fb_input(row,22,50,0,FB_ALPHA,buf,FB_ECHO, FB_OKEND, FB_CONFIRM);
		  if (st == FB_ABORT)
		     break;
		  if (fp->aid->autoname != NULL)
		     fb_free(fp->aid->autoname);
		  fb_trim(buf);
		  if (st == FB_DEFAULT || st == FB_END)
		     fp->aid->autoname = NULL;
		  else{
		     fp->aid->autoname = (char *) fb_malloc(strlen(buf) + 1);
		     strcpy(fp->aid->autoname, buf);
		     }
		  
		  for (tt = head->next; tt != tail; tt = tt->next)
		     if (tt->fp != fp)
			if (equal(fp->aid->autoname, tt->fp->aid->autoname))
			   break;
		  if (tt == tail){	/* then must be unique id */
		     if (strlen(fb_basename(buf, fp->aid->autoname)) > 
		        cdb_max_file_name){
			   fb_screrr(SYSMSG[S_TOO_LONG]);
		           fp->aid->autoname = NULL;
			   continue;
			   }
		     break;
		     }
                  fb_screrr("AutoIndex name already used");
		  fp->aid->autoname = NULL;
		  }
	       
	       if (fp->aid->autoname != NULL){
	          fb_move(row, 22);
		  fb_printw(FB_FSTRING, fp->aid->autoname);
		  if (fp->aid->uniq > 0)
		     fb_printw(" (UNIQUE) ");
		  else
		     fb_printw(" (NOT UNIQUE) ");
	          if (fb_mustbe('y',"Change Uniqueness? (y=yes, <other>=no)?",
	    		cdb_t_lines, 1) == FB_AOK)
		     fp->aid->uniq = -(fp->aid->uniq);
		  }
	       }
	    else
	       break;
	    }
      }     
	    
/* 
 *  getrange - show range parameter for a fb_field and maybe replace it.
 */
 
   static void getrange(fp, fld)
      fb_field *fp;
      int fld;
      
      {
         char buf[FB_MAXNAME];
	 int row, st;
	 
	 if (fp->type == FB_FORMULA || fp->type == FB_LINK || fp->type == FB_BINARY)
	    return;
         if ((row = (fld % 10)) == 0)
            row = 10;
         row = row * 2 + 2;
	 for (;;){
	    fb_move(row, col[1]); fb_clrtoeol();
	    fb_printw("Data Range: ");
	    if (fp->range == NULL)
	       fb_printw("(VOID)");
	    else
	       fb_printw(FB_FSTRING, fp->range);
	    if (fb_mustbe('y',"Change valid data range? (y=yes, <other>=no)?",
	    		cdb_t_lines, 1) == FB_AOK){
	       st = fb_input(row, 18, 25, 0, FB_ALPHA, buf, FB_ECHO, FB_OKEND, FB_CONFIRM);
	       if (st == FB_ABORT)
	          continue;
	       fb_trim(buf);
	       if (strlen(buf) == 0)
	          continue;
	       fb_underscore(buf, 1);
	       if (fp->range != NULL)
	          fb_free(fp->range);
	       if (st == FB_DEFAULT || st == FB_END)
	          fp->range = NULL;
	       else{
		  fp->range = (char *) fb_malloc(strlen(buf) + 1);
		  strcpy(fp->range, buf);
		  }
	       }
	    else
	       break;
	    }
      }
      
/* 
 *  getmacro - show macro parameter for a fb_field and maybe replace it.
 */
 
   static void getmacro(fp, fld)
      fb_field *fp;
      int fld;
      
      {
         char buf[FB_MAXNAME];
	 int row, st;
	 
	 if (fp->type == FB_FORMULA || fp->type == FB_LINK || fp->type == FB_BINARY)
	    return;
         if ((row = (fld % 10)) == 0)
            row = 10;
         row = row * 2 + 2;
	 for (;;){
	    fb_move(row, col[1]); fb_clrtoeol();
	    fb_printw("Macro File: ");
	    if (fp->f_macro == NULL)
	       fb_printw("(VOID)");
	    else
	       fb_printw(FB_FSTRING, fp->f_macro);
	    if (fb_mustbe('y',"Change macro file name? (y=yes, <other>=no)?",
	    		cdb_t_lines, 1) == FB_AOK){
	       st = fb_input(row, 18, 55, 0, FB_ALPHA, buf, FB_ECHO, FB_OKEND,
                  FB_CONFIRM);
	       if (st == FB_ABORT)
	          continue;
	       fb_trim(buf);
	       if (fp->f_macro != NULL)
	          fb_free(fp->f_macro);
	       if (st == FB_DEFAULT || st == FB_END)
	          fp->f_macro = NULL;
	       else{
		  fp->f_macro = (char *) fb_malloc(strlen(buf) + 1);
		  strcpy(fp->f_macro, buf);
		  }
	       }
	    else
	       break;
	    }
      }
      
/* 
 *  editlink - show window parameter for a fb_field and maybe toggle it.
 */

   static editlink(fp, fld)
      fb_field *fp;
      int fld;

      {
         char buf[FB_MAXNAME];
	 int row, st = FB_ERROR, aok = 0;
	 
         if ((row = (fld % 10)) == 0)
            row = 10;
         row = row * 2 + 2;
	 for (;;){
	    fb_move(row, col[1]); fb_clrtoeol();
	    fb_printw("Link Address: ");
            if (fp->idefault != NULL)
	       fb_prints(fp->idefault);
	    if (fb_mustbe('y', "Change Link Address? (y=yes, <other>=no)?",
                  cdb_t_lines, 1) == FB_AOK){
	       fb_fmessage("Enter LINK Address...Field!index!dbase!Field");
               st = fb_input(row, 20, 48, 0, FB_ALPHA, buf, FB_ECHO, FB_OKEND,
                  FB_CONFIRM);
	       if (st == FB_DEFAULT){
	          strcpy(buf, "1!index!dbase!2");
		  st = FB_AOK;
		  }
               if (st == FB_AOK){
                  fb_mkstr(&(fp->idefault), buf);
                  aok++;
                  }
	       }
	    else
	       break;
	    }
         if (aok > 0)
            st = FB_AOK;
         fb_move(row, 20); fb_clrtoeol();
         return(st);
      }

/* 
 *  fput - fb_put an entire dict item to a constant screen location.
 */
 
   static fput(k, i, pos)
      fb_field *k;
      int i, pos;
      
      {
         int row;
         char buf[FB_MAXLINE];
         
         if ((row = (i % 10)) == 0)
            row = 10;
         row = row * 2 + 2;
         fb_move(row, col[pos]);
         switch (pos){
            case 0: fb_printw("%3d)", i); break;
            case 1: fb_printw("%-10s", k->id); break;
            case 2: fb_printw(FB_FCHAR, k->type); break;
            case 3: fb_printw("%6d", k->size); break;
            case 4: 
               if (k->idefault == NULL)
                  fb_printw("*forced entry*");
               else{
                  strcpy(buf, k->idefault);
                  buf[15] = NULL;
                  fb_printw("%-15s", buf);
                  }
               break;
            case 5: 
               if (k->comment != NULL)
	          fb_printw("%-24s", k->comment);
	       else
	          fb_printw("%24s", " ");
               break; 
            case 6: fb_printw(FB_FCHAR, k->comloc); break;
            case 7: 
               if (k->lock == 'y')
                  fb_printw("yes");
               else
                  fb_printw("no");
               break;
            }
            
      }
      
/*
 *  initlist - initialize the doubly linked lists head and tail nodes
 */
    
   static initlist()
      {
         head = makeitem();
	 tail = makeitem();
	 head->next = tail;
	 tail->prev = head;
	 dot = 1;
	 hp->nfields = 0;
      }
 
/*
 *  tolist - convert normal cdb_kp array format to doubly linked list
 *     structure to enable it to grow without bounds...
 */
 
   static tolist()
      {
       
	 int i;
	 struct item *p;
         fb_autoindex *fb_ixalloc();
       
	 for (i = 0; i < hp->nfields; i++){
	    fb_nounders(cdb_kp[i]);
	    p = makeitem();
	    p->fp = cdb_kp[i];
	    insert(p, tail);
	    if (p->fp->aid == NULL)
	       p->fp->aid = fb_ixalloc();
	    }
	 dot = 1;
      }

/*
 *  tokp - fb_free old cdb_kp list and and reallocate new cdb_kp storage...
 *     push these linked fp's into it.
 *     enables use of fb_help and formula parsing.
 */

   static tokp()
      {
         int i;
	 struct item *it;
	 
	 if (cdb_kp != NULL)
	    fb_free((char *) cdb_kp);
	 cdb_kp = (fb_field **) 
	       fb_malloc((hp->nfields + 1) * sizeof(fb_field *));
	 i = 0;
	 it = head->next; 
	 for (; i < hp->nfields && it != tail; i++, it = it->next)
	    cdb_kp[i] = it->fp;
	 cdb_kp[i] = NULL;			/* paranoia */
      }

/*
 *  insert - insert item p before item q
 */
    
    static void insert(p, q)
       struct item *p, *q;
       
       {
          if (p == NULL || q == NULL){
	     fb_screrr("dbdbas: Could not insert()");
	     return;
	     }
          p->next = q;
	  p->prev = q->prev;
	  q->prev->next = p;
	  q->prev = p;
       }
       
/*
 *  ddelete - delete item p
 */
    
    static void ddelete(p)
       struct item *p;
       
       {
          if (p == head || p == tail || p == NULL)
	     return;
	  p->next->prev = p->prev;
	  p->prev->next = p->next;
	  if (p->fp != NULL)
	     fb_free((char *) p->fp);
	  fb_free((char *) p);
       }

/*
 *  makeitem - allocate an item and define it well.
 */
 
    static struct item 
    *makeitem()
       {
          struct item *p;
	  
	  p = (struct item *) fb_malloc(sizeof(struct item));
	  p->fp = NULL;
	  p->prev = NULL;
	  p->next = NULL;
	  return(p);
       }
       
/*
 *  pitem - return pointer to item numer n 
 */
 
   static struct item
   *pitem(n)
      int n;
      
      {
         struct item *it;
       
	 for (it = head; n > 0 && it != tail; n--)
	    it = it->next;
	 if (n != 0)
	    fb_screrr("Warning: dbdbas: pitem fell short.");
	 return(it);
      }

/* 
 *  update the dbase dictionary file 
 */
 
   static void update(f)
      char *f;
      
      {
         FILE *fs, *fb_mustfopen();
	 struct item *it;

         if (head->next == tail)	/* if no database, no dictionary */
	    return;
         fs = fb_mustfopen(f, "w");
         for (it = head->next; it != tail; it = it->next){
            fb_unders(it->fp);
	    fb_sdict(it->fp, fs, 1);
            }
         fclose(fs);
      }

/* 
 *  headers - print headers at top of screen.
 */
 
   static headers()
      {
         fb_move(2,col[1]); fb_clrtobot(); fb_printw("Field");	/* fb_clrtobot() */
         fb_move(3,col[1]); fb_clrtoeol(); fb_printw("=====");
         fb_move(2,col[2]-3); fb_printw("Type");
         fb_move(3,col[2]-3); fb_printw("====");
         fb_move(2,col[3]); fb_printw("  Size");
         fb_move(3,col[3]); fb_printw("  ====");
         fb_move(2,col[4]); fb_printw("Default");
         fb_move(3,col[4]); fb_printw("=======");
         fb_move(2,col[5]); fb_printw("Comment");
         fb_move(3,col[5]); fb_printw("=======");
         fb_move(2,col[6]); fb_printw("C.Loc");
         fb_move(3,col[6]); fb_printw("=====");
         fb_move(2,col[7]); fb_printw("Lock?");
         fb_move(3,col[7]); fb_printw("=====");
      }

/* 
 * is_restricted - if restricted, print error message. return restricted flag.
 */
   
   static is_restricted()
      {
         if (restricted)
	    fb_screrr("Sorry. That would destroy your existing Database");
	 return(restricted);
      }

/*
 *  usage - diagnostic error message 
 */
  
   static usage()
      {
         fb_xerror(FB_MESSAGE, "usage: dbdbas [-d dbase]", NIL);
      }

#if DEBUGM

/*
 *  tracelist - to debug the linked list structure
 */
 
   static tracelist()
      {
	 struct item *it;
   
	 fb_printw("tracing\n");   
	 for (it = head->next; it != tail ; it = it->next)
	    fb_printw("%s %c %d\n\n", it->fp->id, it->fp->type, it->fp->size);
	 fb_screrr("");
      }

#endif /* DEBUGM */
