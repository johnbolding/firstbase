/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: addfield.c,v 9.0 2001/01/09 02:55:33 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Addfield_sid[] = "@(#) $Id: addfield.c,v 9.0 2001/01/09 02:55:33 john Exp $";
#endif

#include <dbedit.h>

extern short int cdb_e_st;
extern short int cdb_edit_input;

/* 
 *  addfield - for use on an existing record.
 *	allow entry into each fb_field of the record starting at
 *	the passed in fld or 1. An FB_END returns to record level.
 *	A <CTL>X goes to the next fb_field.
 *      note: edit_field uses stores, which puts fields in the hp->arec area.
 */

static char *MSG  = "Auto Field Level";

   addfield(fld, top)
      int fld, *top;
      
      {
         int j, up = 0, changes = 0;
   
         if (fld <= 1 || fld > cdb_sfields)
	    fld = 1;
	 fb_scrstat(MSG);
         for (; fld >= 1 && fld <= cdb_sfields; ){	/* fld is 1 based */
	    if (fld < *top || fld - *top >= 10){
	       *top = fld;
               db_display(*top);
	       }
            for (j = 1; j > 0 && j <= 10; j++){
               if (cdb_edit_input && cdb_e_st != 0){
                  st = cdb_e_st;
                  cdb_e_st = 0;
                  }
	       else if (cdb_sp[fld-1]->type != FB_FORMULA &&
                     cdb_sp[fld-1]->dflink == NULL &&
                        cdb_sp[fld-1]->type != FB_BINARY)
                  st = edit_field(fld, -*top);
	       else
	          st = FB_ERROR;
	       if (st == FB_AOK){
                  /* old code was nulling out dup_fld here. wrong, i think */
	          db_checkformula(fld, *top);
		  }
	       else{
		  fb_fetch(cdb_sp[fld-1], cdb_afld, hp);
		  db_putfield(fld, cdb_sp[fld-1], cdb_afld,
		     ((fld - *top) * 2 + 4) / 2);	/* to use putfield */
		  }
	       switch(st){
                  case FB_ABORT:
                  case FB_WSIGNAL:
                  case FB_DELSIGNAL:
                  case FB_PSIGNAL:
                  case FB_PAGEUP:
                  case FB_PAGEDOWN:
                  case FB_END:
                  case FB_DSIGNAL:
		     return(changes);
		  case FB_ERROR:
		     if (!up)		/* if last action was not YSIG */
		        break;		/* ... then break out, else do YSIG */
		     /* fall through- to FB_YSIGNAL */
		  case FB_YSIGNAL:
		     fld--; j -= 2;
		     if (fld > 0 && (fld < *top || fld - *top >= 10)){
			j = 0;
			*top = fld;
			db_display(*top);
			}
		     up = 1;
		     continue;
		  case FB_AOK:
		  case FB_DEFAULT:
		     changes++;
                     /*
                      * i dont really like this behavior, but it could
                      * be tied to an FB_ESCAPE_AOK signal coming from
                      * edit_field (ala fb_input, fb_choiceinput, etc, etc)
                      * but for now, its gone.
                      *
		      * if (up == 1){
		      *    fld--; j -= 2;
		      *    if (fld > 0 && (fld < *top || fld - *top >= 10)){
		      *       j = 0;
		      *       *top = fld;
		      *       db_display(*top);
		      *       }
		      *     continue;
                      *     }
                      */
		     break;
		  case FB_ESIGNAL:
		  case FB_QSIGNAL:
		  default:
		     break;
		  }
               if (cdb_edit_input && cdb_e_st != 0){
                  j--;
                  continue;		/* to do the saved command */
                  }
	       fld++;
	       up = 0;
               if (fld > cdb_sfields)
                  return(changes);
	       else if (fld >= *top + 10)
	          break;
               }
            }
	 return(changes);
      }
