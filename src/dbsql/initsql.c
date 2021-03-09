/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: initsql.c,v 9.1 2001/01/24 12:36:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Initsql_sid[] = "@(#) $Id: initsql.c,v 9.1 2001/01/24 12:36:57 john Exp $";
#endif

#include "dbsql_e.h"

static char *USAGE = 
   "usage: dbdql [-f FILE] [-v] [-q] [-n] [-c separator] [format options]";

static char *S_TERM = "TERM";
static char *S_LI = "li";
static char *S_CO = "co";

extern short int cdb_returnerror;
extern short int cdb_locklevel;
extern short int cdb_cgi_flag;

/*
 * initsql - initialize the sql system
 */

   initsql(argc, argv)
      int argc;
      char *argv[];
   
      {
         int t, i;
         FILE *fb_mustfopen();
         cell **makesymtab();
         char *ep, Bp[2048], *p;

	 cdb_batchmode = 1;
         cdb_returnerror = 1;
         cdb_locklevel = -1;
         fb_getargs(argc, argv, FB_NODB);
         fb_getco((fb_database *) NULL);
         cdb_t_lines = cdb_t_cols = 0;
         if ((p = getenv(S_TERM)) != 0){
            if (tgetent(Bp, p) == 1){
               cdb_t_lines = tgetnum(S_LI);
               cdb_t_cols = tgetnum(S_CO);
               }
            }
         if (cdb_t_lines == 0)
            cdb_t_lines = 24;
         if (cdb_t_cols == 0)
            cdb_t_cols = 80;
         infile = stdin;
         interactive = 1;
         lineno = 0;
         for (i = 0; i < 5; i++)
            margin[i] = 2;
         pagenumber = 0;
         pagelength = cdb_t_lines;
         pageindent = 5;
         linenumber = 0;
         strcpy(sql_tempdir, "/usr/tmp");
         for (i = 1; i < argc; i++){
            if (argv[i][0] == CHAR_MINUS && argv[i][1] != NULL){
               switch (argv[i][1]) {
                  case 'e':
                     emitflag = 1;
                     break;
                  case 'v':
                     verbose = 1;
                     break;
                  case 'q':
                     quoteflag = 1;
                     break;
                  case 'f':
                     if (++i >= argc)
                        usage();
                     infile = fb_mustfopen(argv[i], "r");
                     interactive = 0;
                     break;
                  case 'c':
                     if (++i >= argc)
                        usage();
                     if (argv[i][1] != NULL)
                        usage();
                     separator = argv[i][0];
                     break;
                  case 't':
                     if (++i >= argc)
                        usage();
                     strcpy(sql_tempdir, argv[i]);
                     break;
#if DEBUG
                  case 'T':
                     if (++i >= argc)
                        t = 1;
                     else
                        t = atoi(argv[i]);
                     traceflag = (t > 0) ? t : 1;
                     break;
#endif /* DEBUG */
                  case 'n':
                     newline_flag = 0;
                     break;
                  case 'i':
                     if (++i >= argc)
                        usage();
                     pageindent = atoi(argv[i]);
                     formatpage = 1;
                     break;
                  case 'p':
                     if (++i >= argc)
                        usage();
                     pagelength = atoi(argv[i]);
                     formatpage = 1;
                     break;
                  case 'P':		/* Pager */
                     pagerflag = 1;
                     if ((ep = getenv("PAGER")) != 0)
                        strcpy(sql_pager, ep);
                     else
                        strcpy(sql_pager, "more");
                     break;
                  case 'm':
                     ep = argv[i];
                     if (++i >= argc)
                        usage();
                     ep += 3;
                     t = atoi(ep);
                     if (t < 1)
                        t = 1;
                     else if (t > 4)
                        t = 4;
                     margin[t] = atoi(argv[i]);
                     formatpage = 1;
                     break;
                  case 'h':		/* headers flag == formatpage */
                     formatpage = 1;
                     break;
                  case 'w':		/* to force the width to pipeline */
                     if (++i >= argc)
                        usage();
                     cdb_t_cols = atoi(argv[i]);
                     break;
                  case 'H':		/* HTML flag */
                     html = 1;
                     switch (argv[i][2]){
                        case 'B':
                           if ((html_border = atoi(argv[i] + 3)) == 0)
                              html_border = 1;
                           break;
                        case 'C':
                           if ((html_cellpadding = atoi(argv[i] + 3)) == 0)
                              html_cellpadding = 1;
                           break;
                        }
                     break;
                  }
               }
            }
         if (!emitflag)
            verbose = 1;
         if (html){
            formatpage = 0;
            emitflag = 1;
            verbose = 1;
            quoteflag = 0;
            pagerflag = 0;
            cdb_cgi_flag = 1;
            }

         i_mem[0] = NULL;
         symtab = makesymtab();
         user_home[0] = NULL;
         if ((ep = getenv("HOME")) != 0){
            strcpy(user_home, ep);
            i = strlen(user_home);
            if (i > 0 && user_home[i - 1] != '/'){
               user_home[i] = '/';
               user_home[i + 1] = NULL;
               }
            }
         if (!isatty(1))
            interactive = 0;
         if (pagerflag && !interactive)
            pagerflag = 0;
         linelength = cdb_t_cols - pageindent;
      }

/* 
 * usage message 
 */

   usage()
      {
         fb_xerror(FB_MESSAGE, USAGE, NIL);
      }
