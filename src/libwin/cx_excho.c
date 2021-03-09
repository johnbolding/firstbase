/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: cx_excho.c,v 9.1 2001/02/16 19:36:31 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Cx_exchoice_sid[] = "@(#) $Id: cx_excho.c,v 9.1 2001/02/16 19:36:31 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <vdict.h>

/*
 * Cx_exchoice - provide communication routines for dbvedi extended choice type
 */

extern char cdb_EOREC;
extern short int cdb_secure;

/*
 * cx_deposit_exchoices - locate the extended choices and deposit them
 *	to a temp file. this temp file then is the cx_seekfile and run
 *	just like a normal choicefile.
 */

   fb_cx_deposit_exchoices(e)
      fb_exchoice *e;

      {
	 long ixrec, rec;
	 int ifd, ofd, recsiz, row, label, i;
	 char *buf, *fld, dline[1000], tname[FB_MAXNAME];
	 fb_field *f;
         extern char *cdb_tempdir;

         if (!fb_cx_testcontrol())
            return(FB_AOK);
         row = e->ex_firstrow;
	 ixrec = e->ex_ptop;
	 ifd = e->ex_db->ifd;
	 recsiz = e->ex_db->irecsiz;
	 buf = e->ex_db->irec;
	 fld = cdb_bfld;
	 label = 1;
         strcpy(tname, cdb_tempdir);
         fb_assure_slash(tname);
         strcat(tname, "cdbECH_XXXXXX");
         close(mkstemp(tname));
         ofd = open(tname, WRITE);
         if (ofd < 0)
            return(FB_ERROR);
         fb_w_init(1, ofd, 0);
	 for (; row <= e->ex_lastrow; row++, ixrec++, label++){
	    if (ixrec <= 0L || ixrec > e->ex_last)
	       break;
	    if (fb_fgetrec(ixrec, ifd, recsiz, buf, 0) == FB_ERROR){
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
	    sprintf(dline, "%2d", label);
	    for (i = 0; i < 10; i++){
	       f = e->ex_displays[i];
	       if (f == NULL)
	          break;
	       if (FB_OFNUMERIC(f->type) || f->type == FB_DATE || 
	             f->type == FB_FORMULA)
	          fb_formfield(fld, f->fld, f->type, f->size);
	       else
	          fb_pad(fld, f->fld, f->size);
               strcat(dline, " ");
               strcat(dline, fld);
	       }
            strcat(dline, "\n");
            fb_nextwrite(0, dline);
	    }
         fb_wflush(1);
         fb_sync_fd(ofd);
         close(ofd);
         fb_cx_set_seekfile(tname);
         return(FB_AOK);
      }

/*
 * cx_delete_exchoices - delete the exchoice temp file.
 */

   fb_cx_delete_exchoices()
      {
         char fname[FB_MAXNAME];

         if (!fb_cx_testcontrol())
            return(FB_AOK);
         fname[0] = NULL;
         fb_cx_get_seekfile(fname);
         if (fname[0] != NULL)
            unlink(fname);
         fb_cx_set_seekfile(NIL);
         return(FB_AOK);
      }
