/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbdind.c,v 9.1 2001/02/16 19:43:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)dbdind.c	8.3 05 Apr 1996 FB";
#endif

/* 
 *  dbdind.c - screen editor for index generator dictionary file: idicti
 *     this program is unnecessarilly complicated due to lack of forethought.
 *     the problem is that i used the same structure to edit as i do for
 *     actually generating indexes from a selection tree.
 *     tsk tsk. deep sigh...(this works though!, and quite well.)
 */

#include <fb.h>
#include <fb_vars.h>
#include <igen.h>
#include <dbd.h>

int btree = 1;			/* gen a Btree index when done */

#define MAXWORD 20
#define SHOWP 4			/* zero based limit of screen postioins */

#define HLP_DBDIND 	"dbdind.hlp"
#define DBDIND_VAL1	"dbdind1.hlp"
#define DBDIND_VAL2	"dbdind2.hlp"
#define DBDIND_VAL3	"dbdind3.hlp"

int col[8] = {1, 6, 28, 50, 72};

int dot = 0,			/* current pointer...postional from top */
    last = 0;			/* last andtree...postional from top */

fb_field *by[FB_MAXBY];		/* sortby fields */
struct self *top = NULL;	/* top pointer */

static fb_database *hp;
int cflag = 0;			/* case-insensative flag */

extern short int cdb_locklevel;
extern short int cdb_askgen;
extern char *cdb_IGEN;
extern short int cdb_choosefield;
extern char *cdb_T_ALL;
extern char *cdb_T_NOT;
extern char *cdb_T_NONE;
extern char *cdb_T_NOTEQUAL;
extern char *cdb_T_EQUAL;
extern char *cdb_T_LESSEQUAL;
extern char *cdb_T_GREATEREQUAL;
extern char *cdb_T_LESSTHAN;
extern char *cdb_T_GREATERTHAN;
extern char *cdb_T_PATTERN;
extern char *cdb_T_EMPTY;

static no_chooseval = 0;

static struct self *bpos();	/* function to get pointer into self list */
static iedit();
static fillscreen();
static escreen();
static void fdelete();
static fadder();
static efield();
static fput();
static output();
static convert();
static headers();
static begin();
static anychange();

/*
 *  dbdind - main driver.
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
      
      {
         struct self *stree();
         int i, st;

         cdb_locklevel = -1;
         fb_getargs(argc, argv, FB_ALLOCDB);
	 hp = cdb_db;
	 fb_initdbd(hp);
         if (fb_getd_dict(hp) == FB_ERROR)
            fb_xerror(FB_BAD_DICT, hp->ddict, NIL);
	 if (fb_testargs(argc, argv, "-n") > 0)
	    no_chooseval = 1;
	 begin();
	 for (i = 0; i < hp->nfields; i++)
	    fb_nounders(cdb_keymap[i]);
         if ((top = stree(cdb_keymap, hp, by)) == NULL){
	    for (i = 0; i < FB_MAXBY; i++)
	       by[i] = NULL;
	    }
	 else
	    convert();
	 if ((st = iedit(by)) == FB_END){
	    if (by[0] == NULL){
	       getby(by);
	       if (cdb_askgen)
	          headers();
	       }
	    }
	 if (st == FB_END){
	    output(by);
	    if (cdb_askgen)
	       fb_chainto("Generate Index", cdb_IGEN, argv);
	    }
	 else
	    fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
         fb_ender();
      }

/* 
 * iedit - edit the self structures 
 */
 
   static iedit(by)
      fb_field *by[];
   
      {
         int st, i;
	 struct self *p, *q, *r;

         dot = 1;
	 p = top;
	 for (i = 0; p != NULL; ){
	    for(q = p, r = p; q != NULL; q = q->orp){
	       i++;
	       r = q;
	       }
	    if (r == p)
	       p = p->andp;
	    else
	       p = r->andp;
	    }
	 last = i;
         fillscreen();
         while ((st = escreen(by)) != FB_END){
            if (st == FB_ABORT){
	       if (fb_mustbe('y', "Really Quit? (y/n) ", cdb_t_lines, 1) ==
                     FB_AOK)
		  break;
	       }
            else if (st >= dot && st <= dot + 9 && st <= last)
               efield(st, 0, (struct self *) NULL);
            else if (st >= 1 && st <= last){
               dot = fb_onpage(st);
               fillscreen();
               }
            else
               fb_screrr("Invalid Input");
            }
         return(st);
      }

