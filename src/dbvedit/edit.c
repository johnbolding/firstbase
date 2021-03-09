/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: edit.c,v 9.2 2001/02/16 19:42:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Edit_sid[] = "@(#) $Id: edit.c,v 9.2 2001/02/16 19:42:57 john Exp $";
#endif

#include <dbve_ext.h>

static askwrite();

static char *MSG = "Field must be Non-Empty:";
static char *WRITEMSG = "Write Record Now? (y=yes, n=no, <other>=cancel):";
static char *INCRMESG = "AutoIncr Error";
static char *FB_ABORT_MSG = "Really Abort Record? (y=yes, <other>=no):";
extern char *cdb_prompt_recordmsg;    /* prompt for record level message */
extern char *cdb_dbvedit_rec_ploc;
extern short int cdb_dbvedit_rec_pilength;
extern short int cdb_record_level;
extern short int cdb_record_window;
extern short int cdb_serror_window;
extern char *cdb_pgm;
extern short int cdb_secure;
extern short int cdb_askwrite;
extern char *cdb_putfile;
extern short int cdb_usrlog;
extern short int cdb_scr_autoincr;
extern short int cdb_lockmessage;

int modeless_status;	/* value of current modeless status */
int modeless_field;	/* value of current modeless data point */
short int cdb_create_it;
short int cdb_write_it;
short int cdb_did_create_it;
short int cdb_did_write_it;
short int cdb_end_keystroke = 0;
short int cdb_record_is_locked;
static int n_autos;
static char ecom[FB_MAXINPUT];

static char *HLP_V_EDFLD = 	"v_edfld.hlp";

static writerec();
static testexit();

