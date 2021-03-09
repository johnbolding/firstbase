/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: column.c,v 9.0 2001/01/09 02:56:04 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Column_sid[] = "@(#) $Id: column.c,v 9.0 2001/01/09 02:56:04 john Exp $";
#endif

#include <dbvi_ext.h>

/*
 * column routines
 */

/*
 * makecolumn - create a column, zero it out, return it.
 */

   column *makecolumn()
      {
         column *p;
	 
	 p = (column *) fb_malloc(sizeof(column));
	 p->p_field = NULL;
	 p->p_ioloc = 0;
	 p->p_label = NULL;
	 p->p_width = 0;
	 p->p_array = 0;
	 return(p);
      }

/*
 * initcolumn - initialize the column structures.
 */

   initcolumn()
      {
         int i;

         for (i = 0; i < ncolumns; i++)
	    c_bufsize += (gcolumn[i]->p_field->size + 1);
	 c_bufsize += ncolumns;		/* defensive paranoia - extra space */
         col_mhead = gcolumn[0];
	 col_mtail = gcolumn[ncolumns - 1];
         col_phead = col_ptail  =  col_current = NULL;
         crec_mhead = crec_mtail = NULL;
         crec_phead = crec_ptail = crec_current = NULL;
      }

/*
 * readcolumn - read in all of the db records -- load columns and fb_cell records
 */

   readcolumn()
      {
         int loadcolumn();

	 fb_foreach(cdb_db, loadcolumn);
	 col_current = col_mhead;
	 crec_current = crec_mhead;
	 set_screen();
	 clearlink();
      }

/*
 * loadcolumn - load a single record into the a crec struct, and link
 *    after crec_mtail.
 */

   loadcolumn(d)
      fb_database *d;

      {
         crec *c, *makecrec();
	 char *p, *q;
	 fb_field *fp;
	 int i, len, tlen;
	 
	 c = makecrec();
	 p = c->c_buf;
	 for (i = 0; i < ncolumns; i++){
	    c->c_cell[i] = p;
	    fp = gcolumn[i]->p_field;
	    len = 0;
	    tlen = fp->size + 1;
	    if (fp->type == FB_FORMULA){
	       fb_fetch(fp, cdb_afld, d);
	       q = cdb_afld;
	       }
	    else
	       q = fp->fld;
	    for (; ; q++){
	       *p++ = *q;
	       len++;
	       if (*q == NULL)
		  break;
	       }
	    for (; len < tlen; len++)
	       *p++ = CHAR_BLANK;
	    }
	 insert_crec(c, NULL);
      }

   tracecolumn()
      {
         int i;

	 fb_move(4,1); fb_refresh();
	 printf("....PILLAR ncolumns=%d,c_bufsiz is %d\n",ncolumns,c_bufsize);
         for (i = 0; i < ncolumns; i++)
	    printf("...%d ... %s \n", i, gcolumn[i]->p_field->id);
	 FB_PAUSE();
      }

   tracecrec()
      {
         int i, j;
	 crec *c;
	 
         for (i = 0, c = crec_mhead; c; c = c->c_next, i++){
	    for (j = 0; j < ncolumns; j++)
	       printf("%d %d - %s\n", i, j, c->c_cell[j]);
	    }
	 FB_PAUSE();
      }
