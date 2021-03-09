/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: inp_entr.c,v 9.0 2001/01/09 02:57:04 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Input_entry_sid[] = "@(#) $Id: inp_entr.c,v 9.0 2001/01/09 02:57:04 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short int cdb_edit_input;	/* OLD == 1, NEW == 2 */
extern short int cdb_decimal;
extern char *cdb_e_buf;

/*
 * input_entry - this is the entry point to the two input mechanisms.
 *	e_input() is the preferred method, but input() has been retained
 *	for backward compability, though renamed o_input().
 */

   fb_input(row, col, maxc, minc, fmt, addr, xecho, okend, confirm)
      int   row, col, maxc, minc, xecho, okend, confirm;
      char fmt, *addr;

      {
         int s;

         if (!isatty(1) || !isatty(0))
            fb_xerror(FB_MESSAGE, "Trying input from non-interactive device.",
               NIL);
         if (cdb_edit_input){
            if (fmt == FB_DOLLARS && cdb_decimal && cdb_e_buf[0] != NULL)
               fb_putdecimal(cdb_e_buf);
            s = fb_e_input(row, col, maxc, minc, fmt, addr, xecho, okend,
               confirm);
            }
         else
            s = fb_o_input(row, col, maxc, minc, fmt, addr, xecho, okend,
               confirm);
         cdb_e_buf[0] = NULL;
         return(s);
      }
