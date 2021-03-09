/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: shell.h,v 9.0 2001/01/09 02:56:17 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

#include <fb.h>
#include <menu.h>

#define SCREENMAX 100

#if FB_PROTOTYPES
extern initmenu(void);
extern execute(char *id);
extern void mcommand(void);
extern showenv(void);
extern startcommand(void);
#else /* FB_PROTOTYPES */
extern initmenu();
extern execute();
extern void mcommand();
extern showenv();
extern startcommand();
#endif /* FB_PROTOTYPES */