/* 
 * fillscreen - fill a screen with fields 
 */
 
   static fillscreen()
      {
	 char type;
         int j, i, k;
	 struct self *q, *r;

         fb_move(4,1); fb_clrtobot();
	 r = bpos(dot);
	 if (r == NULL){
	    q = top;
	    r = top;
	    dot = 1;
	    }
	 else 
	    q = r->orp;
         j = dot + 9;
         for (i = dot; i <= j && i <= last; i++, r = q, q = q->orp){
	    if (r->orp != NULL && r->orp == r->sandp)
	       type = 's';
	    else if (q == NULL){
	       if ((r == NULL) || ((q = r->andp) == NULL))
	          break;
	       type = 'a';			/* and = 'a' , or = 'o' */
	       }
	    else
	       type = 'o';
	    for (k = 0; k <= SHOWP; ++k)
	       fput(q, i, k, type);
	    }
	 fb_infoline();
         fb_cx_set_viewpage(dot);
      }

/* 
 * escreen - set up a screen, accept commands for screen control 
 */
 
   static escreen(by)
      fb_field *by[];
      
      {
         int st;
	 char com[5], buf[FB_MAXLINE];
         
         for (;;){
            fb_cx_push_env("E1", CX_DBD_SELECT, NIL);
            if (last > 10)
               fb_cx_add_buttons("Ge");
            fb_cx_add_buttons("sXH");
            if (btree)
               strcpy(buf, "BTree+ Index");
            else
               strcpy(buf, "Flat Index");
            st = anychange(com, buf);
            fb_cx_pop_env();
            switch (st) {
               case FB_ABORT: case FB_END:
                  return(st);
               case ADDMODE:
                  fadder(); 
		  break;
               case DELMODE:
                  fdelete(); 
		  break;
	       case '?':				/* FB_HELP */
	          fb_move(2, 1);
		  fb_clrtobot();
	          fb_help(com, hp);
		  headers();
		  break;
               case PAGEF:
                  dot = dot + 10;
                  if (dot > last)
                     dot = 1;
                  break;
               case PAGEB:
                  dot = dot - 10;
                  if (dot < 1)
                     dot = fb_onpage(last);
                  break;
	       case FB_QHELP:
	          fb_move(2, 1); fb_clrtobot();
	          fb_fhelp(HLP_DBDIND);
		  headers();
		  break;
	       case SORTBYMODE:
	          getby(by);
		  headers();
		  break;
	       case TRACEMODE:
	          fb_move(2, 1); fb_clrtobot();
	          itrace(top, by, 1);
		  headers();
		  break;
	       case FB_AOK:
                  if (equal(com, "tree") || equal(com, "norm")){
                     if (btree)
                        btree = 0;
                     else
                        btree = 1;
                     continue;
                     }
                  return(atoi(com));
               default:
                  return(st);
               }
            fillscreen();
            }
      }

/* 
 * fdelete - delete a fb_field 
 */
 
   static void fdelete()
      {
         int st, n;
	 struct self *p, *q;
         
         fb_fmessage("Delete what number ? (#,-) ");
         st = fb_input(cdb_t_lines, 28, 3, 0, FB_INTEGER, (char *) &n,
            FB_ECHO,FB_OKEND,FB_CONFIRM);
         if (st == FB_END || st == FB_ABORT)
            return;
	 else if (n < 1 || n > last)
	    return;
	 if ((p = bpos(n)) == NULL){
            p = top;
	    if ((top = p->orp) == NULL)
	       top = p->andp;
	    fb_free((char *) p);
	    }
	 else{
	    if ((q = p->andp) == NULL)
	       q = p->orp;
	    p->andp = NULL;
	    p->orp = NULL;
	    if (q != NULL && q->andp != NULL)
	       p->andp = q->andp;
	    else if (q != NULL && q->orp != NULL)
	       p->orp = q->orp;
	    fb_free((char *) q);
	    }
         last--;
      }

