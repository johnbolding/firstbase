/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dballoc.c,v 9.0 2001/01/09 02:56:45 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dballoc_sid[] = "@(#) $Id: dballoc.c,v 9.0 2001/01/09 02:56:45 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern char CHAR_O;

/*
 *  dballoc - create a db space and null out all the nasties.
 */

   fb_database *fb_dballoc()
      
      {
         fb_database *db;
	 
         db = (fb_database *) fb_malloc(sizeof(fb_database));
	 db->dbase = 	NULL;
	 db->dindex = 	NULL;
	 db->dmap = 	NULL;
	 db->ddict = 	NULL;
	 db->idict = 	NULL;
	 db->sdict = 	NULL;
	 db->dlog = 	NULL;
	 db->reccnt = 	0L;
	 db->dirty = 	CHAR_0;
	 db->delcnt = 	0L;
	 db->rec = 	0L;
	 db->recsiz = 	0;
	 db->nfields = 	0;
	 db->fd = 	-1;
	 db->ifields = 	0;
	 db->ifd = 	-1;
	 db->ihfd = 	-1;
	 db->logfd = 	-1;
	 db->irecsiz = 	0;
	 db->bsmax = 	0L;
	 db->bsend = 	0L;
	 db->bsrec = 	0L;
	 db->mfd = 	-1;
	 db->kp = 	NULL;
	 db->ip = 	NULL;
	 db->orec = 	NULL;
	 db->arec = 	NULL;
	 db->irec = 	NULL;
	 db->afld = 	NULL;
	 db->bfld = 	NULL;
	 db->refcnt = 	0;

         db->b_tree =	0;
         db->b_seq =	NULL;
         db->b_idx =	NULL;
         db->b_seqtmp =	NULL;
         db->b_idxtmp =	NULL;

         db->b_autoindex = NULL;
         db->b_maxauto = 0;
         db->b_curauto = -1;

         db->fixedwidth = 0;
         db->b_sid = -1;

         db->inside_bulkrec = 0;
         db->sequence = 0;

	 return(db);
      }

   fb_field *fb_makefield()
      {
         fb_field *f;

         f = (fb_field *) fb_malloc(sizeof(fb_field));
         f->id = NULL;
         f->type = NULL;
         f->size = -1;
         f->comloc = FB_BLANK;
         f->lock = CHAR_n;
         f->choiceflag = FB_BLANK;
         f->fld = NIL;
         f->idefault = NULL;
         f->comment = NULL;
         f->help = NULL;
         f->prev = NULL;
         f->range = NULL;
         f->a_template = NULL;
         f->incr = 0;
         f->aid = NULL;
         f->dflink = NULL;
         f->xflink = NULL;
         f->mode = NULL;
         f->f_macro = NULL;
         f->f_prec = 0;
         return(f);
      }
