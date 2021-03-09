/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbpwd.h,v 9.0 2001/01/09 02:56:08 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

#ifndef FB_DBPWD_H
#define FB_DBPWD_H

#define PFILE	"/fbetc/passwd.fb"

#define DBPWD_FIELDS	10

typedef struct fb_s_passwd fb_passwd;
struct  fb_s_passwd {
   char *dbpw_name;
   char *dbpw_passwd;
   int dbpw_uid;
   int dbpw_gid;
   char *dbpw_gecos;
   char *dbpw_term;
   char *dbpw_time;
   char *dbpw_date;
   char	*dbpw_dir;
   char	*dbpw_menu;
   };

#if !FB_PROTOTYPES
fb_passwd *fb_getpwent(), *fb_getpwuid(), *fb_getpwnam();
char *fb_getlogin();
#endif /* FB_PROTOTYPES */

#endif /* FB_DBPWD_H */
