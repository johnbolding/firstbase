/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: totpr.c,v 9.0 2001/01/09 02:55:43 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Totpr_sid[] = "@(#) $Id: totpr.c,v 9.0 2001/01/09 02:55:43 john Exp $";
#endif

/*
 *  totpr.c - library used for dbpgen to do breaklines/totalslines
 */

#include <fb.h>
#include <fb_ext.h>
#include <pgen.h>

extern fb_database *hp;

extern char *cdb_T_TOTAL;
extern char *cdb_T_BREAK;
extern char *cdb_T_SBREAK;

/* 
 *  inittotals - initialize the totals and breaks for a printout 
 */
 
  inittotals(pp, tptr, bptr)
      fb_field **pp;
      struct totlist **tptr;
      struct brklist **bptr;
      
      {
         char fname[FB_MAXNAME], word[FB_MAXLINE], line[FB_MAXLINE];
	 FILE *fs, *fb_mustfopen();
	 struct totlist *tp, *tq, *fb_copylist();
	 struct brklist *bp, *bq;
	 int i, j, bc;
	 
	 bc = 0;
	 tp = NULL; tq = NULL;
	 bp = NULL; bq = NULL;
         *tptr = NULL;
	 *bptr = NULL;
	 sprintf(fname, "%sp", hp->idict);
	 fs = fb_mustfopen(fname, "r");
	 while (fgets(line, FB_MAXLINE, fs) != NULL){
	    if (line[0] == '%' || (*pp)->size == 0)
	       break;		/* end of list of fields in idictp file */
	    for (i = 1; i != 0 && ((i = fb_getword(line, i, word)) != 0); ){
	       if (strcmp(cdb_T_TOTAL, word) == 0){
	          tq = (struct totlist *) fb_malloc(sizeof(struct totlist));
		  tq->tfld = *pp;
		  tq->tval = 0.0;
		  tq->tlink = NULL;
		  if (tp == NULL){
		     tp = tq;
		     *tptr = tq;
		     }
		  else{
		     tp->tlink = tq;
		     tp = tq;
		     }
		  }
	       else if (equal(cdb_T_BREAK, word) || equal(cdb_T_SBREAK, word)){
	          bq = (struct brklist *) fb_malloc(sizeof(struct brklist));
		  bq->bfld = *pp;
		  bq->bval = NULL;
		  bq->blink = NULL;
		  bq->bcount = 0;
	          if (equal(cdb_T_BREAK, word))
		     bq->simple_flag = 0;	/* full break */
		  else
		     bq->simple_flag = 1;	/* simple break */
		  if ((i = fb_getword(line, i, word)) != 0){
		     if ((j  = atoi(word)) > 0)
		        bc = j;
		     }
		  else 
		     j = bc++;
		  bq->blevel = j;
		  if (bp == NULL){
		     bp = bq;
		     *bptr = bq;
		     }
		  else{
		     bp->blink = bq;
		     bp = bq;
		     }
	          }
	       }
	    pp++;
	    }
	 fclose(fs);
	 for (bq = *bptr; bq != NULL; bq = bq->blink)
	    bq->btlist = fb_copylist(*tptr);
      }

/* 
 *  copylist - copy a totals list - return pointer to first one
 *     this needs doing since some C's don't do structure assignment
 */
 
   struct totlist *fb_copylist(t)
      struct totlist *t;
      
      {
         struct totlist *p, *q, *r;
	 
	 r = NULL;
	 for (p = NULL, q = NULL; t != NULL; t = t->tlink){
	    q = (struct totlist *) fb_malloc(sizeof(struct totlist));
	    q->tfld = t->tfld;
	    q->tval = 0.0;
	    q->tlink = NULL;
	    if (p != NULL)
	       p->tlink = q;
	    else
	       r = q;
	    p = q;
	    }
	 return(r);
      }

/* 
 *  ftrace - trace the fieldlists t and b.
 */
 
   ftrace(t, b, lc)
      struct totlist *t;
      struct brklist *b;
      int lc;
   
      {
         fb_printw("\n");
	 bump(&lc);
	 for (; t != NULL; t = t->tlink, bump(&lc))
	    fb_printw("total %s\n", (t->tfld)->id);
	 fb_printw("\n");
	 bump(&lc);
	 for (; b != NULL; b = b->blink, bump(&lc))
	    fb_printw("break on %s (level: %d)\n", (b->bfld)->id, b->blevel);
	 fb_printw("\n");
      }