/* 
 *  edit - edit a single record. could be that the lock will fail.
 *	and no editing will occur. top refers to top of screen. 
 *	the global 'rec' has the physical record number in it.
 *	if rec is -1, then it is an add rec, with actual rec calced later.
 */

   edit(top)
      fb_node *top;
   
      {
         char *fb_trim(), *cp, tbuffer[10];
         int fld, i, j, addf, dflag = 0, icol;
	 int disp_st, b_end, col, ploc_row, ploc_col, t_st;
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

         cdb_record_is_locked = 0;
	 autodef = 0;				/* paranoia */
         if (scanner == 0 && rec != -1L){	/* ie, not in ADD mode */
	    if (fb_lock(rec, hp, FB_NOWAIT) != FB_AOK) /* dont wait if locked*/
	       return(REDRAW);
            cdb_record_is_locked = 1;
            }
         cdb_create_it = 0;
         cdb_write_it = 0;
         cdb_did_create_it = 0;
         cdb_did_write_it = 0;
         n_autos = 0;
	 modeless_field = 1;
	 modeless_status = 0;
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
         else if (fb_getrec(rec, hp) == FB_ERROR)
            fb_xerror(FB_FATAL_GETREC, cdb_pgm, (char *) &rec);
         else if (cdb_secure &&
               fb_record_permission(hp, (scanner ? READ: WRITE)) == FB_ERROR){
            fb_serror(FB_MESSAGE, "Record Permission Denied", NIL);
            return(REDRAW);
            }
	 else {					/* successful getrec */

	    /*  we have to save the autofields here in case they are changed,
	     *  and we have to look up the old value to zero it out.
	     */

            if (!scanner)
               fb_set_autoindex(hp);
            if (scanner != 1)
               fb_macrobegin(rec);
	    }
	 if (FB_ISDELETED(hp)){
            sprintf(msg, SYSMSG[S_EDIT_FMT1],rec,SYSMSG[S_DEL_REC]);
            fb_bell();
            }
         else{
	    if (rec < 0)
	       strcpy(msg, SYSMSG[S_NEW]);
	    else if (hp->ifd > 0 && strlen(hp->irec) > 0)
	       sprintf(msg, SYSMSG[S_EDIT_FMT3], irec[pindx]);
	    else	/* for btree index and no index situations ... */
	       sprintf(msg, SYSMSG[S_EDIT_FMT4], rec);
	    if (pcur->p_maxedit > 0)
               sprintf(msg, SYSMSG[S_EDIT_FMT2], msg, 
	          pcur->p_nedit[0]->n_fp->fld);
	    }
	 fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
	 fb_scrstat2(msg);			/* individual record status */
         fb_display(dflag);
         for(;;) {
	    fb_move(cdb_t_lines, 1), fb_clrtoeol();
            fb_move(ploc_row, ploc_col);
	    ecom[0] = NULL;
            if (FB_ISDELETED(hp) && scanner == 0){
               /* emulate the fb_mustbe() code to get FB_ESIGNAL/FB_FSIGNAL */
               cp = SYSMSG[S_RESTORE];
               icol = strlen(cp) + 1;
               fb_move(cdb_t_lines, 1);
               fb_prints(cp);
               fb_cx_push_env("ynx", CX_KEY_SELECT, NIL);
               st = fb_input(cdb_t_lines, icol, 1, 0, FB_ALPHA,
                  tbuffer, FB_ECHO, -FB_OKEND, FB_CONFIRM);
               fb_cx_pop_env();
               if (st == FB_AOK && (tbuffer[0]==CHAR_y || tbuffer[0]==CHAR_Y)){
                  sprintf(ecom, FB_FDIGITS, hp->nfields + 1);
                  st = FB_DELETED;  /* so as to fall into edit() */
                  }
               else {
                  if (st == FB_ESIGNAL || st == FB_FSIGNAL)
                     mode = st;
                  if (cdb_record_is_locked)
                     fb_unlock(rec, hp);
                  return(REDRAW);
                  }
               }
            else {
	       if (cdb_record_level){
		  fb_printw(cdb_prompt_recordmsg);
		  fb_percentage(pcur->p_num, ptail->p_num);
                  fb_cx_set_viewpage(pcur->p_num);
                  if (scanner == 0)
                     fb_cx_push_env("WXNP", CX_VEDIT_SELECT, NIL); /* reclev */
                  else
                     fb_cx_push_env("E", CX_KEY_SELECT, NIL); 	/* reclev-scan*/
                  if (pcur->p_next != NULL || pcur->p_prev != NULL)
                     fb_cx_add_buttons("GU");
		  if (cdb_create_it)	/* if not create rec, allow fsig/bsig */
		     b_end = FB_OKEND;
		  else{
		     b_end = -FB_OKEND;
                     fb_cx_add_buttons("FB");
                     }
		  if (ploc_col > 0)
		     col = ploc_col + strlen(cdb_prompt_recordmsg);
		  else
		     col = ploc_col;
                  fb_cx_add_buttons("H");
                  st = fb_input(-ploc_row, col + 2, -cdb_dbvedit_rec_pilength, 
                     0, FB_ALPHA, ecom, FB_ECHO, b_end, FB_CONFIRM);
                  fb_cx_pop_env();
		  fb_move(ploc_row, ploc_col);
		  fb_clrtoeol();
		  fb_trim(ecom);
		  if ((st == FB_AOK && equal(ecom, SYSMSG[S_DEL])) ||
		        st == FB_DELSIGNAL){
		     sprintf(ecom, FB_FDIGITS, hp->nfields + 1);
		     st = FB_DELETED;
		     }
		  else if (st == FB_AOK && equal(ecom, SYSMSG[S_PUT]))
		     st = FB_PUT;
		  }
	       else{	/* cdb_record_level == OFF - attempt at modeless */
		  if (scanner == 1)
		     fb_xerror(FB_MESSAGE,"No scan without cdb_record_level",
                        NIL);
		  if (cdb_create_it != 1 || modeless_status != FB_END)
		     if (addfield(modeless_field, top) > 0)
			cdb_write_it = 1;
		  st = modeless_status;
		  if (st == FB_DELSIGNAL){
		     sprintf(ecom, FB_FDIGITS, hp->nfields + 1);
		     st = FB_DELETED;
		     }
	          }
	       if (st == FB_BSIGNAL || st == FB_FSIGNAL || st == FB_SSIGNAL){
	          mode = st;
	          st = FB_END;
		  dot = top;
	          }
               }
            if (st == FB_END || st == FB_WSIGNAL){
               st = FB_END;
               if (scanner)
                  return(REDRAW);
               cdb_end_keystroke = 1;
	       t_st = testexit();
	       if (t_st == FB_END){
                  /*
                   * since askwrite() is now inside of testexit()
                   * at this point, always break.
                   */
                  break;
		  /* st = modeless_status = 0; */
		  /* continue;  */ /* to top of loop for next command */
		  }
               else if (t_st == FB_ERROR)
                  continue;
	       else	/* something needs editing still ... */
	          top = ncur;	/* set top to whatever testexit() did */
	       }
            if (st == FB_ABORT){
               if (scanner == 1)
                  return(REDRAW);
	       if (cdb_write_it == 1 || cdb_create_it == 1){
	          st = fb_mustbe(CHAR_y, FB_ABORT_MSG, cdb_t_lines, 1);
	          if (st == FB_AOK){
                     fb_serror(FB_MESSAGE, SYSMSG[S_RECORD],
                        SYSMSG[S_NO_UPDATE]);
                     if (cdb_record_is_locked)
	                fb_unlock(rec, hp);
                     return(REDRAW);
		     }
		  }
	       else{
                  if (cdb_record_is_locked)
	             fb_unlock(rec, hp);
                  return(REDRAW);
	          }
	       modeless_status = 0;
               }
	    else if (st == FB_PUT){
	       fb_put(cdb_putfile);  /* put index entry and update dict */
	       }
	    else if (st == FB_QSIGNAL){
	       fb_scanfor_query();      /* determine number of matches */
               /* restore the original record */
               if (fb_getrec(rec, cdb_db) == FB_ERROR)
                  fb_xerror(FB_FATAL_GETREC, cdb_db->dbase, (char *) &rec);
	       }
            else if (st == FB_ESIGNAL){
	       if (scanner == 0){
	          if (cdb_record_level){
		     if (addfield(1, top) > 0)
			cdb_write_it = 1;
		     fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
		     }
		  else
		     modeless_field = 1;
		  }
	       }
            else if (st == FB_YSIGNAL){
	       if (scanner == 0){
	          if (cdb_record_level){
		     if (addfield(pcur->p_maxedit, top) > 0)
			cdb_write_it = 1;
		     fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
		     }
		  else
		     modeless_field = pcur->p_maxedit;
	          }
	       }
            else if (st == FB_DEFAULT || st == FB_PAGEDOWN || ecom[0]==CHAR_f){
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
	       modeless_field = 1;
               }
	    else if (st == FB_PSIGNAL){
               cdb_end_keystroke = 0;
	       if ((t_st = testexit()) == FB_END &&
                     (cdb_create_it || cdb_write_it)){
	          /* no errors - create or write and come back */
	          /* if ((!cdb_create_it && !cdb_write_it)) */
		  writerec();
		  }
               if (t_st == FB_ERROR)
                  continue;
	       if (cdb_create_it == 0 && cdb_write_it == 0){
		  custom("print");
		  fb_scrhdr(cdb_db, SYSMSG[S_RECORD_LEVEL]);
		  fb_scrlbl(hp->sdict);
		  fb_scrstat2(msg);		/* individual record status */
		  fb_display(1);
		  fb_checkformula(1);
		  }
	       modeless_status = 0;
	       }
	    else if (testcustom(ecom) == FB_AOK){
               cdb_end_keystroke = 0;
	       if ((t_st = testexit()) == FB_END &&
                     (cdb_create_it || cdb_write_it)){
	          /* no errors - create or write and come back */
	          /* if ((!cdb_create_it && !cdb_write_it)) */
		  writerec();
		  }
               if (t_st == FB_ERROR)
                  continue;
	       if (cdb_create_it == 0 && cdb_write_it == 0){
		  custom(ecom);
		  /* custom commands done in custom() -- now clean up */
		  fb_scrhdr(cdb_db, SYSMSG[S_RECORD_LEVEL]);
		  fb_scrlbl(hp->sdict);
		  fb_scrstat2(msg);		/* individual record status */
		  fb_display(1);
		  fb_checkformula(1);
		  }
	       modeless_status = 0;
	       }
	    else if (st == FB_CSIGNAL){
	       fb_serror(FB_MESSAGE, "Not implemented.", NIL);
	       }
	    else if (st == FB_WSIGNAL || ecom[0] == 'w'){
               if (scanner)
                  return(REDRAW);
               cdb_end_keystroke = 0;
	       if ((t_st = testexit()) == FB_END &&
                     (cdb_create_it || cdb_write_it)){
	          /* no errors - create or write and come back */
	          /* if ((!cdb_create_it && !cdb_write_it)) */
		  writerec();
		  modeless_status = 0;
		  fb_display(1);
		  fb_checkformula(1);
                  n_autos = 0;
		  }
               else if (t_st == FB_ERROR)
                  continue;
	       else		/* something needs editing still ... */
	          top = ncur;	/* set top to whatever testexit() did */
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
	       modeless_field = 1;
	       modeless_status = 0;
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
	       modeless_status = 0;
	       }
            else if  (ecom[0] == FB_HELP){
	       fb_move(3, 1); fb_clrtobot();
	       fb_help(ecom, hp);
               fb_display(1);
               }
	    else if (ecom[0] == FB_DEREF){	/* '@' for auto fb_field */
	       if (scanner == 0 && cdb_record_level){
		  j = atoi(ecom+1);
		  if (addfield(j, top) > 0)
		     cdb_write_it = 1;
		  fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
		  }
	       }
	    else if (ecom[0] == 'p'){		/* 'p' for page N */
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
            else if (cdb_record_level || st == FB_DELETED){
               fld = atoi(ecom);
               i = strlen(fb_trim(ecom)) - 1;
               if (ecom[i] != NULL && ecom[i] != FB_BLANK &&
                     !isdigit(ecom[i]) &&
                     scanner == 0 && fld >= 1 && fld <= pcur->p_maxedit){
		  fb_scrstat(SYSMSG[S_EADD_LEVEL]);
                  st = edit_add(fld);       /* edit_add flag */
		  fb_fmessage(NIL);
                  if (st == FB_AOK){
                     cdb_write_it = 1;
		     fb_checkformula(fld);
		     }
                  else if (st == FB_ABORT)
                     fb_serror(FB_MESSAGE,SYSMSG[S_FIELD],SYSMSG[S_NO_UPDATE]);
		  fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
                  }
               else if (fld <= 0 || (fld > pcur->p_maxedit && st!=FB_DELETED)){
	          if (st != FB_END)
                     fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
		  }
               else if ((fld >= 1 && fld <= pcur->p_maxedit) ||st==FB_DELETED){
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
                        fb_serror(FB_MESSAGE, SYSMSG[S_FIELD],
                           SYSMSG[S_NO_UPDATE]);
                     else
                        st = FB_ABORT;
                     }
		  fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
                  if (st == FB_DELETED || st == FB_UNDELETED)
                     break;
                  }
               else if (st == FB_AOK)
                  fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
               }
            }
         if (cdb_write_it == 1 || cdb_create_it == 1)
	    writerec();
	 if (cdb_record_is_locked)
	    fb_unlock(rec, hp);
         if ((cdb_create_it == 1 || n_autos > 0) && cdb_scr_autoincr){
	    showauto(hp);
	    disp_st = ZAP_REDRAW;
	    }
	 else
	    disp_st = REDRAW;
         if (cdb_did_write_it == 1 || cdb_did_create_it == 1)
            fb_macroend(1);
	 return(disp_st);
      }

