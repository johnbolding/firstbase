/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mustbe.c,v 9.0 2001/01/09 02:57:04 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Mustbe_sid[] = "@(#) $Id: mustbe.c,v 9.0 2001/01/09 02:57:04 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/* 
 *  mustbe - get a one character response from row, col. store in com[0].
 *     return FB_AOK for yes, FB_ERROR for no...FB_END always fails
 *     pass in 'n' for must be 'n' to be no.
 *     pass in 'y' for "mustbe 'y' to be yes"
 */
 
   fb_mustbe(typ, msg, row, col)
      register int row, col;
      char typ, *msg;
      
      {
         int cl, st = 0;
	 char com[5];
	 
	 cl = 1;
	 if (row < 0){
	    row = -row;
	    cl = 0;
	    }
	 fb_move(row, col);
	 if (cl)
	    fb_clrtoeol();
	 if (row != cdb_t_lines)
	    fb_stand(msg);
	 else
	    fb_force(msg);
         if (typ == CHAR_y)
            fb_cx_push_env("ny", CX_KEY_SELECT, NIL);
         else
            fb_cx_push_env("yn", CX_KEY_SELECT, NIL);
         st = fb_input(row,(int) (col+strlen(msg)+1) ,1,0,FB_ALPHA,com,FB_ECHO,
            FB_OKEND,FB_CONFIRM);
	 if (st == FB_END || st == FB_ABORT)
	    st = FB_ERROR;
	 else if (typ == CHAR_y && (com[0] == CHAR_y || com[0] == CHAR_Y))
	    st = FB_AOK;
	 else if (typ == CHAR_n && (com[0] == CHAR_n || com[0] == CHAR_N))
	    st = FB_AOK;
	 else if (typ == CHAR_y)
	    strcpy(com, SYSMSG[S_STRING_n]), st = FB_ERROR;
	 else if (typ == CHAR_n || typ == NULL || typ == FB_BLANK)
	    strcpy(com, SYSMSG[S_STRING_y]), st = FB_ERROR;
	 else
	    strcpy(com, SYSMSG[S_STRING_QUES]), st = FB_ERROR;
	 fb_move(row, col);
         fb_clrtoeol();
         fb_cx_pop_env();
	 return(st);
      }
