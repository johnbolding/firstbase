/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: edt_fld.c,v 9.2 2001/02/16 19:41:43 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Edit_field_sid[] = "@(#) $Id: edt_fld.c,v 9.2 2001/02/16 19:41:43 john Exp $";
#endif

#include <dbedit.h>

static char *FB_LONGDIGIT = "%ld";
static char *RANGE_MSG = "Entry out of range.";
static char *VIFB_PAUSE_MSG =
   "Visual Edit Field `%s' Now? (y=yes, d=display, <other>=no):";
static char *CHOICEFB_PAUSE_MSG =
	"Edit Choice Field `%s' Now? (y=yes, <other>=no):";

extern char *cdb_vipause_msg;
extern char *cdb_prompt_addmodemsg1;
extern char *cdb_prompt_addmodemsg2;
extern char *cdb_prompt_autofldmsg;
extern char *cdb_prompt_normalfldmsg;
extern short cdb_choicepause;		/* fb_setup() flag */
extern short cdb_choiceaddpause;	/* fb_setup() flag */
extern char *cdb_choicepause_msg;
extern short int cdb_edit_input;
extern char *cdb_e_buf;

extern char *cdb_T_PREV;
extern char *cdb_T_NPREV;
extern char *cdb_T_AUTOINCR;
extern char *cdb_T_DATE;
extern char *cdb_T_TIME;
extern char *cdb_T_MTIME;
extern char *cdb_T_INCR;
extern char *cdb_T_USER;
extern char *cdb_T_AUTOMARK;
extern char *cdb_T_PLUS;
extern char *cdb_T_MINUS;

extern short int cdb_vipause;
extern short int cdb_viaddpause;
extern short int cdb_forceautoincr;
extern short int cdb_skip_null_auto;

extern char *cdb_user;
extern char *cdb_AUTOMARK;

#if !FB_PROTOTYPES
static void db_ghostfield();
#else /* FB_PROTOTYPES */
static void db_ghostfield(fb_field *, int, int, int);
#endif /* FB_PROTOTYPES */

