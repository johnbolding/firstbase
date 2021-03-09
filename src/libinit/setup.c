/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: setup.c,v 9.3 2001/02/16 19:10:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Setup_sid[] = "%W% %G% " ;
#endif

#include <fb.h>
#include <fb_ext.h>
#include <keypad.h>
#include <pwd.h>
#if RPC
#include <fblserve.h>
#include <dblserve.h>
#endif /* RPC */
/*#include <dbfwd.h>*/
/*#include <dblwd.h>*/

#if RPC
#include <rpc/rpc.h>
/* fbserver/fblserver stuff for verification */
static CLIENT *cl = NULL;
static char oserver[FB_MAXNAME] = {""};
static char *EXSTR = "%s%s";
static fb_floatwd cdb_v_fwd = { 0, 0, 0, 0, 0, 0, 0 };
fb_floatwd *cdb_v_f = &cdb_v_fwd;
char cdb_v_fid[FB_MAXNAME];
#endif /* RPC */

#if FB_PROTOTYPES
static setup_keyboard(void);
static void e_file(char *dname);
static e_argline(int argc, char **argv);
static e_init(void);
static e_set(char *e_var, char *word, int perm);
static interpret_signal(char *s);
static mmc(void);
static q_init(void);
#else /* FB_PROTOTYPES */
static setup_keyboard();
static void e_file();
static e_argline();
static e_init();
static e_set();
static interpret_signal();
static q_init();
static mmc();
#endif /* FB_PROTOTYPES */

#define MAX_CCOMMAND	10

static char *ASKGEN = 		"ASKGEN";
static char *ASKWRITE =		"ASKWRITE";
static char *AUTOBTREE = 	"AUTOBTREE";
static char *AUTODATE_LINES = 	"AUTODATE_LINES";
static char *BOURNE_SHELL =  	"/bin/sh";
static char *BLOCKSIZE =  	"BLOCKSIZE";
static char *CCOMMAND = 	"CCOMMAND";
static char *CCOMMAND_PROMPT = 	"CCOMMAND_PROMPT";
static char *CCOMMAND_SHELL = 	"CCOMMAND_SHELL";
static char *CDBLSERVER_VAR = 	"LSERVER";
static char *CDBSERVER_VAR = 	"SERVER";
static char *CDBLSERVERLOG = 	"LSERVERLOG";
static char *CDBSERVERLOG = 	"SERVERLOG";
static char *CDBLSERVERERR = 	"LSERVERERROR";
static char *CDBSERVERERR = 	"SERVERERROR";
static char *CDBLOCKDERR = 	"LOCKDERROR";

static char *FIRSTBASEHOME =	"FIRSTBASEHOME";
static char *HOME_CCOMMAND = 	"HOME_CCOMMAND";
static char *WDIR_CCOMMAND = 	"WDIR_CCOMMAND";
static char *CENTURYMARK = 	"CENTURYMARK";
static char *CENTURYBASE = 	"CENTURYBASE";
static char *CENTURYNEXT = 	"CENTURYNEXT";
static char *CHOOSEFIELD =  	"CHOOSEFIELD";
static char *CHOICEPAUSE =  	"CHOICEPAUSE";
static char *CHOICEPAUSE_MSG = 	"CHOICEPAUSE_MSG";
static char *CHOICEADDPAUSE =  	"CHOICEADDPAUSE";
static char *CPU_BYTEORDER =  	"CPU_BYTEORDER";
static char *EX_CHOICEPAUSE =  	"EX_CHOICEPAUSE";
static char *EX_CHOICEPAUSE_MSG = "EX_CHOICEPAUSE_MSG";
static char *EX_CHOICEADDPAUSE ="EX_CHOICEADDPAUSE";
static char *EX_FULLKEY =	"EX_FULLKEY";
static char *EX_REVERSE =	"EX_REVERSE";
static char *EX_FORCEONE =	"EX_FORCEONE";
static char *DATEBASIS = 	"DATEBASIS";
static char *DATEDISPLAY = 	"DATEDISPLAY";
static char *DATESTYLE = 	"DATESTYLE";
static char *DBASE_BYTEORDER = 	"DBASE_BYTEORDER";
static char *DBASE = 		"DBASE";
static char *DBSHELL_PSTRING = 	"DBSHELL_PSTRING";
static char *DBSHELL_PLOC = 	"DBSHELL_PLOC";
static char *DBSHELL_PILENGTH =	"DBSHELL_PILENGTH";
static char *DBSHELL_ECOMMAND =	"DBSHELL_ECOMMAND";
static char *DBSHELL_SHELL =	"DBSHELL_SHELL";
static char *DBSHELL_CHECKMAIL ="DBSHELL_CHECKMAIL";
static char *DBVEDIT_REC_PLOC = "DBVEDIT_REC_PLOC";
static char *DBVEDIT_REC_PILENGTH = "DBVEDIT_REC_PILENGTH";
static char *DBVEDIT_CHO_PLOC = "DBVEDIT_CHO_PLOC";
static char *DBVEDIT_CHO_PILENGTH = "DBVEDIT_CHO_PILENGTH";
static char *DBVEDIT_CHO_FIRSTLINE = "DBVEDIT_CHO_FIRSTLINE";
static char *DIRNAME = 		"DIRNAME";
static char *DECIMAL = 		"DECIMAL";
char *cdb_DEFAULT_FIRSTBASE_HOME= "/usr/local/firstbase/";
static char *DINDEX = 		"INDEX";
static char *ERRORLOG=		"ERRORLOG";
static char *FIXEDWIDTH =	"FIXEDWIDTH";
static char *FORCEAUTOINCR =	"FORCEAUTOINCR";
static char *FIXEDNODE =	"FIRSTBASE_FIXEDNODE";
static char *HITANYKEY =	"HITANYKEY";
static char *INFOLINE =		"INFOLINE";
static char *INTERRUPT=		"INTERRUPT";
static char *EDITINPUT=		"EDITINPUT";
/* keypad stuff */
static char *FUNCTIONKEYS=	"FUNCTIONKEYS";
static char *K_UALARM=		"UALARM";
static char *KEYPAD=		"KEYPAD";
static char *UP=		"UP";
static char *DOWN=		"DOWN";
static char *LEFT=		"LEFT";
static char *RIGHT=		"RIGHT";
static char *TAB=		"TAB";
static char *K_ESCAPE=		"ESCAPE";
/* end keypad stuff */
static char *UNIXTYPE = 	"UNIXTYPE";
static char *LOCALRC =		"LOCALRC";
static char *LOCKDAEMON=	"LOCKDAEMON";
static char *LOCKLEVEL=		"LOCKLEVEL";
static char *LOCKMESSAGE=	"LOCKMESSAGE";
static char *LOCKTIME=		"LOCKTIME";
static char *LOGFILE=		"USRLOG";
static char *MAKEINDEX = 	"MAKEINDEX";
static char *MENUPAUSE = 	"MENUPAUSE";
static char *MENUMATCH = 	"MENUMATCH";
static char *MERGECOLS = 	"MERGECOLS";
static char *NAME_LENGTH = 	"NAME_LENGTH";
static char *NEGATIVE = 	"NEGATIVE";
static char *NOADDREC = 	"NOADDREC";
static char *OKSTOP = 		"OKSTOP";
static char *PUTFILE = 		"PUTFILE";
static char *PGENCOLS = 	"PGENCOLS";
static char *PROMPT_ADDMODE1 =	"PROMPT_ADDMODE1";
static char *PROMPT_ADDMODE2 =	"PROMPT_ADDMODE2";
static char *PROMPT_AUTOFIELD =	"PROMPT_AUTOFIELD";
static char *PROMPT_NORMALFIELD = "PROMPT_NORMALFIELD";
static char *PROMPT_RECORDMSG =	"PROMPT_RECORDMSG";
static char *PROMPT_COMMANDMSG ="PROMPT_COMMANDMSG";
static char *PROMPT_CHOICEMSG =	"PROMPT_CHOICEMSG";
/*
 * do we need to test for SHORTNAMES anymore ? and how? using MAXNAMLEN?
 * static char *RCFILE = 		".fb-init";
 * what does MAXNAMLEN look like on a short file name system?
 *
 */
