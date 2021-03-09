/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: showauto.c,v 9.0 2001/01/09 02:56:41 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Showauto_sid[] = "@(#) $Id: showauto.c,v 9.0 2001/01/09 02:56:41 john Exp $";
#endif

#include <dbve_ext.h>

/* showauto - show any forced auto increments.
*/

static char *SCRMSG = "Display AutoIncr";
static char *FMT1 = "%-10s ... ";
static char *MSG = "Display Auto Increment Values";
static int clear = 0;
static int row = 0;
static int oneauto(fb_field *f);
extern char *cdb_T_AUTOINCR;

   int showauto(hp)
      fb_database *hp;

      {
         int j;
	 int nflds = 0;
	 fb_field *f;
         fb_autoindex *ix;

         clear = 0;
         /* first, show any -a defaults */
	 for (j = 0; j < hp->nfields; j++){
	    f = hp->kp[j];
	    if (f->aid != NULL && f->aid->hfd > 0 &&
                  equal(f->idefault, cdb_T_AUTOINCR)){
               /* show this one */
               oneauto(f);
	       nflds++;
               }
            }
         for (j = 0; j < hp->b_maxauto; j++){
            ix = hp->b_autoindex[j];
            f = ix->ix_ip[0];
	    if (equal(f->idefault, cdb_T_AUTOINCR)){
               /* show this one */
               oneauto(f);
               nflds++;
               }
            }

	 if (nflds > 0 && clear != 0){
	    fb_infoline();
	    FB_PAUSE();
	    }
	 return(FB_AOK);
      }

   static oneauto(f)
      fb_field *f;

         {
            if (clear == 0){
               clear = 1;
               fb_move(3,1); fb_clrtobot();
               fb_scrstat(SCRMSG);
               fb_move(4, 27); fb_stand(MSG);
               row = 4;
               }
            row += 2;
            fb_move(row, 10);
            fb_printw(FMT1, f->id);
            fb_rjustify(cdb_afld, f->fld, f->size, f->type); /*NUMERIC*/
            fb_stand(cdb_afld);
            if (row >= cdb_t_lines-2){
               fb_infoline();
               FB_PAUSE();
               clear = 0;
               }
         }
