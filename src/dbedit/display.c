/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: display.c,v 9.0 2001/01/09 02:55:34 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Display_sid[] = "@(#) $Id: display.c,v 9.0 2001/01/09 02:55:34 john Exp $";
#endif

#include <dbedit.h>

/* 
 *  display ten fields starting at fld or just fld if fld < 0 
 */

   int fb_local_display(fld)
      int fld;

      {
         return(db_display(fld));
      }

   int db_display(fld)
      int fld;
   
      {
         int i, row, ntop, k;
         char *sfld;

         k = 0;
         sfld = cdb_afld;
         row = 2;
         ntop = fld;
	 k = fld - 1;
	 if (fld > 0){
	    fb_move(4,1);
            fb_clrtobot();
            }
         for (i = 1; i <= 10; i++){
            if (fld > cdb_sfields)			/* fld is 1 based */
               break;
	    fb_fetch(cdb_sp[k], sfld, cdb_db);		/* anh thuong em */
            db_putfield(fld++, cdb_sp[k++], sfld, row++);
            }
	 fb_infoline();
         return(ntop);
      }
