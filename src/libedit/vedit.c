/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: vedit.c,v 9.2 2006/01/16 17:44:47 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Vedit_sid[] = "@(#) $Id: vedit.c,v 9.2 2006/01/16 17:44:47 john Exp $";
#endif

/*
 *  visual.c - library for visual, filter, cshell modules.
 */

#include <fb.h>
#include <fb_ext.h>

extern char *cdb_tempdir;			/* setup ENVIRONMENT var */
extern char *cdb_DEFAULT_FIRSTBASE_HOME;
extern short int cdb_autodate_lines;

#define FREADONLY 	(00444)			/* read by all */
#define FREADWRITE 	(00444|00222)		/* read/write by all */

static char lastcomm[FB_MAXLINE] = {NULL};		/* for simple history */
static char tempname[FB_MAXNAME];			/* for mkstemp */
static char *MSG3 = "enter system command: ";
static char *MSG4 = "Escaping to Operating system...";

static char *FMT1 = ".%s/%s_lck";
static char *EM1 = "cannot make readonly";
static char *EM2 = "cannot make readwrite";
static char *EM3 = "truncating edited field:";
static char *EDITOR = "EDITOR";
static char *T_AUTODATE = "$AUTODATE";
static char *TNAME = "cdbXXXXXX";

static int tofile(char *t, char *f, int readonly, int autodate, int vis);
static int fromfile(char *t, char *f, int s, int typ);

extern char *cdb_pgm;
extern char *cdb_CSHELL_BASE;
extern char *cdb_FILTER_BASE;
extern char *cdb_VISUAL_BASE;

/* 
 *  vedit - actual edit of fb_field 
 *	whether filter or visual, test for system lock, then local dbase
 *	lock. if both do not exist, then test path:
 *	'~dbase ~ $FIRSTBASEHOME' for existance of 'visual' - 'filter'
 *
 *	now returns FB_AOK on a changed fb_field, FB_ABORT on unchanged.
 */
 
   fb_vedit(readonly, vis, f)
      int readonly, vis;
      fb_field *f;
      
      {
         char *fld, com[FB_MAXLINE], *p, *fnptr, *ev;
	 char fname[FB_MAXNAME], bname[FB_MAXNAME], word[FB_MAXLINE];
         char line[FB_MAXLINE];
	 FILE *fs;
	 int autodate, st = FB_ERROR, j;

         fld = cdb_afld; 		/* set local to global */
         if (vis == 1)		/* visual */
	    strcpy(bname, cdb_VISUAL_BASE);
	 else			/* filter */
	    strcpy(bname, cdb_FILTER_BASE);
	 sprintf(fname, FMT1, cdb_DEFAULT_FIRSTBASE_HOME, bname);
	 if (access(fname, 0) == 0){ 		/* system lock if file */
	    fb_serror(FB_MESSAGE, SYSMSG[S_ACCESSDENIED], NIL);
	    return(FB_ERROR);
	    }
         fb_dirname(com, cdb_db->dbase);
	 sprintf(fname, FMT1, com, bname);
	 if (access(fname, 0) == 0){		/* local dirctory lock */
	    fb_serror(FB_MESSAGE, SYSMSG[S_ACCESSDENIED], NIL);
	    return(FB_ERROR);
	    }
	 sprintf(fname, SYSMSG[S_FMT_SSLASHS], com, bname);
         if (vis == 1 && (p = getenv(EDITOR)) != 0){
	    strcpy(com, p);
	    strcat(com, SYSMSG[S_STRING_BLANK]);
	    }
	 else{
	    if ((fs = fopen(fname, FB_F_READ)) == NULL){
	       fb_pathname(fname, bname);
	       if (fname[0] == NULL || (fs = fopen(fname, FB_F_READ)) == NULL){
                  fnptr = NULL;
                  fb_homefile(&fnptr, bname);
                  strcpy(fname, fnptr);
                  fb_free(fnptr);
		  fs = fopen(fname, FB_F_READ);
		  }
	       }
	    if (fs == NULL){
	       fb_serror(FB_CANT_OPEN, fname, NIL);
	       return(FB_ERROR);
	       }
	    if (fgets(line, FB_MAXLINE, fs) == NULL){
	       fb_serror(FB_MESSAGE, SYSMSG[S_BAD_DATA], fname);
	       fclose(fs);
	       return(FB_ERROR);
	       }
            com[0] = NULL;
            for (j = 1; (j = fb_gettoken(line, j, word, CHAR_DOLLAR)) != 0; ){
               if (word[0] == CHAR_DOLLAR){
                  if ((ev = getenv(word+1)) != 0){
                     strcat(com, ev);
                     word[0] = NULL;
                     }
                  }
               strcat(com, word);
               }
	    com[strlen(com)-1] = FB_BLANK;	/* get rid of newline */
	    fclose(fs);
	    }
	 sprintf(tempname, SYSMSG[S_FMT_SSLASHS], cdb_tempdir, TNAME);
	 close(mkstemp(tempname));
	 if (equal(f->idefault, T_AUTODATE) && !readonly)
	    autodate = 1;
	 else
	    autodate = 0;
	 if (tofile(tempname, f->fld, readonly, autodate, vis) == FB_ERROR){
	    fb_serror(FB_CANT_OPEN, tempname, NIL);
	    return(FB_ERROR);
	    }
	 strcat(com, tempname);
         fb_cx_push_env("T", CX_NO_SELECT, NIL);
         fb_cx_write(1);				/* since no input */
	 fb_system(com, FB_NOROOT);
         fb_cx_pop_env();
	 if (!readonly){
	    if (fromfile(tempname, fld, f->size, f->type) == FB_ERROR){
	       fb_serror(FB_MESSAGE, SYSMSG[S_NOT_DONE], NIL);
	       return(FB_ERROR);
	       }
	    /* defensive programming */
	    if (f->type != FB_FORMULA && f->dflink == NULL){
               if (!equal(fld, f->fld)){
	          fb_store(f, fld, cdb_db);	/* (also done in store)  */
                  st = FB_AOK;
                  }
               }
	    }
	 unlink(tempname);
         return(st);
      }