static char *RCFILE = 		".firstbase-init";
static char *TRAP_XON = 	"TRAP_XON";
static char *RECLOG = 		"RECLOG";
static char *RECORD_LEVEL =	"RECORD_LEVEL";
static char *RECORD_UMASK =	"RECORD_UMASK";
static char *REFRESH_CLEARS = 	"REFRESH_CLEARS";
static char *REGEXP = 		"REGEXP";
static char *RUNFLAGS = 	"RUNFLAGS";
static char *RUNDBD = 		"RUNDBD";
static char *SCREENPRINT = 	"SCREENPRINT";
static char *SCREENPRINT_PAUSE ="SCREENPRINT_PAUSE";
static char *SCREEN = 		"SCREEN";
static char *SCR_AUTOINCR = 	"SCR_AUTOINCR";
static char *SCR_HEADER = 	"SCR_HEADER";
static char *SCR_INFOLINE = 	"SCR_INFOLINE";
static char *SCR_INFOLINE_SOLID = "SCR_INFOLINE_SOLID";
static char *SCR_STATUS = 	"SCR_STATUS";
static char *SCR_STATUS_MASK = 	"SCR_STATUS_MASK";
static char *SCR_VERSION_MASK = "SCR_VERSION_MASK";
static char *SCR_TOOLNAME =	"SCR_TOOLNAME";
static char *SCR_INPUTDOTS = 	"SCR_INPUTDOTS";
static char *SCR_INPUTCLEAR = 	"SCR_INPUTCLEAR";
static char *SCR_INPUTPASTEDGE = "SCR_INPUTPASTEDGE";
static char *SCR_STAT2 = 	"SCR_STAT2";
static char *SCR_HELP = 	"SCR_HELP";
static char *SCR_LABEL = 	"SCR_LABEL";
static char *SET = 		"set";
static char *SHELL = 		"SHELL";
static char *SHOWRECCNT = 	"SHOWRECCNT";
static char *SIGNATURE = 	"SIGNATURE";
static char *SKIP_NULL_AUTO = 	"SKIP_NULL_AUTO";
static char *SUB_TTY_ROWS = 	"SUB_TTY_ROWS";
static char *SUB_TTY_COLS = 	"SUB_TTY_COLS";
static char *CDB_SIG_END = 	"END";
static char *CDB_SIG_ABORT = 	"ABORT";
static char *CDB_SIG_HELP = 	"HELP";
static char *CDB_SIG_PAGEUP = 	"PAGEUP";
static char *CDB_SIG_PAGEDOWN = "PAGEDOWN";
static char *CDB_SIG_NEXT = 	"NEXT";
static char *CDB_SIG_PREV = 	"PREV";
static char *CDB_SIG_PRINT = 	"PRINT";
static char *CDB_SIG_CLEARFIELD="CLEARFIELD";
static char *CDB_SIG_DEFAULT = 	"DEFAULT";
static char *CDB_SIG_WRITEREC =	"WRITEREC";
static char *CDB_SIG_DELETEREC ="DELETEREC";
static char *CDB_REVERSE =	"REVERSE";
static char *CDB_STANDOUT = 	"STANDOUT";
static char *TEMPDIR =		"TEMPDIR";
static char *USE_INSERT_CHAR = 	"USE_INSERT_CHAR";
static char *UMASK = 		"UMASK";
static char *USRLOG = 		"USRLOG";
static char *VIPAUSE = 		"VIPAUSE";
static char *VIPAUSE_MSG = 	"VIPAUSE_MSG";
static char *VIADDPAUSE = 	"VIADDPAUSE";
static char *VIEW = 		"VIEW";
static char *WRAPSCAN = 	"WRAPSCAN";
static char *WRITEDIR = 	"WRITEDIR";
/*
static char *CONTACT =
   "No License. Contact FirstBase, support@firstbase.com, (520) 742-7897";
*/
static char *ADDMODE_MSG1 = 
   "Enter Data, -=END, <CTL>-D=Defaults, <CTL>-H=help";		/* 1 */
static char *ADDMODE_MSG2 = 
   "Enter Data, <CTL>-X=Abort Record, <CTL>-H=help";		/* 2 */
static char *AUTOFLDMSG = 
   "Enter Data, -=END, <CTL>-X=Abort Field, <CTL>-H=help"; 	/* 3 */
static char *NORMALFLDMSG = 
   "Enter Data, <CTL>-X=Abort Field, <CTL>-H=help";		/* 4 */

extern short int cdb_refresh_clears;

short int cdb_datebasis;	/* day count method */
short int cdb_centurymark;	/* mark point of century assumptions */
char *cdb_centurybase;		/* century base - 19 for now */
char *cdb_centurynext;		/* next century  - 20 for now */
short int cdb_datedisplay;	/* number of date chars to display-6 or 8 */
short int cdb_datestyle;	/* date style - 1 = american, 2 = european */
short int cdb_decimal;		/* flag for dollar input : w/wo decimal */
short int cdb_choicepause;	/* flag for pausing before a choice fb_field */
short int cdb_choiceaddpause;	/* flag for pausing before an add choice */
short int cdb_ex_choicepause;	/* flag for pausing before a exchoice fb_field */
short int cdb_ex_choiceaddpause; /* flag for pausing before an add exchoice */
char *cdb_ccommand[MAX_CCOMMAND];/* area for custom commands */
char *cdb_ccommand_prompt[MAX_CCOMMAND]; /* area for custom commands prompts*/
char *cdb_wdir_ccommand = NULL;	/* custom commands working directory */
char *cdb_home_ccommand = NULL;	/* custom commands home directory (scripts) */
char *cdb_prompt_commandmsg = NULL;	/* command level message */
char *cdb_prompt_recordmsg = NULL;	/* record level message */
char *cdb_prompt_choicemsg = NULL;	/* choice message */
char *cdb_prompt_addmodemsg1 = NULL;	/* add mode message 1 */
char *cdb_prompt_addmodemsg2 = NULL;	/* add mode message 2 */
char *cdb_prompt_autofldmsg = NULL;	/* auto fb_field message */
char *cdb_prompt_normalfldmsg = NULL;	/* normal fb_field message */
short int cdb_showreccnt = 1;		/* record counters infoline toggle */
char *cdb_tempdir = NULL;		/* temp directory for temp files */
short int cdb_scr_autoincr = 1;		/* toggle switch on autoincr screen */
short int cdb_scr_header = 1;		/* toggle switch on screen header */
short int cdb_scr_fb_infoline = 1;	/* toggle switch on infoline */
short int cdb_scr_fb_infoline_solid = 0;/* toggle switch for solid infoline */
short int cdb_scr_status = 1;		/* toggle switch on scr status area */
short int cdb_scr_status_mask = 0; 	/* toggle switch on "Status:" */
short int cdb_scr_version_mask = 0; 	/* toggle switch to turn off VERSION:*/
short int cdb_scr_toolname = 1; 	/* toggle switch to turn off toolname*/
short int cdb_scr_inputdots = 1;	/* toggle switch on input dots looks */
short int cdb_scr_inputclear = 1;	/* toggle: clr input before edit */
short int cdb_scr_inputpastedge = 1;	/* toggle: disp field past edge */
short int cdb_scr_stat2 = 1;		/* toggle on screen status2 area */
short int cdb_scr_help = 1;		/* toggle on screen help area */
short int cdb_scr_label = 1;		/* toggle switch on screen label */
short int cdb_keypad[FB_MAXFUNCTIONS];	/* keypad signal values */
short int cdb_functionkeys = 1;		/* toggle switch on function keys */
unsigned cdb_ualarm = 150000; 		/* ualarm call - 100000 == 1 second */

