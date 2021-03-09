/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: edit.c,v 9.2 2001/02/16 19:49:16 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Edit_sid[] = "@(#) $Id: edit.c,v 9.2 2001/02/16 19:49:16 john Exp $";
#endif

#include <dbve_ext.h>

static char *MSG = "Field must be Non-Empty:";
static char *WRITEMSG = "Write Record Now? (y=yes, n=no, <other>=cancel):";
static char *FB_ABORT_MSG = "Really Abort Record? (y=yes, <other>=no):";
extern char *cdb_prompt_recordmsg;
extern char *cdb_dbvedit_rec_ploc;
extern short int cdb_dbvedit_rec_pilength;
extern short int cdb_askwrite;
short int cdb_create_it;
short int cdb_write_it;
short int cdb_did_create_it;
short int cdb_did_write_it;
short int cdb_end_keystroke = 0;

static char *HLP_V_EDFLD = 	"v_edfld.hlp";
static char ecom[FB_MAXINPUT];

/* 
 *  edit - edit a single record. could be that the lock will fail.
 *	and no editing will occur. top refers to top of screen. 
 *	the global 'rec' has the physical record number in it.
 *	if rec is -1, then it is an add rec, with actual rec calced later.
 */

   edit(top)
      fb_node *top;
   
      {
         char *fb_trim(), *cp;
         int fld, i, j = 0, addf, dflag = 0, n_autos = 0;
	 int disp_st, ploc_row, ploc_col, col, icol;
	 fb_node *n, *ptop;
	 fb_page *p;
	 fb_field *f;
   
	 ploc_row = cdb_t_lines;
	 ploc_col = 1;
	 if (cdb_dbvedit_rec_ploc != NULL){
	    strcpy(ecom, cdb_dbvedit_rec_ploc);
	    cp = strchr(ecom, ',');
	    *cp = NULL;
	    ploc_row = atoi(ecom);
	    ploc_col = atoi(cp + 1);
	    }

	 autodef = 0;				/* paranoia */
         cdb_create_it = 0;
         cdb_write_it = 0;
         cdb_did_create_it = 0;
         cdb_did_write_it = 0;
         fb_macrobegin(rec);
         if (rec == -1L){			/* an ADD record */
            if (scanner == 1)
               return(REDRAW);
            if ((st = newrec()) == FB_ABORT){
	       fb_fmessage(NIL);
               fb_serror(FB_MESSAGE, SYSMSG[S_RECORD], SYSMSG[S_NO_CREATE]);
               return(REDRAW);
               }
            else if (st == FB_END){
               mode = NULL;
	       oldrec = 0L;			/* position pointer at top */
               return(REDRAW);
               }
            cdb_create_it = 1;
	    autodef = 0;			/* in case it was turned on */
	    pcur = ptail;
	    if (pcur->p_maxedit > 0)
	       top = ncur = pcur->p_nedit[0];
	    else
	       top = ncur = pcur->p_nhead;
	    if (phead != ptail)
	       dflag = 1;			/* force complete redisplay */
            }
	 if (rec < 0)
	    strcpy(msg, SYSMSG[S_NEW]);
	 else if (hp->ifd > 0 && strlen(hp->irec) > 0)
	    sprintf(msg, SYSMSG[S_EDIT_FMT3], irec[pindx]);
	 else
	    sprintf(msg, SYSMSG[S_EDIT_FMT4], rec);
	 if (pcur->p_maxedit > 0)
	    sprintf(msg, SYSMSG[S_EDIT_FMT2], msg, 
	       pcur->p_nedit[0]->n_fp->fld);
	 fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
	 fb_scrstat2(msg);			/* individual record status */
         fb_display(dflag);
         for(;;) {
            fb_move(cdb_t_lines,1); fb_clrtoeol();
            fb_move(ploc_row, ploc_col);
	    ecom[0] = NULL;
	    fb_printw(cdb_prompt_recordmsg);
	    fb_percentage(pcur->p_num, ptail->p_num);
            fb_cx_set_viewpage(pcur->p_num);
            if (scanner == 0)
               fb_cx_push_env("WXNP", CX_VEDIT_SELECT, NIL); /* reclev */
            else
               fb_cx_push_env("E", CX_KEY_SELECT, NIL); 	/* reclev-scan*/
            if (pcur->p_next != NULL || pcur->p_prev != NULL)
               fb_cx_add_buttons("GU");
            if (ploc_col > 0)
               col = ploc_col + strlen(cdb_prompt_recordmsg);
            else
               col = ploc_col;
            fb_cx_add_buttons("H");
            st = fb_input(-ploc_row, col + 2, -cdb_dbvedit_rec_pilength, 
               0, FB_ALPHA, ecom, FB_ECHO, FB_OKEND, FB_CONFIRM);
            fb_cx_pop_env();
            fb_move(ploc_row, ploc_col);
            fb_clrtoeol();
	    fb_trim(ecom);
	    if (st == FB_AOK && equal(ecom, SYSMSG[S_DEL])){
	       sprintf(ecom, FB_FDIGITS, hp->nfields + 1);
	       st = FB_DELETED;
	       }
	    else if (st == FB_AOK && equal(ecom, SYSMSG[S_PUT]))
	       st = FB_PUT;
            if (st == FB_END || st == FB_WSIGNAL){
               st = FB_END;
               cdb_end_keystroke = 1;
	       for (i = 0, p = phead; p; p = p->p_next){
	          /* j marks the position of the fb_field for the sim. command */
		  for (j = 0, n = p->p_nhead; n; n = n->n_next){
		     f = n->n_fp;
		     if (f != NULL)
		        j++;
	             if (f != NULL && 
		           (f->idefault == NULL || strlen(f->idefault)==0) &&
		              f->fld == NIL){
		        fb_serror(FB_MESSAGE, MSG, f->id);
		        i++;
			break;
		        }
		     }
		  if (i > 0)
		     break;
		  }
	       if (i > 0){	/* errors exist -- try and sim edit fb_field */
	          st = FB_AOK;
		  sprintf(ecom, FB_FDIGITS, j);
		  if (p != pcur){
		     pcur = p;
		     if (pcur->p_maxedit > 0)
			top = ncur = pcur->p_nedit[0];
		     else
			top = ncur = pcur->p_nhead;
		     fb_display(1);
		     fb_checkformula(1);
		     }
	          }
               /* no errors - do macro if applicapable */
               else if ((st = fb_macroend(0)) == FB_ERROR)
                  continue;	/* cop out, but this is the thing to do */
	       else {		/* no errors - create or write and out */
	          if ((!cdb_create_it && !cdb_write_it) || !cdb_askwrite)
		     break;
                  cp = WRITEMSG;
		  icol = strlen(cp) + 1;
		  fb_move(cdb_t_lines, 1);
		  fb_prints(cp);
                  fb_cx_push_env("ynx", CX_KEY_SELECT, NIL);
		  st = fb_input(cdb_t_lines, icol, 1, 0, FB_ALPHA,
		        ecom, FB_ECHO, FB_OKEND, FB_CONFIRM);
                  fb_cx_pop_env();
		  if (st == FB_AOK){
		     if (ecom[0] == CHAR_n || ecom[0] == CHAR_N){
		        cdb_create_it = cdb_write_it = 0;
		        break;
		        }
		     else if (ecom[0] == CHAR_y || ecom[0] == CHAR_Y){
		        break;
		        }
		     }
		  continue;  /* to top of loop for next command */
		  }
	       }
            if (st == FB_ABORT){
               if (scanner == 1)
                  return(REDRAW);
	       if (cdb_write_it == 1 || cdb_create_it == 1){
	          st = fb_mustbe(CHAR_y, FB_ABORT_MSG, cdb_t_lines, 1);
	          if (st == FB_AOK){
                     fb_serror(FB_MESSAGE, SYSMSG[S_RECORD], SYSMSG[S_NO_UPDATE]);
	             /* unlock(rec); */
                     return(REDRAW);
		     }
		  }
	       else{
	          /* unlock(rec); */
                  return(REDRAW);
	          }
               }
	    else if (st == FB_DSIGNAL)
	       fb_infotoggle();
            else if (st == FB_ESIGNAL){
	       if (scanner == 0){
		  if (addfield(1, top) > 0)
		     cdb_write_it = 1;
		  fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
		  }
	       }
            else if (st == FB_YSIGNAL){
	       if (scanner == 0){
		  if (addfield(pcur->p_maxedit, top) > 0)
		     cdb_write_it = 1;
		  fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
	          }
	       }
            else if (st == FB_DEFAULT || st == FB_PAGEDOWN || ecom[0] == CHAR_f){
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
               }
            else if (ecom[0] == CHAR_b || st == FB_PAGEUP){
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
               }
	    else if (ecom[0] == CHAR_v || ecom[0] == CHAR_x){
	       st = fb_visual(ecom, scanner, ((ecom[0] == CHAR_v) ? 1 : 0));
	       fb_scrhdr(cdb_db, SYSMSG[S_RECORD_LEVEL]);
	       fb_scrlbl(hp->sdict);
	       fb_scrstat2(msg);		/* individual record status */
	       fb_display(1);
	       fb_checkformula(1);
	       if (scanner == 0 && st == FB_AOK)
	          cdb_write_it = 1;
	       }
	    else if (ecom[0] == CHAR_d){	/* display fb_field */
	       fb_dfield(ecom);
	       fb_display(1);
	       }
	    else if (ecom[0] == CHAR_BANG){		/* cshell */
	       fb_cshell(ecom);
	       fb_serror(FB_MESSAGE, NIL, NIL);
	       fb_scrhdr(cdb_db, SYSMSG[S_RECORD_LEVEL]);
	       fb_scrlbl(hp->sdict);
	       fb_scrstat2(msg);		/* individual record status */
	       fb_display(1);
	       fb_checkformula(1);
	       }
	    else if (st == FB_QHELP){
	       fb_move(3, 1); fb_clrtobot();
	       if (pcur->p_help == NULL)
	          fb_fhelp(HLP_V_EDFLD);
	       else
	          fb_fhelp(pcur->p_help);
               fb_display(1);
	       }
            else if  (ecom[0] == FB_HELP){
	       fb_move(3, 1); fb_clrtobot();
	       fb_help(ecom, hp);
               fb_display(1);
               }
	    else if (ecom[0] == FB_DEREF){	/* '@' for auto fb_field */
	       if (scanner == 0){
		  j = atoi(ecom+1);
		  if (addfield(j, top) > 0)
		     cdb_write_it = 1;
		  fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
		  }
	       }
	    else if (ecom[0] == 'p'){		/* 'p' for fb_page N */
	       j = atoi(ecom+1);
	       for (p = phead; p; p = p->p_next)
	          if (p->p_num == j)
		     break;
	       if (p == NULL)
                  fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
	       else{
	          pcur = p;
		  if (pcur->p_maxedit > 0)
		     top = ncur = pcur->p_nedit[0];
		  else
		     top = ncur = pcur->p_nhead;
		  fb_display(1);
		  fb_checkformula(1);
	          }
	       }
            else {
               fld = atoi(ecom);
               i = strlen(fb_trim(ecom)) - 1;
               if (ecom[i] != NULL && ecom[i] != FB_BLANK && !isdigit(ecom[i])&&
                   scanner == 0 && fld >= 1 && fld <= pcur->p_maxedit){
		  fb_scrstat(SYSMSG[S_EADD_LEVEL]);
                  st = edit_add(fld);       /* edit_add flag */
		  fb_fmessage(NIL);
                  if (st == FB_AOK){
                     cdb_write_it = 1;
		     fb_checkformula(fld);
		     }
                  else if (st == FB_ABORT)
                     fb_serror(FB_MESSAGE, SYSMSG[S_FIELD], SYSMSG[S_NO_UPDATE]);
		  fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
                  }
               else if (fld <= 0 || (fld > pcur->p_maxedit && st != FB_DELETED))
                  fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
               else if ((fld >= 1 && fld <= pcur->p_maxedit) || st==FB_DELETED){
	          fb_scrstat(SYSMSG[S_FIELD_LEVEL]);
		  if (st != FB_DELETED)
		     addf = 0;
		  else
		     addf = -1;
                  st = edit_field(fld, top, addf);
		  fb_fmessage(NIL);
                  if (st == FB_AOK || st == FB_DEFAULT || st == FB_UNDELETED ||
                        st == FB_DELETED){
                     cdb_write_it = 1;
		     if (st == FB_AOK || st == FB_DEFAULT)
		        fb_checkformula(fld);
		     }
                  else if ((st == FB_END || st == FB_ESIGNAL || st ==FB_YSIGNAL
                        || st == FB_ABORT || st == FB_DSIGNAL || st == FB_QSIGNAL)
                        && scanner == 0){
		     n = pcur->p_nedit[fld-1];
		     f = n->n_fp;
		     fb_fetch(f, cdb_afld, hp);
                     fb_putfield(n, f, cdb_afld);
                     if (st == FB_ABORT)
                        fb_serror(FB_MESSAGE, SYSMSG[S_FIELD], SYSMSG[S_NO_UPDATE]);
                     else
                        st = FB_ABORT;
                     }
		  fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
                  if (st == FB_DELETED || st == FB_UNDELETED)
                     break;
                  }
               else
                  fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
               }
            }
         if (cdb_write_it == 1 || cdb_create_it == 1){
	    if (cdb_create_it == 1)
	       fb_fmessage(SYSMSG[S_CREATING]);
	    else
	       fb_fmessage(SYSMSG[S_WRITING]);
/*	    lock(0L, FB_WAIT);
*	    fb_setdirty(hp, 1);
*	    if (cdb_create_it == 1 || st == FB_DELETED || st == FB_UNDELETED){
*	       if (fb_gethead(hp) == FB_ERROR){
*	          unlock(0L);
*		  fb_xerror(FB_READ_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
*		  }
*	       if (cdb_create_it == 1)
*	          rec = ++(hp->reccnt);
*	       else if (st == FB_DELETED)
*		  hp->delcnt++;
*	       else
*		  hp->delcnt--;
*	       if (fb_puthead(hp->fd, hp->reccnt, hp->delcnt) == FB_ERROR)
*		  fb_xerror(FB_WRITE_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
*	       }
*/
	    /* if a putrec causes fb_xerror, rec 0 is still locked
	     * this will keep others out of the dbase until the
	     * problem can be fixed. this is a feature, not a bug.
	     */

	    output(hp);
/*
*	    hp->rec = rec;
*	    if ((n_autos = fb_checkauto(hp)) == FB_ERROR)
*	       fb_serror(FB_BAD_INDEX, hp->dbase, INCRMESG);
*	    fb_putlog(hp);
*            if (fb_putrec(rec, hp)==FB_ERROR)
*	       fb_xerror(FB_WRITE_ERROR, SYSMSG[S_RECORD], hp->dbase);
*	    fb_checklog(hp);
*	    if (autoindex(hp, rec, cdb_autoregen) == FB_ERROR)
*	       fb_serror(FB_BAD_INDEX, hp->dbase, NIL);
*	    fb_setdirty(hp, 0);
*
*	    fb_allsync(hp);
*
*	    unlock(0L);
*/

            }

/*	 if (scanner == 0 && cdb_create_it != 1)
*	    unlock(rec);
*/
         if (cdb_create_it == 1 || n_autos > 0){
	    /* showauto(hp); */
	    disp_st = ZAP_REDRAW;
	    }
	 else
	    disp_st = REDRAW;
         cdb_did_write_it = cdb_write_it;
         cdb_did_create_it = cdb_create_it;
         if (cdb_did_write_it == 1 || cdb_did_create_it == 1)
            fb_macroend(1);
	 return(disp_st);
      }
