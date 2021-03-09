/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: fbtstmac.c,v 9.1 2001/01/16 02:46:48 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Cdbtestmacro_sid[] = "@(#) $Id: fbtstmac.c,v 9.1 2001/01/16 02:46:48 john Exp $";
#endif

#include <fb.h>
#include <fb_vars.h>
#include <macro_v.h>
#include <pwd.h>

char macrotrace_filename[FB_MAXNAME];

static char user_home[FB_MAXNAME];

static pwd_home();
static help();

void change_dir();

/*
 * fbtestmacro - provide simple interface for parsing macros.
 */

   main()

      {

         char line[FB_MAXLINE], tname[FB_MAXNAME], command[FB_MAXLINE];
         int j, traceflag = -1, i;
         char *ep, buf[FB_MAXLINE];

         cdb_batchmode = 1;
         strcpy(tname, "/usr/tmp/fbXXXXXX");
         close(mkstemp(tname));
         strcpy(macrotrace_filename, tname);
         user_home[0] = NULL;
         if ((ep = getenv("HOME")) != 0){
            strcpy(user_home, ep);
            fb_assure_slash(user_home);
            }

         for (;;){
            printf("fbtestmacro> ");
            fflush(stdout);
            if (fgets(line, FB_MAXLINE, stdin) == NULL){
               printf("\n");
               break;
               }
            if (equal(line, "\n"))
               continue;
            else if (strncmp(line, "quit", 4) == 0
                  || strncmp(line, "exit", 4) == 0)
               break;
            else if (strncmp(line, "trace", 5) == 0){
               traceflag = -traceflag;
               continue;
               }
            else if (strncmp(line, "cd", 2) == 0){
               change_dir(line + 3);
               continue;
               }
            else if (strncmp(line, "pwd", 3) == 0){
               fb_getwd(buf);
               printf("%s\n", buf);
               continue;
               }
            else if (strncmp(line, "ls\n", 3) == 0){
               fb_system("ls", FB_NOROOT);
               continue;
               }
            else if (strncmp(line, "help", 4) == 0){
               help();
               continue;
               }
            else if (line[0] == '!'){
               fb_system(line + 1, FB_NOROOT);
               continue;
               }
            j = strlen(line);
            if (line[j-1] == '\n')
               line[j-1] = 0;
            if (access(line, 0) != 0){
               printf("could not find file: %s\n", line);
               continue;
               }
            symtab = NULL;
            g_symtab = NULL;
            n_ghead = NULL;
            c_ghead = NULL;
            winner = NULL;
            if (fb_macrotree(line) == FB_AOK){
               printf("%s: macro file parsed AOK\n", line);
               if (traceflag > 0){
                  printf("begin macrotrace ...\n");
                  fb_macrotrace(winner, line);
                  if ((ep = getenv("PAGER")) != 0)
                     sprintf(command, "%s %s", ep, tname);
                  else
                     sprintf(command, "more %s", tname);
                  system(command);
                  unlink(tname);
                  }
               }
            else{
               fb_tracesource(stdout, line);
               printf("%s: macro file DID NOT parse\n", line);
               }
#if X_DEBUG
            mem_check("pre garbage");
#endif
            fb_gcollect(n_ghead, c_ghead);
            fb_expunge_symtab(symtab);
            fb_expunge_symtab(g_symtab);
#if X_DEBUG
            mem_check("post garbage");
#endif
            }
      }

/*
 * change_dir - implement the change directory command 
 */

   void change_dir(dname)
      char *dname;

      {
         char buf[FB_MAXLINE], *p, pname[FB_MAXNAME], pfile[FB_MAXNAME];

         strcpy(buf, dname);
         remove_newline(buf);
         fb_rmlead(buf);
         if (buf[0] == NULL)
            strcpy(buf, user_home);
         /* do cshell style processing of ~ */
         if (buf[0] == '~'){
            if (buf[1] == '/' || buf[1] == NULL){
               strcpy(dname, user_home);
               strcat(dname, buf + 2);
               }
            else{
               /* must be ~user/xxx which means passwd lookup */
               p = strchr(buf, '/');
               if (p != 0)
                  *p = NULL;
               strcpy(pname, buf + 1);
               pfile[0] = NULL;
               if (p != 0)
                  strcpy(pfile, p + 1);
               if (pwd_home(buf, pname) == FB_ERROR){
                  fb_serror(FB_MESSAGE, "No user named", pname);
                  return;
                  }
               sprintf(dname, "%s/%s", buf, pfile);
               }
            strcpy(buf, dname);
            }
         fb_trim(buf);
         if (chdir(buf) < 0)
            fb_serror(FB_MESSAGE, "Could not change to directory", buf);
      }

/*
*  pwd_home - look up pname in the passwd file and return its home in buf
*		return FB_ERROR if not a valid pname.
*/

   static pwd_home(buf, pname)
      char *buf, *pname;

      {
         struct passwd *getpwnam(), *gp;

         gp = getpwnam(pname);
         if (gp == NULL)
            return(FB_ERROR);
         strcpy(buf, gp->pw_dir);
         return(FB_AOK);
      }

   static help()
      {
         printf( "fbtestmacro commands:\n");
         printf( "   help           - general help\n");
         printf( "   exit           - exit \n");
         printf( "   quit           - exit \n");
         printf( "   cd             - change directory\n");
         printf( "   pwd            - print working directory\n");
         printf( "   ls             - list working directory\n");
         printf( "   !command       - submit command to UNIX shell\n");
         printf( "   filename       - submit file to macro parser\n");
      }

/*
 * remove_newline - remove trailing FB_NEWLINE
 */

   remove_newline(buf)
      char *buf;

      {
         int lc;

         lc = strlen(buf) - 1;
         if (buf[lc] == FB_NEWLINE)
            buf[lc] = NULL;
      }

#if X_DEBUG
/*
 * mem_check - verify the heap of storage
 */

   mem_check(s)
      char *s;

      {
         fprintf(stderr, "Mem check @ %s: ", s);
         if (malloc_verify() == 1)
            fprintf(stderr, "PASS\n");
         else
            fprintf(stderr, "FALSE\n");
      }
#endif /* X_DEBUG */
