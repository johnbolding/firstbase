/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: edit.c,v 9.1 2001/02/16 19:41:43 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Edit_sid[] = "@(#) $Id: edit.c,v 9.1 2001/02/16 19:41:43 john Exp $";
#endif

#include <dbedit.h>

static char *MSG = "Field must be Non-Empty:";
static char *INCRMESG = "AutoIncr Error";
static char *FB_ABORT_MSG = "Really Abort Record? (y=yes, <other>=no):";
static char *WRITEMSG = "Write Record Now? (y=yes, n=no, <other>=cancel):";
extern char *cdb_HLP_EDFLD;

static char ecom[FB_MAXINPUT];
extern char *cdb_pgm;
extern short int cdb_secure;
extern short int cdb_askwrite;
extern char *cdb_putfile;
extern short int cdb_scr_autoincr;

/* 
 *  edit - edit a single record. could be that the lock will fail.
 *	and no editing will occur. top refers to top of screen. 
 *	the global 'rec' has the physical record number in it.
 *	if rec is -1, then it is an add rec, with actual rec calced later.
 */

   void edit(top)
      int top;
   
      {
         int write_it, create_it, ptop, fld, i, j, n_autos = 0;
         int b_end;
   
	 autodef = 0;				/* paranoia */
         if (scanner == 0 && rec != -1L)	/* ie, not in ADD mode */
	    if (fb_lock(rec, hp, FB_NOWAIT) != FB_AOK)/* dont wait if locked */
	       return;
         create_it = 0;
         write_it = 0;
         if (rec == -1L){			/* an ADD record */
            if (scanner == 1)
               return;
            if ((st = newrec()) == FB_ABORT){
	       fb_fmessage(NIL);
               fb_serror(FB_MESSAGE, SYSMSG[S_RECORD], SYSMSG[S_NO_CREATE]);
               return;
               }
            else if (st == FB_END){
               mode = NULL;
	       oldrec = 0L;			/* position pointer at top */
               return;
               }
	    /* fb_getlink(hp); */
            create_it = 1;
	    autodef = 0;			/* in case it was turned on */
            }
         else if (fb_getrec(rec, hp) == FB_ERROR)
            fb_xerror(FB_FATAL_GETREC, cdb_pgm, (char *) &rec);
         else if (cdb_secure &&
               fb_record_permission(hp, (scanner ? READ: WRITE)) == FB_ERROR){
            fb_serror(FB_MESSAGE, "Record Permission Denied", NIL);
            return;
            }
	 else {					/* successful getrec */

	    /*  we have to save the autofields here in case they are changed,
	     *  and we have to look up the old value to zero it out.
	     */

            if (!scanner)
               fb_set_autoindex(hp);
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
            sprintf(msg, SYSMSG[S_EDIT_FMT2], msg, cdb_sp[0]->fld);
	    }
	 fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
	 fb_scrstat2(msg);			/* individual record status */
         top = db_display(top);
         for(;;) {
            fb_move(cdb_t_lines,1); fb_clrtoeol();
	    ecom[0] = NULL;
            if (FB_ISDELETED(hp) && scanner == 0){
	       if (fb_mustbe(CHAR_y, SYSMSG[S_RESTORE], cdb_t_lines, 1) ==
                     FB_AOK){
                  sprintf(ecom, FB_FDIGITS, hp->nfields + 1);
                  st = FB_DELETED;  /* so as to fall into edit() */
                  }
               else{
                  fb_serror(FB_MESSAGE, SYSMSG[S_RECORD],SYSMSG[S_NO_RESTORE]);
		  if (scanner == 0)
		     fb_unlock(rec, hp);
                  return;
                  }
               }
            else {
               fb_printw(SYSMSG[S_REC_MSG]);
               fb_percentage(top, cdb_sfields);
               fb_cx_set_viewpage(top);
               if (scanner == 0)
                  fb_cx_push_env("WXNP", CX_EDIT_SELECT, NIL);	/* reclev */
               else
                  fb_cx_push_env("E", CX_KEY_SELECT, NIL);     /* reclev-scan*/
               if (cdb_sfields > 10)
                  fb_cx_add_buttons("GU");
               if (create_it){	/* if not create rec, allow fsig/bsig */
                  b_end = FB_OKEND;
                  }
               else{
                  b_end = -FB_OKEND;
                  fb_cx_add_buttons("FB");
                  }
               fb_cx_add_buttons("H");
               st = fb_input(-cdb_t_lines, -32, -4, 0, FB_ALPHA, ecom,
                  FB_ECHO, b_end, FB_CONFIRM);
               fb_cx_pop_env();
	       fb_move(cdb_t_lines,1), fb_clrtoeol();
	       fb_trim(ecom);
               if (st == FB_AOK && equal(ecom, SYSMSG[S_DEL])){
                  sprintf(ecom, FB_FDIGITS, hp->nfields + 1);
                  st = FB_DELETED;
                  }
               else if (st == FB_AOK && equal(ecom, SYSMSG[S_PUT]))
	          st = FB_PUT;
	       if (st == FB_BSIGNAL || st == FB_FSIGNAL || st == FB_SSIGNAL){
	          mode = st;
	          st = FB_END;
		  dot = top;
	          }
               }
            if (st == FB_END || st == FB_WSIGNAL || ecom[0] == 'w'){
               st = FB_END;
               for (i = j = 0; j < cdb_sfields; j++)   /* j is 0 based */
	          if ((cdb_sp[j]->idefault==NULL ||
                        strlen(cdb_sp[j]->idefault) == 0) &&
		        cdb_sp[j]->type != FB_BINARY &&
		        cdb_sp[j]->fld == NIL){
		     fb_serror(FB_MESSAGE, MSG, cdb_sp[j]->id);
		     i++;
		     }
	       if (!i){
                  if ((create_it || write_it) &&
                        fb_checkfields(hp, 1) == FB_ERROR)
                     continue;  /* to top of loop for next command */
	          if ((!create_it && !write_it) || !cdb_askwrite)
		     break;
	          st = fb_mustbe(CHAR_y, WRITEMSG, cdb_t_lines, 1);
                  if (st == FB_AOK)
		     break;
		  }
               continue;  /* to top of loop for next command */
	       }
            else if (st == FB_ABORT){
               if (scanner == 1)
                  return;
	       if (write_it == 1 || create_it == 1){
	          st = fb_mustbe(CHAR_y, FB_ABORT_MSG, cdb_t_lines, 1);
	          if (st == FB_AOK){
                     fb_serror(FB_MESSAGE, SYSMSG[S_RECORD],
                        SYSMSG[S_NO_UPDATE]);
	             fb_unlock(rec, hp);
                     return;
		     }
		  }
	       else{
	          fb_unlock(rec, hp);
                  return;
		  }
               }
	    else if (st == FB_DSIGNAL)
	       fb_infotoggle();
	    else if (st == FB_PUT){
	       fb_put(cdb_putfile);  /* put index entry & update dictionary */
	       }
	    else if (st == FB_QSIGNAL){
	       fb_scanfor_query();      /* determine number of matches */
               /* restore the original record */
               if (fb_getrec(rec, hp) == FB_ERROR)
                  fb_xerror(FB_FATAL_GETREC, hp->dbase, (char *) &rec);
	       }
            else if (st == FB_ESIGNAL){
	       if (scanner == 0){
		  if (addfield(top, &top) > 0)
		     write_it = 1;
		  fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
                  }
               }
            else if (st == FB_YSIGNAL){
	       if (scanner == 0){
                  ptop = top + 9;
                  if (ptop > cdb_sfields)
                     ptop = cdb_sfields;
		  if (addfield(ptop, &top) > 0)
		     write_it = 1;
		  fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
                  }
/*
*               ptop = top;
*               top -= 1;
*               if (top <= 0){
*                  if (ptop != 1)
*                     top = 1;
*                  else if ((top = cdb_sfields - 9) < 1)
*                     top = 1;
*                  }
*               if (ptop != top){
*                  top = db_display(top);
*		  db_checkformula(top, top);
*		  }
*/
               }
            else if (st == FB_DEFAULT || st == FB_PAGEDOWN || ecom[0]==CHAR_f){
               ptop = top;
               top += 10;
               if (top > cdb_sfields)
                  top = 1;
               if (ptop != top){
                  top = db_display(top);
		  db_checkformula(top, top);
		  }
               }
            else if (ecom[0] == CHAR_b || st == FB_PAGEUP){
               ptop = top;
               top -= 10;
               if (top <= 0){
                  if (ptop != 1)
                     top = 1;
                  else if ((top = cdb_sfields - 9) < 1)
                     top = 1;
                  }
               if (ptop != top){
                  top = db_display(top);
		  db_checkformula(top, top);
		  }
               }
	    else if (ecom[0] == CHAR_v || ecom[0] == CHAR_x){
	       st = fb_visual(ecom, scanner, ((ecom[0] == CHAR_v) ? 1 : 0));
	       fb_scrhdr(hp, SYSMSG[S_RECORD_LEVEL]);
	       fb_scrlbl(hp->sdict);
	       fb_scrstat2(msg);		/* individual record status */
	       db_display(top);
	       db_checkformula(top, top);
	       if (scanner == 0 && st == FB_AOK)
	          write_it = 1;
	       }
	    else if (ecom[0] == CHAR_d){	/* display fb_field */
	       dfield(ecom);
	       db_display(top);
	       }
	    else if (ecom[0] == CHAR_BANG){	/* cshell */
	       fb_cshell(ecom);
	       fb_serror(FB_MESSAGE, NIL, NIL);
	       fb_scrhdr(hp, SYSMSG[S_RECORD_LEVEL]);
	       fb_scrlbl(hp->sdict);
	       fb_scrstat2(msg);		/* individual record status */
	       db_display(top);
	       db_checkformula(top, top);
	       }
	    else if (st == FB_QHELP){
	       fb_fhelp(cdb_HLP_EDFLD);
               db_display(top);
	       }              
            else if  (ecom[0] == FB_HELP){
	       fb_help(ecom, hp);
               db_display(top);               
               }
	    else if (ecom[0] == FB_DEREF){	/* '@' for auto field */
	       if (scanner == 0){
		  j = atoi(ecom+1);
		  if (addfield(j, &top) > 0)
		     write_it = 1;
		  fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
		  }
	       }
            else {
               fld = atoi(ecom);
               i = strlen(fb_trim(ecom)) - 1;
               if (ecom[i] != NULL && ecom[i] != FB_BLANK &&
                     !isdigit(ecom[i]) &&
                     scanner == 0 && fld >= top && fld <= top + 9){
		  fb_scrstat(SYSMSG[S_EADD_LEVEL]);
                  st = edit_add(fld, top);       /* edit_add flag */
		  fb_fmessage(NIL);
                  if (st == FB_AOK){
                     write_it = 1;
		     db_checkformula(fld, top);
		     }
                  else if (st == FB_ABORT)
                     fb_serror(FB_MESSAGE, SYSMSG[S_FIELD],
                        SYSMSG[S_NO_UPDATE]);
		  fb_scrstat(SYSMSG[S_RECORD_LEVEL]);
                  }
               else if (fld <= 0 || (fld > cdb_sfields && st != FB_DELETED) || 
                     (fld == hp->nfields + 1 && st != FB_DELETED))
                  fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
               else if (((fld >= top && fld <= top + 9) || st == FB_DELETED)){
	          fb_scrstat(SYSMSG[S_FIELD_LEVEL]);
                  while ((st = edit_field(fld, top)) == FB_DSIGNAL)
	             fb_infotoggle();
		  fb_fmessage(NIL);
                  if (st == FB_AOK || st == FB_DEFAULT || st == FB_UNDELETED ||
                        st == FB_DELETED){
                     write_it = 1;
		     if (st == FB_AOK || st == FB_DEFAULT)
		        db_checkformula(fld, top);
		     }
                  else if ((st == FB_END || st == FB_ESIGNAL ||
                        st ==FB_YSIGNAL || st == FB_ABORT ||
                        st == FB_DSIGNAL || st == FB_QSIGNAL) &&
                        scanner == 0){
		     fb_fetch(cdb_sp[fld-1], cdb_afld, hp);
                     db_putfield(fld, cdb_sp[fld-1], cdb_afld,
		        ((fld - top) * 2 + 4) / 2);	/* to use putfield */
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
               else if (fld > 0 && fld <= cdb_sfields){
                  top = db_display(fld);
		  db_checkformula(top, top);
		  }
               else
                  fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
               }
            } 
         if (write_it == 1 || create_it == 1){
	    if (create_it == 1)
	       fb_fmessage(SYSMSG[S_CREATING]);
	    else
	       fb_fmessage(SYSMSG[S_WRITING]);
	    fb_lock_head(hp);
	    fb_setdirty(hp, 1);
	    fb_allsync(hp);
	    if (create_it == 1 || st == FB_DELETED || st == FB_UNDELETED){
	       if (fb_gethead(hp) == FB_ERROR){
	          fb_unlock_head(hp);
		  fb_xerror(FB_READ_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
		  }
	       if (create_it == 1)
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
	    if (fb_put_autoindex(hp) == FB_ERROR)
	       fb_serror(FB_BAD_INDEX, hp->dbase, NIL);
            if (st == FB_DELETED)
               if (fb_delidx(hp, hp->rec) == FB_ERROR)
	          fb_serror(FB_BAD_INDEX, hp->dbase, NIL);
	    fb_setdirty(hp, 0);
	    fb_allsync(hp);
	    fb_unlock_head(hp);
            fb_set_autoindex(hp);
            }
	 if (scanner == 0 && create_it != 1){
	    fb_unlock(rec, hp);
            }
         if ((create_it == 1 || n_autos > 0) && cdb_scr_autoincr){
	    showauto(hp);
            }
      }
