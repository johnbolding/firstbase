/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: newrec.c,v 9.0 2001/01/09 02:55:35 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Newrec_sid[] = "@(#) $Id: newrec.c,v 9.0 2001/01/09 02:55:35 john Exp $";
#endif

#include <dbedit.h>

extern short int cdb_e_st;
extern short int cdb_edit_input;
extern short int cdb_secure;

static char *FB_ABORT_MSG = "Really Abort Record? (y=yes, <other>=no):";

/* 
 *  newrec - create new record at end. allow entry into each fb_field.
 *     detection of the wacky autodefault signal is done here.
 *     this builds the record before knowing where it will physically go.
 *     edit_field uses stores, which puts fields in the hp->arec area.
 */
 
   newrec()
      {
         int fld, top, j, up = 0;
   
         hp->orec[0] = hp->orec[1] = NULL;
 	 for (j = 0; j <= hp->nfields; j++)
	    FB_FLD(j, hp) = NIL;			/* all fields = true null */
         fb_clear_autoindex(hp);
         st = 0;
         fld = 1;
	 autodef = 0;
	 fb_scrstat(SYSMSG[S_FIELD_LEVEL]);
	 fb_scrstat2(SYSMSG[S_ADDMODE]);
	 fb_nullall();
         for (; fld <= cdb_sfields; ){           	/* fld is 1 based */
            top = fld;
            db_display(top);
	    db_checkformula(fld, top);		/* for links on 'next' page */
            for (j = 1; j <= 10; j++){
               if (cdb_edit_input && cdb_e_st != 0){
                  st = cdb_e_st;
                  cdb_e_st = 0;
                  }
	       else if (cdb_sp[fld-1]->type != FB_FORMULA &&
                     cdb_sp[fld-1]->dflink == NULL && cdb_sp[fld-1]->type != FB_BINARY)
                  st = edit_field(-fld, -top);
	       else
	          st = FB_ERROR;
	       if (st == FB_AOK || st == FB_DEFAULT)
	          db_checkformula(fld, top);
               if (st == FB_END || st == FB_QSIGNAL){
                  if (cdb_secure)
	             fb_recmode(hp, FB_BLANK, fb_getuid(), fb_getgid(), "666");
                  else
                     fb_store(hp->kp[hp->nfields], " ", hp); /* so no thrash */
                  return(st);
                  }
	       else if (st == FB_ABORT){
	          st = fb_mustbe(CHAR_y, FB_ABORT_MSG, cdb_t_lines, 1);
                  if (st == FB_AOK)
		     return(FB_ABORT);
		  continue;
	          }
	       if (st != FB_AOK){
		  fb_fetch(cdb_sp[fld-1], cdb_afld, hp);
		  db_putfield(fld, cdb_sp[fld-1], cdb_afld,
		     ((fld - top) * 2 + 4) / 2);	/* to use putfield */
		  }
	       if (st == FB_YSIGNAL || (st == FB_ERROR && up)){
	          j--;
	          if (fld > 1){
		     fld--; j--;
		     if (fld < top || fld - top >= 10){
			top = fld;
			j = 0;
			db_display(top);
			}
		     up = 1;
		     continue;
		     }
		  }
	       else if (st == FB_DSIGNAL){
	          j--;
		  autodef = 1;
	          fb_scrstat2(SYSMSG[S_AUTODEF_MODE]);
		  fb_refresh();
		  }
               else if (cdb_edit_input && cdb_e_st != 0){
                  j--;
                  continue;		/* to do the saved command */
                  }
               else if (++fld > cdb_sfields){	   	/* set del fb_field */
                  if (cdb_secure)
	             fb_recmode(hp, FB_BLANK, fb_getuid(), fb_getgid(), "666");
                  else
                     fb_store(hp->kp[hp->nfields], " ", hp); /* so no thrashing */
                  return(st);
		  }
	       up = 0;
	       if (fld >= top + 10 || fld < top)
	          break;
               }
            }
         return(0);
      }
