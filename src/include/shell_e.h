/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: shell_e.h,v 9.0 2001/01/09 02:56:17 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

#include <fb_ext.h>

extern char **environ;

extern short cdbmenu;			/* some flags */

extern char *COMHELP[];

extern char cmenu[], pmenu[], *screen[];

extern FILE *fs;
extern long seekpoint;

extern char *EXITMSG;

extern struct an_rmenu ahead, *head, atail, *tail;

extern char *META_DBASE;
extern char *META_INDEX;
extern char *META_SCREEN;
extern char *META_VIEW;
extern char *META_ARGUMENT;
extern char *META_RUNFLAGS;
extern char *META_CDBHOME;
extern char *DBLS;			/* dbls command */
extern char *SHELL;

extern char tmpname[];

extern char *NON_CDB;

extern char *COMMAND_MSG;
extern char *MSG1;
extern char *MSG2;
extern char *MSG3;
extern char *STR_END;
extern char *MSG4;
extern char *MSG5;
extern char *MSG6;
extern char *MSG7;
extern char *MSG7V;
extern char *STRING_MINUS;
extern char *BOURNE_SHELL;
extern char *FMT3;
extern char *MSG8;
extern char *MSG9;
extern char *MSG10;
extern char *STRING_DOT;
extern char *MSG12;
extern char *MSG13;
extern char *MSG14;
extern char *MSG15;
extern char *USER;
extern char *DIRECTORY;
extern char *DATABASE;
extern char *INDEX;
extern char *SCREEN;
extern char *VIEW;
extern char *MSG16;
extern char *SCREENDEFAULT;


extern char dname[], sname[], iname[];		/* various storage areas */
extern char vname[];
extern char *tempfile;
extern char *SCREEN_EXT;
extern char *VIEW_EXT;
extern char *FIRSTBASE;
