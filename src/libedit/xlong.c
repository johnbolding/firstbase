/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: xlong.c,v 9.0 2001/01/09 02:56:42 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Xlong_sid[] = "@(#) $Id: xlong.c,v 9.0 2001/01/09 02:56:42 john Exp $";
#endif

#include <dbve_ext.h>

/* 
 *  xlong.c - routines to handle extra fb_field display and input.
 */

static char *MSG1 = "--- Long Field Screen ---";
static char *FMT1 = "%3d) %-10s <display area>";
static char *FMT2 = "%3d) %-10s <input area>";
static char *FMT3 = "%-50.50s";
static char *MSG2 = "Enter Data, <CTL>-X=Keep, <Ret>=quit";
static char CHAR_ANGLE_R = '>';
static char *FMT4 = "%.*s";
extern short int cdb_edit_input;
extern char cdb_e_buf[];

static int getit(char *p, int osize, int minc, int typ);
static int anychange(void);
static void display(fb_field *f, char *p);	/* local display func */

/* 
 * longinput - routine called from edit_field in dbedit.c 
 *   idea here is to display entire fb_field and
 *   allow changes if necessary...entire screen is used.
 */
 
   int fb_longinput(f, line, n, new)
      fb_field *f;
      char *line;
      int n, new;
   
      {
	 int minc, o_new;
         char buttons[20], default_button[3];
	 
	 strcpy(line, f->fld);
         fb_move(3, 1), fb_clrtobot(); fb_refresh();
	 fb_move(4, 17);
	 fb_stand(MSG1);
         o_new = new;
	 for (;;){
	    fb_move(6, 1); fb_clrtobot();
	    fb_printw(FMT1, n+1, f->id);
	    display(f, line);			/* display at top */
	    fb_infoline();
	    if (!new)
	       st = anychange();
	    else{
	       new = 0;
	       st = FB_AOK;
	       }
	    switch (st){
	       case FB_END:
               case FB_DEFAULT:		/* null defaults for xlongs */
	          return(FB_AOK);
	       case FB_ESIGNAL: case FB_YSIGNAL:
	       case FB_ABORT:
	          return(st);
	       case FB_DSIGNAL:
                  if (!o_new)
                     st = FB_END;
                  return(st);
	       case FB_QHELP:
	          fb_fhelp(f->help);
		  break;
	       case FB_AOK:
	          if (scanner == 1)
		     return(FB_ERROR);
	          fb_move(14, 1);
	          fb_printw(FMT2, n+1, f->id);
                  strcpy(default_button, "D");
		  if (f->idefault != NULL)
		     minc = 0;
		  else{
		     minc = 1;
                     default_button[0] = NULL;
                     }
                  sprintf(buttons, "75%s", default_button);
                  fb_cx_push_env(buttons, CX_KEY_SELECT, NIL);	/* fld lev */
	    	  st = getit(line, f->size, minc, f->type);
                  fb_cx_pop_env();
		  if (st == FB_DEFAULT || st == FB_ABORT)
		     return(st);
		  fb_move(cdb_t_lines, 1); fb_clrtoeol();
		  break;
	       default:
	          fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
	       }
	    }
         /* NOTREACHED */
         return(st);
      }

/* 
 *  display - display a line at lines 5-10...50 each... 
 */
 
   static void display(f, p)
      fb_field *f;
      char *p;
      
      {
         int row, s, sp;

	 fb_trim(p);	 
	 s = strlen(p);
	 for (sp = 0, row = 8; row <= 12; row++){
	    if (sp >= f->size)
	       break;
	    fb_move(row, 15);
	    fb_s_putw(CHAR_ANGLE_R);
	    fb_s_putw(CHAR_BLANK);
	    if (sp < s){
	       fb_printw(FMT3, p);
	       p += 50;
	       }
	    sp += 50;
	    }
      }

/* 
 *  getit - get lines that make up extra long fb_field 
 */
 
   static int getit(p, osize, minc, typ)
      char *p;
      int osize, minc, typ;
      
      {
         char *inp, tcom[FB_SCREENFIELD+2], *ptr;
	 int i, j, size, row, xlen, count;

         inp = cdb_bfld; /* set local to global -- p == cdb_afld already */
         ptr = p;
	 xlen = strlen(ptr);
         size = osize;
	 count = 0;
	 inp[0] = NULL;
	 if (typ != FB_ALPHA && typ != FB_UPPERCASE)
	    typ = FB_ALPHA;
	 fb_fmessage(MSG2);
	 for (row = 16; row <= 20 && size > 0; row++){
	    fb_move(row, 15);
	    fb_s_putw(CHAR_ANGLE_R);
	    size -= 50;
	    }
	 size = osize;
	 for (row = 16; row <= 20 && size > 0; row++){
	    i = (size > 50) ? 50 : size;
            size -= 50;
	    if (row != 16)	/* kludge: NO DEFAULT if minc>0 first pass */
	       minc = 0;
	    tcom[0] = NULL;
            if (cdb_edit_input && count < osize && xlen > 0)
	       sprintf(cdb_e_buf, FMT4, i, ptr); /* ok since Max is 50 */
	    st = fb_input(row, 17, i, minc, typ, tcom, FB_ECHO, FB_NOEND,
               FB_CONFIRM);
	    if (st == FB_ABORT){
	       if (count < osize){
		  if (xlen > 0)
		     sprintf(tcom, FMT4, i, ptr); /* ok since Max is 50 */
		  else
		     sprintf(tcom,SYSMSG[S_FMT_STAR_S],i,
		        SYSMSG[S_STRING_BLANK]);
		  }
	       fb_move(row, 17);
	       fb_printw(FB_FSTRING, tcom);
	       }
	    else if (st == FB_DEFAULT)
	       break;
	    fb_trim(tcom);
	    if ((j = strlen(tcom) - 1) < 0)
	       j = 0;
	    if (tcom[j] != CHAR_BACKSLASH){	/* backslash */
	       strcat(tcom, SYSMSG[S_STRING_BLANK]);/* fb_put B bet.words */
	       if (j == 49)			/* filled line + blank */
		  size--;			/* subtract blank width */
	       }
	    else
	       tcom[j] = NULL;			/* null out the backslash */
	    strcat(inp, tcom);
	    ptr += i;
	    count += i;
	    xlen -= 50;
	    }
	 if (st == FB_DEFAULT && inp[0] == NULL)
	    return(FB_DEFAULT);
	 strcpy(p, inp);
	 return(FB_AOK);
      }
	       
/* 
 *  anychange - ask for anychange to xlong line 
 */
 
   static int anychange()
      
      {
         char tcom[5];
	 
         fb_move(cdb_t_lines, 1); fb_clrtoeol();
	 if (scanner == 0)
	    fb_printw(SYSMSG[S_ANYCHANGE]);
	 else{
	    fb_serror(FB_MESSAGE, NIL, NIL);
	    return(FB_AOK);
	    }
         fb_percentage(pcur->p_num, ptail->p_num);
         fb_cx_push_env("EXNPynH", CX_KEY_SELECT, NIL);
	 st = fb_input(-cdb_t_lines, -36, -1, 0, FB_ALPHA, tcom, FB_ECHO,
            FB_OKEND, FB_CONFIRM);
	 				/* allow FB_QHELP and FB_DSIGNAL */
         fb_cx_pop_env();
	 if (st == FB_AOK){
	    if (tcom[0] == CHAR_y)
	       return(FB_AOK);
	    else
	       return(FB_DEFAULT);
	    }
	 else 
	    return(st);
      }
