/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: choicenod.c,v 9.0 2001/01/09 02:56:39 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Choicenode_sid[] = "@(#) $Id: choicenod.c,v 9.0 2001/01/09 02:56:39 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <vdict.h>

static char *PREVIOUS = "Previous";
static char *CHOICES = "Choices";
static char *EX_HELP = "$HELP";
fb_cnode *chead, *cptr;
fb_cnode *com_head, *com_ptr;

/*
 * choicenode - various routines to provide the multiple layers of choices
 *	as well as to handle link lists of comments.
 */

   void fb_initcnode()
      {
         chead = NULL;
	 cptr = NULL;
      }

   void fb_addcnode(val)
      char *val;

      {
         fb_cnode *c;

         c = fb_makecnode();
	 fb_mkstr(&(c->c_meaning), val);
	 if (chead == NULL)
	    chead = cptr = c;
	 else{
	    cptr->c_next = c;
	    cptr = c;
	    }
      }

   void fb_freecnode()
      {
         fb_cnode *nptr;

         for (cptr = chead; cptr != NULL; cptr = nptr){
	    fb_free(cptr->c_meaning);
	    nptr = cptr->c_next;
	    fb_free((char *) cptr);
	    }
	 cptr = chead = NULL;
      }

   void fb_freeccom()
      {
         fb_cnode *nptr;

         for (com_ptr = com_head; com_ptr != NULL; com_ptr = nptr){
	    fb_free(com_ptr->c_meaning);
	    nptr = com_ptr->c_next;
	    }
	 com_ptr = com_head = NULL;
      }

   fb_cnode *fb_makecnode()
      {
         fb_cnode *c;
	 
	 c = (fb_cnode *) fb_malloc(sizeof(fb_cnode));
	 c->c_label = NULL;
	 c->c_meaning = NULL;
	 c->c_next = NULL;
	 c->c_row = 0;
	 c->c_col = 0;
	 return(c);
      }

/*
 * showcnode - show the comment nodes at a suggested column
 */

   void fb_showcnode(sug_col)
      int sug_col;
         {
	    int col, cwidth = 0, maxlen, row;
	    fb_cnode *c;
	    
	    if (chead == NULL)
	       return;
	    for (c = chead; c != NULL; c = c->c_next)
	       cwidth = MAX(cwidth, strlen(c->c_meaning));
	    cwidth = MAX(cwidth, 8); 	/* 8 is strlen of 'Previous' */
	    col = cdb_t_cols - cwidth - 1;
	    if (col < sug_col && cwidth > 20)
	       col = cdb_t_cols - 20 - 1;	/* force cwidth of 20 */
	    fb_move(5,col); fb_stand(PREVIOUS);
	    fb_move(6,col); fb_stand(CHOICES);
	    maxlen = cdb_t_cols - col;
	    for (row = 8, c = chead; c != NULL; c = c->c_next, row++){
	       strcpy(cdb_bfld, c->c_meaning);
	       cdb_bfld[maxlen] = NULL;
	       fb_move(row, col); fb_clrtoeol();
	       fb_stand(cdb_bfld);
	       }
	    fb_refresh();
	 }

/*
 * initcomment - initialize the comments from a choicefile. deposit help.
 */

   void fb_initcomment(cfs, helpfile)
      FILE *cfs;
      char *helpfile;
      
      {
         char line[FB_MAXLINE], word[FB_MAXLINE], hdir[FB_MAXNAME];
	 int j;

         com_head = NULL;
	 com_ptr = NULL;
	 helpfile[0] = NULL;
	 for (; fgets(line, FB_MAXLINE, cfs) != NULL; ){
	    if (line[0] == '"')
	       fb_addcomment(line);
	    else if (line[0] == '$'){
	       j = fb_getword(line, 1, word);
	       if (equal(word, EX_HELP)){
	          fb_getword(line, j, word);
                  fb_dirname(hdir, cdb_db->ddict);
                  if (word[0] == CHAR_SLASH || 
                        word[0] == CHAR_DOT || hdir[0] == NULL)
                     strcpy(helpfile, word);
                  else{
                     strcpy(helpfile, hdir);
                     strcat(helpfile, word);
                     }
		  }
	       }
	    }
	 rewind(cfs);
      }

/*
 * addcomment - add a comment line as defined by val into the link list.
 */

   void fb_addcomment(val)
      char *val;

      {
         fb_cnode *c;
	 char subject[FB_MAXLINE];
	 int row, col;

         if (sscanf(val, "\"%[^\"]\":%d,%d",subject,&row,&col) == 3){
	    c = fb_makecnode();
	    fb_mkstr(&(c->c_meaning), subject);
	    c->c_row = row;
	    c->c_col = col;
	    if (com_head == NULL)
	       com_head = com_ptr = c;
	    else{
	       com_ptr->c_next = c;
	       com_ptr = c;
	       }
	    }
      }

/*
 * showcomment - show the choice comments to the screen
 */

   void fb_showcomment()

         {
	    fb_cnode *c;
	    
	    if (com_head == NULL)
	       return;
	    for (c = com_head; c != NULL; c = c->c_next){
	       fb_move(c->c_row, c->c_col); fb_clrtoeol();
	       fb_prints(c->c_meaning);
	       }
	    fb_refresh();
	 }
