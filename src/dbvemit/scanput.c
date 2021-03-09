/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: scanput.c,v 9.0 2001/01/09 02:56:02 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Scanput_sid[] = "@(#) $Id: scanput.c,v 9.0 2001/01/09 02:56:02 john Exp $";
#endif

#include <dbve_ext.h>

/*
 *  scanput - provides the first layer of command interpretation for
 *     dbedit. an empty screen template is layed out, and the cursor
 *     positioned on the current index pointer...
 *     standard alpha fb_input is taken for the command.
 *     note: if disp_st == 1, then template is redrawn.
 *           always draw the surrounding stuff.
 */

   scanput(tip, hlp, disp_st)
      fb_field *tip;
      char *hlp;
      int disp_st;
   
      {
         fb_page *p;
	 fb_node *n;
	 int lastp = 0;
	 
	 if (disp_st == REDRAW && pcur != NULL)
	    lastp = pcur->p_num;
	 pcur = NULL;
	 ncur = NULL;
	 for (p = phead; p; p = p->p_next){
	    for (n = p->p_nhead; n; n = n->n_next){
	       if (n->n_fp == tip){
	          pcur = p;
		  ncur = n;
		  dot = n;
		  break;
		  }
	       }
	    if (pcur != NULL)
	       break;
	    }
	 if (pcur == NULL)
	    fb_xerror(FB_BAD_INDEX, hp->dindex, NIL);
	 if (disp_st == REDRAW && lastp != pcur->p_num)
	    disp_st = ZAP_REDRAW;
	 if (disp_st == REDRAW)			/* redraw the template */
	    fb_clrpage();
	 else if (disp_st == ZAP_REDRAW){
	    fb_scrlbl(hp->sdict);
	    fb_scanset();
	    }
	 /*
	  * fb_fmessage(SYSMSG[S_HELP_END]);
	  * fb_scrhlp(hlp);
	  */
	 strcpy(com, "@");
	 return(FB_AOK);
      }
