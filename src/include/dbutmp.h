/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbutmp.h,v 9.0 2001/01/09 02:56:09 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* structure of utmp and wtmp files */

typedef struct fb_s_utmp fb_utmp;
struct  fb_s_utmp {
   char ut_line[8];
   char ut_dbname[8];
   char ut_uname[8];
   char ut_type;
   long ut_time;
   int ut_slot;
   };

#define UT_LOGIN	'i'
#define UT_LOGOUT	'o'
#define UTMP	"/fbetc/utmp.fb"
#define WTMP	"/fbetc/wtmp.fb"