short int cdb_dbshell_ecommand = 1; 	/* toggle: allow dbshell env commands */

char *cdb_dbshell_pstring = NULL;	/* prompt string for dbshell */
char *cdb_dbshell_ploc = NULL;		/* prompt string location for dbshell */
short int cdb_dbshell_pilength = 15; 	/* length of prompt input in dbshell */
short int cdb_dbshell_checkmail = 60; 	/* time between checkmail in dbshell */

char *cdb_dbvedit_rec_ploc = NULL;	/* prompt location for rec level */
short int cdb_dbvedit_rec_pilength = 8; /* length, dbvedit, rec level */

char *cdb_dbvedit_cho_ploc = NULL;	/* prompt location for choice level */
short int cdb_dbvedit_cho_pilength = 10; /* length, dbvedit, choice level */
short int cdb_dbvedit_cho_firstline = 3; /* location of top line of display */
short int cdb_record_level = 1; 	/* record level on or off (what?) */
short int cdb_signature = 1;		/* toggle switch on -jb signature */
char *cdb_hitanykey = NULL;		/* to override HIT ANY KEY message */
char *cdb_ex_choicepause_msg = NULL;
char *cdb_choicepause_msg = NULL;
char *cdb_vipause_msg = NULL;

short int cdb_ex_fullkey = 0;		/* exchioce full key, with blanks */
short int cdb_ex_reverse = 1;		/* exchioce reverse video flag */
short int cdb_ex_forceone = 0;		/* exchioce force one when no others */
char *cdb_ccommand_shell = NULL;
char *cdb_dbshell_shell = NULL;

short int cdb_use_insert_char = 0;
short int cdb_sub_tty_rows = 25;	/* sub tty window rowXcol sizing */
short int cdb_sub_tty_cols = 80;

short int cdb_mergecols = 80;
short int cdb_screenprint_pause = 0;	/* pause before screenprint flag */

char *cdb_home = NULL;
char *cdb_lserver = NULL;
char *cdb_server = NULL;
char *cdb_lserverlog = NULL;
char *cdb_serverlog = NULL;
char *cdb_lservererr = NULL;
char *cdb_servererr = NULL;
char *cdb_lockderr = NULL;
char *cdb_logfile = NULL;
char *cdb_errorlog = NULL;
char *cdb_help = NULL;
char *cdb_menu = NULL;
char *cdb_passfile = NULL;
char *cdb_seqfile = NULL;
char *cdb_writedir = NULL;
char *cdb_HLP_EDCOM = "edcom.hlp";
char *cdb_HLP_EDFLD = "edfld.hlp";
char *cdb_HLP_XLONG = "xlong.hlp";

short int cdb_xit = 0;
long cdb_blocksize = 1024;		/* cdb_blocksize: fb_blockeach() */
short int cdb_edit_input = 0;		/* EDIT == 1, NON-EDIT (old) == 0 */

char *cdb_e_buf = NULL;			/* editable input variables */
short int cdb_e_st;

fb_field **cdb_sp;			/* screen fields - used by editors */
short int cdb_sfields = 0;

int cdb_umask = 0;
int cdb_record_umask = 0;

short int cdb_allow_links = 1;
short int cdb_debugmode = 0;
short int cdb_autodate_lines = 1;
short int cdb_no_acct = 0;
short int cdb_autobtree = 1;
short int cdb_fixedwidth = 0;
short int cdb_trap_xon = 0;
short int cdb_lockdaemon = 0;
short int cdb_use_rpc = 0;
short int cdb_t_irecsiz = 0;		/* talk betw. getd_ddict--geti_dict */
short int cdb_initscreen_done = 0;	/* used to determine if screen done */
short int cdb_opendb_level = 0;
short cdb_InfoLineToggle = 1;
fb_database *cdb_LastDbase = NULL;	/* used to prime fb_infoline */
short int cdb_error_row_offset = 1;
short int cdb_screrr_cx_writeflag = 1;
short int cdb_cgi_flag = 0;
short int cdb_dbase_byteorder = 0;	/* assume native endian */
short int cdb_cpu_byteorder = 0;	/* default to native endian */

/*
 * for years, these have been in fb.h, which did not promote quick
 * changes that would compile "soon". so, maybe keeing globals here,
 * though more bothersome in the short run, might promote better code.
 */

char *cdb_coname = NULL;		/* company name */
char *cdb_pgm = NULL;			/* program name */
char *cdb_user = NULL;			/* logged user name */
char *cdb_user_home = NULL;		/* $HOME of logged user name */
char *cdb_dbuser = NULL;		/* cdblogged user name */
char *cdb_work_dir = NULL;		/* working directory */
char *cdb_unixtype = NULL;		/* Unix type == SCO, INTERACTIVE, .. */

short int cdb_max_file_name = 7;	/* maximum file name length */
short int cdb_e_negative = 0;		/* negative sign flag */

short int cdb_setup_done = 0;		/* flag set after fb_setup() called */
char *cdb_dbase = NULL;
char *cdb_index = NULL;
char *cdb_screen = NULL;
char *cdb_view = NULL;
char *cdb_putfile = NULL;
char *cdb_runflags = NULL;
char *cdb_shell = NULL;
short int cdb_askgen = 0;
short int cdb_askwrite = 0;
short int cdb_autoregen = 0;
short int cdb_choosefield = 0;
short int cdb_error = 0;
short int cdb_forceautoincr = 0;
short int cdb_free_all_memory = 0;
short int cdb_returnerror = 0;
short int cdb_interrupt = 0;
short int cdb_localrc = 1;
short int cdb_locklevel = 0;
short int cdb_lockmessage = 2;
short int cdb_locktime = 60;
short int cdb_makeindex = 0;
short int cdb_menupause = 0;
short int cdb_menumatch = 0;
short int cdb_noaddrec = 0;
short int cdb_okstop = 0;
short int cdb_pgencols = 80;
short int cdb_reclog = 0;
short int cdb_regexp = 0;
short int cdb_reverse = 1;
short int cdb_rundbd = 1;
short int cdb_screenprint = 1;
short int cdb_skip_null_auto = 1;
short int cdb_standout = 1;
short int cdb_usrlog = 0;
short int cdb_vipause = 1;
short int cdb_viaddpause = 1;
short int cdb_wrapscan = 1;
short int cdb_secure = 0;		/* flag used by cdb, not settable! */
short int cdb_license = 0;		/* flag used by cdb, not settable! */
short int cdb_limited = 0;		/* flag used by cdb, not settable! */

long cdb_headsize = 24L;

