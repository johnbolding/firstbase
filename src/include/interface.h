/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: interface.h,v 9.2 2001/01/16 02:46:50 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/*
 * interface.h is used to augment or override any system interface
 * function definitions, like lseek or atol, etc.
 */

#if LONG_LONG_LSEEK
/*
 * if the system library is using type long long for lseek, this is set
 * to override the FirstBase use of lseek as a long. argument pos is affected.
 */
#define lseek LSEEK
#endif

#if BAD_ATOF
/*
 * The SCO DEVELOPMENT SYSTEM 5.0.0b has a bad atof().
 * When fed things like atof("Nancy") it returns a NAN number
 * (Not a Number) which will cause Floating Point Exceptions when
 * mixed with other numbers or casted. So, ATOF() is used in this situation.
 */
#define atof ATOF
#endif

#if STDC_HEADERS
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
extern char *strchr (), *strrchr ();
#endif /* STDC_HEADERS */

#if TM_IN_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif

/*
 * firstbase code overloads the strcmp function to be able
 * to do a strcmp() on pointers that might be (char *) 0.
 */

#ifdef strcmp
#undef strcmp
#endif
#define strcmp		fb_strcmp

/* NULL is not transportable.ie, (char *) 0 != 0 on some chips */
#undef NULL
#define NULL 0

/* overload the library definitions a bit -- makes code portable */
#define READ 		0
#define WRITE		1
#define READWRITE	2
#ifndef isdigit
#define isdigit(c)	('0' <= (c) && (c) <= '9')
#endif
#ifndef MAX
#define MAX(x, y)	(((x) < (y)) ? (y) : (x))
#endif
#ifndef MIN
#define MIN(x, y)	(((x) < (y)) ? (x) : (y))
#endif
#define equal(x, y)	(fb_strcmp(x, y) == 0)

/* memory and bytes -- used to swap bytes when using different architectures */
typedef unsigned long cdb_u_int32;
#define M_32_SWAP(a) { 			\
   cdb_u_int32 _tmp = a;    		\
   ((char *)&a)[0] = ((char *)&_tmp)[3]; \
   ((char *)&a)[1] = ((char *)&_tmp)[2]; \
   ((char *)&a)[2] = ((char *)&_tmp)[1]; \
   ((char *)&a)[3] = ((char *)&_tmp)[0]; \
   }

/*
 * firstbase code uses re_comp and re_exec as needed.
 * gnu code uses regcomp and regexec code.
 * RE_COMP and RE_EXEC is firstbase code that interfaces
 * re_comp to regcomp, etc
 */

#define re_comp RE_COMP
#define re_exec RE_EXEC

#if !FB_PROTOTYPES

extern char *tgoto();
extern char *getenv();

/*
 * #if LINUX
 * extern double atof();
 * #endif
 */

#else  /* FB_PROTOTYPES */

extern char *tgoto(char *cap, int col, int row);
extern char *getenv(const char *name);

/*
 * these functions want the third argument to be unsigned and
 * these defines help when using something like gcc -Wconversion
 */
#define strncmp(a, b, c)        strncmp(a, b, (unsigned) c)
#define strncpy(a, b, c)        strncpy(a, b, (unsigned) c)

#endif /* FB_PROTOTYPES */

/*
 * firstbase code uses mkstemp() instead of mktemp() if possible.
 * but, sometimes its not possible.
 */

#ifndef HAVE_MKSTEMP
#define mkstemp fb_mkstemp
#endif
