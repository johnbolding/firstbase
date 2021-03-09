/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: edit_add.c,v 9.1 2001/02/16 19:41:43 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Edit_add_sid[] = "@(#) $Id: edit_add.c,v 9.1 2001/02/16 19:41:43 john Exp $";
#endif

#include <dbedit.h>

/* 
 *  edit_add - edit fb_field by allowing numeric plus or minus entries 
 */
 
   edit_add(fld, top)
      int fld, top;
   
      {
         int row, k;
         long new;
	 double newf;
         char inp[FB_NUMLENGTH+1], line[FB_MAXLINE+1], s2[FB_MAXLINE+1];

         k = fld - 1;
         if (cdb_sp[k]->lock == CHAR_y){
            fb_serror(FB_MESSAGE, SYSMSG[S_FIELD], SYSMSG[S_LOCKED]);
            return(FB_ERROR);
            }
         if (!(FB_OFNUMERIC(cdb_sp[k]->type))){
             fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], SYSMSG[S_NON_NUMERIC]);
             return(FB_ERROR);
             }
	 fb_fmessage(SYSMSG[S_EADD_MSG]);
         st = fb_input(cdb_t_lines, 36, cdb_sp[k]->size, 0, cdb_sp[k]->type,
              inp, FB_ECHO,FB_OKEND,FB_CONFIRM);
	 fb_move(cdb_t_lines, 1); fb_clrtoeol();
         if (st != FB_AOK)
            return(st);
         row = ((fld - top) * 2 + 4) / 2;  /* for compatibility w/putfield */
	 strcpy(line, cdb_sp[k]->fld);
	 if (cdb_sp[k]->type != FB_FLOAT){
	    new = atol(fb_rmlead(line)) + atol(fb_rmlead(inp));
	    sprintf(line, SYSMSG[S_EADD_FMT1], new);
	    }
	 else{
	    newf = atof(fb_rmlead(line)) + atof(fb_rmlead(inp));
	    sprintf(line, SYSMSG[S_EADD_FMT2], newf);
	    }
         strcpy(s2, fb_rmlead(line));
         db_putfield(fld, cdb_sp[k], line, row);
         fb_move(cdb_t_lines, 50), fb_clrtoeol();
         fb_printw(SYSMSG[S_ASK_OK]);
         if (fb_input(cdb_t_lines, 76, 1, 0, FB_ALPHA, inp, FB_ECHO, FB_OKEND,
               FB_NOCONFIRM) == FB_DEFAULT){
            fb_store(cdb_sp[k], s2, hp);
            return(FB_AOK);
            }
         else{
	    strcpy(line, cdb_sp[k]->fld);
            db_putfield(fld, cdb_sp[k], line, row);
            return(FB_ABORT);
            }
      }
