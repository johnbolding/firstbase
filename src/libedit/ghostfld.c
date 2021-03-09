/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: ghostfld.c,v 9.2 2001/02/16 19:15:28 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Ghostfield_sid[] = "@(#) $Id: ghostfld.c,v 9.2 2001/02/16 19:15:28 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <vdict.h>

/*
 * ghostfield - place a ghost image of a fb_field in place. dbvedit/dbvemit
 */

   void fb_ghostfield(n, revflag)
      fb_node *n;
      int revflag;
      
      {
         char temp[FB_PCOL+1], temp1[FB_PCOL+1], *line, *p;

         line = cdb_bfld;
	 if (n->n_fp->fld[0] != NULL){
	    strcpy(line, n->n_fp->fld);
	    strncpy(temp1, line, cdb_t_cols);
	    if (n->n_type == T_SUBELEMENT_R || n->n_type == T_SUBELEMENT_N)
               if ((p = strchr(temp1, '|')) != 0)	/* choice_attribute */
                  *p = NULL;
	    }
	 else
	    temp1[0] = NULL;
	 fb_pad(temp, temp1, n->n_len);
         fb_move(n->n_row, n->n_col);
	 if (revflag)
	    fb_reverse(temp);
	 else
	    fb_printw(temp);
      }