/*
 * writerec - actual code to write/ceate the record.
 */

   static writerec()
      {
         int j, save_cdb_lockmessage;

	 if (cdb_create_it == 1)
	    fb_fmessage(SYSMSG[S_CREATING]);
	 else
	    fb_fmessage(SYSMSG[S_WRITING]);
	 fb_lock_head(hp);
         if (cdb_usrlog > 10)
            fb_usrlog_msg("CS-begin");
	 fb_allsync(hp);
	 fb_setdirty(hp, 1);
	 if (cdb_create_it == 1 || st == FB_DELETED || st == FB_UNDELETED){
	    if (fb_gethead(hp) == FB_ERROR){
	       fb_unlock_head(hp);
	       fb_xerror(FB_READ_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
	       }
	    if (cdb_create_it == 1)
	       rec = ++(hp->reccnt);
	    else if (st == FB_DELETED)
	       hp->delcnt++;
	    else				/* st == FB_UNDELETED */
	       hp->delcnt--;
	    if (fb_puthead(hp->fd, hp->reccnt, hp->delcnt) == FB_ERROR)
	       fb_xerror(FB_WRITE_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
	    }
	 /* if a putrec causes fb_xerror, rec 0 is still locked
	  * this will keep others out of the dbase until the
	  * problem can be fixed. this is a feature, not a bug.
	  */
	 hp->rec = rec;
	 if ((n_autos = fb_checkauto(hp)) == FB_ERROR)
	    fb_serror(FB_BAD_INDEX, hp->dbase, INCRMESG);
	 fb_putlog(hp);
	 if (fb_putrec(rec, hp)==FB_ERROR)
	    fb_xerror(FB_WRITE_ERROR, SYSMSG[S_RECORD], hp->dbase);
	 fb_checklog(hp);
         if (cdb_usrlog > 20){
            if (cdb_create_it)
               fb_usrlog_msg("CS-mid-ai - CREATEREC");
            else
               fb_usrlog_msg("CS-mid-ai - WRITEREC");
            }
	 if (fb_put_autoindex(hp) == FB_ERROR)
	    fb_serror(FB_BAD_INDEX, hp->dbase, NIL);
	 if (hp->ihfd > 0)
	    fb_getxhead(hp->ihfd, &(hp->bsmax), &(hp->bsend));
         if (st == FB_DELETED)
            if (fb_delidx(hp, hp->rec) == FB_ERROR)
               fb_serror(FB_BAD_INDEX, hp->dbase, NIL);
	 fb_setdirty(hp, 0);
	 fb_allsync(hp);
         if (cdb_usrlog > 10)
            fb_usrlog_msg("CS-end");
	 fb_unlock_head(hp);
         /*
          * if this is a new record, lock it in case user does not exit now.
          * there are more than a few of these: WSIGNAL, 'w', PSIGNAL, etc
          */
	 if (!cdb_record_is_locked){
            save_cdb_lockmessage = cdb_lockmessage;
            cdb_lockmessage = 0;
	    if (fb_lock(hp->rec, hp, FB_NOWAIT) != FB_AOK)
               fb_serror(FB_MESSAGE,
                  "WARNING: Could not set lock on new record.", NIL);
            else
               cdb_record_is_locked = 1;
            cdb_lockmessage = save_cdb_lockmessage;
            }
         fb_set_autoindex(hp);
         cdb_did_write_it = cdb_write_it;
         cdb_did_create_it = cdb_create_it;
	 cdb_create_it = 0;
	 cdb_write_it = 0;
      }

/*
 * testexit - test whether an exit is ok .. i.e. are all forced
 *	fields in the view area non-empty? if not, simulate the command
 *	to edit JUST that fb_field. nothing else.
 *	return of FB_END means no errors, FB_AOK means simul command is going.
 */

   static testexit()
      {
         int st = FB_END, i, j = 0;
	 fb_page *p;
	 fb_field *f;
	 fb_node *n;

         if (cdb_create_it || cdb_write_it){
            if (cdb_askwrite){
               st = askwrite();
               if (st != FB_END)
                  return(st);
               }
            }
	 for (i = 0, p = phead; p; p = p->p_next){
	    /* j marks the position of the fb_field for the sim. command */
	    for (j = 0, n = p->p_nhead; n; n = n->n_next){
	       f = n->n_fp;
	       if (f != NULL)
		  j++;
	       if (f != NULL && f->type != FB_FORMULA && f->dflink == NULL &&
                     f->type != FB_BINARY &&
		     (f->idefault == NULL || strlen(f->idefault)==0) &&
			f->fld[0] == NULL){
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
		  ncur = pcur->p_nedit[0];
	       else
		  ncur = pcur->p_nhead;
	       fb_display(1);
	       fb_checkformula(1);
	       }
	    if (mode != FB_FADD)
	       mode = NULL;
	    modeless_field = j;
	    modeless_status = 0;
	    }
         else if ((cdb_create_it || cdb_write_it) &&
               fb_checkfields(hp, 1) == FB_ERROR)
            st = FB_ERROR;
         else
            st = fb_macroend(0);
	 return(st);
      }

/*
 * askwrite - encapsulate the askwrite code here
 *    return an END only if askwrite followed by yes.
 */

   static askwrite()

      {
         char *cp, tbuffer[FB_MAXLINE];
         int icol, st;

         cp = WRITEMSG;
         icol = strlen(cp) + 1;
         fb_move(cdb_t_lines, 1);
         fb_prints(cp);
         fb_cx_push_env("ynx", CX_KEY_SELECT, NIL);
         st = fb_input(cdb_t_lines, icol, 1, 0, FB_ALPHA,
            tbuffer, FB_ECHO, FB_OKEND, FB_CONFIRM);
         fb_cx_pop_env();
         if (st == FB_END)
            st = FB_ERROR;
         if (st == FB_AOK){
            if (tbuffer[0] == CHAR_n || tbuffer[0] == CHAR_N){
               cdb_create_it = cdb_write_it = 0;
               st = FB_ERROR;
               }
            else if (tbuffer[0] == CHAR_y || tbuffer[0] == CHAR_Y)
               st = FB_END;
            }
         fb_move(cdb_t_lines, 1); fb_clrtoeol();
         return(st);
      }
