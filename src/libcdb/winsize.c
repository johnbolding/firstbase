/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: winsize.c,v 9.3 2001/02/05 18:25:02 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Winsize_sid[] = "@(#) $Id: winsize.c,v 9.3 2001/02/05 18:25:02 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#include <sys/ioctl.h>

static char *S_LINES = "LINES";
static char *S_COLUMNS = "COLUMNS";

static void get_frame_size();

/*
 * fb_winsize - if possible, get the window size since it may not
 *    be done via termcap ...
 */

   void fb_winsize()
      {

         int width = 0, height = 0;
         char *lines, *cols;

         /*
          * at this point, cdb_t_lines and cdb_t_cols are
          * set to whatever was found (or not found) in the termcap
          * so, could be -1. could be 0. could be 24 x 80.
          * this code now tries to guess a bit better at the screen size.
          */
         get_frame_size(&width, &height);
         if (width > 0 && height > 0){
            cdb_t_lines = height;
            cdb_t_cols = width;
            }
         /*
          * cdb_t_lines and cdb_t_cols are now set to frame size, if posible.
          * now, if environment variables exist, they trump all else
          */
         if ((lines = getenv(S_LINES))){
            height = atoi(lines);
            if (height >= 24 && height <= FB_PROW)
               cdb_t_lines = height;
            }
         if ((cols = getenv(S_COLUMNS))){
            width = atoi(cols);
            if (width >= 80 && width <= FB_PCOL)
               cdb_t_cols = width;
            }

         /*
          * finally, as a sanity check, make sure we have minimum 24x80
          * and maximum of FB_PROW x FB_PCOL
          */
         if (cdb_t_lines <= 0)
            cdb_t_lines = 24;
         else if (cdb_t_lines > FB_PROW)	/* libscr.h defines PROW */
            cdb_t_lines = FB_PROW;
         if (cdb_t_cols <= 0)
            cdb_t_lines = 80;
         else if (cdb_t_cols > FB_PCOL)		/* libscr.h defines PCOL */
            cdb_t_cols = FB_PCOL;
      }

/*
 * The code in get_frame_size is from GNU Emacs
 * Copyright (C) 1985, 86, 87, 88, 93, 94, 95 Free Software Foundation, Inc.
 */

static void get_frame_size (widthp, heightp)
     int *widthp, *heightp;
{

int input_fd = 0;

#ifdef TIOCGWINSZ

  /* BSD-style.  */
  struct winsize size;

  if (ioctl (input_fd, TIOCGWINSZ, &size) == -1)
    *widthp = *heightp = 0;
  else
    {
      *widthp = size.ws_col;
      *heightp = size.ws_row;
    }

#else
#ifdef TIOCGSIZE

  /* SunOS - style.  */
  struct ttysize size;  

  if (ioctl (input_fd, TIOCGSIZE, &size) == -1)
    *widthp = *heightp = 0;
  else
    {
      *widthp = size.ts_cols;
      *heightp = size.ts_lines;
    }

#else
#ifdef VMS

  struct sensemode tty;
  
  SYS$QIOW (0, input_fd, IO$_SENSEMODE, &tty, 0, 0,
	    &tty.class, 12, 0, 0, 0, 0);
  *widthp = tty.scr_wid;
  *heightp = tty.scr_len;

#else
#ifdef MSDOS
  *widthp = ScreenCols ();
  *heightp = ScreenRows ();
#else /* system doesn't know size */
  *widthp = 0;
  *heightp = 0;
#endif

#endif /* not VMS */
#endif /* not SunOS-style */
#endif /* not BSD-style */
}
