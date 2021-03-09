/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: edt_cell.c,v 9.2 2003/03/29 18:04:31 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Edit_cell_sid[] = "@(#) $Id: edt_cell.c,v 9.2 2003/03/29 18:04:31 john Exp $";
#endif

#include <dbvi_ext.h>

static char *RANGE_MSG = "Entry out of range.";

extern short int cdb_edit_input;
extern char *cdb_e_buf;
extern char *cdb_T_PREV;
extern char *cdb_T_NPREV;

extern short int dbvi_column_mode;

/* 
 *  edit_cell - edit actual fb_field fb_cell - calculate row and col on fly.
 *	remember: all values are stored memory -- nothing is written.
 */
    edit_cell()

      {
         int row, col, s, forced = 0, pend, isize;
         char *inp, *line, *fb_simpledate(), *fb_rmlead(), *p;
	 long atol();
	 fb_field *f;
	 int save_cdb_edit_input;
   
         inp = cdb_afld;
	 line = cdb_bfld;
	 *inp = *line = NULL;
	 if (scanner == 1)
	    return(FB_ERROR);
	 f = col_current->p_field;
         if (f->type == FB_FORMULA || f->dflink != NULL)
	    return(FB_AOK);
         row = whichrow(crec_current);
         col = col_current->p_ioloc;
         if (f->idefault == NULL || strlen(f->idefault) == 0)
            forced = 1;
	 else if (equal(f->idefault, cdb_T_PREV) && f->prev == NULL &&
	       crec_current->c_prev == NULL)
	    forced = 1;
	 else if (equal(f->idefault, cdb_T_NPREV) && f->prev == NULL &&
	       crec_current->c_prev == NULL)
	    forced = 1;

	 /* strange use of mode - but FB_FADD is just a bit pattern.
	  * i do not care if it is the same bit pattern accross machines.
	  */
	 
         pend = FB_OKEND;
	 for(;;){
	    s = FB_ERROR;
	    /* allow HELP, CTL-SIGNALs, ESCAPES */
	    isize = f->size;
	    save_cdb_edit_input = cdb_edit_input;
            if (cdb_edit_input == 0){
	       cdb_edit_input = 1;
	       cdb_e_buf[0] = NULL;
	       }
            else if (cdb_edit_input && dbvi_column_mode == 0)
               strcpy(cdb_e_buf, crec_current->c_cell[col_current->p_array]);
            else if (cdb_edit_input)
               cdb_e_buf[0] = NULL;
	    s = fb_input(-row, col, -isize, forced, f->type, 
		  inp, -(FB_ECHO), pend, FB_CONFIRM);
	    cdb_edit_input = save_cdb_edit_input;
	    if ((s == FB_AOK || s == FB_ESCAPE_AOK) && f->range != NULL){
	       if (fb_checkrange(f, inp) == FB_ERROR){
		  fb_serror(FB_MESSAGE, RANGE_MSG, NIL);
		  continue;
		  }
	       }
	    if (s != FB_QHELP)	/* if not fb_help and normal */
	       break;
	    if (s == FB_QHELP)			/* else print fb_help file */
	       fb_fhelp(f->help);
	    dbvi_display();
	    }
	 /* handle all the defaults */
         if (s == FB_DEFAULT && f->idefault != NULL){
	    makedef(inp, f);			/* make the default for f */
	    s = FB_AOK;
	    /* again, could check for uniqueness here if needed
	     * for (;;) , makedef, then this code.
	     * if (f->aid == NULL || f->aid->uniq <= 0)
	     *    break;
	     */
	    }
         if (s == FB_ESCAPE_AOK || s == FB_AOK || s == FB_DEFAULT){
	    if (FB_OFNUMERIC(f->type))
	       fb_rmlead(inp);
	    fb_trim(inp);
            strcpy(crec_current->c_cell[col_current->p_array], inp);
	    checklink(3, crec_current);
	    setdef(inp, f);		/* set defaults for next time */
            if (f->type !=FB_DATE || s == FB_DEFAULT){
	       if (inp[0] != NULL){
		  fb_formfield(line, inp, f->type, f->size);
		  strcpy(inp, line);
		  }
	       if (f->type != FB_DOLLARS)
		  fb_trim(inp);
	       p = inp;
	       fb_move(row, col);
	       fb_printw(FB_FSTRING, p);
	       }
            }
	 if (s == FB_END)
	    s = FB_ESCAPE_AOK;
         return(s);
      }
