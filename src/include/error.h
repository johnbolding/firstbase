/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: error.h,v 9.0 2001/01/09 02:56:11 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* 
 * signal errors 1-15 are system defined.
 * SIGILL(4) SIGBUS(10) SIGSEGV(11) are all caught before exit
 * and will display only their numbers.
 */

#define FB_BAD_TTY	101
#define FB_BAD_TERMCAP	102

#define FB_MESSAGE	111
#define FB_LMESSAGE	112

#define FB_BAD_DATA	121
#define FB_BAD_DICT	122
#define FB_BAD_INDEX	123
#define FB_EXEC_FAILURE	124
#define FB_FATAL_FGETREC	125
#define FB_FATAL_GETREC	126
#define FB_FATAL_PUTREC	127
#define FB_IO_ERROR	128
#define FB_OUT_OF_MEMORY	129
#define FB_READ_ERROR	130
#define FB_SEEK_ERROR	131
#define FB_WRITE_ERROR	132
#define FB_WRONG_INDEX	133
#define FB_DIRTY_DBASE	134
#define FB_LINK_ERROR	135

#define FB_ABORT_ERROR	200
#define FB_CANT_OPEN	201
#define FB_CANT_CREATE	202
