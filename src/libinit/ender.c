/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: ender.c,v 9.2 2001/02/05 18:19:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Ender_sid[] = "@(#) $Id: ender.c,v 9.2 2001/02/05 18:19:57 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <dbacct.h>

extern short int cdb_signature;
extern short int cdb_no_acct;
extern short int cdb_secure;
extern char *cdb_pgm;
extern char *cdb_DBVEMIT;

/* 
 *  end sequence for all Cdb programs 
 */
 
   fb_ender()
      {
         if (!cdb_batchmode){
	    fb_clear(), fb_move(1, cdb_t_cols - 2);
	    if (cdb_signature)
	       fb_printw(SYSMSG[S_FB]);
	    fb_move(cdb_t_lines,1);
	    fb_refresh();
            fb_cx_set_toolname("NOTOOL");
            fb_cx_write(1);
	    }
#if FB_CDB_SECURE
         if (cdb_secure)
            if (!equal(cdb_pgm, cdb_DBVEMIT) && !cdb_no_acct &&
                  !fb_magic_environ())
	       fb_acctlog(FB_AC_ENDTOOL, NIL, NIL);
#endif /* FB_CDB_SECURE */
         fb_exit(0);
      }
