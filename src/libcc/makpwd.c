/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: makpwd.c,v 9.0 2001/01/09 02:56:20 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Makpwd_sid[] = "@(#) $Id: makpwd.c,v 9.0 2001/01/09 02:56:20 john Exp $";
#endif

#include <fb.h>

#if FB_PROTOTYPES
static char xor(int a, int b);
#else /* FB_PROTOTYPES */
static char xor();
#endif /* FB_PROTOTYPES */

/* 
 *  makpwd - this is by no means an effective password mechanism.
 *     it can be used on top of unix though.
 */
 
   char *fb_makpwd(s, key, siz)
      char s[], *key;
      register int siz;
      {
         register int i, j;
	 int keylen;

         keylen = strlen(key);
         for (i = 0, j = 0; j < siz; i = (i % keylen) + 1){
            s[j] = xor(s[j], key[i]);
            j++;
            }
         return(s);
      }

/* 
 *  xor for cryption
 */
 
   static char xor(a, b)
      int a, b;
      
      {
         return((a & ~b) | (~a & b));
      }
