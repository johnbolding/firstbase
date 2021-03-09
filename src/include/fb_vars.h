/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: fb_vars.h,v 9.0 2001/01/09 02:56:12 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* 
 * FirstBase global variables: included for any use.
 *    fb_keymap and fb_keyindx are allocated on the fly
 *       in getd_dict and geti_dict.
 *
 *    fb_keymap is used for the fields of the database. (size is calculated).
 *    fb_kp will point to base of keymap, making them synonamous.
 *    fb_keyindex is used for all flat indexes (btrees are more internal)
 *    fb_ip points to base of keyindx, making them synonamous.
 *    fb_db is the global database pointer, allocated in getargs!
 *    (note that fb_kp[fb_db->nfields] is the del marker.)
 *    fb_db->dbase, fb_db->ddict, fb_db->idict, fb_db->dindex fb_db->dmap
 *    all hold (after getargs) full pathnames for the indicated file.
 *    fb_coname holds string printed in many program headers.
 *    fb_pgm has the name of the invoking program, no frills.
 */
 
fb_database *cdb_db = NULL;		/* header database (hp) */
fb_field 
      **cdb_keymap = NULL,		/* database field pointers */
      **cdb_keyindx  = NULL,		/* index field pointers */
      **cdb_kp = NULL, 			/* points to kaymap (later) */
      **cdb_ip = NULL;			/* points to keyindx */

fb_link *cdb_linktop = NULL;		/* top of link list of fields */

char *cdb_afld = NULL, 			/* alloc of fld lines */
     *cdb_bfld = NULL;

short int cdb_t_lines = 0, cdb_t_cols = 0;	/* screen size variables */
int cdb_fieldlength = 0;		/* max length of fields */

short int cdb_batchmode = 0;		/* cdb_batchmode flag */
short int cdb_yesmode = 0;		/* cdb_yesmode flag */

#if SYS_V
char *cdb_Re_pat;			/* sys_v storage for compiled RE */
#endif

char cdb_nil[1] = {NULL}, *NIL = cdb_nil;/* a true NULL pointer */

char CHAR_DOLLAR = 	'$';
char CHAR_AT = 		'@';
char CHAR_BACKSLASH = 	'\\';
char CHAR_BANG = 	'!';
char CHAR_BLANK = 	' ';
char CHAR_COLON = 	':';
char CHAR_COMMA = 	',';
char CHAR_DOT = 	'.';
char CHAR_G_THAN = 	'>';
char CHAR_L_THAN = 	'<';
char CHAR_MINUS = 	'-';
char CHAR_NEWLINE = 	'\n';
char CHAR_PERCENT = 	'%';
char CHAR_PLUS = 	'+';
char CHAR_SLASH = 	'/';
char CHAR_STAR = 	'*';
char CHAR_QUESTION = 	'?';
char CHAR_QUOTE = 	'"';
char CHAR_UNDERSCORE =	'_';
char CHAR_0 = '0';
char CHAR_1 = '1';
char CHAR_A = 'A';
char CHAR_C = 'C';
char CHAR_D = 'D';
char CHAR_F = 'F';
char CHAR_L = 'L';
char CHAR_N = 'N';
char CHAR_S = 'S';
char CHAR_Y = 'Y';
char CHAR_U = 'U';
char CHAR_X = 'X';
char CHAR_Z = 'Z';
char CHAR_a = 'a';
char CHAR_b = 'b';
char CHAR_c = 'c';
char CHAR_d = 'd';
char CHAR_f = 'f';
char CHAR_h = 'h';
char CHAR_i = 'i';
char CHAR_l = 'l';
char CHAR_n = 'n';
char CHAR_r = 'r';
char CHAR_s = 's';
char CHAR_t = 't';
char CHAR_u = 'u';
char CHAR_v = 'v';
char CHAR_w = 'w';
char CHAR_x = 'x';
char CHAR_y = 'y';
char CHAR_z = 'z';

