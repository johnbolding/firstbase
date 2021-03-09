/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: visual.c,v 9.1 2001/02/16 19:41:43 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Visual_sid[] = "@(#) $Id: visual.c,v 9.1 2001/02/16 19:41:43 john Exp $";
#endif

/*
 *  visual.c - library for visual interface - to vdict functions
 *	dbedit only
 */

#include <dbedit.h>

static char *MSG1 = "Visual/Filter what field? ";
static char *MSG2 = "Visual/Filter [reading]...";

/* 
 *  visual - pop a fb_field into visual mode 
 *     if vis = 1-> visual, else filter 
 *     if readonly = 1, then readonly. 
 *     if readonly = 0, then readwrite.
 */
 
   fb_visual(p, readonly, vis)
      char *p;
      int vis, readonly;
      
      {
	 int st = FB_ABORT, num;
	 
	 if (p[1] == NULL || !(isdigit(p[1]))){
	    fb_fmessage(MSG1);
	    st = fb_input(cdb_t_lines, 27, 3, 0, FB_INTEGER, (char *) &num, 
	          FB_ECHO, FB_END, FB_CONFIRM);
	    if (st == FB_END || st == FB_ABORT)
	       return(st);
	    }
	 else
	    num = atoi(p+1);
	 num--;
	 if (!vis && num >= 0 && num < cdb_sfields)
	    st = fb_vedit(readonly, vis, cdb_sp[num]);	/* filter */
	 else if (vis && num >= 0 && num < cdb_sfields && 
	       (cdb_sp[num]->type == FB_ALPHA || cdb_sp[num]->type == FB_DOCUMENT ||
	        cdb_sp[num]->type == FB_UPPERCASE) &&
	       cdb_sp[num]->size > FB_SCREENFIELD){
	    fb_fmessage(MSG2);
	    st = fb_vedit(readonly, vis, cdb_sp[num]);	/* limit allowed visuals */
	    }
	 else  
	    fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
         return(st);
      }
