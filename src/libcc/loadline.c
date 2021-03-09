/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: loadline.c,v 9.0 2001/01/09 02:56:20 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Loadline_sid[] = "@(#) $Id: loadline.c,v 9.0 2001/01/09 02:56:20 john Exp $";
#endif

#include <fb.h>

#define BLKSIZE	1024
#define NWRITES	3

static char *r_buffer;			/* for buffered read */
static char **r_lines;			/* for buffered read */
static int r_curline = 0;		/* current pointer into buffer */
static int r_maxline = 0;		/* current pointer into buffer */
static int r_fd;			/* read file descriptor */

static int t_maxline = 0;

/*
 * fb_ln_init - initialize loadlines system, loadlines, return line count
 */
   fb_ln_init(fd)
      int fd;

      {
         unsigned s = 0;
	 struct stat sbuf;
         char *p;
         int j = 0, trigger_line = 0, cnt;

	 r_fd = fd;
         /* determine size of file in fd */
         fstat(fd, &sbuf);
         s = (unsigned) sbuf.st_size;
         /*
          * set t_maxline to size / 2 since this mechanism covers dict types
          * there should be no blank lines, so each line has at least
          * two characters (one character and one newline).
          * so, this should cover N lines ...
          */
         t_maxline = s / 2;
         r_buffer = (char *) fb_malloc(s + 1);
         r_lines = (char **) fb_malloc((sizeof(char *)) * (t_maxline + 2));
	 cnt = read(r_fd, r_buffer, (int) s);
	 if (cnt <= 0)
            return(0);
         r_buffer[cnt] = 0;
         /* loop through r_buffer; place pointers from r_lines to r_buffer */
         r_lines[j] = r_buffer;
         for (p = r_buffer; *p && j < t_maxline; p++){
            if (*p == '\n')
               trigger_line = 1;
            else if (trigger_line){
               r_lines[++j] = p;
               trigger_line = 0;
               }
            }
         r_curline = 0;
         r_maxline = j;
         if (j < t_maxline)
            r_maxline++;
         return(r_maxline);
      }

/*
 * fb_ln_free - end buffered read system
 */
   fb_ln_free()

      {
         fb_free((char *) r_buffer);
         fb_free((char *) r_lines);
         r_buffer = 0;
         r_lines = 0;
         r_curline = 0;
         r_maxline = 0;
         t_maxline = 0;
      }
 
/*
 * fb_ln_end - end buffered read system
 */
   fb_ln_end()

      {
         close(r_fd);
         fb_ln_free();
      }

/*
 * fb_ln_load - load a line from input, strip NEWLINE, and store NULL
 */

   fb_ln_load(buf, max)
      char *buf;
      int max;

      {
         char *p;
         int n = 0;

         if (r_curline >= r_maxline)
            return(0);
         for (p = r_lines[r_curline]; *p != '\n' && n++ < max; p++)
            *buf++ = *p;
         *buf = 0;
         r_curline++;
         return(1);
      }

/*
 * fb_ln_get - get a specified line from the loaded lines.
 *    strip NEWLINE, and store NULL
 */

   fb_ln_get(i, buf, max)
      int i;
      char *buf;
      int max;

      {
         char *p;
         int n = 0;

         i--;
         if (i >= r_maxline || i < 0)
            return(0);
         for (p = r_lines[i]; *p != '\n' && n++ < max; p++)
            *buf++ = *p;
         *buf = 0;
         return(1);
      }
