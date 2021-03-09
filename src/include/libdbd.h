/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: libdbd.h,v 9.0 2001/01/09 02:56:13 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

#if !FB_PROTOTYPES

extern fb_anychange();
extern void fb_chainto();
extern fb_choosefield();
extern fb_chooseval();
extern fb_initdbd();
extern fb_onpage();

#else /* FB_PROTOTYPES */
extern fb_anychange(char *inp);
extern fb_chainto(char *msg, char *topgm, char **argv);
extern fb_choosefield(char *buf);
extern fb_chooseval(char *f, char *buf, int mlen, int type);
extern fb_initdbd(fb_database *hp);
extern fb_onpage(int f);
#endif
