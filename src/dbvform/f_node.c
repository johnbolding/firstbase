/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: f_node.c,v 9.0 2001/01/09 02:56:03 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)f_node.c	8.1 8/19/93 FB";
#endif

#include <dbvf_ext.h>

extern short cdb_datedisplay;

/*
 * fb_node routines - no limitations on screen location here.
 */

/*
 * makenode - create a fb_node, zero it out, return it.
 */

   static fb_node * fb_makenode()
      {
         fb_node *n;
	 
	 n = (fb_node *) fb_malloc(sizeof(fb_node));
	 n->n_text = NULL;
	 n->n_fp = NULL;
	 n->n_sub1 = -1;
	 n->n_sub2 = -1;
	 n->n_row = -1;
	 n->n_col = -1;
	 n->n_len = -1;
	 n->n_next = NULL;
	 n->n_prev = NULL;
	 n->n_reverse = 1;
	 n->n_readonly = 0;
	 n->n_type = -1;
	 return(n);
      }

/*
 * makepage - create a fb_page, zero it out, return it.
 */

   fb_page * fb_makepage()
      {
         fb_page *p;

	 p = (fb_page *) fb_malloc(sizeof(fb_page));
	 p->p_num = 0;
	 p->p_nhead = NULL;
	 p->p_ntail = NULL;
	 p->p_next = NULL;
	 p->p_prev = NULL;
	 p->p_nedit = NULL;
	 p->p_maxedit = 0;
	 p->p_help = NULL;
	 return(p);
      }

/*
 * install - install a fb_node. use its type to determine fp/text/sub.
 */

   fb_install(type, pagenum, subject, row, col, sub1, sub2, rev)
      int type, pagenum, row, col, sub1, sub2, rev;
      char *subject;
      
      {
         fb_node *n, *fb_makenode();
	 fb_field *fb_findfield();
	 fb_page *p, *fb_makepage();

	 if (sub2 > 0 && sub1 > sub2 && (type == 3 || type == 4))
	    return(FB_ERROR);
	 n = fb_makenode();
	 /* store all fb_node values */
	 if (row < 0){
	    n->n_readonly = 1;
	    row = -row;
	    }
	 if (col < 0){
	    n->n_readonly = 1;
	    col = -col;
	    }
	 if (row <= 0 || row > p_lines || col <= 0 || col > p_cols)
	    return(FB_ERROR);
	 n->n_row = row;
	 n->n_col = col;
	 n->n_sub1 = sub1;
	 n->n_sub2 = sub2;
	 n->n_reverse = rev;
	 n->n_type = type;
	 fb_mkstr(&n->n_text, subject);
	 if (type > 0){	
	    /* fb_field display - fb_field must exist */
	    if ((n->n_fp = fb_findfield(subject, cdb_db)) == NULL)
	       return(FB_ERROR);
	    }
	 if (n->n_fp != NULL){
	    n->n_len = n->n_fp->size;
	    if (type == T_SUBELEMENT_R || type == T_SUBELEMENT_N){
	       n->n_len = sub2;
	       n->n_sub2 = sub2 = 0;
	       }
	    if (sub1 > 0 && sub2 > 0){
	       if (!(FB_OFALPHA(n->n_fp->type)) && 
	             n->n_fp->type != FB_CHOICE &&
	             n->n_fp->type != FB_SILENTCHOICE &&
	             n->n_fp->type != FB_EXCHOICE)
	          return(FB_ERROR);
	       if (sub2 > n->n_fp->size)
	          return(FB_ERROR);
	       n->n_len = (sub2 - sub1) + 1;
	       }
	    else if (n->n_len > p_cols){
	       if (sub1 > 0)
	          n->n_len = sub1;
	       else
	          n->n_len = 1;
	       }
	    else if (n->n_fp->type ==FB_DATE)
	       n->n_len = cdb_datedisplay;
	    if (n->n_col + n->n_len > p_cols)
	       return(FB_ERROR);
	    if ((type == T_SUBELEMENT_R || type == T_SUBELEMENT_N) && 
	          (n->n_fp->type != FB_CHOICE) && (n->n_fp->type != FB_EXCHOICE) && 
	          (n->n_fp->type != FB_SILENTCHOICE) &&
		  !(FB_OFALPHA(n->n_fp->type)))
	       return(FB_ERROR);
	    }
	 if (phead == NULL){			/* init fb_page list */
	    p = phead = ptail = fb_makepage();
	    p->p_num = pagenum;
	    p->p_nhead = p->p_ntail = NULL;
	    p->p_next = NULL;
	    p->p_prev = NULL;
	    }
	 if (pagenum != ptail->p_num){		/* insert new fb_page */
	    p = fb_makepage();
	    p->p_num = pagenum;
	    p->p_nhead = p->p_ntail = NULL;
	    p->p_next = NULL;
	    p->p_prev = ptail;
	    ptail->p_next = p;
	    ptail = p;
	    }
	 /* now insert fb_node on last fb_page */
	 p = ptail;
	 if (p->p_nhead == NULL){		/* init fb_node list */
	    p->p_nhead = p->p_ntail = n;
	    n->n_next = NULL;
	    n->n_prev = NULL;
	    }
	 else {
	    /* insert as tail of fb_page p`s list */
	    n->n_next = NULL;
	    n->n_prev = p->p_ntail;
	    n->n_prev->n_next = n;
	    p->p_ntail = n;
	    }
	 return(FB_AOK);
      }

/*
 * makenedit - loop over the pages, creating p_nedit structures.
 *	       idea is to have p_nedit[0 ... p_maxedit] fb_node pointers
 *	       on each fb_page. p_nedit[p_maxedit] == NULL;
 */

   fb_makenedit()
      {
         fb_page *p;
	 fb_node *n;
	 fb_node **np;
	 int nc;
	 
	 for (p = phead; p; p = p->p_next){
	    for (nc = 0, n = p->p_nhead; n; n = n->n_next)
	       if (n->n_fp != NULL)
	          nc++;
	    p->p_maxedit = nc;
	    }
	 for (p = phead; p; p = p->p_next){
	    if (p->p_maxedit == 0)
	       continue;
	    p->p_nedit = (fb_node **) fb_malloc((p->p_maxedit+1)*(sizeof(fb_node *)));
	    np = p->p_nedit;
	    for (n = p->p_nhead; n; n = n->n_next)
	       if (n->n_fp != NULL)
	          *np++ = n;
	    *np = NULL;
	    }
      }

#if DEBUG
   trace()
      {
         fb_page *p;
	 fb_node *n;
	 int pc, nc;
	 
	 for (pc = 1, p = phead; p; p = p->p_next, pc++){
	    fprintf(stderr, "Page %d\n", pc);
	    for (nc = 1, n = p->p_nhead; n; n = n->n_next, nc++){
	       fprintf(stderr, "   Node %d\n", nc);
	       fprintf(stderr, "      text: %s \n", n->n_text);
	       if (n->n_fp != NULL){
	          fprintf(stderr, "      field name: %s", n->n_fp->id);
	          fprintf(stderr, "[%d,%d]\n", n->n_sub1, n->n_sub2);
		  }
	       fprintf(stderr, "      at: %d, %d\n", n->n_row, n->n_col);
	       }
	    }
      }
#endif /* DEBUG */
