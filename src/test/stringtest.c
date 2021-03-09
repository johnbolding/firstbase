/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: termtest.c,v 9.2 2001/01/16 18:33:01 john Exp $
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
   char buf[100], c;
   int st, fd;
   FILE *fs;

   cdb_batchmode = 1;
   cdb_t_lines = 24;
   cdb_t_cols = 80;
   fb_sprintf(buf, "internal LINES = %d, COLS = %d", cdb_t_lines, cdb_t_cols);
   puts(buf);
}
