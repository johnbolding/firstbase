/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getfname.c,v 9.1 2001/02/16 18:51:46 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getfilename_sid[] = "@(#) $Id: getfname.c,v 9.1 2001/02/16 18:51:46 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/* 
 *  getfilename - get a file name from cdb_t_lines, 1. 
 *     allow default (of def[]) if def length is > 0. Otherwise force entry.
 */
 
   fb_getfilename(p, msg, def)
      char *p, *msg, *def;
      
      {  
         register int i, st;
	 int msize;
      
         fb_move(cdb_t_lines, 1); fb_clrtoeol();
	 fb_force(msg);
	 if (strlen(def) == 0){
	    i = 1;
            fb_cx_push_env("XT", CX_KEY_SELECT, NIL);
            }
	 else{
	    i = 0;
            fb_cx_push_env("DXT", CX_KEY_SELECT, NIL);
            }
	 msize = strlen(msg);
	 st = fb_input(cdb_t_lines, msize+1, 78-msize, i, FB_ALPHA, p, FB_ECHO,
            FB_NOEND, FB_CONFIRM);
         fb_cx_pop_env();
	 fb_move(cdb_t_lines, 1);
         fb_clrtoeol();
         fb_refresh();
	 if (st == FB_ABORT)
	    return(FB_ERROR);
	 else if (st == FB_DEFAULT)
	    strcpy(p,  def);
	 fb_trim(p);
	 return(st);
      }