long cdb_failrec = 0;			/* record number at failure point */
short int cdb_loadfail = 0;		/* flag forcing loading of failrec */
short int cdb_rpc_retries = 10;		/* for retrying rpc connect commands */

char *cdb_T_ALL =	"$ALL";			/* indexes */
char *cdb_T_BREAK =	"$BREAK";		/* break points (prts) */
char *cdb_T_DATE =	"$DATE";		/* meta default */
char *cdb_T_INCR =	"$INCR";		/* meta default */
char *cdb_T_NOT =	"$NOT";			/* indexes */
char *cdb_T_NONE =	"$NONE";		/* indexes */
char *cdb_T_PREV =	"$PREV";		/* meta default */
char *cdb_T_SBREAK=	"$SBREAK";		/* simple break */
char *cdb_T_TIME = 	"$TIME";		/* meta default */
char *cdb_T_MTIME = 	"$MTIME";		/* meta default - mil time */
char *cdb_T_TOTAL =	"$TOTAL";		/* total line (prts) */
char *cdb_T_NPREV = 	"$NPREV";		/* neg of prev */
char *cdb_T_USER = 	"$USER";		/* login user */
char *cdb_T_AUTOINCR = 	"$AUTOINCR";		/* autoincr flag */
char *cdb_AUTOMARK = 	"*";			/* autoincr marker */
char *cdb_T_PLUS = 	"$PLUS";		/* self increment */
char *cdb_T_MINUS =	"$MINUS";		/* self decrement */

char *cdb_T_LESSEQUAL =	"$LE";
char *cdb_T_GREATEREQUAL="$GE";
char *cdb_T_LESSTHAN = 	"$LT";
char *cdb_T_GREATERTHAN="$GT";
char *cdb_T_EQUAL = 	"$EQ";
char *cdb_T_NOTEQUAL =	"$NE";
char *cdb_T_PATTERN = 	"$PATTERN";
char *cdb_T_EMPTY = 	"$EMPTY";
char *cdb_T_DELETE = 	"$DELETE";

char *cdb_CN = 		".coname";		/* company name */
char *cdb_CSHELL_BASE =	".cshell";		/* cshell base name */
char *cdb_DIDX = 	"index";		/* default index name */
char *cdb_DCDB = 	"dbase";		/* default fb_database name */
char *cdb_FILTER_BASE =	".filter";		/* filter base name */
char *cdb_PASSFILE = 	".passwd";		/* password file */
char *cdb_PUTNAME = 	"putidx";		/* default put index name */
char *cdb_VISUAL_BASE =	".visual";		/* visual base name */
char *cdb_SCREEN_EXT = 	".sdict";
char *cdb_KEY = 	"X~|i%";

char *cdb_DBCLEAN =	"dbclean";
char *cdb_CGEN =	"dbcgen";
char *cdb_IGEN =	"dbigen";
char *cdb_LGEN =	"dblgen";
char *cdb_PGEN =	"dbpgen";
char *cdb_UGEN =	"dbugen";
char *cdb_EDIT =	"dbedit";
char *cdb_VEDIT =	"dbvedit";
char *cdb_DBMERGE =	"dbmerge";
char *cdb_DBVEMIT =	"dbvemit";
char *cdb_DBJOIN =	"dbjoin";
char *cdb_DBEMIT =	"dbemit";
char *cdb_VSCAN = 	"dbvscan";
char *cdb_DBVIEW = 	"dbview";
char *cdb_DBVI = 	"dbvi";
char *cdb_DBSHELL = 	"dbshell";

char *cdb_S_FREEC =	"\006";		/* record markers - FREE marker */
char cdb_FREEC =	'\006';
char *cdb_S_EOREC =	"\005";		/* end of record marker */
char cdb_EOREC =	'\005';
char *cdb_S_FILLC =	"\030";		/* fill character */
char cdb_FILLC =	'\030';
char *cdb_S_FILLE =	"\031";		/* fill area end marker */
char cdb_FILLE =	'\031';

/* extern struct passwd *getpwuid(uid_t uid); */

/*
 * setup_argv - provide a hook for initial setup from text files, and arg line.
 *	called from getargs to enable the argline constructs.
 *	rc lines are 'set variable setting', argline are XXX=YYY.
 */

   fb_setup_argv(argc, argv)
      int argc;
      char *argv[];
   
      {
         char dname[FB_MAXNAME], fname[FB_MAXNAME];
	 int n;

	 if (cdb_setup_done)
	    return(FB_AOK);
         fb_mkstr(&cdb_pgm, fb_basename(fname, argv[0]));
	 /*fb_serial();*/
	 e_init();
	 dname[0] = NULL;
	 if ((n = fb_testargs(argc, argv, "-d")) > 0){
            if (n + 1 >= argc){
               cdb_error = FB_BAD_DATA;
               if (cdb_returnerror)
                  return(FB_ERROR);
               fb_xerror(FB_MESSAGE, "Ill formed dbase (-d) argument", NIL);
               }
	    strcpy(fname, argv[n + 1]);
	    fb_dirname(dname, fname);
	    }
	 e_file(dname);
         fb_writeable_homefile(&cdb_seqfile, "SEQF");
         if (cdb_usrlog > 0)
            fb_writeable_homefile(&cdb_logfile, LOGFILE);
         if (cdb_lservererr == NULL)
            fb_writeable_homefile(&cdb_lservererr, "fblserver.err");
         if (cdb_servererr == NULL)
            fb_writeable_homefile(&cdb_servererr, "fbserver.err");
         if (cdb_lockderr == NULL)
            fb_writeable_homefile(&cdb_lockderr, "fblockd.err");
	 if (cdb_localrc)
	    e_argline(argc, argv);
         if (fb_cx_testcontrol())
            cdb_okstop = 0;
         setup_keyboard();
         if (q_init() == FB_ERROR)
            return(FB_ERROR);
	 cdb_setup_done = 1;
         return(FB_AOK);
      }

/*
 * setup - initial setup. called by opendb unless already done. rc files only.
 */

   fb_setup()
   
      {
	 if (cdb_setup_done)
	    return(FB_AOK);
	 /*fb_serial();*/
	 e_init();
	 e_file(NIL);
         fb_writeable_homefile(&cdb_seqfile, "SEQF");
         if (cdb_usrlog > 0)
            fb_writeable_homefile(&cdb_logfile, LOGFILE);
         if (cdb_lservererr == NULL)
            fb_writeable_homefile(&cdb_lservererr, "fblserver.err");
         if (cdb_servererr == NULL)
            fb_writeable_homefile(&cdb_servererr, "fbserver.err");
         if (cdb_lockderr == NULL)
            fb_writeable_homefile(&cdb_lockderr, "fblockd.err");
         if (fb_cx_testcontrol())
            cdb_okstop = 0;
         setup_keyboard();
         if (q_init() == FB_ERROR)
            return(FB_ERROR);
	 cdb_setup_done = 1;
         return(FB_AOK);
      }

   static setup_keyboard()
      {
         if (cdb_edit_input){
            if (fb_init_keyboard() == FB_ERROR){
               cdb_edit_input = 0;
               fb_serror(FB_MESSAGE,
                  "Warning: keyboard initialization failed.", NIL);
               sleep(3);
               }
            }
         if (cdb_e_buf == NULL){
	    cdb_e_buf = (char *) fb_malloc((unsigned) FB_MAXLINE+1);
            cdb_e_buf[0] = NULL;
            }
      }

