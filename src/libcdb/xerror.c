/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: xerror.c,v 9.0 2001/01/09 02:56:32 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Xerror_sid[] = "@(#) $Id: xerror.c,v 9.0 2001/01/09 02:56:32 john Exp $";
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
 *  fb_xerror - fatal error routine. exits this layer of cdb system.
 */

   fb_xerror(e, p, q)
      int e;
      char *p, *q;
      
      {
         char line[FB_MAXLINE], oname[FB_MAXLINE];
	 long *lp;
         
	 switch(e){
	    case FB_MESSAGE:
	    case FB_LMESSAGE:
	       sprintf(line, "%s %s", p, q);
	       break;
	    case FB_ABORT_ERROR:
	       sprintf(line, "%s %s %s", SYSMSG[S_ABORT], p, q);
	       break;
	    case FB_FATAL_GETREC: 
	    case FB_FATAL_FGETREC:
	    case FB_FATAL_PUTREC:
	       lp = (long *) q;
	       sprintf(line, "%s %s (%d) @%ld", 
	          SYSMSG[S_IOERROR], p, e, *lp);
	       break;
	    case FB_CANT_OPEN:
	    case FB_CANT_CREATE:
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
	    case FB_DIRTY_DBASE:
               fb_basename(oname, p);
	       sprintf(line, "%s: %s %s", oname, SYSMSG[S_DIRTY_DBASE], q);
	       break;
	    case FB_WRITE_ERROR:
	       sprintf(line, "%s (%d) %s %s", S_WRITE_ERROR, e, p, q);
	       break;
	    case FB_SEEK_ERROR:
	       sprintf(line, "%s (%d) %s %s", S_SEEK_ERROR, e, p, q);
	       break;
	    case FB_READ_ERROR:
	       sprintf(line, "%s (%d) %s %s (%d)", S_READ_ERROR, e, p,
                  q, errno);
	       break;
	    case FB_IO_ERROR:
	       sprintf(line, "%s (%d) %s %s", SYSMSG[S_IOERROR], e, p, q);
	       break;
	    case FB_RPC_ERROR:
	       sprintf(line, "RPC: %s %s", p, q);
	       break;
	    case FB_OUT_OF_MEMORY:		/* low level errors */
	    case FB_EXEC_FAILURE:
	    case FB_BAD_TTY:
	       sprintf(line, "%s %s", p, q);
	       break;
	    default:
	       sprintf(line, " %s.", SYSMSG[S_FATAL]);
	       sprintf(line, "%s [Error %d]", line, e);
	       break;
	    }
         if (e != FB_MESSAGE){
            if (e != FB_LMESSAGE || cdb_usrlog > 10)
               fb_errorlog(e, line);
            }
         if (error_to_screen)
	    fb_sfatal(e, line);	/* death here -- ie. no return */
         fb_exit(e);		/* if not doing an fb_sfatal, die anyway */
      }

/*
 * fb_lxerror - used to emit an error to the log but not to emit antyhing
 *	to the screen. like when a window dies. see sigint.c - sighup.
 */

   fb_lxerror(e, p, q)
      int e;
      char *p, *q;

      {
         error_to_screen = 0;
         fb_xerror(e, p, q);
         error_to_screen = 1;
      }
