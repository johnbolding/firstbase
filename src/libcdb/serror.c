/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: serror.c,v 9.0 2001/01/09 02:56:29 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Serror_sid[] = "@(#) $Id: serror.c,v 9.0 2001/01/09 02:56:29 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <errno.h>

#define S_READ_ERROR "Read Error:"
#define S_WRITE_ERROR "Write Error:"
#define S_SEEK_ERROR "Seek Error:"

static short int error_to_screen = 1;
extern short int cdb_usrlog;

/*
 *  fb_serror - simple error routine. all simple error messages come here.
 */

   fb_serror(e, p, q)
      int e;
      char *p, *q;
      
      {
         char line[FB_MAXLINE], oname[FB_MAXLINE], tline[FB_MAXLINE];
         
         switch(e){
	    case FB_ABORT_ERROR:
	       sprintf(line, "%s %s %s", SYSMSG[S_ABORT], p, q);
	       break;
	    case FB_MESSAGE:
	    case FB_LMESSAGE:
	       sprintf(line, "%s %s", p, q);
	       break;
	    case FB_BAD_DICT:
               fb_basename(oname, p);
	       if (*q)
	          sprintf(line, "%s: %s (%s)", oname, SYSMSG[S_BAD_DICT], q);
	       else
	          sprintf(line, "%s: %s", oname, SYSMSG[S_BAD_DICT]);
	       break;
	    case FB_BAD_INDEX:
               fb_basename(oname, p);
	       sprintf(line, "%s: %s %s", oname, SYSMSG[S_BAD_INDEX], q);
	       break;
	    case FB_BAD_DATA:
               fb_basename(oname, p);
	       sprintf(line, "%s: %s %s", oname, SYSMSG[S_BAD_DATA], q);
	       break;
	    case FB_WRITE_ERROR:
	       sprintf(line, "%s (%d) %s %s", S_WRITE_ERROR, e, p, q);
	       break;
	    case FB_SEEK_ERROR:
	       sprintf(line, "%s (%d) %s %s", S_SEEK_ERROR, e, p, q);
	       break;
	    case FB_READ_ERROR:
	       sprintf(line, "%s (%d) %s %s", S_READ_ERROR, e, p, q);
	       break;
	    case FB_IO_ERROR:
	       sprintf(line, "%s (%d) %s %s", SYSMSG[S_IOERROR], e, p, q);
	       break;
	    case FB_RPC_ERROR:
	       sprintf(line, "RPC: %s %s", p, q);
	       break;
	    case FB_CANT_CREATE:
	    case FB_CANT_OPEN:
	       sprintf(line, "%s: ", p);
               switch(errno){
                  case EACCES:
	             strcat(line, SYSMSG[S_ACCESSDENIED]);
                     break;
                  case EMFILE:
	             strcat(line, "Too many open files");
                     break;
                  case ENOENT:
                     if (e == FB_CANT_CREATE)
	                strcat(line, SYSMSG[S_ACCESSDENIED]);
                     else
	                strcat(line, SYSMSG[S_NOFILE]);
                     break;
                  case EROFS:
	             strcat(line, "Named file on read-only file system");
                     break;
                  default:
                     if (e == FB_CANT_CREATE)
	                strcat(line, SYSMSG[S_ACCESSDENIED]);
                     else
	                strcat(line, SYSMSG[S_NOFILE]);
                     break;
                  }
               if (q != NIL)
                  sprintf(line, "%s (%s)", line, q);
	       break;
	    case FB_WRONG_INDEX:
	       sprintf(line, "%s: incompatible index", p);
	       break;
	    default:
	       sprintf(line, "firstbase error: %s %s", p, q);
	       break;
	    }
         if (e != FB_MESSAGE){
            if (e != FB_LMESSAGE || cdb_usrlog > 10){
               sprintf(tline, "%d: %s", e, line);
               fb_usrlog_msg(tline);
               }
            }
         if (error_to_screen)
	    fb_screrr(line);	/* simple error or message with temp halt */
      }

   fb_lerror(e, p, q)
      int e;
      char *p, *q;

      {
         error_to_screen = 0;
         fb_serror(e, p, q);
         error_to_screen = 1;
      }
