/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: editor.c,v 9.0 2001/01/09 02:56:01 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Editor_sid[] = "@(#) $Id: editor.c,v 9.0 2001/01/09 02:56:01 john Exp $";
#endif

#include <dbve_ext.h>

static char *FMT1 = "(dot@%ld/%ld)";
extern int add_exactly;			/* counter used to edit -a N records */

/* 
 *  editor - main loop of the dbvedit visual fb_database editor.
 */
 
   editor()
      
      {      
         int i, disp_st;
	 char hlp[FB_MAXNAME];

         pindx = 0;
         rec = oldrec = 0L;
	 for (i = 0; i < FB_MAXIDXS; i++)
            irec[i] = ibase[i] = 0L;
         mode = NULL;
	 disp_st = 0;
	 hp->bsmax = 0; hp->bsend = 0;
         for(;;){
            com[0] = NULL;
	    st = FB_AOK;
	    sprintf(hlp, FMT1, irec[pindx], hp->bsmax);
            if (mode == NULL){
	       fb_scrstat(SYSMSG[S_COMMAND_LEVEL]);
	       fb_move(2, 64), fb_clrtoeol(); /* cleanup individual rec stat */
               st = scanput(hp->ip[pindx], hlp, disp_st);
	       }
            else			/* must be autoadd mode ... */
               com[0] = FB_FADD;		/* ...so set com for docmd. */
	    if (st == FB_DSIGNAL){
	       fb_scrhdr(hp, NIL);
	       fb_infotoggle();
	       disp_st = ZAP_REDRAW;
	       }
	    else if ((disp_st = docmd()) == FB_END)
               break;			/* REDRAW,NOREDRAW flags display */
            if (add_exactly > 0)
               if (--add_exactly <= 0)
                  break;
            }
      }
