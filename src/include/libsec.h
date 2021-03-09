/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: libsec.h,v 9.0 2001/01/09 02:56:14 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

#if !FB_PROTOTYPES

extern char *fb_getlogin();
extern fb_catch_sec();
extern fb_endpwent();
extern fb_getgid();
extern fb_getuid();
extern fb_passwd *fb_getpwent();
extern fb_passwd *fb_getpwnam();
extern fb_passwd *fb_getpwuid();
extern fb_rlogin();
extern fb_rlogout();
extern fb_sec_ender();
extern fb_setecho();
extern fb_setpwent();
extern fb_validpass();
extern int TTYSLOT();

extern char *crypt();

#else /* FB_PROTOTYPES */

extern char *fb_getlogin(void);
extern fb_catch_sec(void);
extern fb_endpwent(void);
extern fb_getgid(void);
extern fb_getuid(void);
extern fb_passwd *fb_getpwent(void);
extern fb_passwd *fb_getpwnam(char *name);
extern fb_passwd *fb_getpwuid(int uid);
extern fb_rlogin(void);
extern fb_rlogout(void);
extern fb_sec_ender(void);
extern fb_setecho(int m);
extern fb_setpwent(void);
extern fb_validpass(char *try, char *rpass);
extern int TTYSLOT(void);

extern char *crypt(char *, char *);
#endif /* FB_PROTOTYPES */
