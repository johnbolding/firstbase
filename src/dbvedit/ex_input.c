/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: ex_input.c,v 9.1 2001/01/16 02:46:48 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Ex_input_sid[] = "@(#) $Id: ex_input.c,v 9.1 2001/01/16 02:46:48 john Exp $";
#endif

#include <dbve_ext.h>
#include <sys/types.h>
#include <sys/stat.h>

extern char *cdb_prompt_choicemsg;	/* prompt for choice message */
extern char *cdb_dbvedit_cho_ploc;	/* prompt location for choice level */
extern short int cdb_dbvedit_cho_pilength; /* length, dbvedit, choice level */
extern short int cdb_record_level;	/* record level toggle switch */
extern short int cdb_dbvedit_cho_firstline; /* top line of display */
extern short int cdb_ex_forceone;	/* exchioce force one when no others */

static int choicepages;

/*
 * ex_input - do the input for the extended choice/help.
 */

   ex_input(exc, k, inp, new, okend)
      fb_exchoice *exc;
      fb_field *k;
      char *inp;
      int new, okend;
      
      {
	 int minc = 0, maxc, icol, ploc_row = cdb_t_lines, ploc_col = 2,
            t_minc, row, screensize;
	 char *cp, buf[FB_MAXLINE], label[FB_MAXLINE], *prompt;
	 long t_ptop;

         if (cdb_ex_forceone && exc->ex_last == exc->ex_first){
	    if ((st = ex_select(exc, "1", inp)) == FB_ERROR)
	       fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
            return(st);
            }
	 if (cdb_dbvedit_cho_ploc != NULL){
	    strcpy(buf, cdb_dbvedit_cho_ploc);
	    cp = strchr(buf, ',');
	    *cp = NULL;
	    ploc_row = atoi(buf);
	    ploc_col = atoi(cp + 1);
	    }

	 row = ploc_row;
	 if (exc->ex_prompt != NULL)
	    prompt = exc->ex_prompt;
	 else
	    prompt = cdb_prompt_choicemsg;
	 icol = (strlen(prompt) + ploc_col);
	 maxc = -cdb_dbvedit_cho_pilength;
	 inp[0] = NULL;
         /* always allow FB_DSIGNAL now */
	 icol = -icol;
	 if (k->idefault == NULL || strlen(k->idefault) == 0)
	    minc = 1;
	 if (!cdb_record_level)
	    t_minc = 0;
	 else
	    t_minc = minc;
	 exc->ex_firstrow = cdb_dbvedit_cho_firstline;
	 exc->ex_lastrow = row - 2;
	 screensize = exc->ex_lastrow - exc->ex_firstrow + 1;
         /* test here whether D and dU belong */
         choicepages = 1;
         if (exc->ex_tree){
            ex_incr_tree(exc, screensize, 1);
            if (exc->ex_ptop != exc->ex_first){
               choicepages = 2;
               ex_decr_tree(exc, screensize);
               }
            }
         else if (exc->ex_last - exc->ex_first >
               exc->ex_lastrow - exc->ex_firstrow)
            choicepages = 2;
         fb_cx_push_env("EXNP", CX_XCHOICE_SELECT, NIL); 	/* fld level */
         if (choicepages > 1)
            fb_cx_add_buttons("dU");
         if (t_minc == 0)
            fb_cx_add_buttons("D");
	 if (exc->ex_help != NULL)
            fb_cx_add_buttons("H");
         if (label[0] == NULL)
            fb_cx_add_buttons("8");
	 for (st = FB_ERROR; st == FB_ERROR ;){
	    for (;;){
               if (ex_display(exc) == FB_ERROR){
                  fb_serror(FB_MESSAGE,
                     "Could not display extended choicefile.", NIL);
                  fb_cx_pop_env();
                  return(FB_ERROR);
                  }
               fb_showcomment();
               fb_infoline();
	       fb_move(ploc_row, ploc_col);
	       fb_printw(prompt);
               if (choicepages <= 1)
                  buf[0] = NULL;
               else
                  strcpy(buf, "<CTL>-F=Next Page, <CTL>-B=Prev Page, ");
               strcat(buf, "<CTL>-X=Abort");
               fb_scrhlp(buf);
               fb_cx_deposit_exchoices(exc);
               st = fb_input(-row,icol,maxc,t_minc,FB_POS_NUM,label,
                  FB_ECHO,-FB_OKEND,FB_CONFIRM);
               fb_cx_delete_exchoices();

               /*
                * this code is intended to make more keystrokes
                * work in more places, more transparent
                * - make ^D work like FB_END for non new records
                * - make FB_END be some FB_QSIGNAL interpreted outside
                */
               if (st == FB_END && okend != FB_OKEND)
                  st = FB_QSIGNAL;
               if (st == FB_DSIGNAL && !new)
                  st = FB_END;

	       if (!cdb_record_level && st == FB_DEFAULT)
		  if ((k->fld != NULL && strlen(k->fld)>0) || minc)
		     st = FB_ESIGNAL;
/*
*              removing this enables the FB_DSIGNAL to filter back up
*
*	       if (st == FB_DSIGNAL){
*		  if (!minc)
*		     st = FB_DEFAULT;
*		  else
*		     st = FB_ESIGNAL;
*		  }
*/
	       if (st == FB_QHELP)
	          fb_fhelp(exc->ex_help);
	       else if (st == FB_PAGEUP || st == FB_BSIGNAL){
                  if (exc->ex_tree){
                     /* btree index */
                     ex_decr_tree(exc, screensize);
                     }
                  else{
                     /* normal style index */
                     t_ptop = exc->ex_ptop;
                     exc->ex_ptop -= screensize;
                     if (exc->ex_ptop < exc->ex_first){
                        if (t_ptop == exc->ex_first)
                           exc->ex_ptop = exc->ex_last - screensize + 1;
                        }
                     if (exc->ex_ptop < exc->ex_first || exc->ex_ptop== t_ptop)
                        exc->ex_ptop = exc->ex_first;
                     }
		  }
	       else if (st == FB_PAGEDOWN || st == FB_FSIGNAL){
                  if (exc->ex_tree){
                     /* btree index */
                     ex_incr_tree(exc, screensize, 1);
                     }
                  else{
                     /* normal style index */
                     exc->ex_ptop += screensize;
                     if (exc->ex_ptop > exc->ex_last)
                        exc->ex_ptop = exc->ex_first;
                     }
		  }
	       else
	          break;
	       }
	    fb_trim(fb_rmlead(label));
	    if (st == FB_AOK){
	       if ((st = ex_select(exc, label, inp)) == FB_ERROR)
	          fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
	       }
	    }
         fb_cx_pop_env();
	 return(st);
      }

