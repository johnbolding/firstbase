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

/*char *STRING="abcdefghi";*/
/*char STRING[]="1abc\n2abcde\n3\n4";*/
char XSTRING[]="111\n222\n333\n444";
char YSTRING[]="1\n\n3\n4\n";
char ZSTRING[]="";

static int fb_subline_maxline = FB_MAXLINE;

static fb_rearline();

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

   fb_subline(buf, XSTRING, -1, CHAR_NEWLINE); printf("-1 buf=%s\n", buf);
   fb_subline(buf, XSTRING, -2, CHAR_NEWLINE); printf("-2 buf=%s\n", buf);
   fb_subline(buf, XSTRING, -3, CHAR_NEWLINE); printf("-3 buf=%s\n", buf);
   fb_subline(buf, XSTRING, -4, CHAR_NEWLINE); printf("-4 buf=%s\n", buf);

   fb_subline(buf, XSTRING, 1, CHAR_NEWLINE); printf("1 buf=%s\n", buf);
   fb_subline(buf, XSTRING, 2, CHAR_NEWLINE); printf("2 buf=%s\n", buf);
   fb_subline(buf, XSTRING, 3, CHAR_NEWLINE); printf("3 buf=%s\n", buf);
   fb_subline(buf, XSTRING, 4, CHAR_NEWLINE); printf("4 buf=%s\n", buf);

   fb_subline(buf, YSTRING, -1, CHAR_NEWLINE); printf("-1 buf=%s\n", buf);
   fb_subline(buf, YSTRING, -2, CHAR_NEWLINE); printf("-2 buf=%s\n", buf);
   fb_subline(buf, YSTRING, -3, CHAR_NEWLINE); printf("-3 buf=%s\n", buf);
   fb_subline(buf, YSTRING, -4, CHAR_NEWLINE); printf("-4 buf=%s\n", buf);

   fb_subline(buf, YSTRING, 1, CHAR_NEWLINE); printf("1 buf=%s\n", buf);
   fb_subline(buf, YSTRING, 2, CHAR_NEWLINE); printf("2 buf=%s\n", buf);
   fb_subline(buf, YSTRING, 3, CHAR_NEWLINE); printf("3 buf=%s\n", buf);
   fb_subline(buf, YSTRING, 4, CHAR_NEWLINE); printf("4 buf=%s\n", buf);

   fb_subline(buf, ZSTRING, 12, CHAR_NEWLINE); printf("12 z buf=%s\n", buf);
   fb_subline(buf, ZSTRING, -12, CHAR_NEWLINE); printf("-12 z buf=%s\n", buf);
}
