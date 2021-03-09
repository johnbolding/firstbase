/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id$
 *
 */

#include <fb.h>
#include <fb_vars.h>

#define BLKSIZE	(1024*4)
#define NWRITES	3
#define MAXLINE 2500

static char r_buffer[BLKSIZE + 1];	/* for buffered read */
static int r_curp = -1;			/* current pointer into buffer */
static int r_maxp = 0;			/* max pointer */
static int r_fd;			/* read file descriptor */

static char *BADPAT = "bad regular expression: ";

   main(argc, argv)
      int argc;
      char *argv[];
      
      {
         char buf[MAXLINE], *p;
         int fd;

         if (argc < 3)
            usage();
         /* pattern is argv[1] */
         /* file is argv[2] */
         fd = open(argv[2], 0);
         if (fd < 0)
            fb_xerror(FB_MESSAGE, "cannot open file", argv[2]);
         r_init(fd);
         /* compile the pattern */
         if (re_comp(argv[1]) != NULL){
            fb_serror(FB_MESSAGE, BADPAT, argv[1]);
            return(FB_ERROR);
            }
         /* read each line of input, compare, output */
         while (nextline(buf, MAXLINE) > 0)
            if (re_exec(buf)){
               for (p = buf; *p != ','; p++)
                  ;
               *p = NULL;
               printf("%s\n", buf);
               }
         r_end();
      }

   usage()
      {
         fprintf(stderr, "usage: dbgrep pattern file\n");
         exit(-1);
      }

/*
 * r_init - initialize buffered read system
 */
   static r_init(fd)
      int fd;

      {
         r_curp = -1;
	 r_fd = fd;
      }

/*
 * r_end - end buffered read system
 */
   static r_end()

      {
	 close(r_fd);
      }

/*
 * r_rewind - re_initialize buffered read system
 */
   static r_rewind()

      {
	 if (lseek(r_fd, 0L, 0) != 0)
            fb_xerror(FB_SEEK_ERROR, SYSMSG[S_BAD_DATA], NIL);
         r_curp = -1;
      }

/*
 * nextread - semi buffered read - read a block if needed.
 */
   static nextread(buf)
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

   static nextline(buf, max)
      char *buf;
      int max;

      {
         char c;
	 int i;

         if (max <= 0)
            max = MAXLINE;
	 for (i = 1; i < max; i++){		/* 1..MAXLINE inclusive */
	    if (nextread(&c) == 0)
	       return(0);
	    if (c == '\n')
	       break;
	    *buf++ = c;
	    }
	 *buf = 0;
	 return(i);
      }
