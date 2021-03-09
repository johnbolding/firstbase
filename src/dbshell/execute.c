/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: execute.c,v 9.0 2001/01/09 02:55:44 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Execute_sid[] = "@(#) $Id: execute.c,v 9.0 2001/01/09 02:55:44 john Exp $";
#endif

#include <shell.h>
#include <shell_e.h>

extern char *cdb_hitanykey;	/* to override HIT ANY KEY message */
extern short int force_exit;
extern short int exit_value;
extern char *cdb_dbshell_shell;
extern char menu_rootdir[];
extern char *cdb_home;
extern char *cdb_tempdir;
extern short int cdb_menumatch;
extern char *cdb_runflags;
extern char *cdb_work_dir;
extern char *cdb_pgm;

#if FB_PROTOTYPES
static shell(void);
static expand(char *s);
#else /* FB_PROTOTYPES */
static shell();
static expand();
#endif /* FB_PROTOTYPES */

/*
 * execute - get the $label from fname, else return FB_ERROR.
 *    if AOK, copy shell script into a temporary file. 
 *    execute the file as a single shell action.
 */

   execute(id)
      char *id;
      
      {
	 int j = 0, msg, len;
	 char inbuf[FB_MAXLINE], *fb_trim(), *fb_rmlead();
         char curid[FB_MAXLINE], *status, c;
	 char xdir[FB_MAXNAME], mword[FB_MAXNAME], *ev;
         FILE *fb_mustfopen(), *tfs;
	 struct an_rmenu *p;
	 
	 strcpy(curid, id);
	 sprintf(id, "$%s", curid);	/* pre-pren '$' to search token */
	 /* try and find current id in file fs */
	 len = strlen(id);
	 while ((status = fgets(inbuf, FB_MAXLINE, fs)) != NULL){
	    j = fb_getword(inbuf, 1, curid);
	    if (!cdb_menumatch)		/* allow expanded matches */
	       curid[len] = NULL;
	    if (equal(id, curid))
	       break;
	    }
	 if (status != NULL){
	    j = fb_getword(inbuf, j, curid);
	    if (equal(curid, "*")){
	       xdir[0] = NULL;
	       p = (struct an_rmenu *) fb_malloc(sizeof(struct an_rmenu));
	       p->rmenu = NULL;
	       p->rdir = NULL;
	       fb_mkstr(&(p->rmenu), cmenu);
	       /* store the current working directory */
	       fb_getwd(xdir);
	       fb_mkstr(&(p->rdir), xdir);
	       p->next = tail;
	       p->prev = tail->prev;
	       tail->prev->next = p;
	       tail->prev = p;
	       /* get any optional new directory */
	       if (j > 0){
	          j = fb_getword(inbuf, j, xdir);
                  /* allow xdir to be a true shell env variable */
                  if (xdir[0] == CHAR_DOLLAR)
                     if ((ev = getenv(xdir+1)) != 0)
                        strcpy(xdir, ev);
                  }
	       fgets(inbuf, FB_MAXLINE, fs);
               expand(inbuf);
	       fb_getword(inbuf, 1, mword);
	       fclose(fs);
	       if (xdir[0] != NULL){
	          if (chdir(xdir) == -1)
                     fb_xerror(FB_MESSAGE, "Could not change to directory",
                        xdir);
	          fb_mkstr(&cdb_work_dir, fb_getwd(xdir));
                  fb_cx_set_workdir(cdb_work_dir);
                  strcpy(menu_rootdir, cdb_work_dir);
                  fb_assure_slash(menu_rootdir);
                  fb_getco((fb_database *) NULL);
		  }
               if (mword[0] != CHAR_SLASH)
                  strcpy(cmenu, menu_rootdir);
               else
                  cmenu[0] = NULL;
               strcat(cmenu, mword);
	       fs = fb_mustfopen(cmenu, "r");
               fb_dirname(menu_rootdir, cmenu);
               fb_cx_set_menu(cmenu);
               fb_cx_write(0);
               fb_cx_signal_1();
	       initmenu();
	       return(FB_AOK);
	       }
	    else if (equal(curid, "$EXIT")){
	       j = fb_getword(inbuf, j, curid);
	       force_exit = 1;
	       exit_value = atoi(curid);
	       msg = 0;
	       }
	    else{
	       if (equal(curid, "-1"))
		  msg = 0;
	       else
		  msg = 1;
	       }
            strcpy(tmpname, cdb_tempdir);
            fb_assure_slash(tmpname);
            strcat(tmpname, "cdbEXE_XXXXXX");
	    close(mkstemp(tmpname));
	    tfs = fb_mustfopen(tmpname, "w");
	    for (;;){
	       if (fgets(inbuf, FB_MAXLINE, fs) == NULL)
	          break;
	       fb_trim(inbuf);
	       if (inbuf[0] == '$' || inbuf[0] == NULL)
	          break;
	       expand(inbuf);
	       fputs(inbuf, tfs);
	       }
	    fclose(tfs);
            fb_cx_set_toolname("NOTOOL");
            fb_cx_push_env("T", CX_NO_SELECT, NIL);
            fb_cx_write(1);		/* since no fb_input() is used */
	    shell();
            fb_cx_pop_env();
            fb_cx_set_toolname(cdb_pgm);
            fb_cx_set_workdir(cdb_work_dir);
	    unlink(tmpname);
	    if (msg){
               fb_cx_push_env("C", CX_HITANY_SELECT, NIL);
               fb_cx_write(1);	/* write since no fb_input() */
	       if (cdb_hitanykey == NULL)
	          printf("%s ", SYSMSG[S_HIT_ANY]);
	       else
	          printf("%s ", cdb_hitanykey);
               fflush(stdout);
               read(0, &c, 1);
               fb_cx_pop_env();
	       }
	    }
	 if (status != NULL)
	    return(FB_AOK);
	 else
	    return(FB_ERROR);
      }

/*
 * shell - set up screen, submit file to shell, reset screen.
 */
   static shell()
      {
         char buf[FB_MAXLINE];
	 
	 sprintf(buf, "%s %s", cdb_dbshell_shell, tmpname);
         fb_clear();
         fb_move(1,1);
	 fb_refresh();
	 fb_system(buf, FB_NOROOT);
      }

/* 
 *  expand - expand possible metacommands
 */
 
   static expand(s)
      char *s;
      
       {
         char out[FB_MAXLINE], word[FB_MAXLINE];
         int i;

         strcpy(out, s);
         s[0] = NULL;
         for (i = 1; (i = fb_gettoken(out, i, word, '$')) != 0; ){
            if (strcmp(word, META_DBASE) == 0)
               strcat(s, dname);
            else if (strcmp(word, META_INDEX) == 0)
               strcat(s, iname);
            else if (strcmp(word, META_SCREEN) == 0)
               strcat(s, sname);
            else if (strcmp(word, META_VIEW) == 0)
               strcat(s, vname);
            else if (strcmp(word, META_CDBHOME) == 0)
               strcat(s, cdb_home);
            else if (strcmp(word, META_RUNFLAGS) == 0){
	       if (cdb_runflags != NULL)
                  strcat(s, cdb_runflags);
	       }
            else
               strcat(s, word);
            }
         return(FB_AOK);
      }
