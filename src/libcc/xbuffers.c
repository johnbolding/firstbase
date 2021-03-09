/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: xbuffers.c,v 9.0 2001/01/09 02:56:22 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Xbuffers_sid[] = "@(#) $Id: xbuffers.c,v 9.0 2001/01/09 02:56:22 john Exp $";
#endif

#include <fb.h>

#define BLKSIZE	1024
#define NREADS	3
#define MAXLINE 250

static char *x_buffer[NREADS];		/* for buffered write buffers */
static int x_curp[NREADS];		/* for buffer current pointers */
static int x_fd[NREADS];		/* for file descriptors */
static int x_maxp[NREADS];		/* for file descriptors */

/*
 * x_init - initialize N buffered read systems, fds of a ... z
 */
   fb_x_init(n, a, b)
      int n, a, b;

      {
         int i;
	 extern char *fb_malloc(unsigned s);

	 x_fd[0] = a; x_fd[1] = b;
         for (i = 0; i < n; i++){
	    x_curp[i] = 0;
	    x_buffer[i] = (char *) fb_malloc(BLKSIZE + 1);
	    }
      }

/*
 * x_end - end N buffered read system
 */
   fb_x_end(n)
      int n;

      {
         int i;

         for (i = 0; i < n; i++)
	    close(x_fd[i]);
      }

/*
 * x_nextread - semi buffered read - read a block if needed.
 */
   fb_x_nextread(ch, buf)
      int ch;
      char *buf;
      
      {
         if (x_curp[ch] < 0 || x_curp[ch] >= x_maxp[ch]){
	    x_maxp[ch] = read(x_fd[ch], x_buffer[ch], BLKSIZE);
	    if (x_maxp[ch] <= 0)
	       return(0);
	    x_curp[ch] = 0;
	    }
	 *buf = x_buffer[ch][x_curp[ch]++];
         return(1);
      }
 
/*
 * x_nextline - get a line from input, strip NEWLINE, and store NULL
 */

   fb_x_nextline(ch, buf, max)
      int ch;
      char *buf;
      int max;

      {
         char c;
	 int i;

         if (max <= 0)
            max = MAXLINE;
	 for (i = 1; i < max; i++){		/* 1..MAXLINE inclusive */
	    if (fb_x_nextread(ch, &c) == 0)
	       return(0);
	    if (c == '\n')
	       break;
	    *buf++ = c;
	    }
	 *buf = 0;
	 return(i);
      }
