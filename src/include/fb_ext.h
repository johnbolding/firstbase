/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: fb_ext.h,v 9.0 2001/01/09 02:56:12 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* 
 *  FirstBase global variables: included for any external use.
 *     see fb_vars.h for detailed description.
 */

extern fb_database *cdb_db;
extern fb_field **cdb_keymap, **cdb_keyindx, **cdb_kp, **cdb_ip;

extern char *cdb_afld, *cdb_bfld;

extern fb_link *cdb_linktop;

extern int cdb_fieldlength;
extern short int cdb_t_lines;
extern short int cdb_t_cols;

extern short int cdb_batchmode;
extern short int cdb_yesmode;

#if SYS_V
extern char *cdb_Re_pat;
#endif

extern char cdb_nil[], *NIL;

extern char CHAR_DOLLAR;
extern char CHAR_AT;
extern char CHAR_BACKSLASH;
extern char CHAR_BANG;
extern char CHAR_BLANK;
extern char CHAR_COLON;
extern char CHAR_COMMA;
extern char CHAR_DOT;
extern char CHAR_G_THAN;
extern char CHAR_L_THAN;
extern char CHAR_MINUS;
extern char CHAR_NEWLINE;
extern char CHAR_PERCENT;
extern char CHAR_PLUS;
extern char CHAR_SLASH;
extern char CHAR_STAR;
extern char CHAR_QUESTION;
extern char CHAR_QUOTE;
extern char CHAR_UNDERSCORE;
extern char CHAR_0;
extern char CHAR_1;
extern char CHAR_A;
extern char CHAR_C;
extern char CHAR_D;
extern char CHAR_F;
extern char CHAR_L;
extern char CHAR_N;
extern char CHAR_S;
extern char CHAR_Y;
extern char CHAR_U;
extern char CHAR_X;
extern char CHAR_Z;
extern char CHAR_a;
extern char CHAR_b;
extern char CHAR_c;
extern char CHAR_d;
extern char CHAR_f;
extern char CHAR_h;
extern char CHAR_i;
extern char CHAR_l;
extern char CHAR_n;
extern char CHAR_r;
extern char CHAR_s;
extern char CHAR_t;
extern char CHAR_u;
extern char CHAR_v;
extern char CHAR_w;
extern char CHAR_x;
extern char CHAR_y;
extern char CHAR_z;
