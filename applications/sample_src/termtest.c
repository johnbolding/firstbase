/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: termtest.c,v 9.0 2001/01/22 18:28:07 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 * termtest.c - simple test of screen control using firstbase
 *
 */

#include <fb.h>
#include <fb_vars.h>

main(argc, argv)
int argc;
char **argv;

{
   char buf[11], c;
   int st, fd;
   FILE *fs;

   fb_getargs(argc, argv, FB_NODB);

   fb_allow_int();
   fb_clear();
   fb_scrhdr((fb_database *) NULL, "Hello World");

   fb_move(3, 1); fb_printw("X is at (3,1)");
   fb_move(10, 10); fb_printw("(10,10) starts at row 10, column 10 ");
   fb_move(12, 15); fb_printw("(12,15) starts at row 12, column 15 ");
   fb_move(5, 1);
   fb_printw("internal LINES = %d, COLS = %d", cdb_t_lines, cdb_t_cols);
   fb_infoline();
   fb_refresh();
   fb_move(15, 1);
   fb_printw("Prompt for input:");
   st = fb_input(15, 20, 10, 0, 'a', buf, FB_ECHO, FB_OKEND, FB_CONFIRM);
   /* all input is padded. fb_trim() removes these blanks */
   fb_trim(buf);
   fb_move(17, 1);
   fb_printw("Status of input: %d", st);
   fb_move(18, 1);
   fb_printw("Text of input: %s", buf);
   fb_serror(FB_MESSAGE, "Hit any key to Continue", "");
   fb_ender();
}
