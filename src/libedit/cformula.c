/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: cformula.c,v 9.0 2001/01/09 02:56:39 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Checkformula_sid[] = "@(#) $Id: cformula.c,v 9.0 2001/01/09 02:56:39 john Exp $";
#endif

#include <dbve_ext.h>

/* 
 * checkformula - if fb_field is not in [$fn], return (since it could not
 *    affect any virtual (formula) fields on the screen.
 *    - else (for all fields on screen, redraw those of type F)
 */
 
   void fb_checkformula(fld)
      int fld;
     
      {
         int st_link;
	 fb_field *f;
	 fb_link *ak;
	 fb_node *n;
	 
	 if (pcur->p_maxedit <= 0)
	    return;
	 st_link = fb_check_valuelink();
         if (fld <= 0)
            return;
	 n = pcur->p_nedit[fld-1];
	 f = n->n_fp;
	 /* if Not FB_OFNUMERIC AND no changed links, nothing to do */
         if (!(FB_OFNUMERIC(f->type)) && st_link == FB_ERROR &&
               f->f_macro == NULL)
	    return;
         for(n = pcur->p_nhead; n; n = n->n_next){
	    f = n->n_fp;
	    if (f == NULL)
	       continue;
	    if (f->type == FB_FORMULA){	/* anh thuong em */
	       if (f->dflink == NULL)
	          fb_getformula(f, f->idefault, cdb_afld, 0, hp);
	       else{
	          ak = f->dflink;
	          fb_getformula(ak->f_ffp, ak->f_ffp->idefault, ak->f_fld, 0,
		     ak->f_dp);
	          ak->f_tfp->fld = ak->f_fld;
		  strcpy(cdb_afld, ak->f_tfp->fld);
		  }
               fb_putfield(n, f, cdb_afld);
	       }
            }
      }

/*
 * check_valuelink - for all fields on screen, redraw those of type L
 *	iff the reference value has changed.
 *	if a value Is changed, return FB_AOK, else return FB_ERROR.
 *
 *	ALSO: check all fields some can be used (macros) but now displayed
 */

   int fb_check_valuelink()
      {
         int st = FB_ERROR;
	 fb_field *f;
	 fb_link *ak;
	 fb_node *n;
         int i;
	 
         for(n = pcur->p_nhead; n; n = n->n_next){
	    f = n->n_fp;
	    if (f == NULL)
	       continue;
	    if (f->dflink != NULL){
	       ak = f->dflink;
	       if (!equal(ak->f_xfld, ak->f_tix->fld)){
	          fb_s_getlink(ak);
		  st = FB_AOK;
		  }
               /*
                * since the screen mechanism takes care of optimization
                * its FB_AOK to print these every time
                */
               if (f->type != FB_FORMULA){	/* FORMULAS are put above */
                  strcpy(cdb_afld, f->fld);
                  fb_putfield(n, f, cdb_afld);
                  }
	       }
            }
         /* check all the fields now. for macros */
         for (i = 0; i < cdb_db->nfields; i++){
            f = cdb_db->kp[i];
	    if (f->dflink != NULL){
	       ak = f->dflink;
	       if (!equal(ak->f_xfld, ak->f_tix->fld)){
	          fb_s_getlink(ak);
		  st = FB_AOK;
		  }
               }
            }
	 return(st);
      }

/*
 * check_abslink - check for any absolute links and print them
 *	called from scanfb_put()
 */

   fb_check_abslink()
      {
         int st = FB_ERROR;
	 fb_field *f;
	 fb_link *ak;
	 fb_node *n;
	 
         for(n = pcur->p_nhead; n; n = n->n_next){
	    f = n->n_fp;
	    if (f == NULL)
	       continue;
	    if (f->dflink != NULL){
	       ak = f->dflink;
	       if (ak->f_absrec > 0){
	          fb_s_getlink(ak);
		  st = FB_AOK;
                  if (f->type != FB_FORMULA){	/* FORMULAS are put above */
                     strcpy(cdb_afld, f->fld);
                     fb_putfield(n, f, cdb_afld);
                     }
		  }
	       }
            }
	 return(st);
      }
