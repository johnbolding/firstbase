/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbsql.c,v 9.1 2001/01/16 02:46:46 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbsql_sid[] = "@(#) $Id: dbsql.c,v 9.1 2001/01/16 02:46:46 john Exp $";
#endif

#include "dbsql_v.h"
#include <setjmp.h>
#include <signal.h>

static jmp_buf jmp_env;
static RETSIGTYPE dbsql_onintr();
static RETSIGTYPE dbsql_pager_onintr();

/*
 * dbsql - in interactive mode, provides an sql shell interface
 */

   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         char line[FB_MAXLINE], buf[FB_MAXLINE];
         int i_len = 0, blanklines = 0;

         initsql(argc, argv);
         for (;;){
            if (i_len == 0 && interactive){
               if (setjmp(jmp_env)){	/* return from interrupt */
                  i_len = 0;
                  i_mem[0] = NULL;
                  blanklines = 0;
                  }
               printf("dbsql> ");
               fflush(stdout);
               }
            signal(SIGINT, dbsql_onintr);
            signal(SIGPIPE, dbsql_pager_onintr);
            if (fgets(line, FB_MAXLINE, infile) == NULL){
               if (interactive)
                  printf("\n");
               break;
               }
            if (equal(line, "\n")){
               if (i_len > 0 && ++blanklines >= 2)
                  strcpy(line, ";\n");
               else
                  continue;
               }
            else if (strncmp(line, "quit", 4) == 0
                  || strncmp(line, "exit", 4) == 0)
               exit_sql();
            else if (strncmp(line, "cd", 2) == 0){
               change_dir(line + 3);
               continue;
               }
            else if (strncmp(line, "pwd", 3) == 0){
               fb_getwd(buf);
               printf("%s\n", buf);
               continue;
               }
            else if (strncmp(line, "help", 4) == 0){
               helpsql(line + 5);
               continue;
               }
            else if (line[0] == '!'){
               fb_system(line + 1, FB_NOROOT);
               continue;
               }
            else if (strncmp(line, "version", 7) == 0){
               sprintf(buf,"FirstBase, dbsql %s - %s",
                  VERSION, "Copyright by FirstBase Software");
               printf("%s\n", buf);
               continue;
               }
            else if (line[0] == '#')
               continue;
            i_len += strlen(line);
            if (i_len < MAX_INTER)
               strcat(i_mem, line);
            if (strchr(line, ';') != 0){
               i_cur = 0;
               i_ptr = i_mem;
               if (interactive)
                  lineno = 0;
               winner = NULL;
               npatterns = 0;
               yyparse();
               if (winner != NULL)
                  execute(winner);
               i_len = 0;
               i_mem[0] = NULL;
               blanklines = 0;
               gcollect();
               }
            }
         exit_sql();
      }

/*
 * dbsql_onintr - interrupt routine for dbsql
 */

   static RETSIGTYPE dbsql_onintr()
      {
         if (int_open != NULL)
            u_closetables(int_open);
         if (pagerflag && sql_ofs != NULL)
            pclose(sql_ofs);
         u_vir_remove();
         gcollect();
         fprintf(stderr, "\nInterrupt\n");
         fflush(stderr);
         if (interactive)
            longjmp(jmp_env, 1);
         else
            exit_sql();
      }

/*
 * dbsql_pager_onintr - interrupt routine for dbsql - for PIPE errors.
 */

   static RETSIGTYPE dbsql_pager_onintr()
      {
         gcollect();
         if (interactive)
            longjmp(jmp_env, 1);
         else
            exit_sql();
      }
