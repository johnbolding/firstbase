/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: docmd.c,v 9.0 2001/01/09 02:56:00 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Docmd_sid[] = "@(#) $Id: docmd.c,v 9.0 2001/01/09 02:56:00 john Exp $";
#endif

#include <dbve_ext.h>

static int ext_command = 0;
static char *NOADD = ".noadd";
extern char *cdb_HLP_EDCOM;

/* 
 *  docmd - execute one dbedit command 
 */

   docmd()
      {
         int eflag, disp_st;
         long atol();
	 char *fb_trim(), tfile[FB_MAXNAME];

	 disp_st = REDRAW;
         eflag = 0;
         if (st == FB_END || st == FB_ABORT){
	    if (st == FB_ABORT){
	       if (ext_command == 1){
		  ext_command = 0;
		  fb_xcommand();
		  return(NOREDRAW);
		  }
	       else
		  ext_command = 1;
	       }
            oldrec = 0;
            if (pindx == 0 && st == FB_END && mode == NULL)
               return(FB_END);
            irec[pindx] = ibase[pindx];
            mode = NULL;
            return(NOREDRAW);
            }
	 ext_command = 0;
	 if (st == FB_QHELP){
	    fb_move(3,1); fb_clrtoeol();
	    if (phead->p_help == NULL)
	       fb_fhelp(cdb_HLP_EDCOM);
	    else
	       fb_fhelp(pcur->p_help);
	    return(ZAP_REDRAW);
	    }
	 fb_trim(com);
         switch (com[0]){
            case FB_FADD:
               eflag = 1;
               rec = -1L;
               break;
            case '?':				/* FB_HELP */
	       fb_move(3, 1); fb_clrtoeol();
               fb_help(com, hp);
	       return(ZAP_REDRAW);
               break;
	    case '!':				/* FB_BANG */
	       fb_cshell(com);
	       fb_serror(FB_MESSAGE, NIL, NIL);
	       fb_scrhdr(hp, NIL);
	       fb_scrlbl(hp->sdict);
	       return(ZAP_REDRAW);
	       break;
            case '>':				/* FB_INTO */
               if (pindx + 1 < cdb_db->ifields - 1){ /* iptr is there */
                  ibase[pindx + 1]  = irec[pindx];
                  irec[pindx + 1] = irec[pindx];
                  pindx++;
                  }
               else
                  fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
               return(NOREDRAW);
            case '<':				/* FB_OUTOF */
               if (pindx - 1 >= 0)
                  --pindx;
               else
                  fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
               return(NOREDRAW);
            case '.':				/* FB_DOT */
               eflag = 1;
               /* rec = fb_scanfor(NIL, 2); */
               break;
            case '@':				/* FB_DEREF */
               if ((com[1] == NULL || com[1] == CHAR_a || com[1] == CHAR_A) &&
	             scanner == 0){
	          fb_rootname(tfile, hp->dbase);
		  strcat(tfile, NOADD);
		  if (access(tfile, 0) == 0){
		     if (globaladd)
                        fb_xerror(FB_MESSAGE, SYSMSG[S_ILLEGAL],
                           SYSMSG[S_LOCKED]);
		     /* else */
                     fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], SYSMSG[S_LOCKED]);
                     return(NOREDRAW);
		     }
                  mode = FB_FADD;	/* so next time, com will be FB_FADD */
		  }
               else{		/* must be a simple deref */
                  eflag = 1;
                  rec = atol(com+1);
		  hp->irec[0] = NULL;
                  }
               break;
            default:
               if (st == FB_END || st == FB_ABORT)
                  def = 2;
               else if (st == FB_DEFAULT || st == FB_ESIGNAL){
                  if (hp->bsmax <= 0L){
                     def = -1;
                     if ((rec = oldrec + 1L) > cdb_db->reccnt)
		        rec = 1L;
                     }
                  else
                     def = 1; /* bump index ptr */
                  }
               else if (st == FB_YSIGNAL){
                  if (hp->bsmax <= 0L){
                     def = -1;
                     if ((rec = oldrec - 1L) < 1L)
		        rec = cdb_db->reccnt;
                     }
                  else
                     def = 3; /* decr index ptr */
                  }
               else
                  def = 0;
/*             if (def >= 0)
*                 rec = fb_scanfor(com, def);
*/
               eflag = 1;
               break;
            }
         if (eflag == 1){
/*
*           lock(0L, FB_WAIT);
*	    st = fb_gethead(hp);
*           unlock(0L);
*	    if (st == FB_ERROR)
*	       fb_xerror(FB_READ_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
*/
            if ((rec > 0L && rec <= (hp->reccnt)) || rec == -1L){
               oldrec = rec;
               if ((disp_st = edit(dot)) == FB_END)  /* from autoadd mode */
	          disp_st = REDRAW;
               }
            else{
	       fb_fmessage(NIL);
               fb_serror(FB_MESSAGE, SYSMSG[S_SEARCHING], SYSMSG[S_NOT_FOUND]);
	       return(NOREDRAW);
	       }
            }
	 if (mode == NULL && globaladd)
	    return(FB_END);
         return(disp_st);
      }