/* 
 *  tofile - pump fb_trimed fb_field into a temp file.
 */
 
   static tofile(t, f, readonly, autodate, vis)
      char *t, *f;
      int readonly;	/* 0 = readwrite, 1 = readonly */
      int autodate;	/* 0 = no auto date, 1 = store auto date */
      int vis;		/* 1 = visual, 0 = filter */
      
      {
         int fd, i;
	 char bdate[FB_MAXLINE], btime[FB_MAXLINE];
	 
         close(creat(t, 0666));			/* create file */
	 if ((fd = open(t, WRITE)) < 0)
	    return(FB_ERROR);
	 fb_w_init(1, fd, 0);
	 fb_nextwrite(0, f);
	 fb_wflush(1);
	 if (!readonly && autodate && vis){
	    fb_simpledate(bdate, 1);
	    fb_formfield(btime, bdate, FB_DATE, 6);
	    strcpy(bdate, btime);
	    /* abuse lastcomm buffer -- its unused at moment */
            for (i = 0; i < cdb_autodate_lines; i++)
	       fb_nextwrite(0, SYSMSG[S_STRING_NEWLINE]);
	    sprintf(lastcomm, "%s %s -\n", bdate, fb_simpledate(btime,0));
	    fb_nextwrite(0, lastcomm);
	    }
	 fb_wflush(1);
	 fb_sync_fd(fd);
         close(fd);
	 if (readonly == 1){	/* readonly */
	    if (chmod(t, FREADONLY) == -1){
	       fb_serror(FB_MESSAGE, t, EM1);
	       return(FB_ERROR);
	       }
	    }
	 else if (chmod(t, FREADWRITE) == -1){	/* must be readwrite */
	    fb_serror(FB_MESSAGE, t, EM2);
	    return(FB_ERROR);
	    }
/*
* #if cdb_secure
*    static char *EM2a = "cannot chown";
* 	 else if (chown(t, getuid(), -1) == -1){
* 	    fb_serror(FB_MESSAGE, t, EM2a);
* 	    return(FB_ERROR);
* 	    }
* #endif
*/
	 return(FB_AOK);
      }