/* 
 *  edit_field - edit actual fb_field - 
 *     fld is pointer into cdb_kp, top is top of screen.
 *     hp contains the record, and autodef signals autodef mode
 */
    edit_field(fld, top)
      int fld, top;

      {
         int row, col, s, forced = 0, pend, new = 0, k = 0, xlong, addf = 0;
	 int isize, prompt = 0, icol;
         char *inp, *line, tbuffer[FB_MAXNAME];
         char autodef_button[5];
	 long urec;
         fb_field *f;
         fb_autoindex *ix;
   
         inp = cdb_afld;
	 line = cdb_bfld;
	 *inp = *line = NULL;
         if (fld < 0){
            fld = -fld;
            new = 1;
            }
	 if (top < 0){
	    top = -top;
	    addf = 1;
	    }
	 k = fld - 1;
         if (fld == hp->nfields + 1){           /* trying to delete/undel */
	    if (scanner == 1)
	       return(FB_ERROR);
            return(fb_delete(cdb_kp[k]));
	    }
         f = cdb_sp[k];
         if (cdb_sp[k]->type == FB_FORMULA || cdb_sp[k]->dflink != NULL ||
               cdb_sp[k]->type == FB_BINARY ){
	    if (scanner == 1)
	       return(FB_ERROR);
	    fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
            return(FB_ERROR);
            }
         else if (cdb_sp[k]->lock == CHAR_y && !new && cdb_sp[k]->fld != NULL){
	    if (scanner == 1)
	       return(FB_ERROR);
	    fb_serror(FB_MESSAGE, SYSMSG[S_FIELD], SYSMSG[S_LOCKED]);
            return(FB_ERROR);
            }
	 if (scanner == 1 && cdb_sp[k]->size <= FB_SCREENFIELD && 
	       cdb_sp[k]->type != FB_CHOICE)
	    return(FB_ERROR);
         row = (fld - top) * 2 + 4;
         col = 17;
         if (cdb_sp[k]->comloc == CHAR_b )
            col = col + 1 + strlen(cdb_sp[k]->comment);
         if (cdb_sp[k]->idefault == NULL || strlen(cdb_sp[k]->idefault) == 0)
            forced = 1;
	 else if (equal(cdb_sp[k]->idefault, cdb_T_PREV) &&
               cdb_sp[k]->prev == NULL)
	    forced = 1;
	 else if (equal(cdb_sp[k]->idefault, cdb_T_NPREV) &&
               cdb_sp[k]->prev == NULL)
	    forced = 1;

	 /* strange use of mode - but FB_FADD is just a bit pattern.
	  * i do not care if it is the same bit pattern accross machines.
	  */
	 
         if ((fld == 1 && mode == FB_FADD && new) ||
               (fld == 2 && mode == FB_FADD && new && 
	       cdb_sp[0]->idefault != NULL &&
               equal(cdb_sp[0]->idefault, cdb_T_AUTOINCR))){
            prompt = 1;
            pend = FB_OKEND;
            }
         else
            pend = FB_NOEND;
	 if (addf && !new)
	    pend = FB_OKEND;
	 if (prompt == 0){
	    if (new && fld == 1)
	       prompt = 1;
	    else if (new)
	       prompt = 2;
	    else if (addf == 1)
	       prompt = 3;
	    else
	       prompt = 4;
	    }
	 for(;;){
	    xlong = 0;
	    s = FB_ERROR;
	    if (autodef == 0 || forced == 1){	/* ie, forced or auto */
	       if (cdb_sp[k]->size > FB_LONGFIELD){
	          if ((cdb_vipause && !new) || (cdb_viaddpause && new)){
                     /* this for loop is for the fb_help file mechanism only */
                     for (;;){
                        if (cdb_vipause_msg == NULL)
                           sprintf(tbuffer, VIFB_PAUSE_MSG, f->id);
                        else
                           sprintf(tbuffer, cdb_vipause_msg);
                        fb_move(cdb_t_lines, 1); fb_clrtoeol();
                        fb_printw(tbuffer);
                        icol = strlen(tbuffer) + 1;
                        db_ghostfield(cdb_sp[k], 1, row, col);
                        fb_cx_push_env("EXNPyn", CX_KEY_SELECT, NIL);
                        if (cdb_sp[k]->help !=NULL && *cdb_sp[k]->help != NULL)
                           fb_cx_add_buttons("H");
                        fb_cx_add_buttons("9");
                        s = fb_input(-cdb_t_lines,-icol,-1,0,FB_ALPHA,tbuffer,
                              FB_ECHO,FB_OKEND,FB_CONFIRM);
                        fb_cx_pop_env();
                        if (s == FB_QHELP){	/* else print fb_help file */
                           fb_fhelp(f->help);
                           db_display(1);
                           continue;
                           }
                        else if (tbuffer[0] == CHAR_d || tbuffer[0] == CHAR_D){
                           fb_d_dfield(f);
                           db_display(1);
                           continue;
                           }
                        else if (s == FB_END && pend != FB_OKEND)
                           s = FB_ESIGNAL;
                        break;
                        }

                     db_ghostfield(cdb_sp[k], 0, row, col);
                     fb_move(cdb_t_lines, 1); fb_clrtoeol();
                     if (s == FB_DEFAULT)
                        s = FB_ESIGNAL;
                     if (s != FB_AOK)
                        return(s);
                     else if (tbuffer[0] != CHAR_y && tbuffer[0] != CHAR_Y)
                        return(FB_ESIGNAL);
		     }
		  sprintf(line, "v%d", k + 1);
                  fb_cx_push_env("T", CX_NO_SELECT, NIL);
                  fb_cx_write(1);
		  s = fb_visual(line, scanner, 1);
                  fb_cx_pop_env();
                  fb_scrhdr(cdb_db, SYSMSG[S_RECORD_LEVEL]);
                  fb_scrlbl(cdb_db->sdict);
                  fb_scrstat2(msg);		/* individual record status */
                  db_display(top);
		  if (scanner == 0)
		     return(s);
		  else
		     return(FB_ABORT);
		  }
	       if (cdb_sp[k]->size > FB_SCREENFIELD){
	          s = fb_longinput(cdb_sp[k], inp, k, new);
                  fb_move(4, 1), fb_clrtobot(); fb_refresh();
		  xlong = 1;
		  if (scanner == 0 && s != FB_ABORT && 
		        s != FB_ESIGNAL && s != FB_YSIGNAL)
		     break;	/* get out, redisplay down lower */
	          db_display(top);
		  if (scanner == 1)
		     s = FB_ABORT;
		  return(s);
		  }
	       else if (cdb_sp[k]->type == FB_CHOICE ||
                     cdb_sp[k]->type == FB_SILENTCHOICE){
	          if (cdb_sp[k]->type == FB_CHOICE &&
                        ((cdb_choicepause && !new) || 
		        (cdb_choiceaddpause && new))){
                     /* this for loop is for the fb_help file mechanism only */
                     for (;;){
                        if (cdb_choicepause_msg == NULL)
                           sprintf(tbuffer, CHOICEFB_PAUSE_MSG, cdb_sp[k]->id);
                        else
                           sprintf(tbuffer, cdb_choicepause_msg);
                        fb_move(cdb_t_lines, 1); fb_clrtoeol();
                        fb_printw(tbuffer);
                        icol = strlen(tbuffer) + 1;
                        db_ghostfield(cdb_sp[k], 1, row, col);
                        fb_cx_push_env("EXNPyn", CX_KEY_SELECT, NIL);
                        if (cdb_sp[k]->help != NULL && *cdb_sp[k]->help !=NULL)
                           fb_cx_add_buttons("H");
                        s = fb_input(-cdb_t_lines, -icol, -1, 0, FB_ALPHA,
                           tbuffer, FB_ECHO, FB_OKEND, FB_CONFIRM);
                        fb_cx_pop_env();
                        if (s == FB_QHELP){	/* else print fb_help file */
                           fb_fhelp(cdb_sp[k]->help);
                           db_display(top);
                           continue;
                           }
                        else if (s == FB_END && pend != FB_OKEND)
                           s = FB_ESIGNAL;
                        break;
                        }
                     db_ghostfield(cdb_sp[k], 0, row, col);
		     fb_move(cdb_t_lines, 1); fb_clrtoeol();
		     if (s == FB_DEFAULT)
			s = FB_ESIGNAL;
		     if (s != FB_AOK)
		        return(s);
		     if (tbuffer[0] != CHAR_y && tbuffer[0] != CHAR_Y)
			return(FB_ERROR);
		     }
	          s = fb_choiceinput(cdb_sp[k], inp, new, row, col,
                     cdb_sp[k]->size,pend);
		  if (s != FB_ERROR && cdb_sp[k]->type != FB_SILENTCHOICE){
		     fb_move(4,1), fb_clrtobot(); fb_refresh();
		     }
		  xlong = 1;
		  if (s == FB_END || s == FB_DSIGNAL || s == FB_QSIGNAL)
		     db_display(top);
                  if (s == FB_ERROR && pend == FB_OKEND)
                     return(FB_END);
		  if (scanner == 0 && s != FB_ABORT && 
		        s != FB_ESIGNAL && s!= FB_YSIGNAL)
		     break;	/* get out, redisplay down lower */
	          db_display(top);
		  if (scanner == 1)
		     s = FB_ABORT;
		  return(s);
		  }
	       else if (scanner == 0)
		  for (;;){
                     if (!autodef)
                        strcpy(autodef_button, "A");
                     else
                        autodef_button[0] = NULL;
		     switch(prompt){
		        /* add mode, fb_field level 1 */
		        case 1: fb_smessage(cdb_prompt_addmodemsg1);
                                sprintf(tbuffer, "E%sNP", autodef_button);
                                break;
		        /* add mode, fb_field level 2 */
		        case 2: fb_smessage(cdb_prompt_addmodemsg2);
                                sprintf(tbuffer, "E%sNP", autodef_button);
                                break;
		        /* auto fb_field level */
		        case 3: fb_smessage(cdb_prompt_autofldmsg);
                                strcpy(tbuffer, "EXNP");
                                break;
		        /* normal fb_field level */
		        case 4: fb_smessage(cdb_prompt_normalfldmsg);
                                strcpy(tbuffer, "EX");
                                break;
			}
		     fb_percentage(fld, cdb_sfields);
		     /* allow HELP, FB_DSIGNAL */
		     isize = -cdb_sp[k]->size;
                     icol = -col;
		     /* if AUTOINCR, force it */
		     if (cdb_forceautoincr && 
			   equal(cdb_sp[k]->idefault, cdb_T_AUTOINCR))
		        s = FB_DEFAULT;
		     else{
                        if (!forced)
                           strcat(tbuffer, "D");
                        if (cdb_sp[k]->help != NULL && *cdb_sp[k]->help !=NULL)
                           strcat(tbuffer, "H");
                        fb_cx_push_env(tbuffer, CX_KEY_SELECT, NIL);/*fld lev*/
                        if (cdb_edit_input)
                           strcpy(cdb_e_buf, cdb_sp[k]->fld);
		        s = fb_input(-row, icol, isize, forced,cdb_sp[k]->type,
                           inp, FB_ECHO, FB_OKEND, FB_CONFIRM);
                        fb_cx_pop_env();
                        
                        /*
                         * this code is intended to make more keystrokes
                         * work in more places, more transparent
                         * - make ^D work like FB_END for non new records
                         * - make FB_END be some FB_QSIGNAL interpreted outside
                         */
                        if (s == FB_END && pend != FB_OKEND)
                           s = FB_QSIGNAL;
                        if (prompt == 4 && (s == FB_YSIGNAL || s ==FB_ESIGNAL))
                           s = FB_END;
  		        if (s == FB_DSIGNAL && !new)
		           s = FB_END;
                        }
		     if (s == FB_AOK && cdb_sp[k]->range != NULL){
		        if (fb_checkrange(cdb_sp[k], inp) == FB_ERROR){
		           fb_serror(FB_MESSAGE, RANGE_MSG, NIL);
			   continue;
			   }
		        }
                     /* test any autoindex for uniqness */
                     ix = cdb_sp[k]->aid;
		     if (s != FB_AOK || ix == NULL || ix->uniq <= 0)
		        break;
                     if (cdb_skip_null_auto && fb_str_is_blanks(inp))
                        break;
		     /* else it needs to be a uniqe entry */
		     
		     /* this next action is mutexed */
		     fb_lock_head(hp);

                     /* b_tree handled if needed */
                     if (ix->ix_tree)			/* btree index */
                        urec = fb_btree_search(inp, ix->ix_idx, ix->ix_seq);
                     else {				/* standard FB index */
		        fb_getxhead(ix->hfd, &ix->ix_bsmax, &ix->ix_bsend);
		        urec = fb_megasearch(ix->afd, inp, 0, 1L,
                           ix->ix_bsend, ix->ix_bsmax,
                           cdb_sp[k]->size + FB_RECORDPTR + 1, 0, hp->irec);
                        }
		     fb_unlock_head(hp);
                     if (urec <= 0)			/* a unique entry */
                        break;
		     fb_serror(FB_MESSAGE, SYSMSG[S_UNIQUE_MSG], NIL);
		     }
	       }
	    else
	       s = FB_DEFAULT;	/* forces auto default */
	    if (s != FB_QHELP && xlong == 0)	/* if not fb_help and normal */
	       break;
	    if (s == FB_QHELP)			/* else print fb_help file */
	       fb_fhelp(cdb_sp[k]->help);
	    db_display(top);
	    if (xlong == 1)			/* if an extralong, quit */
	       break;
	    }
	 if (scanner == 1)
	    return(FB_ERROR);
         if (s == FB_DEFAULT && cdb_sp[k]->idefault != NULL)
	    for (;;){
	       if (equal(cdb_sp[k]->idefault, cdb_T_DATE))
	          strcpy(inp, fb_simpledate(tbuffer, 1));
	       else if (equal(cdb_sp[k]->idefault, cdb_T_TIME))
	          strcpy(inp, fb_simpledate(tbuffer, 0));
	       else if (equal(cdb_sp[k]->idefault, cdb_T_MTIME))
	          strcpy(inp, fb_simpledate(tbuffer, 2));
	       else if (equal(cdb_sp[k]->idefault, cdb_T_INCR)){
	          sprintf(line, FB_FDIGITS, ++(cdb_sp[k]->incr));
                  /* FB_NUMERIC */
		  fb_rjustify(inp, line, cdb_sp[k]->size, cdb_sp[k]->type);
		  }
	       else if (equal(cdb_sp[k]->idefault, cdb_T_PREV)){
	          if (cdb_sp[k]->prev != NULL)
		     strcpy(inp, cdb_sp[k]->prev);
	          }
	       else if (equal(cdb_sp[k]->idefault, cdb_T_NPREV)){
	          if (cdb_sp[k]->prev != NULL){
		     strcpy(line, cdb_sp[k]->prev);
		     sprintf(inp, FB_LONGDIGIT, -atol(line));
		     }
	          }
	       else if (equal(cdb_sp[k]->idefault, cdb_T_USER))
		  strcpy(inp, cdb_user);
	       else if (equal(cdb_sp[k]->idefault, cdb_T_AUTOINCR)){
		  strcpy(inp, cdb_AUTOMARK);
                  break;		/* break out past unique test */
                  }
	       else if (equal(cdb_sp[k]->idefault, cdb_T_PLUS))
                  sprintf(inp, "%d", atoi(cdb_sp[k]->fld) + 1);
	       else if (equal(cdb_sp[k]->idefault, cdb_T_MINUS))
                  sprintf(inp, "%d", atoi(cdb_sp[k]->fld) - 1);
	       else
                  strcpy(inp, cdb_sp[k]->idefault);
		  
	       /* now check for uniqueness if needed */
               ix = cdb_sp[k]->aid; 
	       if (ix == NULL || ix->uniq <= 0)
		  break;
               if (cdb_skip_null_auto && fb_str_is_blanks(inp))
                  break;
	       /* inner for checks uniqueness and forces fb_input if needed */
	       for (; ;){
                  /* this next action is mutexed */
                  fb_lock_head(hp);

                  /* b_tree handled if needed */
                  if (ix->ix_tree)		/* btree index */
                     urec = fb_btree_search(inp, ix->ix_idx, ix->ix_seq);
                  else {			/* standard FB index */
                     fb_getxhead(ix->hfd, &ix->ix_bsmax, &ix->ix_bsend);
                     urec = fb_megasearch(ix->afd, inp, 0, 1L, ix->ix_bsend,
                        ix->ix_bsmax, cdb_sp[k]->size + FB_RECORDPTR + 1, 0,
                        hp->irec);
                     }
                  fb_unlock_head(hp);
                  if (urec <= 0){		/* a unique entry */
		     s = FB_AOK;		/* set for double break */
		     break;			/* a unique entry */
		     }
                  fb_serror(FB_MESSAGE, SYSMSG[S_UNIQUE_MSG], NIL);

		  /* DO NOT allow HELP, FB_DSIGNAL, and now force the entry */
		  s = fb_input(row, col, cdb_sp[k]->size, 0, cdb_sp[k]->type, 
		       inp, FB_ECHO, pend, FB_CONFIRM);
		  if (s != FB_AOK)
		     break;			/* let outer for do work */
		  }

	       if (s != FB_DEFAULT)
	          break;
	       }				/* end of default for() */
         if (s == FB_AOK || s == FB_DEFAULT){
	    if (FB_OFNUMERIC(cdb_sp[k]->type))
	       fb_rmlead(inp);
	    fb_trim(inp);
            fb_store(cdb_sp[k], inp, hp);
	    if (equal(cdb_sp[k]->idefault, cdb_T_INCR))
	       cdb_sp[k]->incr = atoi(inp);
	    else if (equal(cdb_sp[k]->idefault, cdb_T_PREV) ||
	             equal(cdb_sp[k]->idefault, cdb_T_NPREV)){
	       if (cdb_sp[k]->prev != NULL)
	          fb_free(cdb_sp[k]->prev);
	       strcpy(line, inp);
	       fb_trim(line);
	       cdb_sp[k]->prev = (char *) fb_malloc(strlen(line) + 1);
	       strcpy(cdb_sp[k]->prev, line);
	       }
            if (cdb_sp[k]->type !=FB_DATE || s==FB_DEFAULT){
	       if (inp[0] != NULL){
	          fb_formfield(line, inp, cdb_sp[k]->type, cdb_sp[k]->size);
		  strcpy(inp, line);
		  }
	       if (xlong == 0){
	          if (cdb_sp[k]->type != FB_DOLLARS)
		     fb_trim(inp);
                  fb_move(row, col); fb_stand(inp);
		  }
	       else{			/* redraw the whole screen */
	          db_display(top);
                  }
               }
            }
         return(s);
      }

/*
 * ghostfield - place a ghost image of a fb_field in place. dbvedit/dbvemit
 */

   static void db_ghostfield(f, revflag, row, col)
      fb_field *f;
      int revflag, row, col;
      
      {
         char temp[81], temp1[81], *line, *p;
         int size;

         line = cdb_bfld;
	 if (f->fld[0] != NULL){
	    strcpy(line, f->fld);
	    strncpy(temp1, line, FB_SCREENFIELD);
            if ((p = strchr(temp1, '\n')) != 0)
               *p = NULL;
	    }
	 else
	    temp1[0] = NULL;
         if (f->size < FB_SCREENFIELD)
            size = f->size;
         else
            size = FB_SCREENFIELD;
	 fb_pad(temp, temp1, size);
         fb_move(row, col);
	 if (revflag)
	    fb_reverse(temp);
	 else
	    fb_printw(temp);
      }
