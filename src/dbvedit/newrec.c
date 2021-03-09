/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: newrec.c,v 9.0 2001/01/09 02:55:59 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Newrec_sid[] = "@(#) $Id: newrec.c,v 9.0 2001/01/09 02:55:59 john Exp $";
#endif

#include <dbve_ext.h>

static char *FB_ABORT_MSG = "Really Abort Record? (y=yes, <other>=no):";

extern int modeless_status;	/* value of current modeless status */
extern int modeless_field;	/* value of current modeless data point */

extern short int cdb_e_st;
extern short int cdb_edit_input;
extern short int cdb_secure;
extern short int st_up;
extern char *cdb_T_AUTOINCR;

static autodef_off();

/* 
 *  newrec - create new record at end. allow entry into each fb_field.
 *     detection of the wacky autodefault signal is done here.
 *     this builds the record before knowing where it will physically go.
 *     edit_field uses stores, which puts fields in the hp->arec area.
 */
 
   newrec()
      {
         int fld, j, n_full;
	 fb_node *top, *ptop;
	 fb_page *p, *pt;
	 fb_node *n, *nt;
	 fb_field *f;

         st_up = 0;
         hp->orec[0] = hp->orec[1] = NULL;
 	 for (j = 0; j <= hp->nfields; j++)
	   FB_FLD(j, hp) = NIL;			/* all fields = true null */
         fb_clear_autoindex(hp);
         st = 0;
	 autodef = 0;
	 fb_scrstat(SYSMSG[S_FIELD_LEVEL]);
         strcpy(msg, SYSMSG[S_ADDMODE]);
	 fb_scrstat2(msg);
	 fb_nullall();
         for (p = phead; p; p = p->p_next){           	/* fld is 1 based */
	    pcur = p;
	    if (p->p_maxedit <= 0)
	       continue;
	    ncur = p->p_nedit[0];
            top = ncur;
            fb_display(1);
	    fb_checkformula(1);		/* for links on 'next' page */
            for (fld = 1; fld <= p->p_maxedit; ){
	       n = p->p_nedit[fld - 1];
	       f = n->n_fp;
               if (cdb_edit_input && cdb_e_st != 0){
                  st = cdb_e_st;
                  cdb_e_st = 0;
                  }
	       else if (f->type != FB_FORMULA && f->dflink == NULL &&
                     !n->n_readonly && f->type != FB_BINARY){
                  st = edit_field(-fld, top, 1);	/* 1 == addf flag */
		  if (f->type == FB_CHOICE)
		     fb_display(1);
		  }
	       else
	          st = FB_ERROR;
	       modeless_status = st;
	       if (st == FB_AOK || st == FB_DEFAULT)
	          fb_checkformula(fld);
               if (st == FB_END || st == FB_QSIGNAL){
	          /* if there is data somewhere, do not allow an FB_END */
		  n_full = 0;
		  for (st = FB_END, pt = phead; pt; pt = pt->p_next){
		     for (nt = pt->p_nhead; nt; nt = nt->n_next){
		        f = nt->n_fp;
	                if (f != NULL && 
			      f->type != FB_FORMULA && f->dflink == NULL &&
                              f->type != FB_BINARY &&
	                      f->fld != NIL && f->fld[0] != NULL){
			   if (f->idefault == NULL || 
			         !(equal(f->idefault,cdb_T_AUTOINCR))){
			      n_full++;
			      break;
			      }
			   }
			}
		     }
		  if (n_full == 0)
		     st = FB_END;
                  else
		     st = FB_AOK;
                  if (cdb_secure)
                     fb_recmode(hp, FB_BLANK, fb_getuid(), fb_getgid(), "666");
                  else
                     fb_store(hp->kp[hp->nfields], " ", hp); /* so no thrashing */
                  return(st);
		  }
	       else if (st == FB_ABORT){
	          st = fb_mustbe(CHAR_y, FB_ABORT_MSG, cdb_t_lines, 1);
                  if (st == FB_AOK)
		     return(FB_ABORT);
		  continue;
	          }
	       if (st != FB_AOK){
		  fb_fetch(f, cdb_afld, hp);
		  fb_putfield(n, f, cdb_afld);
		  }
	       if (st == FB_YSIGNAL || (st == FB_ERROR && st_up)){
                  autodef_off();
	          if (fld > 1){
		     fld--;
		     st_up = 1;
		     continue;
		     }
		  else{
		     st_up = 0;
		     continue;
		     }
		  }
	       else if (st == FB_ESIGNAL)
                  autodef_off();
	       else if (st == FB_DSIGNAL){
		  autodef = 1;
	          fb_scrstat2(SYSMSG[S_AUTODEF_MODE]);
		  fb_refresh();
		  continue;
		  }
	       else if (st == FB_PAGEUP){
                  autodef_off();
		  ptop = top;
		  if (pcur->p_prev != NULL)
		     pcur = pcur->p_prev;
		  else
		     pcur = ptail;
		  top = ncur = pcur->p_nhead;
		  if (pcur->p_maxedit > 0)
		     top = ncur = pcur->p_nedit[0];
		  else
		     top = ncur = pcur->p_nhead;
		  if (ptop != top){
		     fb_display(1);
		     fb_checkformula(1);
		     }
		  fld = 0;
		  p = pcur;
		  }
	       else if (st == FB_PAGEDOWN){
                  autodef_off();
		  ptop = top;
		  if (pcur->p_next != NULL)
		     pcur = pcur->p_next;
		  else
		     pcur = phead;
		  if (pcur->p_maxedit > 0)
		     top = ncur = pcur->p_nedit[0];
		  else
		     top = ncur = pcur->p_nhead;
		  if (ptop != top){
		     fb_display(1);
		     fb_checkformula(1);
		     }
		  fld = 0;
		  p = pcur;
		  }
               if (cdb_edit_input && cdb_e_st != 0)
                  continue;		/* to do the saved command */
	       st_up = 0;
	       if (st != FB_CSIGNAL)
	          fld++;
               }
            }
						/* set del fb_field */
         if (cdb_secure)
	    fb_recmode(hp, FB_BLANK, fb_getuid(), fb_getgid(), "666");
         else
            fb_store(hp->kp[hp->nfields], " ", hp); /* so no thrashing */
	 return(st);
      }

   static autodef_off()
      {
         if (autodef){
            autodef = 0;
            fb_scrstat2(NIL);
            fb_refresh();
            }
      }
