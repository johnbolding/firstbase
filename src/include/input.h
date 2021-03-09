/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: input.h,v 9.0 2001/01/09 02:56:12 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* input types */
#define FB_ALPHA   	CHAR_a     	/* actually alphanumeric */
#define FB_BINARY   	CHAR_b
#define FB_CHOICE	CHAR_c
#define FB_DATE    	CHAR_d
#define FB_DOCUMENT 	CHAR_D
#define FB_DOLLARS 	CHAR_DOLLAR
#define FB_FLOAT	CHAR_f
#define FB_FORMULA 	CHAR_F
#define FB_INTEGER 	CHAR_i
#define FB_LINK		CHAR_L
#define FB_LONG    	CHAR_l
#define FB_NUMERIC 	CHAR_n
#define FB_POS_NUM	CHAR_N
#define FB_STRICTALPHA 	CHAR_A
#define FB_TIME		CHAR_t
#define FB_UPPERCASE	CHAR_U
#define FB_EXCHOICE	CHAR_X
#define FB_SILENTCHOICE	CHAR_C
#define FB_SLASH_NUMERIC CHAR_S

/* editing keys for input routine */
#define FB_BACKSPACE 	'\010'	/* ^H */
#define FB_BACKWORD 	'\027' 	/* ^W for back-word ala csh */
#define FB_ENDKEY 	'-'	/* standard CDB END key */
#define FB_ERASE 	'\025'	/* ^U */
#define FB_HELPKEY 	'\010'	/* also control H */
#define FB_KILL		'\030'	/* ^X -- ABORT */
#define FB_PANICKEY 	'\007'	/* ^G */
#define FB_RETURN	'\012'	/* enter or carriage return (^j) */
#define FB_CRET		'\015'	/* true carriage return in raw mode (^m) */
#define FB_RUBOUT	'\177'	/* rubout/del  */
#define FB_SPACE 	' '
#define FB_ESCAPE	'\033'

/* 
 * some input signals
 *     -- these really have no meaning other than to be useful as macros
 */

/* these are returned from input() */
#define FB_DSIGNAL 	'\004'	/* ^D */
#define FB_ESIGNAL	'\005'	/* ^E */
#define FB_YSIGNAL	'\031'	/* ^Y */
#define FB_REDRAW1 	'\014'	/* ^L */
#define FB_REDRAW2 	'\022'	/* ^R */
#define FB_KILL2	'\003'	/* ^C */
#define FB_ESIGNAL2 	'\016'	/* ^N */
#define FB_ESIGNAL3	'\011'	/* ^I */
#define FB_YSIGNAL2 	'\020'	/* ^P */
#define FB_FSIGNAL 	'\006'	/* ^F */
#define FB_BSIGNAL 	'\002'	/* ^B */
#define FB_SSIGNAL 	'\023'	/* ^S */
#define FB_QSIGNAL 	'\021'	/* ^Q */
#define FB_SSIGNAL1	'\037'	/* ^_ (^/) */
#define FB_RSIGNAL	'\034'	/* ^\ */
#define FB_VSIGNAL	'\026'	/* ^V */

#define FB_AOK 		1
#define FB_DEFAULT 	-2
#define FB_ERROR 	-1
#define FB_ABORT 	-3
#define FB_QHELP 	-4
#define FB_END 		-5
#define FB_PAGEUP	-6
#define FB_PAGEDOWN	-7
#define FB_ARROWUP	-8
#define FB_ARROWDOWN	-9
#define FB_ARROWLEFT	-10
#define FB_ARROWRIGHT	-11
/* #define FB_NEXT	-12 -- really ESIGNAL */
/* #define FB_PREV	-13 -- really YSIGNAL */
#define FB_PSIGNAL	-14
#define FB_CSIGNAL	-15
#define FB_WSIGNAL	-16
#define FB_DELSIGNAL	-17

#define FB_DELETED 	-18
#define FB_UNDELETED 	-19
#define FB_PUT 		-20
#define FB_XLONG 	-21
#define FB_LOCKED_ERROR	-22
#define FB_RPC_ERROR	-23

#define FB_ESCAPE_AOK 	-24
#define FB_ESCAPE_END 	-25

/* these are passed into input */
#define FB_ECHO		1
#define FB_NOECHO    	2

#define FB_CONFIRM   	1
#define FB_NOCONFIRM 	2

#define FB_OKEND     	1
#define FB_NOEND     	2

/* input constants */
#define FB_MAXINPUT 	80	/* maximum input size */
