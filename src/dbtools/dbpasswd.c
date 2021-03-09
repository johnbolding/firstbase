/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbpasswd.c,v 9.0 2001/01/09 02:55:55 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbpasswd_sid[] = "@(#) $Id: dbpasswd.c,v 9.0 2001/01/09 02:55:55 john Exp $";
#endif

/*
 *  dbpasswd.c - make or change a cdb password
 */

#include <fb.h>
#include <fb_vars.h>

#if !FB_PROTOTYPES
static int getnew();
static int getpwd();
#else /* FB_PROTOTYPES */
extern main(int, char **);
static int getnew(char *, char *);
static int getpwd(char *, char *);
#endif /* FB_PROTOTYPES */

extern char *cdb_PASSFILE;
extern char *cdb_KEY;

char passname[FB_MAXNAME] = {"./"};	/* default password directory */

/* 
 *  dbpasswd @ main driver 
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
      
      {
         char passwd[11], try[11];
         FILE *fs;

         (void) Dbpasswd_sid;
         fb_getargs(argc, argv, FB_NODB);
         fb_scrhdr((fb_database *) NULL, "Change Password"); fb_infoline();
         fb_move(20,1);
         if (argc >= 2)
            strcpy(passname, argv[1]);
         if (passname[strlen(passname) - 1] != '/')
            strcat(passname, "/");
         strcat(passname, cdb_PASSFILE);
         fb_refresh();
         if (getpwd(passwd, try) == FB_AOK && getnew(passwd, try) == FB_AOK){
            fs = fb_mustfopen(passname, "w");
            fb_makpwd(passwd, cdb_KEY, FB_PSIZE);
            fwrite(passwd, sizeof(char), 10, fs);
            fclose(fs);
            fb_serror(FB_MESSAGE, "password changed.", NIL);
            fb_ender();
            }
         else
            fb_xerror(FB_MESSAGE, "password unchanged", NIL);
      }     

/* 
 *  getnew - get a new password 
 */
 
   static int getnew(passwd, try)
      char *passwd, *try;
      
      {
         int done, st;
         
         for (done = 0; done == 0; ){
            passwd[0] = NULL;
            try[0] = NULL;
            fb_move(21, 1);
            fb_printw("New password: ");
            fb_refresh();
            st = fb_input(21, 15, 10, 0, FB_ALPHA, passwd, FB_NOECHO, FB_OKEND, FB_CONFIRM);
	    fb_move(21, 1), fb_clrtoeol();
            if (st == FB_END || st == FB_ABORT){
               fb_serror(FB_MESSAGE, "Password Unchanged!", NIL);
               return(st);
               }
            fb_move(22, 1);
            fb_printw("Again: ");
            fb_refresh();
            fb_input(22, 15, 10, 0, FB_ALPHA, try, FB_NOECHO, FB_NOEND, FB_CONFIRM);
	    fb_move(22, 1), fb_clrtoeol();
            if (strcmp(passwd, try) == 0)
               done = 1;
            else
               fb_serror(FB_MESSAGE, 
	          "You must type it accurately at least twice!", NIL);
            }
         return(FB_AOK);
      }

/* 
 *  getpwd - get existing password
 */
 
   static int getpwd(passwd, try)
      char *passwd, *try;
      {
         int i;
	 FILE *fs;

         if ((fs = fopen(passname, "r")) != NULL){
            fread(passwd, sizeof(char), 10, fs);
            fclose(fs);
            fb_printw("Old password: ");
            fb_refresh();
            fb_input(20, 15, 10, 0, FB_ALPHA, try, FB_NOECHO, FB_NOEND,
               FB_CONFIRM);
	    fb_move(20, 1), fb_clrtoeol();
            fb_makpwd(try, cdb_KEY, FB_PSIZE);
            for (i = 0; i < FB_PSIZE; i++)
               if (try[i] != passwd[i]){
                  fb_screrr("Sorry.");
                  return(FB_ERROR);
                  }
            }
         return(FB_AOK);
      }
