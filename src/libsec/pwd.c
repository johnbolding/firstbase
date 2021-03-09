/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: pwd.c,v 9.1 2001/01/16 02:46:55 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char pwd_sid[] = "@(#) $Id: pwd.c,v 9.1 2001/01/16 02:46:55 john Exp $";
#endif

#include <fb.h>
#include <dbpwd.h>

extern char *cdb_menu;

static FILE *pwfs = NULL;
static fb_passwd p = { 0,0,0,0,0,0,0,0,0,0 };

#if FB_PROTOTYPES
static loadpwd(char *buf);
#else /* FB_PROTOTYPES */
static loadpwd();
#endif /* FB_PROTOTYPES */

   fb_passwd *fb_getpwent()
      {
         char buf[FB_MAXLINE];

         if (pwfs == NULL){
	    if ((pwfs = fopen(PFILE, "r")) == NULL){
	       fprintf(stderr, "Cannot open %s\n", PFILE);
	       exit(1);
	       }
	    }
	 if (fgets(buf, FB_MAXLINE, pwfs) == NULL)
	    return((fb_passwd *) NULL);
	 if (loadpwd(buf) != FB_AOK){
#if DEBUG
            fprintf(stderr, "...error loading db_passwd struct\n");
#endif
	    return((fb_passwd *) NULL);
	    }
	 return(&p);
      }

   static loadpwd(buf)
      char *buf;
      
      {
         char pflds[DBPWD_FIELDS][FB_MAXLINE];
	 
	 int i, cc = 0;
	 char *ap, *sp;
	 
	 buf[strlen(buf) - 1] = NULL; /* get rid of newline */
	 sp = buf;
	 for (i = 0; i < DBPWD_FIELDS; i++){
	    pflds[i][0] = NULL;
            sscanf(sp, "%[^:]", pflds[i]);
	    ap = strchr(sp, ':');
	    if (ap == 0)
	       break;
	    if (++cc >= DBPWD_FIELDS)
	       break;
	    sp = ++ap;
	    }
	 if (cc < DBPWD_FIELDS - 1)
	    return(FB_ERROR);
	 fb_mkstr(&(p.dbpw_name), pflds[0]);
	 fb_mkstr(&(p.dbpw_passwd), pflds[1]);
	 p.dbpw_uid = atoi(pflds[2]);
	 p.dbpw_gid = atoi(pflds[3]);
	 fb_mkstr(&(p.dbpw_gecos), pflds[4]);
	 fb_mkstr(&(p.dbpw_term), pflds[5]);
	 fb_mkstr(&(p.dbpw_time), pflds[6]);
	 fb_mkstr(&(p.dbpw_date), pflds[7]);
	 fb_mkstr(&(p.dbpw_dir), pflds[8]);
	 if (strlen(pflds[9]) == 0){
            strcpy(pflds[9], cdb_menu);
	    strcat(pflds[9], "MAIN");
            }
	 fb_mkstr(&(p.dbpw_menu), pflds[9]);
	 return(FB_AOK);
      }

   fb_passwd *fb_getpwuid(uid)
      int uid;

      {
         char buf[FB_MAXLINE];
	 fb_passwd *q;

         if (pwfs == NULL){
	    if ((pwfs = fopen(PFILE, "r")) == NULL){
	       fprintf(stderr, "Cannot open %s\n", PFILE);
	       exit(1);
	       }
            }
         else
            fb_setpwent();
         q = (fb_passwd *) NULL;
	 for (;;){
	    if (fgets(buf, FB_MAXLINE, pwfs) == NULL)
	       break;
	    if (loadpwd(buf) != FB_AOK){
#if DEBUG
               fprintf(stderr, "...error loading db_passwd struct\n");
#endif
	       break;
	       }
	    if (p.dbpw_uid == uid){
	       q = &p;
	       break;
	       }
	    }
	 /*fb_endpwent();*/
	 return(q);
      }

   fb_passwd *fb_getpwnam(name)
      char *name;

      {
         char buf[FB_MAXLINE];
	 fb_passwd *q;

         if (pwfs == NULL)
	    if ((pwfs = fopen(PFILE, "r")) == NULL){
	       fprintf(stderr, "Cannot open %s\n", PFILE);
	       exit(1);
	       }
         q = (fb_passwd *) NULL;
	 for (;;){
	    if (fgets(buf, FB_MAXLINE, pwfs) == NULL)
	       break;
	    if (loadpwd(buf) != FB_AOK){
#if DEBUG
               fprintf(stderr, "...error loading db_passwd struct\n");
#endif
	       break;
	       }
	    if (equal(p.dbpw_name, name)){
	       q = &p;
	       break;
	       }
	    }
	 fb_endpwent();
	 return(q);
      }

   fb_setpwent()
      {
         rewind(pwfs);
      }

   fb_endpwent()
      {
         fclose(pwfs);
	 pwfs = NULL;
      }

   fb_validpass(try, rpass)
      char *try, *rpass;

      {
         char *cpw;
	 
	 cpw = crypt(try, rpass);
         if (equal(cpw, rpass))
	    return(1);
	 return(0);
      }
