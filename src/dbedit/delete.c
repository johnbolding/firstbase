/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: delete.c,v 9.1 2001/02/16 19:41:42 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Delete_sid[] = "@(#) $Id: delete.c,v 9.1 2001/02/16 19:41:42 john Exp $";
#endif

#include <dbedit.h>

static char *NODELETE = ".nodel";

/* 
 * delete - delete a record.
 */
 
   fb_delete(k)
      fb_field *k;
      
      {
         char tfile[FB_MAXNAME];
	 
	 fb_rootname(tfile, hp->dbase);
	 strcat(tfile, NODELETE);
	 if (access(tfile, 0) == 0){
	    fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], SYSMSG[S_LOCKED]);
	    return(FB_ERROR);
	    }
         if (k->fld[0] == CHAR_STAR)
            return(fb_undelete(k));
         fb_bell();
	 if (fb_mustbe(CHAR_y, SYSMSG[S_DEL_QUES], cdb_t_lines, 1) == FB_AOK){
            fb_serror(FB_MESSAGE, SYSMSG[S_DEL_REC], NIL);
            k->fld[0] = CHAR_STAR;
            return(FB_DELETED);
            }
         else
            fb_serror(FB_MESSAGE, SYSMSG[S_NOT_DONE], NIL);
         return(FB_ERROR);
      }
