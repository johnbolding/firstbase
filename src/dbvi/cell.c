/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: cell.c,v 9.0 2001/01/09 02:56:04 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Cell_sid[] = "@(#) $Id: cell.c,v 9.0 2001/01/09 02:56:04 john Exp $";
#endif

#include <dbvi_ext.h>

/*
 * fb_cell record routines
 */

/*
 * makecrec - create a fb_cell record (crec), zero it out, return it.
 */

   crec *makecrec()
      {
         crec *c;
	 char *p;
	 int i;
	 fb_field *fp;
	 
	 c = (crec *) fb_malloc(sizeof(crec));
	 c->c_cell = (char **) fb_malloc(ncolumns * sizeof(char *));
	 c->c_buf = (char *) fb_malloc(c_bufsize);
	 c->c_next = NULL;
	 c->c_prev = NULL;
	 /* initialize the fields */
	 p = c->c_buf;
	 for (i = 0; i < ncolumns; i++){
	    *p = NULL;
	    c->c_cell[i] = p;
	    fp = gcolumn[i]->p_field;
	    p = p + fp->size + 1;
	    }
	 return(c);
      }

/*
 * freecrec - fb_free a fb_cell record.
 */

   freecrec(c)
      crec *c;

      {
	 fb_free(c->c_buf);
	 fb_free(c->c_cell);
	 fb_free(c);
      }

/*
 * freeclist - fb_free a list of fb_cell records.
 */

   freeclist(c)
      crec *c;

      {
         crec *nc;
	 
	 for (; c != NULL; c = nc){
	    nc = c->c_next;
	    freecrec(c);
	    }
      }

/*
 * insert_crec - insert fb_cell record c in front of crec ac.
 *	if ac == NULL, then append beyond tail.
 */

   insert_crec(c, ac)
      crec *c, *ac;
      
      {
         if (ac == NULL){		/* append beyond tail */
	    if (crec_mtail == NULL)
	       crec_mtail = crec_mhead = c;
	    else{
	       crec_mtail->c_next = c;
	       c->c_prev = crec_mtail;
	       c->c_next = NULL;
	       crec_mtail = c;
	       }
	    }
	 else if (ac == crec_mhead){	/* insert before head
	    c->c_prev = NULL;
	    c->c_next = crec_mhead;
	    crec_mhead->c_prev = c;
	    crec_mhead = c;
	    }
	 else{				/* normal case */
	    c->c_next = ac;
	    c->c_prev = ac->c_prev;
	    ac->c_prev->c_next = c;
	    ac->c_prev = c;
	    }
      }

/*
 * emptyclist - generate an one element list -- assign to memory
 */

   emptyclist()

      {
         crec *c, *makecrec();

	 crec_mhead = crec_mtail = NULL;	 
	 crec_phead = crec_ptail = NULL;	 
	 c = makecrec();
	 insert_crec(c, NULL);
	 col_current = col_mhead;
	 crec_current = crec_mhead;
	 set_screen();
      }

/*
 * copyclist - generate a copy of the list passed in -- pass new one back.
 */

   crec *copyclist(cf, count)
      crec *cf;
      int count;

      {
         crec *ct, *ct_head = NULL, *ct_tail = NULL, *makecrec();
	 char *p, *q;
	 int j;

         for (; cf != NULL; cf = cf->c_next){
	    ct = makecrec();
	    p = cf->c_buf;
	    q = ct->c_buf;
	    /* do the copy -- just the buffer needs copying. */
	    for (j = c_bufsize; j > 0; j--)
	       *q++ = *p++;
	    if (ct_head == NULL){
	       ct_head = ct;
	       ct_tail = ct;
	       ct->c_prev = ct->c_next = NULL;
	       }
	    else{
	       ct_tail->c_next = ct;
	       ct->c_prev = ct_tail;
	       ct->c_next = NULL;
	       ct_tail = ct;
	       }
	    if (count != -1){
	       if (--count == 0)
	          break;
	       }
	    }
	 return(ct_head);
      }
