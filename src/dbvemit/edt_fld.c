/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: edt_fld.c,v 9.0 2001/01/09 02:56:01 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Edit_field_sid[] = "@(#) $Id: edt_fld.c,v 9.0 2001/01/09 02:56:01 john Exp $";
#endif

#include <dbve_ext.h>

extern char *choice_attribute;		/* defined in choice.c */
extern char *cdb_prompt_addmodemsg1;
extern char *cdb_prompt_addmodemsg2;
extern char *cdb_prompt_autofldmsg;
extern char *cdb_prompt_normalfldmsg;
extern char *cdb_choicepause_msg;
extern char *cdb_vipause_msg;
extern short cdb_choicepause;		/* fb_setup() flag */
extern short cdb_choiceaddpause;	/* fb_setup() flag */
extern short int cdb_edit_input;
extern short int cdb_vipause;
extern short int cdb_viaddpause;
extern char *cdb_e_buf;
extern char *cdb_T_PREV;
extern char *cdb_T_NPREV;
extern char *cdb_T_DATE;
extern char *cdb_T_TIME;
extern char *cdb_T_MTIME;
extern char *cdb_T_INCR;
extern char *cdb_T_USER;
extern char *cdb_T_AUTOINCR;
extern char *cdb_AUTOMARK;
extern char *cdb_T_PLUS;
extern char *cdb_T_MINUS;
extern char *cdb_user;

static char *FB_LONGDIGIT = "%ld";
static char *RANGE_MSG = "Entry out of range.";
static char *VIPAUSE_MSG =
   "Visual Edit Field `%s' Now? (y=yes, d=display, <other>=no):";
static char *CHOICEPAUSE_MSG =
	"Edit Choice Field `%s' Now? (y=yes, <other>=no):";

static replace();

