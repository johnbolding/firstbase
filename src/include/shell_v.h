/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: shell_v.h,v 9.0 2001/01/09 02:56:17 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

#include <fb_vars.h>

short cdbmenu = 1;			/* some flags */

char *COMHELP[] = {
	"[d]base        change FirstBase database name",
	"[i]ndex        change FirstBase index name",
	"[s]creen       change FirstBase screen name",
	"[v]iew         change FirstBase view name", 
	"",
	"[e]nvironment  display current FirstBase environment",
	"[u]nix         unix command",
	"[x]            start unix command shell",
	"",
	"[c]hdir        change working directory name",
	"[l]s           list contents of directory in FirstBase format",
	"",
	"[p]wd          print working directory",
	"[h]elp         simple help file",
	"", "",
	"<RETURN> or <-> will return to database shell tool",
	0
	};

char cmenu[FB_MAXLINE],
     pmenu[FB_MAXLINE],
     *screen[SCREENMAX+1];

FILE *fs;
long seekpoint;

char *EXITMSG = "Really Exit? ('y' = YES; <other> = NO)? ";

struct an_rmenu ahead, *head = &ahead,
                atail, *tail = &atail;

char *META_DBASE = "$DBASE";
char *META_INDEX = "$INDEX";
char *META_SCREEN = "$SCREEN";
char *META_VIEW = "$VIEW";
char *META_ARGUMENT = "$ARGUMENT";
char *META_RUNFLAGS = "$RUNFLAGS";
char *META_CDBHOME = "$FIRSTBASEHOME";
char *DBLS = "dbls";			/* dbls command */
char *SHELL = "SHELL";

char tmpname[FB_MAXLINE];

char *NON_CDB = "-n";

char *COMMAND_MSG = "Enter Command: ";
char *MSG1 = "Enter Selection:";
char *MSG2 =  "-=End, <CTL>-E=Environment, <CTL>-H=Help";
char *MSG3 = "Selection Does Not Exist!";
char *STR_END = "END";
char *MSG4 = "enter command Argument: ";
char *MSG5 = "enter unix command: ";
char *MSG6 = "enter database name: ";
char *MSG7 = "enter screen name: ";
char *MSG7V = "enter view name: ";
char *STRING_MINUS = "-";
char *BOURNE_SHELL = "/bin/sh";
char *FMT3 = "%s > %s";
char *MSG8 = "enter index name: ";
char *MSG9 = "change to directory: ";
char *MSG10 = "Current FirstBase Environment";
char *STRING_DOT = ".";
/* char *MSG11 = "starting unix command shell:"; */
char *MSG12 = " [no file]";
char *MSG13 = " [not found]";
char *MSG14 = "Set Environment";
char *MSG15 = "Environment Commands";
char *USER = "user: ";
char *DIRECTORY = "directory: ";
char *DATABASE = "database: ";
char *INDEX = "index: ";
char *SCREEN = "screen: ";
char *VIEW = "view: ";
char *MSG16 = "Really Exit? ('y' = YES; <other> = NO)? ";

char 					/* various storage areas */
     dname[FB_MAXNAME] = {""},
     sname[FB_MAXNAME] = {""},
     vname[FB_MAXNAME] = {""},
     iname[FB_MAXNAME] = {""};

char *tempfile = "/tmp/dblsXXXXXX";
char *SCREEN_EXT = ".sdict";
char *VIEW_EXT = ".vdict";
char *FIRSTBASE = "FB_";
char *SCREENDEFAULT = "-";

#if DEMODISK

static char *MOTD[] = {
   "FirstBase is the proprietary RDBMS",
   "produced and distributed by FirstBase Software.",
   "",
   "This tool, dbshell(1), is a small part of FirstBase.",
   "",
   "No further distribution of dbshell(1) is allowed without the consent",
   "of FirstBase Software.",
   "", "", "", "",
   "FirstBase Software",
   "7090 N. Oracle, Suite 165",
   "Tucson, AZ  85704",
   "520/742-7897",
   0 };

#endif /* DEMODISK */
