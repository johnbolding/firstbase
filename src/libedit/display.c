/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: display.c,v 9.0 2001/01/09 02:56:39 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Display_sid[] = "@(#) $Id: display.c,v 9.0 2001/01/09 02:56:39 john Exp $";
#endif

#include <dbve_ext.h>

/* 
 *  display  - start at node n. print whole page if pflag == 1,
 *             else just the node n.
 */
 
   int fb_local_display(fld)
      int fld;

      {
         fb_display(fld);
         return(FB_AOK);
      }

   void fb_display(pflag)
      int pflag;
   
      {
         char *sfld;
	 fb_field *f;
	 fb_node *n;
	 int nc;

         sfld = cdb_afld;
	 if (pflag == 1)
	    fb_scanset();
         for(nc = 0; nc < pcur->p_maxedit; nc++){
	    n = pcur->p_nedit[nc];
	    f = n->n_fp;
	    fb_fetch(f, sfld, cdb_db);		/* anh thuong em */
	    fb_putfield(n, f, sfld);
	    }
	 fb_infoline();
      }

/*
 * Clrpage - clear a page instead of REDRAWing the whole picture
 */

   void fb_clrpage()
      {
         int nc;
	 fb_node *n;
	 fb_field *f;

	 fb_move(3, 64), fb_clrtoeol();
         for(nc = 0; nc < pcur->p_maxedit; nc++){
	    n = pcur->p_nedit[nc];
	    f = n->n_fp;
	    cdb_afld[0] = NULL;
	    fb_putfield(n, f, cdb_afld);
	    }
         fb_infoline();
      }