/* 
 *  edit_field - edit actual fb_field - 
 *     fld is pointer into cdb_kp, top is top of screen. although ** unused **.
 *     hp contains the record, and autodef signals autodef mode
 */
    edit_field(fld, top, addf)
      int fld, addf;
      fb_node *top;

      {
         int row, col, s, forced = 0, pend, new = 0, k = 0, xlong;
	 int isize, prompt = 0, icol;
         char *inp, *line, tbuffer[FB_MAXNAME], *fb_simpledate(), *fb_rmlead(), *p;
         char autodef_button[5];
	 long atol(), fb_search(), fb_megasearch();
	 fb_field *f;
	 fb_node *n;
   
         inp = cdb_afld;
	 line = cdb_bfld;
	 *inp = *line = NULL;
         if (fld < 0){
            fld = -fld;
            new = 1;
            }
         if (addf == -1){           /* trying to fb_delete/undel */
	    if (scanner == 1)
	       return(FB_ERROR);
            return(fb_delete(hp->kp[hp->nfields]));
	    }
	 n = pcur->p_nedit[fld - 1];
	 f = n->n_fp;
	 k = fld - 1;
         if (f->type == FB_FORMULA || f->dflink != NULL ||
	       n->n_readonly){
	    if (scanner == 1)
	       return(FB_ERROR);
	    fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
            return(FB_ERROR);
            }
         else if (f->lock == CHAR_y && !new && f->fld[0] != NULL){
	    if (scanner == 1)
	       return(FB_ERROR);
	    /* fb_serror(FB_MESSAGE, SYSMSG[S_FIELD], SYSMSG[S_LOCKED]); */
            return(FB_ERROR);
            }
	 if (scanner == 1 && f->size <= FB_SCREENFIELD && 
	       f->type != FB_CHOICE)
	    return(FB_ERROR);
         row = n->n_row;
         col = n->n_col;
         if (f->idefault == NULL || strlen(f->idefault) == 0)
            forced = 1;
	 else if (equal(f->idefault, cdb_T_PREV) && f->prev == NULL)
	    forced = 1;
	 else if (equal(f->idefault, cdb_T_NPREV) && f->prev == NULL)
	    forced = 1;

	 /* strange use of mode - but FB_FADD is just a bit pattern.
	  * i do not care if it is the same bit pattern accross machines.
	  */
	 
         if ((fld == 1 && new && pcur == phead) || 
	       (addf && !new))
            pend = FB_OKEND;
         else
            pend = FB_NOEND;
	 if (new && fld == 1 && pcur == phead)
	    prompt = 1;
	 else if (new)
	    prompt = 2;
	 else if (addf == 1)
	    prompt = 3;
	 else
	    prompt = 4;
	 for(;;){
	    xlong = 0;
	    s = FB_ERROR;
	    if (autodef == 0 || forced == 1){	/* ie, forced or auto */
	       if (f->size > FB_LONGFIELD && n->n_sub2 < 0 &&
	             f->type != FB_CHOICE  && f->type != FB_SILENTCHOICE &&
		     f->type != FB_EXCHOICE){
	          if ((cdb_vipause && !new) || (cdb_viaddpause && new)){
                     /* this for loop is for the fb_help file mechanism only */
                     for (;;){
                        if (cdb_vipause_msg == NULL)
                           sprintf(tbuffer, VIPAUSE_MSG, f->id);
                        else
                           sprintf(tbuffer, cdb_vipause_msg);
		        fb_move(cdb_t_lines, 1); fb_clrtoeol();
                        fb_printw(tbuffer);
                        icol = strlen(tbuffer) + 1;
                        fb_ghostfield(n, 1);
                        fb_cx_push_env("EXNPyn", CX_KEY_SELECT, NIL);
                        if (f->help != NULL && *f->help != NULL)
                           fb_cx_add_buttons("H");
                        fb_cx_add_buttons("9");
		        s = fb_input(-cdb_t_lines,-icol,-1,0,FB_ALPHA,tbuffer,
		              FB_ECHO,FB_OKEND,FB_CONFIRM);
                        fb_cx_pop_env();
                        if (s == FB_QHELP){	/* else print fb_help file */
                           fb_fhelp(f->help);
                           fb_display(1);
                           continue;
                           }
                        else if (s == FB_AOK &&
                              tbuffer[0] == CHAR_d || tbuffer[0] == CHAR_D){
                           fb_d_dfield(f);
                           fb_display(1);
                           continue;
                           }
                        /*
                         * this code is intended to make more keystrokes
                         * work in more places, more transparent
                         * - make ^D work like FB_END for non new records
                         * - make FB_END be some FB_QSIGNAL interpreted outside
                         */
                        if (s == FB_END && pend != FB_OKEND)
                           s = FB_QSIGNAL;
                        if (prompt == 4 && (s == FB_YSIGNAL || s == FB_ESIGNAL))
                           s = FB_END;
  		        if (s == FB_DSIGNAL && !new)
		           s = FB_END;
                        break;
                        }
		     fb_ghostfield(n, 0);
		     fb_move(cdb_t_lines, 1); fb_clrtoeol();
		     if (s == FB_DEFAULT)
			s = FB_ESIGNAL;
		     if (s != FB_AOK)
		        return(s);
		     if (tbuffer[0] != CHAR_y && tbuffer[0] != CHAR_Y)
			return(FB_ERROR);
		     }
		  sprintf(line, "v%d", k + 1);
                  fb_cx_push_env("T", CX_NO_SELECT, NIL);
                  fb_cx_write(1);
		  s = fb_visual(line, scanner, 1);
                  fb_cx_pop_env();
                  fb_scrhdr(cdb_db, SYSMSG[S_RECORD_LEVEL]);
                  fb_scrlbl(cdb_db->sdict);
                  fb_scrstat2(msg);	/* individual record status */
                  fb_display(1);
		  if (scanner == 0)
		     return(s);
		  else
		     return(FB_ABORT);
		  }
	       if (f->size > 80 && n->n_sub2 < 0 &&
                     f->type != FB_CHOICE && f->type != FB_SILENTCHOICE &&
		     f->type != FB_EXCHOICE){
	          s = fb_longinput(f, inp, k, new);
                  fb_move(3, 1), fb_clrtobot(); fb_refresh();
		  if (s == FB_END || s == FB_DSIGNAL)
		     fb_display(1);
		  xlong = 1;
		  if (scanner == 0 && s != FB_ABORT && 
		        s != FB_ESIGNAL && s != FB_YSIGNAL)
		     break;	/* get out, redisplay down lower */
	          fb_display(1);
		  if (scanner == 1)
		     s = FB_ABORT;
		  return(s);
		  }
	       else if (f->type == FB_CHOICE || f->type == FB_SILENTCHOICE){
	          if (f->type == FB_CHOICE && ((cdb_choicepause && !new) || 
		         (cdb_choiceaddpause && new))){
                     /* this for loop is for the fb_help file mechanism only */
                     for (;;){
                        if (cdb_choicepause_msg == NULL)
                           sprintf(tbuffer, CHOICEPAUSE_MSG, f->id);
                        else
                           sprintf(tbuffer, cdb_choicepause_msg);
                        fb_move(cdb_t_lines, 1); fb_clrtoeol();
                        fb_printw(tbuffer);
                        icol = strlen(tbuffer) + 1;
                        fb_ghostfield(n, 1);
                        fb_cx_push_env("EXNPyn", CX_KEY_SELECT, NIL);
                        if (f->help != NULL && *f->help != NULL)
                           fb_cx_add_buttons("H");
                        s = fb_input(-cdb_t_lines, -icol, -1, 0, FB_ALPHA,
                              tbuffer, FB_ECHO, FB_OKEND, FB_CONFIRM);
                        fb_cx_pop_env();
                        if (s == FB_QHELP){	/* else print fb_help file */
                           fb_fhelp(f->help);
                           fb_display(1);
                           continue;
                           }
                        /*
                         * this code is intended to make more keystrokes
                         * work in more places, more transparent
                         * - make ^D work like FB_END for non new records
                         * - make FB_END be some FB_QSIGNAL interpreted outside
                         */
                        if (s == FB_END && pend != FB_OKEND)
                           s = FB_QSIGNAL;
                        if (prompt == 4 && (s == FB_YSIGNAL || s == FB_ESIGNAL))
                           s = FB_END;
  		        if (s == FB_DSIGNAL && !new)
		           s = FB_END;
                        break;
                        }
                     fb_ghostfield(n, 0);
                     fb_move(cdb_t_lines, 1); fb_clrtoeol();
		     if (s == FB_DEFAULT)
			s = FB_ESIGNAL;
		     if (s != FB_AOK)
		        return(s);
		     if (tbuffer[0] != CHAR_y && tbuffer[0] != CHAR_Y)
			return(FB_ERROR);
                     }
	          s = fb_choiceinput(f, inp, new, row, col, n->n_len, pend);
		  if (s != FB_ERROR && f->type != FB_SILENTCHOICE){
		     fb_move(3,1), fb_clrtobot(); fb_refresh();
		     }
		  xlong = 1;
		  if (s == FB_END || s == FB_DSIGNAL)
		     fb_display(1);
                  if (s == FB_ERROR && pend == FB_OKEND)
                     return(FB_END);
		  if (scanner == 0 && s != FB_ABORT && 
		        s != FB_ESIGNAL && s!= FB_YSIGNAL)
		     break;	/* get out, redisplay down lower */
	          fb_display(1);
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
                     fb_percentage(pcur->p_num, ptail->p_num);
		     isize = n->n_len;
  		     icol = -col;    /* enable FB_DSIGNAL for autodefault */
  		     /* allow ^N^P always */
  		     isize = -isize;
                     if (!forced)
                        strcat(tbuffer, "D");
                     if (f->help != NULL && *f->help != NULL)
                        strcat(tbuffer, "H");
                     fb_cx_push_env(tbuffer, CX_KEY_SELECT, NIL);/* fld lev */
                     if (cdb_edit_input && f->f_macro == NULL){
                        strcpy(cdb_e_buf, f->fld);
                        if (n->n_sub2 > 0){
                           if (strlen(cdb_e_buf) >= n->n_sub1){
                              strcpy(line, cdb_e_buf);
                              p = line + (n->n_sub1 - 1);
                              line[n->n_sub2] = NULL;
                              strcpy(cdb_e_buf, p);
                              }
                           else
                              cdb_e_buf[0] = NULL;
                           }
                        }
                     if (f->f_macro != NULL)
                        s = fb_macroinput(f, top, addf, row, col, n->n_len, fld);
                     else
                        s = fb_input(-row, icol, isize, forced, f->type, 
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
                     if (prompt == 4 && (s == FB_YSIGNAL || s == FB_ESIGNAL))
                        s = FB_END;
                     if (s == FB_DSIGNAL && !new)
                        s = FB_END;

                     /* short circuit out of here if a macro was done */
                     if (f->f_macro != NULL)
                        return(s);

		     if (s == FB_AOK && f->range != NULL){
		        if (fb_checkrange(f, inp) == FB_ERROR){
		           fb_serror(FB_MESSAGE, RANGE_MSG, NIL);
			   continue;
			   }
		        }
		     if (s != FB_AOK)
		        break;
		     /* else it must be a uniqe entry */
		     
		     /* this next action is mutexed */
/*
*		     lock(0L, FB_WAIT);
*		     fb_getxhead(f->aid->hfd, &bsmax, &bsend);
*		     unlock(0L);
*		     
*		     if (fb_megasearch(f->aid->afd, inp, 0, 1L, bsend,bsmax,
*		           f->size + FB_RECORDPTR + 1, 0, hp->irec) <= 0L)
*		        break;
*		     else
*		        fb_serror(FB_MESSAGE, SYSMSG[S_UNIQUE_MSG], NIL);
*
*                    For dbvemit there is no such thing as unique.
*/
                     break;
		     }
	       }
	    else
	       s = FB_DEFAULT;	/* forces auto default (autodef) */
	    if (s != FB_QHELP && xlong == 0)	/* if not fb_help and normal */
	       break;
	    if (s == FB_QHELP)			/* else print fb_help file */
	       fb_fhelp(f->help);
	    fb_display(1);
	    if (xlong == 1)			/* if an extralong, quit */
	       break;
	    }
	 if (scanner == 1)
	    return(FB_ERROR);
         if (s == FB_DEFAULT && f->idefault != NULL)
	    for (;;){
	       if (equal(f->idefault, cdb_T_DATE))
	          strcpy(inp, fb_simpledate(tbuffer, 1));
	       else if (equal(f->idefault, cdb_T_TIME))
	          strcpy(inp, fb_simpledate(tbuffer, 0));
	       else if (equal(f->idefault, cdb_T_MTIME))
	          strcpy(inp, fb_simpledate(tbuffer, 2));
	       else if (equal(f->idefault, cdb_T_INCR)){
	          sprintf(line, FB_FDIGITS, ++(f->incr));
		  fb_rjustify(inp, line, f->size, f->type); /* FB_NUMERIC */
		  }
	       else if (equal(f->idefault, cdb_T_PREV)){
	          if (f->prev != NULL)
		     strcpy(inp, f->prev);
	          }
	       else if (equal(f->idefault, cdb_T_NPREV)){
	          if (f->prev != NULL){
		     strcpy(line, f->prev);
		     sprintf(inp, FB_LONGDIGIT, -atol(line));
		     }
	          }
	       else if (equal(f->idefault, cdb_T_USER))
		  strcpy(inp, cdb_user);
	       else if (equal(f->idefault, cdb_T_AUTOINCR))
		  strcpy(inp, cdb_AUTOMARK);
	       else if (equal(f->idefault, cdb_T_PLUS))
                  sprintf(inp, "%d", atoi(f->fld) + 1);
	       else if (equal(f->idefault, cdb_T_MINUS))
                  sprintf(inp, "%d", atoi(f->fld) - 1);
	       else
                  strcpy(inp, f->idefault);

	       break;
	       }				/* end of default for() */
         if (s == FB_AOK || s == FB_DEFAULT){
	    if (FB_OFNUMERIC(f->type))
	       fb_rmlead(inp);
	    if (n->n_sub2 > 0){
	       fb_fetch(f, line, hp);
	       replace(line, inp, n->n_sub1, n->n_sub2);
	       strcpy(inp, line);
	       }
	    fb_trim(inp);
            fb_store(f, inp, hp);
	    if (equal(f->idefault, cdb_T_INCR))
	       f->incr = atoi(inp);
	    else if (equal(f->idefault, cdb_T_PREV) ||
	             equal(f->idefault, cdb_T_NPREV)){
	       if (f->prev != NULL)
	          fb_free(f->prev);
	       strcpy(line, inp);
	       fb_trim(line);
	       f->prev = (char *) fb_malloc(strlen(line) + 1);
	       strcpy(f->prev, line);
	       }
            if (f->type !=FB_DATE || s==FB_DEFAULT){
	       if (inp[0] != NULL){
	          fb_formfield(line, inp, f->type, f->size);
		  strcpy(inp, line);
		  }
	       if (xlong == 0){
	          if (f->type != FB_DOLLARS)
		     fb_trim(inp);
		  if (n->n_sub2 > 0){
		     /* take subset of inp,assign to line,move back to inp */
		     p = inp + (n->n_sub1 - 1);
		     inp[n->n_sub2] = NULL;
		     fb_trim(p);
		     }
		  else{
		     if (n->n_type == T_SUBELEMENT_R ||
		           n->n_type == T_SUBELEMENT_N){
			if (f->type == FB_CHOICE){
			   fb_subline(line, inp, n->n_sub1, choice_attribute[0]);
			   strcpy(inp, line);
			   }
			else{
			   fb_subline(line, inp, n->n_sub1, CHAR_NEWLINE);
			   strcpy(inp, line);
			   }
			}
		     if (strlen(inp) > n->n_len)
			inp[n->n_len] = NULL;
		     p = inp;
		     }
                  fb_move(row, col);
		  if (n->n_reverse)
		     fb_stand(p);
		  else
	             fb_printw(FB_FSTRING, p);
		  }
	       else{			/* redraw the whole screen */
	          fb_display(1);
                  }
               }
            }
         return(s);
      }

/*
 * replace - provide a substring replacement mechanism.
 *   substitute into s the string p according to postions n1 and n2.
 */

   static replace(s, p, n1, n2)
      char *s, *p;
      int n1, n2;
      
      {
         int len, n;
	 char *pp;
	 
	 len = strlen(s);
	 for (; len <= n2; len++)
	    strcat(s, " ");
	 pp = p;
	 for (n = n1 - 1; n < n2; n++){
	    if (!*pp)
	       break;
	    s[n] = *pp++;
	    }
      }
