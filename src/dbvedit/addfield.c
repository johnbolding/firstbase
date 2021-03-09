/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: addfield.c,v 9.0 2001/01/09 02:55:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Addfield_sid[] = "@(#) $Id: addfield.c,v 9.0 2001/01/09 02:55:57 john Exp $";
#endif

#include <dbve_ext.h>

static char *MSG  = "Auto Field Level";

extern int modeless_status;	/* value of current modeless status */
extern int modeless_field;	/* value of current modeless data point */

extern short int cdb_e_st;
extern short int cdb_edit_input;
short int st_up = 0;
extern short int cdb_ex_forceone;/* exchioce force one when no others */

/* 
 *  addfield - for use on an existing record.
 *	allow entry into each fb_field of the record on current page starting at
 *	the passed in fld or 1. An FB_END returns to record level.
 *	A <CTL>X exits also, but COULD be remapped elsewhere.
 *      note: edit_field uses stores, which puts fields in the hp->arec area.
 */

   addfield(fld, top)
      int fld;
      fb_node *top;
      
      {
         int changes = 0;
	 fb_node *n;
	 fb_field *f;

         st_up = 0;
         if (fld <= 1 || fld > pcur->p_maxedit)
	    fld = 1;
	 if (fld == pcur->p_maxedit)
	    st_up = 1;
	 fb_scrstat(MSG);
         for (; fld >= 1 && fld <= pcur->p_maxedit; ){   /* fld is 1 based */
	    n = pcur->p_nedit[fld - 1];
	    f = n->n_fp;
            if (cdb_edit_input && cdb_e_st != 0){
               st = cdb_e_st;
               cdb_e_st = 0;
               }
	    else if (f->type != FB_FORMULA && f->dflink == NULL &&
                  !n->n_readonly && f->type != FB_BINARY)
	       st = edit_field(fld, top, 1);	/* 1 == addf flag */
	    else
	       st = FB_ERROR;
	    if (st == FB_AOK){
               /* old code was nulling out dup_fld here. wrong, i think */
	       fb_checkformula(fld);
	       }
	    else{
	       fb_fetch(f, cdb_afld, hp);
	       fb_putfield(n, f, cdb_afld);
	       }
	    modeless_status = st;
	    modeless_field = fld;

            /* this next condition is special cased for FB_EXCHOICE. see below. */
            if (st == FB_AOK && f->type == FB_EXCHOICE && cdb_ex_forceone && st_up)
               st = FB_ESCAPE_AOK;

	    switch(st){
	       case FB_ABORT:
	       case FB_WSIGNAL:
	       case FB_DELSIGNAL:
	       case FB_PSIGNAL:
	       case FB_PAGEUP:
	       case FB_PAGEDOWN:
	       case FB_END:
	       case FB_DSIGNAL:
		  return(changes);
	       case FB_ERROR:
		  if (!st_up)		/* if last action was not YSIG */
		     break;		/* ... then break out, else do YSIG */
		  /* fall through- to FB_YSIGNAL */
	       case FB_YSIGNAL:
		  fld--;
		  st_up = 1;
		  continue;
	       case FB_AOK:
	       case FB_DEFAULT:
		  changes++;
		  break;
               /*
                * this in only interpreted from edit_field on exchoice
                * when the condition st_up is lit and EX_fb_forceONE is ON
                * to enable backwards motion over a forced FB_EXCHOICE fb_field.
                *
                * used to be this was tied to FB_AOK and FB_DEFAULT just above here,
                * but i did not like that behavior.
                */
	       case FB_ESCAPE_AOK:
		  changes++;
		   if (st_up == 1){
		     fld--;
		     continue;
		     }
		  break;
	       case FB_ESIGNAL:
	       case FB_QSIGNAL:
	       default:
		  break;
	       }
            if (cdb_edit_input && cdb_e_st != 0)
               continue;		/* to do the saved command */
	    if (st != FB_CSIGNAL)
	       fld++;
	    else
	       changes++;
	    st_up = 0;
            }
         if (fld > pcur->p_maxedit){
	    modeless_status = 0;
	    modeless_field = 0;
	    }
	 return(changes);
      }
