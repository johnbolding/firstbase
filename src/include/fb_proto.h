/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: fb_proto.h,v 9.0 2001/01/09 02:56:12 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/*
 * these only need including now if FB_PROTOTYPES is being used
 */

#include <sys/stat.h>
#include <mdict.h>
#include <fbserver.h>
#include <dbfwd.h>
#include <dblwd.h>
#include <dbpwd.h>
#include <vdict.h>

/* libcc */

/* The prototypes for the libcc layer live in libcc.h */

/* libcdb */

/* The prototypes for the libcdb layer live in libcdb.h */

/* libdbd */

/* The prototypes for the libdbd layer live in libdbd.h */

/* libinit */

/* The prototypes for the libinit layer live in libinit.h */

/* liblic */

/* The prototypes for the liblic layer live in liblic.h */

/* librec */

/* The prototypes for the librec layer live in librec.h */

/* libscr */

/* The prototypes for the libscr layer live in libscr.h */

/* liwbin */

/* The prototypes for the libwin layer live in libwin.h */


/* libsec */

/* The prototypes for the libsec layer live in libsec.h */


/* libedit.a */

/* The prototypes for the libedit layer live in libedit.h */

/* libclnt.a */

/* The prototypes for the libclnt layer live in libclnt.h */


#if SYS_V
extern char *regcmp(char *s1, char *s2);
extern char *regex(char *re, char *subject);/* SYS_V routines */
#else	/* SYS_V */
extern char *re_comp(char *s);			/* V7/BSD routines */
#endif	/* SYS_V */

extern char *strcpy(char *d, const char *s);
extern char *strcat(char *d, char *s);
extern char *strncpy(char *d, char *s, int n);
extern char *strncat(char *d, char *s, int n);
extern char *mktemp(char *template);

#if SYS_V
extern char *strchr(char *s, int c);
extern char *strrchr(char *s, int c);
#else	/* V7 */
extern char *index(char *s, int c);
extern char *rindex(char *s, int c);
#endif

/* interface */
/*extern long atol(char *s);*/
extern char *tgoto(char *cap, int col, int row);
extern char *getenv(const char *name);
/* extern double atof(char *s); */

extern off_t lseek(int fildes, off_t offset, int whence);

