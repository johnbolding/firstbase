/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: ex_disp.c,v 9.0 2001/01/09 02:55:58 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Ex_display_sid[] = "@(#) $Id: ex_disp.c,v 9.0 2001/01/09 02:55:58 john Exp $";
#endif

#include <dbve_ext.h>
#include <sys/types.h>
#include <sys/stat.h>

extern short int cdb_ex_reverse;
extern short int cdb_secure;
extern char cdb_EOREC;

static int ex_display_tree();

/*
 * ex_display - do the display for the extended choice/help.
 */

   ex_display(e)
      fb_exchoice *e;
      
      {
	 long ixrec, rec;
	 int fd, recsiz, row, label, i;
	 char *buf, *fld, *nfld;
	 fb_field *f;

         if (e->ex_tree)
            return(ex_display_tree(e));

	 row = e->ex_firstrow ;
         fb_move(row, 1); fb_clrtobot();
	 ixrec = e->ex_ptop;
	 fd = e->ex_db->ifd;
	 recsiz = e->ex_db->irecsiz;
	 buf = e->ex_db->irec;
	 fld = e->ex_db->bfld;
         nfld = e->ex_db->afld;
	 label = 1;
	 for (; row <= e->ex_lastrow; row++, ixrec++, label++){
	    if (ixrec <= 0L || ixrec > e->ex_last)
	       break;
	    if (fb_fgetrec(ixrec, fd, recsiz, buf, 0) == FB_ERROR){
	       fb_serror(FB_BAD_INDEX, e->ex_db->dindex, NIL);
	       return(FB_ERROR);
	       }
	    if (buf[recsiz - 2] == cdb_EOREC)
	       continue;			/* ignore this find */
	    rec = atol((char *) (buf + recsiz - 11));
	    if (fb_getrec(rec, e->ex_db) == FB_ERROR){
	       fb_serror(FB_BAD_DATA, e->ex_db->dbase, NIL);
	       return(FB_ERROR);
	       }
            if (cdb_secure && fb_record_permission(e->ex_db, READ) == FB_ERROR)
               continue;
	    
	    /* display rec here */
	    fb_move(row, 1);
	    sprintf(fld, "%2d", label);
	    if (cdb_ex_reverse)
	       fb_stand(fld);
	    else
	       fb_prints(fld);
	    for (i = 0; i < 10; i++){
	       f = e->ex_displays[i];
	       if (f == NULL)
	          break;
               strcpy(nfld, f->fld);
	       if (FB_OFNUMERIC(f->type) || f->type ==FB_DATE || 
	             f->type == FB_FORMULA)
	          fb_formfield(fld, nfld, f->type, f->size);
	       else
	          fb_pad(fld, nfld, f->size);
	       fb_printw("  %s", fld);
	       }
	    }
	 return(FB_AOK);
      }

/*
 * ex_display_tree - the btree version of ex_display.
 */

   static ex_display_tree(e)
      fb_exchoice *e;
      
      {
	 long srec, rec;
	 int row, label, i;
	 char *buf, *fld, *nfld;
	 fb_field *f;
         fb_bseq *bs;

         bs = e->ex_db->b_seq;
	 row = e->ex_firstrow ;
         fb_move(row, 1); fb_clrtobot();
	 srec = e->ex_ptop;
         bs->bs_curkey = e->ex_ptop_curkey;
         fb_seq_getrec(srec, bs);
	 fld = e->ex_db->bfld;
         nfld = e->ex_db->afld;
	 label = 1;
         /*
          * sequence through btree bseq nodes, displaying what is needed.
          */
	 for (; row <= e->ex_lastrow; bs->bs_curkey++){
            if (srec > e->ex_last ||
                  srec == e->ex_last && bs->bs_curkey > e->ex_last_curkey){
               break;
               }
            if (bs->bs_curkey > 3){
               /* read next node */
               srec = bs->bs_next;
               bs->bs_curkey = 1;
               if (srec == 0){
                  srec = e->ex_first;
                  bs->bs_curkey = e->ex_first_curkey;
                  break;
                  }
               if (fb_seq_getrec(srec, bs) == FB_ERROR)
                  return(FB_ERROR);
               }
            rec = fb_key_eval(bs);
            if (rec <= 0)
               continue;

	    if (fb_getrec(rec, e->ex_db) == FB_ERROR){
	       fb_serror(FB_BAD_DATA, e->ex_db->dbase, NIL);
	       return(FB_ERROR);
	       }
            if (cdb_secure && fb_record_permission(e->ex_db, READ) == FB_ERROR)
               continue;
	    
	    /* display rec here */
	    fb_move(row++, 1);
	    sprintf(fld, "%2d", label++);
	    if (cdb_ex_reverse)
	       fb_stand(fld);
	    else
	       fb_prints(fld);
	    for (i = 0; i < 10; i++){
	       f = e->ex_displays[i];
	       if (f == NULL)
	          break;
               strcpy(nfld, f->fld);
	       if (FB_OFNUMERIC(f->type) || f->type ==FB_DATE || 
	             f->type == FB_FORMULA)
	          fb_formfield(fld, nfld, f->type, f->size);
	       else
	          fb_pad(fld, nfld, f->size);
	       fb_printw("  %s", fld);
	       }
	    }
	 return(FB_AOK);
      }