/*
 * e_file -  process any existing .rc files.
 *	dname is the name of the fb_database directory. search there too.
 */

   static void e_file(dname)
      char *dname;
   
      {
	 int  p, i, fd;
	 char line[FB_MAXLINE], word[FB_MAXLINE], e_var[FB_MAXLINE];
         char *q, rcfile[FB_MAXNAME];

	 for (i = 1; i <= 3; i++){
	    rcfile[0] = NULL;
	    switch(i){
	       case 1:
                  q = NULL;
                  fb_homefile(&q, RCFILE);
                  strcpy(rcfile, q);
                  fb_free(q);
		  break;
	       case 2:
		  if (cdb_localrc)
		     fb_pathname(rcfile, RCFILE);
		  break;
	       case 3:
		  if (cdb_localrc){
		     strcpy(rcfile, dname);
		     strcat(rcfile, RCFILE);
                     }
	          break;
	       }
	    if (rcfile[0] == NULL)
	       continue;
	    if ((fd = open(rcfile, READ)) < 0)
               return;
            fb_ln_init(fd);
            while (fb_ln_load(line, FB_MAXLINE) != 0){
               p = fb_getword(line, 1, word);
               if (equal(word, SET)){
                  p = fb_getword(line, p, e_var);
                  word[0] = NULL;
                  p = fb_getword(line, p, word);
                  e_set(e_var, word, i);	/* i=1,2,3 - set origin/perm */
                  }
               }
            fb_ln_end();
	    }
      }

   static e_argline(argc, argv)		/* process an arg vector */
      int argc;
      char *argv[];

      {
	 int  i;
	 char word[FB_MAXLINE], e_var[FB_MAXLINE], *p;
	 
         for (i = 1; i < argc; i++){
	    if ((p = strchr(argv[i], '=')) != 0){
	       *p = NULL;
	       strcpy(e_var, argv[i]);
	       strcpy(word, p + 1);
	       e_set(e_var, word, 4);	/* 4 is permission, sort of */
	       *p = '=';
	       }
	    }
      }

   static e_init()		/* initialize the environment variables */
      {
	 char word[FB_MAXLINE], *p;
	 int i, save_use_rpc;
	 struct passwd *pw;

	 pw = getpwuid(getuid());
	 fb_mkstr(&cdb_user, pw->pw_name);

         p = getenv("HOME");
         if (p != NULL)
            fb_mkstr(&cdb_user_home, p);
         p = getenv(FIRSTBASEHOME);
         if (p != NULL)
            fb_mkstr(&cdb_home, p);
         else
            fb_mkstr(&cdb_home, cdb_DEFAULT_FIRSTBASE_HOME);
         fb_homefile(&cdb_help, "hlp/");
         fb_homefile(&cdb_menu, "menu/");
         fb_homefile(&cdb_passfile, ".passwd");
	 cdb_max_file_name = 7;
	 cdb_InfoLineToggle = 1;
         save_use_rpc = cdb_use_rpc;
         cdb_use_rpc = 0;
	 fb_getwd(word);
         cdb_use_rpc = save_use_rpc;
	 cdb_work_dir = NULL;
	 fb_mkstr(&cdb_work_dir, word);
	 cdb_putfile = NULL;
	 fb_mkstr(&cdb_putfile, cdb_PUTNAME);
	 cdb_wrapscan = 1;
         cdb_askwrite = 0;
	 if (cdb_locklevel >= 0)
            cdb_locklevel = 2;
         cdb_lockmessage = 2;
         cdb_locktime = 60;
         cdb_interrupt = 0;
	 cdb_shell = NULL;

	 fb_mkstr(&cdb_shell, BOURNE_SHELL);
	 fb_mkstr(&cdb_ccommand_shell, BOURNE_SHELL);
	 fb_mkstr(&cdb_dbshell_shell, BOURNE_SHELL);
	 cdb_runflags = NULL;
	 fb_mkstr(&cdb_runflags, " "); /* store a blank into the runflags */
	 cdb_makeindex = 0;
	 cdb_standout = 1;
	 cdb_reverse = 0;
	 cdb_askgen = 1;
	 cdb_regexp = 0;
	 cdb_refresh_clears = 0;
	 cdb_okstop = 0;
	 cdb_localrc = 1;
	 cdb_datebasis = 360;
	 cdb_centurymark = 50;
	 cdb_centurybase = NULL;
	 fb_mkstr(&cdb_centurybase, "19");
	 cdb_centurynext = NULL;
	 fb_mkstr(&cdb_centurynext, "20");
	 cdb_datedisplay = 8;
	 cdb_datestyle = 1;
	 cdb_decimal = 1;
	 cdb_choicepause = 0;
	 cdb_choiceaddpause = 0;
	 cdb_screenprint = 1;
	 cdb_screenprint_pause = 0;
	 cdb_ex_choicepause = 0;
	 cdb_ex_choiceaddpause = 0;
	 for (i = 0; i < FB_MAXFUNCTIONS; i++)
	    cdb_keypad[i] = 0;		/* all true signals are non-zero */
	 for (i = 0; i < MAX_CCOMMAND; i++){
	    cdb_ccommand[i] = NULL;
	    cdb_ccommand_prompt[i] = NULL;
	    }
	 fb_mkstr(&cdb_wdir_ccommand, "/tmp");
	 fb_mkstr(&cdb_tempdir, "/tmp");
         fb_homefile(&cdb_home_ccommand, "ccommand/");
	 fb_mkstr(&cdb_prompt_commandmsg, SYSMSG[S_HELP_END]);
	 fb_mkstr(&cdb_prompt_recordmsg, SYSMSG[S_REC_MSG]);
	 fb_mkstr(&cdb_prompt_choicemsg, SYSMSG[S_CHOICE_MSG1]);
	 fb_mkstr(&cdb_prompt_addmodemsg1, ADDMODE_MSG1);
	 fb_mkstr(&cdb_prompt_addmodemsg2, ADDMODE_MSG2);
	 fb_mkstr(&cdb_prompt_autofldmsg, AUTOFLDMSG);
	 fb_mkstr(&cdb_prompt_normalfldmsg, NORMALFLDMSG);
	 cdb_hitanykey = NULL;
         cdb_umask = 0;
      }

   static e_set(e_var, word, perm)	/* set environment variable to word */
      char *e_var, *word;
      int perm;				/* permission, or where val is from */

      {
         int n;
	 char *p;

	 if (equal(e_var, NAME_LENGTH))
	    cdb_max_file_name = atoi(word);
         else if (equal(e_var, CPU_BYTEORDER)){
            cdb_cpu_byteorder = atoi(word);
            if (cdb_cpu_byteorder != 1234 && cdb_cpu_byteorder != 4321 &&
                  cdb_cpu_byteorder != 0)
               cdb_cpu_byteorder = 4321;
            }
         else if (equal(e_var, DBASE_BYTEORDER)){
            cdb_dbase_byteorder = atoi(word);
            if (cdb_dbase_byteorder != 1234 && cdb_dbase_byteorder != 4321 &&
                  cdb_dbase_byteorder != 0)
               cdb_dbase_byteorder = 4321;
            }
	 else if (equal(e_var, DATEBASIS)){
	    cdb_datebasis = atoi(word);
	    if (cdb_datebasis != 360 && cdb_datebasis != 365)
	       cdb_datebasis = 360;
	    }
	 else if (equal(e_var, DATEDISPLAY)){
	    cdb_datedisplay = atoi(word);
	    if (cdb_datedisplay != 8 && cdb_datedisplay != 10
                  && cdb_datedisplay != 11)
	       cdb_datedisplay = 8;
	    }
	 else if (equal(e_var, DATESTYLE)){
	    cdb_datestyle = atoi(word);
	    if (cdb_datestyle < 1 || cdb_datestyle > 2)
	       cdb_datestyle = 1;
	    }
	 else if (equal(e_var, CENTURYMARK))
	    cdb_centurymark = atoi(word);
	 else if (equal(e_var, CENTURYBASE))
	    fb_mkstr(&cdb_centurybase, word);
	 else if (equal(e_var, CENTURYNEXT))
	    fb_mkstr(&cdb_centurynext, word);
	 else if (equal(e_var, CDBLSERVER_VAR))
	    fb_mkstr(&cdb_lserver, word);
	 else if (equal(e_var, CDBSERVER_VAR))
	    fb_mkstr(&cdb_server, word);
	 else if (equal(e_var, CDBSERVERLOG))
	    fb_mkstr(&cdb_serverlog, word);
	 else if (equal(e_var, CDBLSERVERLOG))
	    fb_mkstr(&cdb_lserverlog, word);
	 else if (equal(e_var, CDBLSERVERERR))
	    fb_mkstr(&cdb_lservererr, word);
	 else if (equal(e_var, CDBSERVERERR))
	    fb_mkstr(&cdb_servererr, word);
	 else if (equal(e_var, CDBLOCKDERR))
	    fb_mkstr(&cdb_lockderr, word);
	 else if (equal(e_var, NEGATIVE))
	    cdb_e_negative = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, ASKGEN))
	    cdb_askgen = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, ASKWRITE))
	    cdb_askwrite = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, LOCALRC))
	    cdb_localrc = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, OKSTOP))
	    cdb_okstop = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, REFRESH_CLEARS))
	    cdb_refresh_clears = equal(word, SYSMSG[S_ON]) ? 1:0;