/*
 * ex_incr_tree - increment the choice focus by n elements forwards.
 */

   ex_incr_tree(e, n, wrapflag)
      fb_exchoice *e;
      int n, wrapflag;

      {
         int count = 0;
         long srec = 0, fb_key_eval(), rec;
         fb_bseq *bs;

         bs = e->ex_db->b_seq;
         srec = e->ex_ptop;
         bs->bs_curkey = e->ex_ptop_curkey;
         if (fb_seq_getrec(srec, bs) == FB_ERROR)
            return(FB_ERROR);
         for (; count < n; ){
            bs->bs_curkey++;
            if (srec > e->ex_last ||
                  srec == e->ex_last && bs->bs_curkey > e->ex_last_curkey){
               if (wrapflag){
                  srec = e->ex_first;
                  bs->bs_curkey = e->ex_first_curkey;
                  break;
                  }
               else
                  return(FB_ERROR);
               }
            if (bs->bs_curkey > 3){
               /* read next node */
               srec = bs->bs_next;
               bs->bs_curkey = 1;
               if (srec == 0){
                  if (wrapflag){
                     srec = e->ex_first;
                     bs->bs_curkey = e->ex_first_curkey;
                     break;
                     }
                  else
                     return(FB_ERROR);
                  }
               if (fb_seq_getrec(srec, bs) == FB_ERROR)
                  return(FB_ERROR);
               }
            rec = fb_key_eval(bs);
            if (rec > 0)
               count++;
            }
         e->ex_ptop = srec;
         e->ex_ptop_curkey = bs->bs_curkey;
         return(FB_AOK);
      }

/*
 * ex_decr_tree - decrement the choice focus by n elements backwards.
 *	i.e. page backwards, wrap backwards to bottom if at top now.
 */

   ex_decr_tree(e, n)
      fb_exchoice *e;
      int n;

      {
         int count = 0, at_top = 0;
         long srec = 0, fb_key_eval(), rec;
         fb_bseq *bs;

         /* determine if at very top of series */
         if (e->ex_ptop == e->ex_first &&
               e->ex_ptop_curkey == e->ex_first_curkey)
            at_top = 1;
         bs = e->ex_db->b_seq;
         srec = e->ex_ptop;
         bs->bs_curkey = e->ex_ptop_curkey;
         if (fb_seq_getrec(srec, bs) == FB_ERROR)
            return(FB_ERROR);
         for (; count < n; ){
            bs->bs_curkey--;
            /* test to see if past edges of this display section */
            if (srec < e->ex_first ||
                  srec == e->ex_first && bs->bs_curkey < e->ex_first_curkey){
               if (at_top){
                  srec = e->ex_last;
                  bs->bs_curkey = e->ex_last_curkey;
                  if (fb_seq_getrec(srec, bs) == FB_ERROR)
                     return(FB_ERROR);
                  at_top = 0;
                  }
               else{
                  /* if not at top, set to top and get out */
                  srec = e->ex_first;
                  bs->bs_curkey = e->ex_first_curkey;
                  break;
                  }
               }
            /* test to see if this read is going to be out of whack */
            if (bs->bs_curkey <= 0){
               /* read previous node */
               srec = bs->bs_prev;
               bs->bs_curkey = 3;
               if (srec == 0){
                  if (at_top){
                     srec = e->ex_last;
                     bs->bs_curkey = e->ex_last_curkey;
                     at_top = 0;
                     }
                  else{
                     srec = e->ex_first;
                     bs->bs_curkey = e->ex_first_curkey;
                     break;
                     }
                  }
               if (fb_seq_getrec(srec, bs) == FB_ERROR)
                  return(FB_ERROR);
               }
            rec = fb_key_eval(bs);
            if (rec > 0)
               count++;
            }
         e->ex_ptop = srec;
         e->ex_ptop_curkey = bs->bs_curkey;
         return(FB_AOK);
      }