/* 
 * fadder - go into auto add mode 
 */
 
   static fadder()
      {
         struct self *p, *q;
	 
         dot = fb_onpage(last);
	 if ((last + 1) <  (dot + 10))
            fillscreen();
         for (;;){
            p = (struct self *) fb_malloc(sizeof(struct self));
	    p->andp = NULL;
	    p->orp = NULL;
	    p->lword[0] = NULL;
	    p->rword[0] = NULL;
            if (++last >= dot + 10){
               dot += 10;
               fb_move(4, 1); fb_clrtobot(), fb_infoline();
               }
            if (efield(last, 1, p) == FB_END){
	       q = bpos(last);
	       if (q == NULL)
	          q = top;
	       if (q != NULL){
		  q->andp = NULL;
		  q->orp = NULL;
		  }
	       if (top == p)
	          top = NULL;
               fb_free((char *) p);
	       last--;
               break;
               }
	    if (cdb_choosefield && !no_chooseval){
	       if (fb_mustbe('y', "Add More Entries (y=Yes, -=End) ?",
                     cdb_t_lines, 1) != FB_AOK)
	          break;
	       }
            }
	 dot = 1;
      }
      
/* 
 * efield - edit an entire index self structure 
 */
 
   static efield(fld, new, newp)
      int fld, new;
      struct self *newp;
      
      {
         int row, i, st, comflag;
         char buf[FB_MAXLINE], *fb_trim(), temp[FB_MAXLINE], msg[FB_MAXLINE];
	 struct self *p, *q;

         strcpy(buf, cdb_T_EMPTY);
         if ((row = (fld-- % 10)) == 0)
            row = 10;
         row = row * 2 + 2;
         fb_move(cdb_t_lines, 1); fb_clrtoeol();
	 q = bpos(fld+1);
	 p = NULL;
         if (new == 1){
	    if (q != NULL){
	       q->andp = newp;
	       q->orp = newp;
	       q->sandp = NULL;
	       }
            strcpy(msg, "Enter Field, <RET>=Default, -=End");
            fput(p, fld+1, 0, FB_BLANK);	/* puts number on screen */
	    if (top == NULL)
	       top = newp;
            }
         else
            strcpy(msg, "Change Field, <CTL>-X=Skip, <RET>=Default");
	 if (q == NULL)
	    p = top;
	 else if (q->orp == NULL)
	    p = q->andp;
	 else 
	    p = q->orp;
	 
	 /*
	  *  get fp (fb_field pointer)...via fieldname
	  */

         for (;;){
	    if (!cdb_choosefield){
	       fb_fmessage(msg);
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
            else if (st == FB_END || (st == FB_ABORT && new == 0))
               break;
            if (st == FB_DEFAULT || (st == FB_ABORT && new == 1)){
	       p->fp = cdb_kp[0];
	       break;
	       }
            else if (st == FB_AOK){
	       fb_trim(temp);
	       for (i = 0; i < hp->nfields; i++)
		  if (strcmp(temp, cdb_kp[i]->id) == 0)
		     break;
	       if (i < hp->nfields){
		  p->fp = cdb_kp[i];
                  if (p->fp->type != FB_BINARY)
		     break;
                  else
	             fb_screrr("Invalid: Cannot Use Binary Field in that Manner");
		  }
               else
	          fb_screrr("Invalid Field: Not in Database Dictionary");
	       }
            }
	 if (cdb_choosefield){
	    if (!new){
	       fillscreen();
	       }
	    else{				/* must be new */
	       last--;
	       fillscreen();
	       last++;
	       fput(p, fld+1, 0, FB_BLANK);	/* puts number on screen */
	       }
	    }
         fput(p, fld+1, 1, FB_BLANK);
	 
	 /* 
	  *  get lword (left word or val1)
	  */

         for(;;){
	    if (!cdb_choosefield || no_chooseval){
	       fb_fmessage(msg);
	       fb_scrhlp("[or use '$ALL', '$NOT', $OP]");
	       st = fb_input(row, col[2], MAXWORD, 0, FB_ALPHA, buf, 
	          FB_ECHO, FB_NOEND, FB_CONFIRM);
	       }
	    else{
	       st = fb_chooseval(DBDIND_VAL1, buf, MAXWORD, 1);
	       headers();
	       }
            if (st == FB_DEFAULT || (st == FB_ABORT && new == 1))
               strcpy(buf, cdb_T_ALL);
	    else if (st == FB_ABORT || st == FB_END)
	       break;
/* 
*	    if (buf[0] == '$' && (buf[1] == 'N' || buf[1] == 'n'))
*	       strcpy(buf, cdb_T_NOT);
*/
	    if (buf[0] == '$' && (buf[1] == 'A' || buf[1] == 'a'))
	       strcpy(buf, cdb_T_ALL);
            strcpy(p->lword, fb_trim(buf));
	    if (equal(p->lword, cdb_T_LESSTHAN) ||
		   equal(p->lword, cdb_T_GREATERTHAN) ||
		   equal(p->lword, cdb_T_LESSEQUAL) ||
		   equal(p->lword, cdb_T_GREATEREQUAL) ||
		   equal(p->lword, cdb_T_NOTEQUAL) ||
		   equal(p->lword, cdb_T_EQUAL))
	       comflag = 1;
	    else
	       comflag = 0;
	    if (st == FB_AOK && !equal(buf, cdb_T_NOT) &&
                  !equal(buf, cdb_T_EMPTY) && 
	          comflag == 0 && !equal(buf, cdb_T_ALL) &&
	          p->fp->type ==FB_DATE && strlen(p->lword) != 6)
	       fb_screrr("Must use 6 characters for DATE selection");
	    else
	       break;
	    }
	 if (cdb_choosefield && !no_chooseval){
	    if (!new){
	       fillscreen();
	       }
	    else{				/* must be new */
	       last--;
	       fillscreen();
	       last++;
	       fput(p, fld+1, 0, FB_BLANK);	/* puts number on screen */
	       fput(p, fld+1, 1, FB_BLANK);	/* puts id on screen */
	       }
	    }
         fput(p, fld+1, 2, FB_BLANK);
	 
	 /*
	  *  get rword (rightword or val2)
	  */

	 if (equal(p->lword, cdb_T_LESSTHAN) ||
		equal(p->lword, cdb_T_GREATERTHAN) ||
		equal(p->lword, cdb_T_LESSEQUAL) ||
		equal(p->lword, cdb_T_GREATEREQUAL) ||
		equal(p->lword, cdb_T_NOTEQUAL) ||
		equal(p->lword, cdb_T_EQUAL))
	    comflag = 1;
	 else
	    comflag = 0;
         if (!equal(cdb_T_ALL, p->lword) && !equal(cdb_T_EMPTY, p->lword) &&
               comflag == 0){
	    /* normal fb_input of rword */
            for(;;){
	       if (!cdb_choosefield || no_chooseval){
		  fb_fmessage(msg);
		  if (!equal(cdb_T_NOT, p->lword))
		     fb_scrhlp("[or use '$NONE' or '$PATTERN']");
		  st = fb_input(row,col[3],MAXWORD,0,FB_ALPHA, buf,
			FB_ECHO, FB_NOEND, FB_CONFIRM);
		  }
	       else{
		  st = fb_chooseval(DBDIND_VAL2, buf, MAXWORD, 2);
		  headers();
		  }
	       fb_trim(buf);
               if (st == FB_DEFAULT || (st == FB_ABORT && new == 1))
                  strcpy(buf, cdb_T_NONE);
	       else if (st == FB_ABORT || st == FB_END)
	          break;
	       else if (buf[0] == '$' && (buf[1] == 'N' || buf[1] == 'n'))
	          strcpy(buf, cdb_T_NONE);
	       else if (buf[0] == '$' && (buf[1] == 'P' || buf[1] == 'p'))
	          strcpy(buf, cdb_T_PATTERN);
	       if (strcmp(p->lword, cdb_T_NOT) == 0 && 
	             (st == FB_DEFAULT || equal(buf, cdb_T_PATTERN))){
	          fb_screrr("Must enter value here: Val1 is $NOT!");
		  continue;
		  }
	       if (equal(buf, cdb_T_EMPTY) && !equal(p->lword, cdb_T_NOT)){
	          fb_screrr("Must use $NOT as Val1 when $EMPTY is Val2");
		  continue;
                  }
               strcpy(p->rword, buf);
	       if (st == FB_AOK && p->fp->type == FB_DATE && strlen(p->rword) != 6 &&
                  !equal(p->rword, cdb_T_EMPTY))
		  fb_screrr("Must use 6 characters for DATE selection");
	       else
		  break;
               }
	    }
	 else if (!equal(cdb_T_ALL, p->lword) && !equal(cdb_T_EMPTY, p->lword) &&
               comflag == 1){
	    /* selection of a compare fb_field -- lword was a connective OP */
	    for (;;){
	       if (!cdb_choosefield){
		  fb_fmessage(msg);
		  st = fb_input(row, col[3], FB_TITLESIZE, 0, FB_ALPHA, 
		       temp, FB_ECHO, FB_NOEND, FB_CONFIRM);
		  }
	       else{
		  st = fb_choosefield(temp);
		  headers();
		  }
	       if ((st == FB_ABORT || st == FB_END) && new == 0){
	          strcpy(temp, p->rword);
		  st = FB_AOK;
		  }
	       if (st == FB_DEFAULT || st == FB_END || (st == FB_ABORT && new == 1)){
	          strcpy(temp, cdb_kp[0]->id);
		  st = FB_AOK;
		  }
	       if (st == FB_AOK){
		  fb_trim(temp);
	          if ((p->cfp = fb_findfield(temp, hp)) != NULL){
		     strcpy(p->rword, p->cfp->id);
		     break;
		     }
		  fb_screrr("Invalid Field: Not in Database Dictionary");
		  }
	       }
            }
	 else
	    p->rword[0] = NULL;
	 if (cdb_choosefield){
	    if (!new){
	       fillscreen();
	       }
	    else{				/* must be new */
	       last--;
	       fillscreen();
	       last++;
	       fput(p, fld+1, 0, FB_BLANK);	/* puts number on screen */
	       fput(p, fld+1, 1, FB_BLANK);	/* puts id on screen */
	       fput(p, fld+1, 2, FB_BLANK);	/* puts lword on screen */
	       }
	    }
         fput(p, fld+1, 3, FB_BLANK);
	 
	 /*
	  *  get andp/orp/sanp connective (logical connector)
	  */

         buf[0] = NULL;
         for(;;){
	    if (fld <= 0 || q == NULL)
	       break;
	    if ((fld+1) % 10 == 1)	/* 11, 21, 31, 41... */
	       i = row;
	    else
	       i = row - 1;
	    fb_move(i, col[4]);
	    fb_printw("       ");
	    if (!cdb_choosefield || no_chooseval){
	       fb_fmessage(msg);
	       fb_scrhlp("a=And, o=OR, s=Simple And");
	       st = fb_input(i, col[4], 3, 0, FB_ALPHA, buf, 
		     FB_ECHO, FB_NOEND, FB_CONFIRM);
	       }
	    else{
	       st = fb_chooseval(DBDIND_VAL3, buf, MAXWORD, 3);
	       headers();
	       }
            if (st == FB_DEFAULT || (st == FB_ABORT && new == 1))
               buf[0] = 'a';
            else if (st == FB_ABORT){
	       if (q->andp == p) 
	          buf[0] = 'a';
	       else if (q->orp != NULL && q->orp == q->sandp)
	          buf[0] = 's';
	       else
	          buf[0] = 'o';
               break;
	       }
            if (buf[0] != 'a' && buf[0] != 'o' && buf[0] != 's')
               fb_screrr("a=And, o=OR, s=Simple And");
            else{	/* fix up links to reflect AND or OR selection */
	       if (new == 1){
		  switch(buf[0]) {
		     case 'a': q->orp = NULL; break;
		     case 'o': q->andp = NULL; break;
		     case 's':
		        q->andp = NULL; 
			q->sandp = q->orp;
			break;
		     }
		  }
	       else {
		  switch(buf[0]) {
		     case 'a': 
		        q->sandp = q->orp = NULL;
			q->andp = p;
			break;
		     case 'o':
		        q->sandp = q->andp = NULL;
			q->orp = p;
			break;
		     case 's':
		        q->andp = NULL;
			q->orp = q->sandp = p;
			break;
		     }
		  }
               break;
               }
            }
	 if (cdb_choosefield && !no_chooseval){
	    if (!new){
	       fillscreen();
	       }
	    else{				/* must be new */
	       last--;
	       fillscreen();
	       last++;
	       fput(p, fld+1, 0, FB_BLANK);	/* puts number on screen */
	       fput(p, fld+1, 1, FB_BLANK);	/* puts id on screen */
	       fput(p, fld+1, 2, FB_BLANK);	/* puts lword on screen */
	       fput(p, fld+1, 3, FB_BLANK);	/* puts rword on screen */
	       }
	    }
	 fput(p, fld+1, 4, buf[0]);
	 
	 /*
	  *  finally return
	  */
	  
         return(FB_AOK);
      }

/* 
 *  fput - fb_put an entire fb_field to a constant screen location 
 */
 
   static fput(p, i, pos, aflag)
      struct self *p;
      int i, pos;
      char aflag;
      
      {
         int row, j;
         
         if ((row = (i % 10)) == 0)
            row = 10;
         row = row * 2 + 2;
         fb_move(row, col[pos]);
         switch (pos){
            case 0: fb_printw("%3d)", i); break;
            case 1: fb_printw("%-10s", (p->fp)->id); break;
            case 2: fb_printw("%-20s", p->lword); break;
            case 3: fb_printw("%-20s", p->rword); break;
            case 4: 
	       if (i > 1){
	          if (i % 10 == 1)	/* 11, 21, 31, 41... */
	             j = row;
		  else
		     j = row - 1;
		  fb_move(j, col[pos]);
		  switch (aflag){
		     case 'a':   fb_printw("$AND"); break;
		     case 'o':   fb_printw("$OR"); break;
		     case 's':   fb_printw("$S_AND"); break;
		     }
		  }
	       break;
	    }
      }

/* 
 *  bpos - return postion of self structure before n, NULL if n == top.
 */
 
   static struct self *bpos(n)
      int n;
      
      {
         int i;
	 struct self *p, *r;
	 
	 p = top;	/* traces 'or' trees */
	 r = NULL;	/* to hold last touched good fb_node */
	 for (i = 1; i < n && i < last && p != NULL; i++){
	    r = p;
	    if (p->orp == NULL)
	       p = p->andp;
	    else
	       p = p->orp;
	    }
	 return(r);
      }
      
/* 
 * output - output the tree structure 
 */
 
   static output(by)
      fb_field *by[];
      
      {
         char filen[FB_MAXNAME], out[FB_MAXLINE];
         int i;
	 FILE *fs, *fb_mustfopen();
	 struct self *p, *t, *r = NULL;

	 strcpy(filen, hp->idict);
	 strcat(filen, "i");
	 fs = fb_mustfopen(filen, "w");
	 if (top == NULL){
            top = (struct self *) fb_malloc(sizeof(struct self));
	    top->andp = NULL;
	    top->orp = NULL;
	    top->fp = cdb_kp[0];
	    strcpy(top->lword, cdb_T_ALL);
	    top->rword[0] = NULL;
	    sprintf(out, "No Selection Tree: '%s $ALL' assumed", cdb_kp[0]->id);
	    fb_screrr(out);
	    }
         for (p = top; p != NULL; p = p->andp){
            fprintf(fs, "$\n");
            if (p->orp != NULL){
               fprintf(fs, "#\n");
               for (t = p; t != NULL; r = t, t = t->orp){
	          fb_unders(t->fp);
		  fb_underscore(t->lword, 1);
		  fb_underscore(t->rword, 1);
                  fprintf(fs, "%s %s %s\n", (t->fp)->id, t->lword, t->rword);
		  if (t->orp != NULL && t->orp == t->sandp)
		     fprintf(fs, "&\n");
		  }
               fprintf(fs, "#\n");
	       p = r;
               }
            else{
	       fb_unders(p->fp);
	       fb_underscore(p->lword, 1);
	       fb_underscore(p->rword, 1);
               fprintf(fs, "%s %s %s\n", (p->fp)->id, p->lword, p->rword);
	       }
            }
	 fprintf(fs, "%%\n");
         for (i = 0; i < FB_MAXBY; i++)
            if (by[i] != NULL){
	       fb_unders(by[i]);
               fprintf(fs, "%s ", (by[i])->id);
	       }
	 fprintf(fs, "\n");
         if (btree)
	    fprintf(fs, "%%\n");
	 fclose(fs);
      }

/* 
 * convert - convert dual thread format to single thread format
 */
 
   static convert()
      {
         struct self *p, *q, *r = NULL;
	 
	 for (p = top; p != NULL; p = r->andp){
	    for(q = p; q != NULL; r = q, q = q->orp)
	       ;
	    if (r != p){
	       r->andp = p->andp;
	       p->andp = NULL;
	       }
	    }
      }
   
/* 
 * headers - print header at top of screen 
 */
 
   static headers()
      {
         fb_move(2,col[1]); fb_clrtoeol(); fb_printw("Field");
         fb_move(3,col[1]); fb_clrtoeol(); fb_printw("=====");
         fb_move(2,col[2]); fb_printw("Val1 '>=' or $OP");
         fb_move(3,col[2]); fb_printw("================");
         fb_move(2,col[3]); fb_printw("Val2 '<=' or Field");
         fb_move(3,col[3]); fb_printw("==================");
         fb_move(2,col[4]); fb_printw("And/Or");
         fb_move(3,col[4]); fb_printw("======");
      }

/* 
 * begin  -  refresh the screen template
 */
 
   static begin()
      {
         fb_scrhdr(hp, "Define Index");
	 headers();
      }
      
/* 
 *  anychange - set up anychange screen 
 */
 
   static anychange(inp, msg)
      char inp[], *msg;

      {
         register int st;
         
         fb_fmessage("Any Change (#, -=End, <CTL>-H=HELP) ?");
         fb_scrhlp(msg);
         st = fb_input(-cdb_t_lines, 39, 4, 0, FB_ALPHA, inp, FB_ECHO,
            FB_OKEND, FB_CONFIRM);
	 				/* neg row allows FB_QHELP signal */
         if (st == FB_ABORT || st == FB_END || st == FB_QHELP)
            return(st);
         else if (st == FB_DEFAULT)
            return(PAGEF);
         else if (st == FB_AOK && (equal(inp, "tree") || equal(inp, "norm")))
            return(FB_AOK);
	 switch(inp[0]){
	    case PAGEF:
	    case PAGEB:
	    case ADDMODE:
	    case DELMODE:
	    case INSERTMODE:
	    case HELPMODE:
	    case MINFOMODE:
	    case TRACEMODE:
	    case SORTBYMODE:
	       return(inp[0]);
	    default:
               return(st);
	    }
      }

/* 
 * usage message
 */
 
   usage()
      {
         fb_xerror(FB_MESSAGE, "usage: dbdind [-d dbase] [-i index] [-n]", NIL);
      }