#if RPC
	 else if (equal(e_var, LOCKDAEMON))
	    cdb_lockdaemon = equal(word, SYSMSG[S_ON]) ? 1 : 0;
#endif /* RPC */
	 else if (equal(e_var, LOCKLEVEL) && cdb_locklevel >= 0){
	    cdb_locklevel = atoi(word);
	    if (cdb_locklevel < 0 || cdb_locklevel > 3)
	       cdb_locklevel = 2;
	    }
	 else if (equal(e_var, LOCKMESSAGE)){
	    cdb_lockmessage = atoi(word);
	    if (cdb_lockmessage < 0 || cdb_lockmessage > 2)
	       cdb_lockmessage = 2;
	    }
	 else if (equal(e_var, LOCKTIME))
	    cdb_locktime = atoi(word);
	 else if (equal(e_var, CHOOSEFIELD))
	    cdb_choosefield = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, CHOICEPAUSE))
	    cdb_choicepause = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, CHOICEPAUSE_MSG)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_choicepause_msg, word);
	    }
	 else if (equal(e_var, CHOICEADDPAUSE))
	    cdb_choiceaddpause = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, EX_CHOICEPAUSE))
	    cdb_ex_choicepause = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, EX_CHOICEPAUSE_MSG)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_ex_choicepause_msg, word);
	    }
	 else if (equal(e_var, EX_CHOICEADDPAUSE))
	    cdb_ex_choiceaddpause = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, EX_FULLKEY))
	    cdb_ex_fullkey = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, EX_REVERSE))
	    cdb_ex_reverse = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, EX_FORCEONE))
	    cdb_ex_forceone = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, MENUPAUSE))
	    cdb_menupause = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, NOADDREC))
	    cdb_noaddrec = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, INFOLINE))
	    cdb_InfoLineToggle = equal(word, SYSMSG[S_ON]) ? 1 : -1;
	 else if (equal(e_var, VIPAUSE))
	    cdb_vipause = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, VIPAUSE_MSG)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_vipause_msg, word);
	    }
	 else if (equal(e_var, VIADDPAUSE))
	    cdb_viaddpause = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, REGEXP))
	    cdb_regexp = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, RECLOG))
	    cdb_reclog = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, USRLOG) && perm == 1)
	    cdb_usrlog = atoi(word);
	 else if (equal(e_var, ERRORLOG))
	    fb_mkstr(&cdb_errorlog, word);
	 else if (equal(e_var, DIRNAME)){
	    if (!equal(word, SYSMSG[S_ON])){
	       fb_mkstr(&cdb_work_dir, SYSMSG[S_STRING_BLANK]);
	       }
	    }
	 else if (equal(e_var, PUTFILE))
	    fb_mkstr(&cdb_putfile, word);
	 else if (equal(e_var, UMASK))
            sscanf(word, "%o", &cdb_umask);
	 else if (equal(e_var, RECORD_UMASK))
            sscanf(word, "%o", &cdb_record_umask);
	 else if (equal(e_var, AUTODATE_LINES))
	    cdb_autodate_lines = atoi(word);
	 else if (equal(e_var, PGENCOLS))
	    cdb_pgencols = atoi(word);
	 else if (equal(e_var, MERGECOLS))
	    cdb_mergecols = atoi(word);
	 else if (equal(e_var, AUTOBTREE))
	    cdb_autobtree = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, WRAPSCAN))
	    cdb_wrapscan = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, MENUMATCH))
	    cdb_menumatch = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, RUNDBD))
	    cdb_rundbd = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCREENPRINT))
	    cdb_screenprint = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCREENPRINT_PAUSE))
	    cdb_screenprint_pause = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, INTERRUPT))
	    cdb_interrupt = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, RUNFLAGS))
	    fb_mkstr(&cdb_runflags, word);
	 else if (equal(e_var, DBASE))
	    fb_mkstr(&cdb_dbase, word);
	 else if (equal(e_var, DINDEX))
	    fb_mkstr(&cdb_index, word);
	 else if (equal(e_var, SCREEN))
	    fb_mkstr(&cdb_screen, word);
	 else if (equal(e_var, VIEW))
	    fb_mkstr(&cdb_view, word);
	 else if (equal(e_var, SHELL)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_shell, word);
	    }
	 else if (equal(e_var, CCOMMAND_SHELL)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_ccommand_shell, word);
	    }
	 else if (equal(e_var, DBSHELL_SHELL)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_dbshell_shell, word);
	    }
	 else if (equal(e_var, MAKEINDEX))
	    cdb_makeindex = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, CDB_STANDOUT))
	    cdb_standout = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, TEMPDIR))
	    fb_mkstr(&cdb_tempdir, word);
	 else if (equal(e_var, CDB_REVERSE))
	    cdb_reverse = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, FORCEAUTOINCR))
	    cdb_forceautoincr = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, FIXEDWIDTH))
	    cdb_fixedwidth = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, TRAP_XON))
	    cdb_trap_xon = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, DECIMAL))
	    cdb_decimal = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SHOWRECCNT))
	    cdb_showreccnt = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, HOME_CCOMMAND))
	    fb_mkstr(&cdb_home_ccommand, word);
	 else if (equal(e_var, WDIR_CCOMMAND))
	    fb_mkstr(&cdb_wdir_ccommand, word);
	 else if (strncmp(e_var, CCOMMAND_PROMPT, 15) == 0){
	    n = atoi(e_var + 15);
	    if (n >= 0 && n < MAX_CCOMMAND){
	       fb_underscore(word, 0);
	       fb_mkstr(&(cdb_ccommand_prompt[n]), word);
	       }
	    }
	 else if (strncmp(e_var, CCOMMAND, 8) == 0){
	    n = atoi(e_var + 8);
	    if (n >= 0 && n < MAX_CCOMMAND)
	       fb_mkstr(&(cdb_ccommand[n]), word);
	    }
	 else if (strncmp(e_var, KEYPAD, 6) == 0){
	    p = e_var + 6;
	    if (*p != '_'){
	       n = atoi(p);
	       if (n >= 0 && n < 10)
		  cdb_keypad[n] = interpret_signal(word);
	       }
	    else{
	       p++;
	       if (equal(p, UP))
		  cdb_keypad[FB_KEY_UP] = interpret_signal(word);
	       else if (equal(p, DOWN))
		  cdb_keypad[FB_KEY_DOWN] = interpret_signal(word);
	       else if (equal(p, LEFT))
		  cdb_keypad[FB_KEY_LEFT] = interpret_signal(word);
	       else if (equal(p, RIGHT))
		  cdb_keypad[FB_KEY_RIGHT] = interpret_signal(word);
	       else if (equal(p, TAB))
		  cdb_keypad[FB_KEY_TAB] = interpret_signal(word);
	       else if (equal(p, K_ESCAPE))
		  cdb_keypad[FB_KEY_ESCAPE] = interpret_signal(word);
	       }
	    }
	 else if (equal(e_var, PROMPT_COMMANDMSG)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_prompt_commandmsg, word);
	    }
	 else if (equal(e_var, PROMPT_RECORDMSG)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_prompt_recordmsg, word);
	    }
	 else if (equal(e_var, PROMPT_CHOICEMSG)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_prompt_choicemsg, word);
	    }
	 else if (equal(e_var, PROMPT_ADDMODE1)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_prompt_addmodemsg1, word);
	    }
	 else if (equal(e_var, PROMPT_ADDMODE2)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_prompt_addmodemsg2, word);
	    }
	 else if (equal(e_var, PROMPT_AUTOFIELD)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_prompt_autofldmsg, word);
	    }
	 else if (equal(e_var, PROMPT_NORMALFIELD)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_prompt_normalfldmsg, word);
	    }
	 else if (equal(e_var, SCR_AUTOINCR))
	    cdb_scr_autoincr = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCR_HEADER))
	    cdb_scr_header = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCR_INFOLINE))
	    cdb_scr_fb_infoline = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCR_INFOLINE_SOLID))
	    cdb_scr_fb_infoline_solid = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCR_STATUS))
	    cdb_scr_status = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCR_STATUS_MASK))
	    cdb_scr_status_mask = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCR_VERSION_MASK))
	    cdb_scr_version_mask = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCR_TOOLNAME))
	    cdb_scr_toolname = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCR_INPUTDOTS))
	    cdb_scr_inputdots = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCR_INPUTCLEAR))
	    cdb_scr_inputclear = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCR_INPUTPASTEDGE))
	    cdb_scr_inputpastedge = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCR_STAT2))
	    cdb_scr_stat2 = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCR_HELP))
	    cdb_scr_help = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SCR_LABEL))
	    cdb_scr_label = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, FUNCTIONKEYS))
	    cdb_functionkeys = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, K_UALARM))
	    cdb_ualarm = (unsigned) atoi(word);
	 else if (equal(e_var, DBSHELL_ECOMMAND))
	    cdb_dbshell_ecommand = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, DBSHELL_PSTRING)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_dbshell_pstring, word);
	    }
	 else if (equal(e_var, DBSHELL_PLOC)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_dbshell_ploc, word);
	    }
	 else if (equal(e_var, DBSHELL_PILENGTH))
	    cdb_dbshell_pilength = atoi(word);
	 else if (equal(e_var, DBSHELL_CHECKMAIL))
	    cdb_dbshell_checkmail = atoi(word);
	 else if (equal(e_var, DBVEDIT_REC_PLOC)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_dbvedit_rec_ploc, word);
	    }
	 else if (equal(e_var, DBVEDIT_REC_PILENGTH))
	    cdb_dbvedit_rec_pilength = atoi(word);
	 else if (equal(e_var, DBVEDIT_CHO_PLOC)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_dbvedit_cho_ploc, word);
	    }
	 else if (equal(e_var, DBVEDIT_CHO_PILENGTH))
	    cdb_dbvedit_cho_pilength = atoi(word);
	 else if (equal(e_var, DBVEDIT_CHO_FIRSTLINE))
	    cdb_dbvedit_cho_firstline = atoi(word);
	 else if (equal(e_var, RECORD_LEVEL))
	    cdb_record_level = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SIGNATURE))
	    cdb_signature = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, HITANYKEY)){
	    fb_underscore(word, 0);
	    fb_mkstr(&cdb_hitanykey, word);
	    }
	 else if (equal(e_var, USE_INSERT_CHAR))
	    cdb_use_insert_char = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, SUB_TTY_ROWS))
	    cdb_sub_tty_rows = atoi(word);
	 else if (equal(e_var, SUB_TTY_COLS))
	    cdb_sub_tty_cols = atoi(word);
	 else if (equal(e_var, BLOCKSIZE))
	    cdb_blocksize = atol(word);
	 else if (equal(e_var, EDITINPUT))
	    cdb_edit_input = equal(word, SYSMSG[S_ON]) ? 1 : 0;
	 else if (equal(e_var, WRITEDIR))
	    fb_mkstr(&cdb_writedir, word);
	 else if (equal(e_var, UNIXTYPE))
	    fb_mkstr(&cdb_unixtype, word);
	 else if (equal(e_var, SKIP_NULL_AUTO))
	    cdb_skip_null_auto = equal(word, SYSMSG[S_ON]) ? 1 : 0;
      }

   static interpret_signal(s)
      char *s;
      
      {
         if (equal(s, CDB_SIG_END))
	    return(FB_END);
         else if (equal(s, CDB_SIG_ABORT))
	    return(FB_ABORT);
         else if (equal(s, CDB_SIG_HELP))
	    return(FB_QHELP);
         else if (equal(s, CDB_SIG_PAGEUP))
	    return(FB_PAGEUP);
         else if (equal(s, CDB_SIG_PAGEDOWN))
	    return(FB_PAGEDOWN);
         else if (equal(s, CDB_SIG_NEXT))
	    return(FB_ESIGNAL);
         else if (equal(s, CDB_SIG_PREV))
	    return(FB_YSIGNAL);
         else if (equal(s, CDB_SIG_PRINT))
	    return(FB_PSIGNAL);
         else if (equal(s, CDB_SIG_DEFAULT))
	    return(FB_DSIGNAL);
         else if (equal(s, CDB_SIG_CLEARFIELD))
	    return(FB_CSIGNAL);
         else if (equal(s, CDB_SIG_WRITEREC))
	    return(FB_WSIGNAL);
         else if (equal(s, CDB_SIG_DELETEREC))
	    return(FB_DELSIGNAL);
	 else
	    return(0);
      }

   fb_homefile(addr, base)
      char **addr, *base;

      {
         char fname[FB_MAXNAME];

         strcpy(fname, cdb_home);
         fb_assure_slash(fname);
         strcat(fname, base);
         fb_mkstr(addr, fname);
      }

   fb_writeable_homefile(addr, base)
      char **addr, *base;

      {
         char fname[FB_MAXNAME];

         if (cdb_writedir == NULL)
            strcpy(fname, cdb_home);
         else
            strcpy(fname, cdb_writedir);
         fb_assure_slash(fname);
         strcat(fname, base);
         fb_mkstr(addr, fname);
      }

   fb_assure_slash(fname)
      char *fname;

      {
         int j;

         j = strlen(fname);
         if (fname[j - 1] != CHAR_SLASH){
            fname[j] = CHAR_SLASH;
            fname[j + 1] = NULL;
            }
      }

   fb_vfwd()

      {
#if !RPC
	return(FB_ERROR);
#else /* RPC */
         int st = 0;
         char command[FB_MAXLINE], **result, user[FB_MAXLINE], *p;
         char buf[FB_MAXLINE], lbuf[FB_MAXLINE], hostid[FB_MAXLINE];
         struct passwd *s_pwd;

         s_pwd = getpwuid(getuid());
         strcpy(user, s_pwd->pw_name);
         endpwent();
         if (!equal(cdb_lserver, oserver)){
            cl = clnt_create(cdb_lserver, CDBSERVERPROG, CDBSERVERVERS, "tcp");
            strcpy(oserver, cdb_lserver);
            }
         if (cl == NULL){
            clnt_pcreateerror(cdb_lserver);
            return(FB_ERROR);
            }
         command[0] = NULL;
         sprintf(command, EXSTR, SC_VFYLIC, user);
         p = command;
         result = fb_lserver_1(&p, cl);
         if (result == NULL){
            clnt_perror(cl, cdb_lserver);
            return(FB_ERROR);
            }
         if (**result == NULL)
            return(FB_ERROR);
         /* else decode the license and see if its FB_AOK */
         strcpy(buf, *result);
         p = strchr(buf, ',');
         if (p == 0)
            return(FB_ERROR);
         *p++ = NULL;
         strcpy(hostid, buf);
         fb_pad(lbuf, p, FWD_RECLEN - 2);
         strcat(lbuf, "*");
         st = fb_loadfwd(cdb_v_f, lbuf);
         if (st != FB_AOK)
            return(FB_ERROR);
         sprintf(cdb_v_fid, "%s_%03d", hostid, cdb_v_f->dbfw_fid);
         st = fb_cf(cdb_v_fid, cdb_v_f->dbfw_magic, 1);
         if (!st)
            return(FB_ERROR);
         return(FB_AOK);
#endif /* RPC */
      }

