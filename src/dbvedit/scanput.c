/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: scanput.c,v 9.0 2001/01/09 02:55:59 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Scanput_sid[] = "@(#) $Id: scanput.c,v 9.0 2001/01/09 02:55:59 john Exp $";
#endif

#include <dbve_ext.h>
extern int globaledit;
extern char *globalpat;
extern char *cdb_prompt_commandmsg;

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
	 int lastp = 0, st;
         char buttons[10], addbutton[3], itype;
	 
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
	 fb_fmessage(cdb_prompt_commandmsg);
	 fb_scrhlp(hlp);
	 fb_check_abslink();
         fb_macro_level(1);
	 if (globaladd){
	    strcpy(com, "@");
	    return(FB_AOK);
	    }
	 else if (globaledit > 0){
	    sprintf(com, "@%d", globaledit);
	    return(FB_AOK);
	    }
	 else if (globalpat != NULL){
	    strcpy(com, globalpat);
	    return(FB_AOK);
	    }
	 else{
	    if (tip->type == FB_UPPERCASE)
	       itype = FB_UPPERCASE;
            else
               itype = FB_ALPHA;
            if (scanner)
               addbutton[0] = NULL;
            else
               strcpy(addbutton, "@");
            sprintf(buttons, "E%sFBH", addbutton);
            fb_cx_push_env(buttons, CX_KEY_SELECT, NIL);	/* command level */
            st = fb_input(-(ncur->n_row), -(ncur->n_col), 
               -ncur->n_len, 0, itype , com, FB_ECHO, -(FB_OKEND), FB_CONFIRM);
            fb_cx_pop_env();
            return(st);
	    }
      }
