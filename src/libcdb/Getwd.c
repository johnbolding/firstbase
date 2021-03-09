/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: Getwd.c,v 9.0 2001/01/09 02:56:23 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char *Getwd_sid = "%W% %G% FB";
#endif

#include <fb.h>

extern short int cdb_use_rpc;

#if HAVE_GETCWD
   char *fb_getwd(p)
      char *p;

      {
         char *getcwd(char *buf, size_t size);
#if RPC
         char *fb_getwd_clnt(char *p);

         if (cdb_use_rpc){
            return(fb_getwd_clnt(p));
            }
#endif /* RPC */
         return(getcwd(p, FB_MAXNAME));
      }
#else /* HAVE_GETCWD */

/* this path assumes a getwd() function.
 * could test HAVE_GETWD if needed but
 * its hard to believe a modern unix box without
 * one of these functions,
 */
   char *fb_getwd(p)
      char *p;

      {
         char *getwd();
#if RPC
         char *fb_getwd_clnt();

         if (cdb_use_rpc){
            return(fb_getwd_clnt(p));
            }
#endif /* RPC */
         return(getwd(p));
      }
#endif /* HAVE_GETCWD */

#if NOGETWD
char *fb_getwd(p) char *p { *p = 0; return(p); }
#endif /* NOGETWD */