#undef system
/*
 * q_init - init of some license stuff, and the BIGE detection
 */

   static q_init()
      {
         char *p;
         int ffn = 0, st = FB_AOK;
         long bige;
         char *fname;
         int fd;

         if (cdb_cpu_byteorder == 0 || cdb_dbase_byteorder == 0){
            fname = NULL;
            fb_homefile(&fname, "BIGE");
            fd = open(fname, 0);
            if (fd > 0){
               read(fd, (char *) &bige, FB_SLONG);
               if (bige != 4321)
                  bige = 1234;
               if (cdb_cpu_byteorder == 0)
                  cdb_cpu_byteorder = (short int) bige;
               if (cdb_dbase_byteorder == 0)
                  cdb_dbase_byteorder = (short int) bige;
               close(fd);
               }
            fb_free(fname);
            }
         p = getenv(FIXEDNODE);
         if (p != NULL)
            ffn = 1;		/* force fixed node if env exists */
#if RPC
         if (equal(cdb_pgm, "dbshell") && cdb_lserver != NULL && !ffn){
            cdb_license = 0;
               /*
                * firstbase is no longer licensed. -cc 06/18/00
                *
                *  st = system("fbgetlicense");
                *  if (st == 1)
                *     exit(st);
                *  else if (st == 0){
                *     cdb_license = 1;
                *     st = FB_AOK;
                *     }
                */
            cdb_license = 1;
            st = FB_AOK;
            /* if st is 2, then a license was granted to some other process */
            }
#endif /* RPC */
         /* check the license and set secure if needed */
         if (!cdb_xit){
            if (cdb_lserver != NULL && !ffn){
               /*
                * firstbase is no longer licensed. -cc 06/18/00
                *
                * if (fb_vfwd() != FB_AOK)
                *   fb_xerror(FB_MESSAGE, CONTACT, NIL);
                */
               /* check here for cdb_secure */
               if (access("/fbetc", 0) == 0)
                  cdb_secure = 1;
               }
           /* mmc was being called here */
            }
         if (cdb_secure)
            cdb_headsize = 34L;
#if RPC
         if (cdb_use_rpc){
            st = fb_clnt_create();
            if (st != FB_AOK){
               cdb_error = FB_RPC_ERROR;
               if (cdb_returnerror)
                  return(FB_ERROR);
               fb_xerror(FB_RPC_ERROR, "Cannot create client", NIL);
               }
            }
#endif /* RPC */
         return(st);
      }

   fb_set_loadfail(flag)
      int flag;

      {
         int st = FB_AOK;

         cdb_loadfail = flag;
#if RPC
         if (cdb_use_rpc)
            st = fb_tune_clnt(cdb_loadfail);
#endif /* RPC */
         return(st);
      }

