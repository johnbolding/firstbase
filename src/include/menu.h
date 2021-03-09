/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: menu.h,v 9.0 2001/01/09 02:56:16 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* menu definitions */

#define NITEMS 		9		/* max number of items/menu */
#define MENUTITLE 	20		/* max length of menu title */
#define MENUCOMMAND 	52		/* max length of menu command */

struct item {
   char title[MENUTITLE+1],             /* title of menu selection */
        command[MENUCOMMAND+1];         /* command for menu selection */
   short pause;                         /* pause flag */
  };

struct an_rmenu {
   char *rmenu;
   char *rdir;
   struct an_rmenu *next, *prev;
   };
