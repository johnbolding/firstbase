/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: visual.c,v 9.0 2001/01/09 02:56:42 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Visual_sid[] = "@(#) $Id: visual.c,v 9.0 2001/01/09 02:56:42 john Exp $";
#endif

/*
 *  visual.c - library for visual interface - to vdict functions
 *	used by dbvedit and dbvemit
 */

#include <dbve_ext.h>

static char *MSG1 = "Visual/Filter what field? ";
static char *MSG2 = "Visual/Filter [reading]...";

/* 
 *  visual - pop a fb_field into visual mode 
 *     if vis = 1-> visual, else filter 
 *     if readonly = 1, then readonly. 
 *     if readonly = 0, then readwrite.
 *
 *     returns FB_ABORT on unchaged fields, FB_AOK on changed ones.
 */
 
   int fb_visual(p, readonly, vis)
      char *p;
      int vis, readonly;
      
      {
	 int st = FB_ABORT, num;
	 fb_field *f = NULL;
	 fb_node *n;
	 
	 if (p[1] == NULL || !(isdigit(p[1]))){
	    fb_fmessage(MSG1);
	    st = fb_input(cdb_t_lines, 27, 3, 0, FB_INTEGER, (char *) &num, 
	          FB_ECHO, FB_OKEND, FB_CONFIRM);
	    if (st == FB_END || st == FB_ABORT)
	       return(st);
	    }
	 else
	    num = atoi(p+1);
	 num--;
	 if (num >= 0 && num < pcur->p_maxedit){
	    n = pcur->p_nedit[num];
	    f = n->n_fp;
            if (n->n_readonly)
               readonly = 1;
	    }
	 if (num < 0 || num >= pcur->p_maxedit || f->size <= FB_LONGFIELD){
	    fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
	    return(FB_ERROR);;
	    }
	 if (!vis)
	    st = fb_vedit(readonly, vis, f);		/* filter */
	 else if (vis &&
	       (f->type==FB_ALPHA || f->type==FB_DOCUMENT || f->type==FB_UPPERCASE) &&
	       f->size > FB_SCREENFIELD){
	    fb_fmessage(MSG2);
	    st = fb_vedit(readonly, vis, f);	/* limit allowed visuals */
	    }
	 else  
	    fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
         return(st);
      }