/*
 * fb_setup_exit - frees all memory allocated by FirstBase setup mechanism
 */

   fb_setup_exit()
      {
         int i;

         fb_free_keyboard();
	 for (i = 0; i < MAX_CCOMMAND; i++){
	    fb_free(cdb_ccommand[i]);
	    fb_free(cdb_ccommand_prompt[i]);
	    }
         fb_free(cdb_centurybase);
         fb_free(cdb_centurynext);
         fb_free(cdb_wdir_ccommand);
         fb_free(cdb_home_ccommand);
         fb_free(cdb_prompt_commandmsg);
         fb_free(cdb_prompt_recordmsg);
         fb_free(cdb_prompt_choicemsg);
         fb_free(cdb_prompt_addmodemsg1);
         fb_free(cdb_prompt_addmodemsg2);
         fb_free(cdb_prompt_autofldmsg);
         fb_free(cdb_prompt_normalfldmsg);
         fb_free(cdb_tempdir);
         fb_free(cdb_dbshell_pstring);
         fb_free(cdb_dbshell_ploc);
         fb_free(cdb_dbvedit_rec_ploc);
         fb_free(cdb_dbvedit_cho_ploc);
         fb_free(cdb_hitanykey);
         fb_free(cdb_ex_choicepause_msg);
         fb_free(cdb_choicepause_msg);
         fb_free(cdb_vipause_msg);
         fb_free(cdb_ccommand_shell);
         fb_free(cdb_dbshell_shell);
         fb_free(cdb_home);
         fb_free(cdb_lserver);
         fb_free(cdb_server);
         fb_free(cdb_lserverlog);
         fb_free(cdb_serverlog);
         fb_free(cdb_lservererr);
         fb_free(cdb_servererr);
         fb_free(cdb_lockderr);
         fb_free(cdb_logfile);
         fb_free(cdb_errorlog);
         fb_free(cdb_help);
         fb_free(cdb_menu);
         fb_free(cdb_passfile);
         fb_free(cdb_seqfile);
         fb_free(cdb_writedir);
         fb_free(cdb_unixtype);
         fb_free(cdb_coname);
         fb_free(cdb_pgm);
         fb_free(cdb_user);
         fb_free(cdb_user_home);
         fb_free(cdb_dbuser);
         fb_free(cdb_work_dir);
         fb_free(cdb_dbase);
         fb_free(cdb_index);
         fb_free(cdb_screen);
         fb_free(cdb_view);
         fb_free(cdb_putfile);
         fb_free(cdb_runflags);
         fb_free(cdb_shell);
      }
