/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: buffers.c,v 9.0 2001/01/09 02:56:19 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Buffers_sid[] = "@(#) $Id: buffers.c,v 9.0 2001/01/09 02:56:19 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#define BLKSIZE	1024
#define NWRITES	3
#define MAXLINE 250

static char r_buffer[BLKSIZE + 1];	/* for buffered read */
static int r_curp = -1;			/* current pointer into buffer */
static int r_maxp = 0;			/* max pointer */
static int r_fd;			/* read file descriptor */

static char *w_buffer[NWRITES];		/* for buffered write buffers */
static int w_curp[NWRITES];		/* for buffer current pointers */
static int w_fd[NWRITES];		/* for file descriptors */

/*
 * r_init - initialize buffered read system
 */
   fb_r_init(fd)
      int fd;

      {
         r_curp = -1;
	 r_fd = fd;
      }

/*
 * r_end - end buffered read system
 */
   fb_r_end()

      {
	 close(r_fd);
      }

/*
 * r_rewind - re_initialize buffered read system
 */
   fb_r_rewind()

      {
	 if (lseek(r_fd, 0L, 0) != 0)
            fb_xerror(FB_SEEK_ERROR, SYSMSG[S_BAD_DATA], NIL);
         r_curp = -1;
      }

/*
 * nextread - semi buffered read - read a block if needed.
 */
   fb_nextread(buf)
      char *buf;
      
      {
         if (r_curp < 0 || r_curp >= r_maxp){
	    r_maxp = read(r_fd, r_buffer, BLKSIZE);
	    if (r_maxp <= 0)
	       return(0);
	    r_curp = 0;
	    }
	 *buf = r_buffer[r_curp++];
         return(1);
      }
 
/*
 * nextline - get a line from input, strip NEWLINE, and store NULL
 */

   fb_nextline(buf, max)
      char *buf;
      int max;

      {
         char c;
	 int i;

         if (max <= 0)
            max = MAXLINE;
	 for (i = 1; i < max; i++){		/* 1..MAXLINE inclusive */
	    if (fb_nextread(&c) == 0)
	       return(0);
	    if (c == '\n')
	       break;
	    *buf++ = c;
	    }
	 *buf = 0;
	 return(i);
      }

/*
 * w_init - initialize N buffered write systems, fds of a ... z
 */
   fb_w_init(n, a, b)
      int n, a, b;

      {
         int i;
	 extern char *fb_malloc(unsigned s);

	 w_fd[0] = a; w_fd[1] = b;
         for (i = 0; i < n; i++){
	    w_curp[i] = 0;
	    w_buffer[i] = (char *) fb_malloc(BLKSIZE + 1);
	    }
      }

/*
 * w_end - end N buffered write systems
 */
   fb_w_end(n)
      int n;

      {
         int i;

         for (i = 0; i < n; i++)
	    close(w_fd[i]);
      }

/*
 * nextwrite - buffered write - write string. check for a needed flush.
 */
   fb_nextwrite(ch, buf)
      int ch;
      char *buf;
      
      {
         char *p;

         for (p = buf; *p; p++){
	    if (w_curp[ch] < 0 || w_curp[ch] >= BLKSIZE){
	       if (write(w_fd[ch], w_buffer[ch], BLKSIZE) != BLKSIZE)
		  return(0);
	       w_curp[ch] = 0;
	       }
	    w_buffer[ch][w_curp[ch]++] = *p;
	    }
         return(1);
      }

/*
 * w_write - buffered write - write one character. check for a needed flush.
 */
   fb_w_write(ch, buf)
      int ch;
      char *buf;
      
      {
         if (w_curp[ch] < 0 || w_curp[ch] >= BLKSIZE){
	    if (write(w_fd[ch], w_buffer[ch], BLKSIZE) != BLKSIZE)
	       return(0);
	    w_curp[ch] = 0;
	    }
	 w_buffer[ch][w_curp[ch]++] = *buf;
         return(1);
      }

/*
 * w_writen - buffered write - write a block if needed.
 */
   fb_w_writen(ch, buf, n)
      int ch, n;
      char *buf;
      
      {
         char *p;

         for (p = buf; n > 0; p++, n--){
	    if (w_curp[ch] < 0 || w_curp[ch] >= BLKSIZE){
	       if (write(w_fd[ch], w_buffer[ch], BLKSIZE) != BLKSIZE)
		  return(0);
	       w_curp[ch] = 0;
	       }
	    w_buffer[ch][w_curp[ch]++] = *p;
	    }
         return(1);
      }

/*
 * wflush - flush anything left in the wbuffer
 */
   void fb_wflush(n)
      int n;
      
      {

         int i;

         for (i = 0; i < n; i++){
	    if (w_curp[i] > 0){
	       if (write(w_fd[i], w_buffer[i], w_curp[i]) != w_curp[i])
		  return;
	       w_curp[i] = 0;
	       }
	    }
      }
