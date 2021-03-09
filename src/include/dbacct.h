/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbacct.h,v 9.0 2001/01/09 02:56:07 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* structure of acct file */

typedef struct fb_s_acct fb_acct;
struct fb_s_acct {
   char ac_line[8];
   char ac_dbname[8];
   char ac_uname[8];
   char ac_toolname[8];
   char ac_dbase[12];
   char ac_index[12];
   long ac_time;
   short ac_type;
   };

#define FB_AC_BEGINTOOL	1
#define FB_AC_ENDTOOL	2

#define FB_ACCT	"/fbetc/acct.fb"
