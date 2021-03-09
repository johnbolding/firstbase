/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: ex_sel.c,v 9.0 2001/01/09 02:55:59 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Ex_select_sid[] = "@(#) $Id: ex_sel.c,v 9.0 2001/01/09 02:55:59 john Exp $";
#endif

#include <dbve_ext.h>
#include <sys/types.h>
#include <sys/stat.h>

static ex_select_tree();

/*
 * ex_select - interpret the exteneded choice selection made.
 */

   ex_select(e, label, inp)
      fb_exchoice *e;
      char *inp;
      char *label;
      
      {
	 long ixrec, rec;
	 int fd, recsiz, ilabel;
	 char *buf;

         if (e->ex_tree)
            return(ex_select_tree(e, label, inp));

	 ilabel = atoi(label);
	 ixrec = e->ex_ptop + (ilabel - 1);
	 fd = e->ex_db->ifd;
	 recsiz = e->ex_db->irecsiz;
	 buf = e->ex_db->irec;
	 if (ixrec <= 0L || ixrec > e->ex_last)
	    return(FB_ERROR);
	 if (fb_fgetrec(ixrec, fd, recsiz, buf, 0) == FB_ERROR){
	    fb_serror(FB_BAD_INDEX, e->ex_db->dindex, NIL);
	    return(FB_ERROR);
	    }
	 rec = atol((char *) (buf + recsiz - 11));
	 if (fb_getrec(rec, e->ex_db) == FB_ERROR){
	    fb_serror(FB_BAD_DATA, e->ex_db->dbase, NIL);
	    return(FB_ERROR);
	    }
	 strcpy(inp, e->ex_return->fld);
	 fb_trim(fb_rmlead(inp));
	 return(FB_AOK);
      }

/*
 * ex_select_tree - Btree version of ex_select().
 */

   static ex_select_tree(e, label, inp)
      fb_exchoice *e;
      char *inp;
      char *label;
      
      {
	 long srec, rec;
	 int ilabel;
	 char *buf;
         fb_bseq *bs;

         bs = e->ex_db->b_seq;
	 ilabel = atoi(label);
         st = ex_incr_tree(e, ilabel - 1, 0);
         if (st == FB_ERROR)
            return(st);
         rec = fb_key_eval(bs);
	 if (fb_getrec(rec, e->ex_db) == FB_ERROR){
	    fb_serror(FB_BAD_DATA, e->ex_db->dbase, NIL);
	    return(FB_ERROR);
	    }
	 strcpy(inp, e->ex_return->fld);
	 fb_trim(fb_rmlead(inp));
	 return(FB_AOK);
      }
