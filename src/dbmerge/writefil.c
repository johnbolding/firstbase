/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: writefil.c,v 9.0 2001/01/09 02:55:41 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Writefile_sid[] = "@(#) $Id: writefil.c,v 9.0 2001/01/09 02:55:41 john Exp $";
#endif

#include <dbdmrg_e.h>

#define TEMPLATE "DBDMRG_XXXXXX"

extern char filen[];		/* file name of buffer */

   writefile()
      {
         int n;

         fb_move(cdb_t_lines, 1);
	 fb_printw("\"%s\" ", filen);
	 fb_refresh();
	 n = savefile(0);
	 fb_printw("%ld Lines", n);
	 fb_refresh();
	 clear_lastline = 1;
      }

/*
 * savefile - save the file. if type == 0, into filen, else into save area.
 */

   savefile(type)
      int type;
      
      {
         int fd, n = 0;
	 char tmpfile[FB_MAXNAME], buf[1000], *p, tbuf[FB_MAXLINE];
	 fb_aline *a;
         fb_token *t;

         if (type == 0){		/* normal write */
	    strcpy(tmpfile, TEMPLATE);
	    close(mkstemp(tmpfile));
	    }
	 else				/* panic write */
	    strcpy(tmpfile, "dbdmrg_save");
	 fd = fb_mustopen(tmpfile, 2);
         fb_w_init(1, fd, 0);
	 for (a = mpcur->mp_ahead; a != NULL; a = a->a_next){
            strcpy(buf, a->a_text);
            fb_trim(buf);
            for (p = buf, t = a->a_thead; *p; p++){
               fb_w_write(0, p);
               if (*p == CHAR_DOLLAR && t != NULL){
                  if (t->t_field != NULL)
                     sprintf(tbuf, "%s[%d]", t->t_field->id, t->t_width);
                  else
                     sprintf(tbuf, "$");
                  fb_nextwrite(0, tbuf);
                  t = t->t_next;
                  }
               }
            fb_w_write(0, "\n");
	    ++n;
	    }
	 fb_wflush(1);
	 if (type == 0){
            if (access(filen, 0) == 0)
	       if (unlink(filen) < 0)
	          fb_xerror(FB_EXEC_FAILURE, "Could not unlink: ", filen);
	    if (link(tmpfile, filen) < 0)
	       if (fb_copyfile(tmpfile, filen) < 0)
	          fb_xerror(FB_EXEC_FAILURE, "Could not link/copy: ", tmpfile);
	    if (unlink(tmpfile) < 0)
	       fb_xerror(FB_EXEC_FAILURE, "Could not unlink: ", tmpfile);
	    }
         return(n);
      }
