/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: scanput.c,v 9.0 2001/01/09 02:55:36 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Scanput_sid[] = "@(#) $Id: scanput.c,v 9.0 2001/01/09 02:55:36 john Exp $";
#endif

#include <dbedit.h>

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
	 int i, st;
         char buttons[10], addbutton[3], itype;
      
         /* find the top. this is here since the index level may change */
	 for (i = 0; i < cdb_sfields && cdb_sp[i] != tip ; i++)
	    ;
	 if (i >= cdb_sfields)
	    fb_xerror(FB_BAD_INDEX, hp->dindex, NIL);
	 dot = i + 1;				/* dot is one based */
	 if (disp_st == REDRAW)			/* redraw the template */
	    db_scanset(cdb_sp, i, cdb_sfields);
	 fb_fmessage(SYSMSG[S_HELP_END]);
	 fb_scrhlp(hlp);
	 i = tip->size > 50 ? 50 : tip->size;
	 if (!globaladd){
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
	    st = fb_input(-4, -17, -i, 0, itype, com, FB_ECHO, -(FB_OKEND), FB_CONFIRM);
            fb_cx_pop_env();
	    return(st);
	    }
	 else{
	    strcpy(com, "@");
	    return(FB_AOK);
	    }
      }
