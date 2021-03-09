/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: edit_add.c,v 9.0 2001/01/09 02:55:58 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Edit_add_sid[] = "@(#) $Id: edit_add.c,v 9.0 2001/01/09 02:55:58 john Exp $";
#endif

#include <dbve_ext.h>

/* 
 *  edit_add - edit fb_field by allowing numeric plus or minus entries 
 */
 
   edit_add(fld)
      int fld;
   
      {
         long new, atol();
	 double newf, atof();
         char inp[FB_NUMLENGTH+1], line[FB_MAXLINE+1],
              s2[FB_MAXLINE+1], *fb_rmlead();
	 fb_field *f;
	 fb_node *n;

	 n = pcur->p_nedit[fld - 1];
	 f = n->n_fp;
         if (f->lock == CHAR_y){
            fb_serror(FB_MESSAGE, SYSMSG[S_FIELD], SYSMSG[S_LOCKED]);
            return(FB_ERROR);
            }
         if (!(FB_OFNUMERIC(f->type))){
             fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], SYSMSG[S_NON_NUMERIC]);
             return(FB_ERROR);
             }
	 fb_fmessage(SYSMSG[S_EADD_MSG]);
         st = fb_input(cdb_t_lines, 36, f->size, 0, f->type, inp,
              FB_ECHO,FB_OKEND,FB_CONFIRM);
	 fb_move(cdb_t_lines, 1); fb_clrtoeol();
         if (st != FB_AOK)
            return(st);
	 strcpy(line, f->fld);
	 if (f->type != FB_FLOAT){
	    new = atol(fb_rmlead(line)) + atol(fb_rmlead(inp));
	    sprintf(line, SYSMSG[S_EADD_FMT1], new);
	    }
	 else{
	    newf = atof(fb_rmlead(line)) + atof(fb_rmlead(inp));
	    sprintf(line, SYSMSG[S_EADD_FMT2], newf);
	    }
         strcpy(s2, fb_rmlead(line));
         fb_putfield(n, f, line);
         fb_move(cdb_t_lines, 50), fb_clrtoeol();
         fb_printw(SYSMSG[S_ASK_OK]);
         if (fb_input(cdb_t_lines, 76, 1, 0, FB_ALPHA, inp, FB_ECHO, FB_OKEND,
               FB_NOCONFIRM) == FB_DEFAULT){
            fb_store(f, s2, hp);
            return(FB_AOK);
            }
         else{
	    strcpy(line, f->fld);
            fb_putfield(n, f, line);
            return(FB_ABORT);
            }
      }