/* 
 *  fromfile - get from file and fb_put into fld according to size
 */
 
   static fromfile(t, f, s, typ)
      char *t, *f;
      int s, typ;
      
      {
         int fd;
	 char p;
	 int c;
	 
	 if ((fd = open(t, READ)) < 0)
	    return(FB_ERROR);
	 fb_sync_fd(fd);
	 fb_r_init(fd);
	 for (c = 0 ;; f++ ){
	    if (fb_nextread(&p) == 0){
	       *f = NULL;
	       break;
	       }
	    *f = p;
	    if (*f != '\n' && *f != '\t' && *f != ' ' && *f != '\014' &&
                  *f != '\015' && *f != '\012' &&
	          !isprint(*f))
	       *f = CHAR_STAR;
	    else if (typ == FB_UPPERCASE && islower(*f))
	       *f = toupper(*f);
	    if (++c > s){
	       *f = NULL;
	       fb_serror(FB_MESSAGE, EM3, SYSMSG[S_TOO_LONG]);
	       break;
	       }
	    }
	 close(fd);
	 return(FB_AOK);
      }

/* 
 *  cshell - call to cshell if not locked.
 *	a lock is in /usr/lib/cdb/.cshell_lck, or ~dbase/.cshell_lck
 */
 
   void fb_cshell(com)
      char *com;
      
      {
         char fname[FB_MAXNAME], pname[FB_MAXNAME], scom[FB_MAXNAME];
	 int st;
	 
	 strcpy(scom, com);
	 sprintf(fname, FMT1, cdb_DEFAULT_FIRSTBASE_HOME, cdb_CSHELL_BASE);
	 if (access(fname, 0) == 0){ 	/* system lock if file */
	    fb_serror(FB_MESSAGE, SYSMSG[S_ACCESSDENIED], NIL);
	    return;
	    }
         fb_dirname(pname, cdb_db->dbase);
	 sprintf(fname, FMT1, pname, cdb_CSHELL_BASE);
	 if (access(fname, 0) == 0){ 	/* local lock if file */
	    fb_serror(FB_MESSAGE, SYSMSG[S_ACCESSDENIED], NIL);
	    return;
	    }
	 switch(scom[1]){
	    case NULL:
	       fb_fmessage(MSG3);
	       st = fb_input(cdb_t_lines, 24, 50, 0, FB_ALPHA, (char *) scom+1,
	             FB_ECHO, FB_OKEND, FB_CONFIRM);
	       if (st == FB_END || st == FB_ABORT)
	          return;
	       else if (st == FB_AOK)
	          break;
	       /* otherwise , flow into history for FB_DEFAULT */
	    case '!':				/* history reference */
	       if (strlen(lastcomm) == 0){
	          fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
		  return;
		  }
	       strcpy(scom+1, lastcomm);
	       break;
	    }
	 fb_move(cdb_t_lines-1, 1), fb_clrtobot(), fb_force(MSG4);
	 strcpy(lastcomm, scom+1);
         fb_cx_set_toolname("NOTOOL");
         fb_cx_push_env("T", CX_NO_SELECT, NIL);
         fb_cx_write(1);		/* since no fb_input() is used */
	 fb_system(scom+1, FB_NOROOT);
         fb_cx_set_toolname(cdb_pgm);
         /*
          * cx_ddict stuff should be set here, like in custom.c
          * but it does not need to be since what is in this mem
          * will be rewritten next this time. wild.
          */
      }
