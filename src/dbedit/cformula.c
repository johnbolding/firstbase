/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: cformula.c,v 9.0 2001/01/09 02:55:33 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Checkformula_sid[] = "@(#) $Id: cformula.c,v 9.0 2001/01/09 02:55:33 john Exp $";
#endif

#include <dbedit.h>

#if !FB_PROTOTYPES
static int checklink();
#else
static int checklink(int);
#endif

/* 
 * checkformula - if fb_field is not in [$fn], return (since it could not
 *    affect any virtual (formula) fields on the screen.
 *    - else (for all fields on screen, redraw those of type F)
 */
 
   void db_checkformula(fld, top)
      int fld, top;
     
      {
         int i, row, k, st_link;
	 fb_field *f;
	 fb_link *ak;
	 
	 st_link = checklink(top);
	 f = cdb_sp[fld-1];
	 /* if Not OFNUMERIC AND there were no changed links, nothing to do */
         if (!(FB_OFNUMERIC(f->type)) && st_link == FB_ERROR)
	    return;
         for(k = top - 1, row = 2, i = 1; i <= 10; i++){
            if (k >= cdb_sfields)     		/* no deletion fb_field */
               break;
	    if (cdb_sp[k]->type == FB_FORMULA){	/* anh thuong em */
	       if (cdb_sp[k]->dflink == NULL)
	          fb_getformula(cdb_sp[k], cdb_sp[k]->idefault, cdb_afld, 0, hp);
	       else{
	          ak = cdb_sp[k]->dflink;
	          fb_getformula(ak->f_ffp, ak->f_ffp->idefault, ak->f_fld, 0,
		     ak->f_dp);
	          ak->f_tfp->fld = ak->f_fld;
		  strcpy(cdb_afld, ak->f_tfp->fld);
		  }
               db_putfield(top, cdb_sp[k], cdb_afld, row);
	       }
	    top++; k++; row++;
            }
      }

/*
 * checklink - for all fields on screen, redraw those of type L
 *	iff the reference value has changed.
 *	if a value Is changed, return FB_AOK, else return FB_ERROR.
 */

   static int checklink(top)
      int top;

      {
         int i, row, k, st = FB_ERROR;
	 fb_link *ak;

         for(k = top - 1, row = 2, i = 1; i <= 10; i++){
            if (k >= cdb_sfields)     			/* no deletion field */
               break;
	    if (cdb_sp[k]->dflink != NULL){
	       ak = cdb_sp[k]->dflink;
	       if (!equal(ak->f_xfld, ak->f_tix->fld)){
	          fb_s_getlink(ak);
		  if (cdb_sp[k]->type != FB_FORMULA){ /* FORMULAS put above */
		     strcpy(cdb_afld, cdb_sp[k]->fld);
                     db_putfield(top, cdb_sp[k], cdb_afld, row);
		     }
		  st = FB_AOK;
		  }
	       }
	    top++; k++; row++;
            }
	 return(st);
      }
