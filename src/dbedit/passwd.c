/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: passwd.c,v 9.1 2001/02/16 19:41:43 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Passwd_sid[] = "@(#) $Id: passwd.c,v 9.1 2001/02/16 19:41:43 john Exp $";
#endif

#include <dbedit.h>

extern char *cdb_PASSFILE;
extern char *cdb_KEY;

/* 
 *  password - get a password 
 */
 
   void fb_password(dbase)
      char *dbase;
      
      {
         char pass[11], pwd[11], passname[FB_MAXNAME];
         int i;
	 FILE *fs;

         passname[0] = NULL;         
         fb_dirname(passname, dbase);
         strcat(passname, cdb_PASSFILE);
         fb_move(20,1);
         if ((fs = fopen(passname, FB_F_READ)) == NULL)
            return;
         fread(pass, sizeof(char), 10, fs);
         fclose(fs);
         fb_scrhdr(cdb_db, SYSMSG[S_SECURITY]);
	 fb_fmessage(SYSMSG[S_PASSWORD]);
         fb_input(cdb_t_lines, 11, 10, 0, FB_ALPHA, pwd, FB_NOECHO,
            FB_NOEND, FB_CONFIRM);
         fb_makpwd(pwd, cdb_KEY, FB_PSIZE);
         for (i = 0; i < FB_PSIZE; i++)
            if (pass[i] != pwd[i])
	       fb_xerror(FB_MESSAGE, SYSMSG[S_SORRY], NIL);
      }
