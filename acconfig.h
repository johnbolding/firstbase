/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: acconfig.h,v 9.2 2001/01/22 18:03:06 john Exp $
 * 
 * FirstBase config.h header file created by autoheader, part of autoconf
 */

#undef VERSION
#undef PRODUCT
#undef HOST
#undef HOST_OS

/* Define FB_PROTOTYPES to turn on prototyping code */
#define FB_PROTOTYPES 0

/* Define RPC to turn on code allowing rpc calls between CPUs */
#define RPC 0

/* Define if you have a bad atof call. On some sco boxes */
#undef BAD_ATOF

/* Define if you have an lseek that returns a long long. On some bsdi boxen */
#undef LONG_LONG_LSEEK

/* Define if you have an lseek that returns a long long. On some bsdi boxen */
#undef TEST_SUN_CMD

/*
 * Define if you are sure you want to use terminfo via -lncurses
 * Built into firstbase is the preferable GNU emacs termcap emulation,
 * without relying on any termcap/terminfo libraries at all.
 * But, if you are sure you do not support a termcap datafile at all
 * then define TERMINFO
 * (In addition, the LIBEXTRA in Makefile needs to be set to -lncurses)
 */
#undef TERMINFO

/* archive options */
#undef AR_OPTS

/*
 * FLEX is set to 1 if you are using flex, else set to 0 (for lex)
 */
#undef FLEX

/*
 * Before autoconf, this was _M_I386 from an SCO include file, I think.
 * Now its IGNORE_FPE set in configure.in for autoconf to use.
 */
#undef IGNORE_FPE

/*
 * Define this if you have the Berkeley style of lpr program.
 * Otherwise, I assume you have the lp program.
 */
#define HAVE_LPR 0

/*
 * Define this if you have the winsize structure.
 */
#define HAVE_ST_WINSIZE 0

@TOP@
