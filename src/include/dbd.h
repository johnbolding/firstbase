/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: dbd.h,v 9.0 2001/01/09 02:56:07 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* dbd header file - used for many dbd dictionary editor programs */

#define PAGEF 		'f'
#define PAGEB 		'b'
#define ADDMODE 	'@'
#define DELMODE 	'd'
#define INSERTMODE 	'i'
#define HELPMODE 	'?'
#define SORTBYMODE 	's'
#define MINFOMODE	'm'	/* miscellaneous screen fits better */
#define TRACEMODE	't'

#define I_ALPHA   	'a'    	/* actually alphanumeric */
#define I_BINARY	'b'
#define I_CHOICE	'c'
#define I_CCHOICE	'C'
#define I_DATE    	'd'
#define I_DOCUMENT 	'D'
#define I_DOLLARS 	'$'
#define I_EXCHOICE	'X'
#define I_FLOAT		'f'
#define I_FORMULA 	'F'
#define I_INTEGER 	'i'
#define I_LONG    	'l'
#define I_LINK    	'L'
#define I_NUMERIC 	'n'
#define I_POS_NUM	'N'
#define I_STRICTALPHA 	'A'
#define I_TIME		't'
#define I_YESNO		'y'
#define I_UPPERCASE	'U'

typedef struct fb_s_upd fb_upd;
struct fb_s_upd {		/* update structure - */
   fb_field *fp;		/* field pointer to update */
   char *fv;			/* macro or formula to use */
   };
