/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: keyboard.h,v 9.0 2001/01/09 02:56:13 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/
/*
 *
 * keyboard.h - align the varios editing signals used by e_input.
 */

#define FB_MAXKEYS 	100
#if SHORTNAMES
#define FB_KBDMAP	".fb-kbmap"
#else
#define FB_KBDMAP	".firstbase-kbmap"
#endif

#define E_ACTION_START	201		/* same as FIRST E_signal */
#define E_DSIGNAL 	201
#define E_ESIGNAL	202
#define E_YSIGNAL	203
#define E_REDRAW 	204
#define E_HELP		205
#define E_FSIGNAL 	209
#define E_BSIGNAL 	210
#define E_SSIGNAL 	211
#define E_QSIGNAL 	212
#define E_END		213
#define E_ABORT		214
#define E_PAGEUP	216
#define E_PAGEDOWN	217
#define E_NEXT		218
#define E_PREV		219
#define E_PRINT		220
#define E_DEFAULT	221
#define E_CLEARFIELD	222
#define E_WRITEREC	223
#define E_DELETEREC	224
#define E_RSIGNAL	225
#define E_VSIGNAL	226
#define E_ACTION_STOP	226		/* same as LAST interal E_sig */

#define E_DELETE_CHAR_BACKWARD		301
#define E_DELETE_CHAR_FORWARD		302
#define E_MOVE_CHAR_BACKWARD		303
#define E_MOVE_CHAR_FORWARD		304
#define E_DELETE_WORD_BACKWARD		305
#define E_DELETE_WORD_FORWARD		306
#define E_MOVE_WORD_BACKWARD		307
#define E_MOVE_WORD_FORWARD		308
#define E_BEGINNING_OF_LINE		309
#define E_END_OF_LINE			310
#define E_CAPITALIZE_WORD		311
#define E_UPCASE_WORD			312
#define E_DOWNCASE_WORD			313
#define E_DELETE_TO_END_OF_LINE		314
#define E_DELETE_TO_BEGINNING_OF_LINE	315
#define E_QUIT				316
#define E_SYS_INTERRUPT			317
#define E_SYS_STOP			318
