/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: cformula.c,v 9.0 2001/01/09 02:56:04 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Checkformula_sid[] = "@(#) $Id: cformula.c,v 9.0 2001/01/09 02:56:04 john Exp $";
#endif

#include <dbvi_ext.h>

static upperlink();
static void lowerlink();

/*
 * checklink - for all fields on screen, redraw those of type L
 *	iff the reference value has changed.
 *	if a value Is changed, return FB_AOK, else return FB_ERROR.
 *	Level controls screen sections: 1=top, 2=bottom, 3=both.
 */

   checklink(level, cp)
      int level;
      crec *cp;

      {
         int lcount = 0;

	 if (level == 1 || level >= 3)
	    lcount = upperlink(cp);
	 if (level == 2 || level >= 3)
	    lowerlink(lcount, cp);
      }

/* 
 * upperlink - do FB_LINKS/FB_FORMULAS on upper half of screen.
 */

   static upperlink(cp)
      crec *cp;

      {
         int lcount = 0;
	 fb_field *f;
	 fb_link *ak;
	 fb_node *n;
	 column *p;

         if (pcur == NULL)
	    return(0);
	 for(n = pcur->p_nhead; n; n = n->n_next){
	    if ((f = n->n_fp) == NULL)
	       continue;
	    if (f->dflink != NULL || f->type == FB_FORMULA)
	       lcount++;
	    }
	 if (lcount <= 0)
	    return(0);
	 for (p = col_mhead; p; p = p->p_next){
	    f = p->p_field;
	    fb_store(f, cp->c_cell[p->p_array], cdb_db);
	    }
	    				/* FB_LINK section */
	 for(n = pcur->p_nhead; n; n = n->n_next){
	    if ((f = n->n_fp) == NULL)
	       continue;
	    if (f->dflink != NULL){
	       ak = f->dflink;
	       if (!equal(ak->f_xfld, ak->f_tix->fld))
		  fb_s_getlink(ak);
	       if (f->type != FB_FORMULA){
		  strcpy(cdb_afld, f->fld);
		  putfield(n, f, cdb_afld);
		  }
	       }
	    }
	    				/* FB_FORMULA section */
	 for(n = pcur->p_nhead; n; n = n->n_next){
	    if ((f = n->n_fp) == NULL)
	       continue;
	    if (f->type == FB_FORMULA){	/* anh thuong em */
	       fb_fetch(f, cdb_afld, cdb_db);
               putfield(n, f, cdb_afld);
	       }
	    }

	 return(lcount);
      }

/* 
 * lowerlink - do FB_LINKS/FB_FORMULAS on lower half of screen.
 */

   static void lowerlink(up_lcount, cp)
      int up_lcount;		/* lcount from running upperlink */
      crec *cp;

      {
         int lcount = 0;
	 fb_field *f;
	 fb_link *ak;
	 column *p;

	 for(p = col_phead; p; p = p->p_next){
	    if ((f = p->p_field) == NULL)
	       continue;
	    if (f->dflink != NULL || f->type == FB_FORMULA)
	       lcount++;
	    if (p == col_ptail)
	       break;
	    }
	 if (lcount <= 0)
	    return;
	 if (up_lcount <= 0){
	    for (p = col_mhead; p; p = p->p_next){
	       f = p->p_field;
	       fb_store(f, cp->c_cell[p->p_array], cdb_db);
	       if (p == col_ptail)
		  break;
	       }
	    }
	 for(p = col_phead; p; p = p->p_next){
	    if ((f = p->p_field) == NULL)
	       continue;
	    if (f->dflink != NULL){
	       ak = f->dflink;
	       if (!equal(ak->f_xfld, ak->f_tix->fld))
		  fb_s_getlink(ak);
	       if (f->type != FB_FORMULA){
		  strcpy(cp->c_cell[p->p_array], f->fld);
		  put_cell(cp->c_cell, whichrow(cp), p, 0);
		  }
	       }
	    if (p == col_ptail)
	       break;
	    }
	 for(p = col_phead; p; p = p->p_next){
	    if ((f = p->p_field) == NULL)
	       continue;
	    if (f->type == FB_FORMULA){	/* anh thuong em */
	       fb_fetch(f, cdb_afld, cdb_db);
	       strcpy(cp->c_cell[p->p_array], cdb_afld);
	       put_cell(cp->c_cell, whichrow(cp), p, 0);
	       }
	    if (p == col_ptail)
	       break;
	    }
      }

/*
 * clearlink - for all fields on screen, clear out remembered value.
 */

   clearlink()

      {
	 fb_field *f;
	 fb_link *ak;
	 fb_node *n;
	 column *p;

	 if (pcur != NULL)	 
	    for(n = pcur->p_nhead; n; n = n->n_next){
	       if ((f = n->n_fp) == NULL)
		  continue;
	       if (f->dflink != NULL){
		  ak = f->dflink;
		  ak->f_xfld[0] = NULL;
		  }
	       }
	 for(p = col_phead; p; p = p->p_next){
	    if ((f = p->p_field) == NULL)
	       continue;
	    if (f->dflink != NULL){
	       ak = f->dflink;
	       ak->f_xfld[0] = NULL;
	       }
	    }
      }
