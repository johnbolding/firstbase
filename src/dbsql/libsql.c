/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: libsql.c,v 9.1 2001/01/16 02:46:46 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Libsql_sid[] = "@(#) $Id: libsql.c,v 9.1 2001/01/16 02:46:46 john Exp $";
#endif

#include "dbsql_e.h"
#include <pwd.h>

static pwd_home();

/*
 * exit_sql - close down the sql system
 */

   exit_sql()
      {
         /* any closures and cleanups go here */
         exit(0);
      }

/*
 * gcollect - garbage collection.
 *	symtab is not expunged in the idea that some symbols
 *	are likely to be repeated in this session
 */

   gcollect()
      {
         gcollect_node();
         gcollect_instance();
         gcollect_cell();
         gcollect_function();
         gcollect_relation();
      }

/*
 * change_dir - implement the change directory command for dbsql
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
#endif /* DEBUG */
