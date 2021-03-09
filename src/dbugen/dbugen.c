/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbugen.c,v 9.1 2001/02/16 19:48:39 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "%W% %G% FB";
#endif

/*
 *  dbugen.c - update generator. provides mechanism for doing
 *    formula calculations of macros substitution on fields
 *    across an entire fb_database (as defined by an index, of course).
 */

#include <fb.h>
#include <fb_vars.h>
#include <dbd.h>

extern short int cdb_secure;

static fb_upd **itemlist = NULL;	/* for list of items */
static int lastitem = 0;		/* last item in itemlist */
fb_database *hp;

char *T_DELETE = "$DELETE";

static ugen();
static one_ugen();
static update();
static utrace();
static usage();

long n = 0L, atol();

/* 
 *  dbugen - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         char fname[FB_MAXNAME];
	 int i;

         fb_getargs(argc, argv, FB_ALLOCDB);
	 hp = cdb_db;
	 fb_opendb(cdb_db, READWRITE, FB_ALLINDEX, FB_MUST_INDEX);
         fb_scrhdr(cdb_db, "Parsing"); fb_infoline();
	 sprintf(fname, "%su", hp->idict);
         for (i = 0; i < cdb_db->nfields; i++)
            fb_nounders(cdb_db->kp[i]);
	 itemlist =
	     (fb_upd **) fb_malloc((hp->nfields+1) * sizeof(fb_upd *));
	 			/* allocate enough ptrs for all fields */
	 if ((lastitem = read_idictu(fname, itemlist)) == FB_ERROR)
	    fb_xerror(FB_BAD_DICT, fname, NIL);
	 if (!cdb_batchmode && !cdb_yesmode){
            utrace();
	    if (fb_mustbe('y',"If accurate, enter 'y' to continue: ",
	          cdb_t_lines, 1) == FB_ERROR)
               fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    fb_move(4, 1), fb_clrtobot(), fb_infoline();
	    }
         if (ugen()==FB_ERROR)
            fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
         fb_puthead(cdb_db->fd, cdb_db->reccnt, cdb_db->delcnt);
	 fb_closedb(cdb_db);
         fb_ender();
      }
      
/* 
 *  ugen - generate converted data base  according to list in pp 
 */
 
   static ugen()
      
      {

         int max;

         if (!cdb_batchmode){
            fb_scrstat("Updating");
	    FB_XWAIT();
	    fb_gcounter(n);
	    }
         fb_forxeach(hp, one_ugen);
         return(FB_AOK);
      }

   static one_ugen(hp)
      fb_database *hp;

      {
         long rec;

         fb_set_autoindex(hp);
         rec = hp->rec;
         ++n;
         if (!cdb_batchmode)
            fb_gcounter(n);
         if (fb_lock(rec, hp, FB_NOWAIT) != FB_AOK) /* NOWAIT if locked */
            return(FB_AOK);
         update();
         fb_lock_head(hp);
         fb_setdirty(hp, 1);
         if (fb_putrec(rec, hp) == FB_ERROR)
            fb_xerror(FB_FATAL_PUTREC, hp->dbase, (char *) &rec);
         fb_setdirty(hp, 0);
	 if (fb_put_autoindex(hp) == FB_ERROR)
	    fb_serror(FB_BAD_INDEX, hp->dbase, NIL);
	 fb_allsync(hp);
         fb_unlock_head(hp);
         fb_unlock(rec, hp);
         return(FB_AOK);
      }

/* 
 *  update - update the buffer according to the itemlist of upd structs 
 */
 
   static update()

      {
         fb_upd *up;
	 int i;
	 
	 for (i = 1; i <= lastitem; i++){
	    up = itemlist[i];
	    cdb_bfld[0] = NULL;
	    fb_trim(up->fv);		/* not needed i think ??? */
	    if (up->fv[0] == NULL)
	       ;
	    else if (equal(up->fv, T_DELETE)){
               cdb_db->kp[cdb_db->nfields]->fld[0] = CHAR_STAR;
	       cdb_db->delcnt++;
	       continue;
	       }
	    else if (!(FB_OFNUMERIC(up->fp->type))){
	       if (up->fv != NULL)
	          strcpy(cdb_bfld, up->fv);
	       }
	    else{			/* numeric type can be formulated */
	       if (fb_getformula(up->fp, up->fv, cdb_bfld, 0, hp) == FB_ERROR)
	          fb_xerror(FB_BAD_DATA, SYSMSG[S_BAD_FORMULA], NIL);
	       }
	    fb_store(up->fp, cdb_bfld, hp);
	    }
      }

/*  
 *  utrace - trace an itemlist of upd structures to the screen 
 */
 
   static utrace()
      {
      
	 int i;
	 fb_upd *up;
	 int row;
	    
	 for (i = 1; i <= lastitem; ){
	    fb_move(4, 1);
	    for (row = 4; i <= lastitem && row <= 22; row += 2){
	       up = itemlist[i++];
	       fb_printw("%10s %c %6d ---> ", up->fp->id, up->fp->type, 
		     up->fp->size);
	       if (up->fv != NULL)
		  fb_printw(FB_FSTRING, up->fv);
	       else
	          fb_printw("<EMPTY>");
	       fb_printw("\n");
	       }
	    }
      }
			
/* 
 *  usage message 
 */
 
   static usage()
      {
         fb_xerror(FB_MESSAGE, "usage: dbugen [-d dbase] [-i index] [-b] [-y]",
            NIL);
      }
