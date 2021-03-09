/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: macrolev.c,v 9.0 2001/01/09 02:56:40 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macrolevel_sid[] = "@(#) $Id: macrolev.c,v 9.0 2001/01/09 02:56:40 john Exp $";
#endif

#include <dbve_ext.h>
#include <macro_e.h>

extern char cdb_macro_comlevel_file[];
extern char cdb_macro_fldlevel_file[];

extern short int cdb_m_fld;/* this is kind of local to macros, in macroinput */

/*
 * macro_level - macro mechanism called for different levels of dbvedit.
 *	1 = command level, 2 = fb_field level
 */

   int fb_macro_level(t)
      int t;

      {
         fb_stack_vars svars, *sv = &svars;
         fb_mnode *n;
         char *efile;

         cdb_m_fld = -1;
         if (t == 1)
            efile = cdb_macro_comlevel_file;
         else if (t == 2)
            efile = cdb_macro_fldlevel_file;
         else
            return(FB_AOK);
         if (access(efile, 0) != 0)
            return(FB_AOK);
         m_current_filename = 0;
         if (fb_macrotree(efile) == FB_AOK){
            e_winner = winner;
            n = locate_section(S_BODY);
            if (n == (fb_mnode *) NULL)
               return(FB_ERROR);
            m_verify_sub(n);

            /* execute the beast */
            mf_inp_make_frame(sv);
            macro_statement(n, sv);

            /* now do garbage collection */
            mf_inp_destroy_frame(sv);
            }
         else if (t == 0)
            fb_serror(FB_MESSAGE, "Macro CommandLevel-File Error", efile);
         else
            fb_serror(FB_MESSAGE, "Macro FieldLevel-File Error", efile);
         return(FB_AOK);
      }
