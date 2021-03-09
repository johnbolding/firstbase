/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: ex_cho.c,v 9.0 2001/01/09 02:55:58 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Ex_Choice_sid[] = "@(#) $Id: ex_cho.c,v 9.0 2001/01/09 02:55:58 john Exp $";
#endif

#include <dbve_ext.h>
#include <sys/types.h>
#include <sys/stat.h>

fb_exchoice exnode, *exc = &exnode;
extern fb_cnode *com_head, *com_ptr;

extern char *cdb_prompt_choicemsg;	/* prompt for choice message */
extern char *cdb_dbvedit_cho_ploc;	/* prompt location for choice level */
extern short int cdb_dbvedit_cho_pilength; /* length, dbvedit, choice level */
extern short int cdb_record_level;	/* record level toggle switch */

static char *EX_FILTERS = "$FILTERS";
static char *EX_DISPLAYS = "$DISPLAYS";
static char *EX_HELP = "$HELP";
static char *EX_RETURN = "$RETURN";
static char *EX_PROMPT = "$PROMPT";
static char *BAD_FILTER = "No Matching Selections To Choose From";

static initex();

/* 
 * ex_choiceinput - routine called from edit_field in dbedit.c
 *   idea here is to display a list of choices that
 *   actually live in another FirstBase database. All parameters are
 *   stored in the helpfile/choicefile pointed to by this fb_field.
 *
 *   user picks one, ex_choiceinput returns associated string.
 */
 
   fb_ex_choiceinput(row, col, len, k, inp, new, okend)
      fb_field *k;
      char *inp;
      int new, okend, row, col, len;
      
      {
         FILE *cfs;

         if (k->help == NULL || *k->help == NULL ||
	       (cfs = fopen(k->help, FB_F_READ)) == NULL){
	    fb_serror(FB_CANT_OPEN, k->help, NIL);
	    return(FB_ERROR);
	    }
	 if (initex(k, cfs) == FB_ERROR){
	    fb_serror(FB_MESSAGE, "Could not init extended choicefile.", NIL);
	    return(FB_ERROR);
	    }
	 fclose(cfs);
         st = ex_setlimit(exc);
	 if (st != FB_ERROR)
	    st = ex_input(exc, k, inp, new, okend);
	 else{
	    fb_serror(FB_MESSAGE, BAD_FILTER, NIL);
            }
	 /* ex_help and ex_prompt are only dynamic items in exnode */
	 fb_free(exc->ex_help);
	 fb_free(exc->ex_prompt);
	 exc->ex_help = NULL;
	 exc->ex_prompt = NULL;
	 fb_freeccom();
	 return(st);
      }

/*
 * initex - initialize the extended choice/fb_help field
 *	load up everything EXCEPT the dbase and index, which are
 *	aleady stored in the fields xflink structure.
 */

   static initex(k, cfs)
      fb_field *k;
      FILE *cfs;
      
      {
         char line[FB_MAXLINE], word[FB_MAXLINE];
	 char arg[FB_MAXLINE];
	 fb_database *fb_dballoc();
	 fb_field *fb_findfield();
	 int j, slot, isize;

	 nullexc();
         com_head = NULL;
	 com_ptr = NULL;
	 /* retrieve db from xflink */
	 exc->ex_db = k->xflink->f_dp;
         exc->ex_tree = exc->ex_db->b_tree;
	 /* now load the rest of the items in the extended fb_help file */
	 for (; fgets(line, FB_MAXLINE, cfs) != NULL; ){
	    if (line[0] == '#')
	       continue;
	    if (line[0] == '"'){
	       fb_addcomment(line);
	       continue;
	       }
	    j = fb_getword(line, 1, word);
	    j = fb_getword(line, j, arg);
	    if (equal(word, EX_HELP))
	       fb_mkstr(&(exc->ex_help), arg);
	    else if (equal(word, EX_PROMPT)){
	       fb_underscore(arg, 0);
	       fb_mkstr(&(exc->ex_prompt), arg);
	       }
	    else if (equal(word, EX_FILTERS)){
	       /* for each of the filter words, look up its fb_field pointer */
	       for (isize = 0, slot = 0; j > 0; slot++){
		  if (slot >= NFILTERS)
		     break;
                  /* local fb_database has had underscores removed */
	          fb_underscore(arg, 0);
	          if ((exc->ex_filters[slot] = fb_findfield(arg, hp)) == NULL){
	             fb_serror(FB_MESSAGE, "ExChoice: cant find field", arg);
		     return(FB_ERROR);
		     }
		  j = fb_getword(line, j, arg);
		  isize += exc->ex_filters[slot]->size;
		  }
	       isize += 11;
	       if (isize > exc->ex_db->irecsiz){
	          fb_serror(FB_MESSAGE, "$FILTERS/Choice Index mis-aligned",
                     NIL);
		  return(FB_ERROR);
	          }
	       }
	    else if (equal(word, EX_DISPLAYS)){
	       /* for each of the filter words, look up its fb_field pointer */
	       for (slot = 0; j > 0; slot++){
		  if (slot >= NDISPLAYS)
		     break;
                  /* far fb_database has NOT had underscores removed */
	          /* fb_underscore(arg, 0); */
	          if ((exc->ex_displays[slot] = 
		        fb_findfield(arg, exc->ex_db))==NULL){
	             fb_serror(FB_MESSAGE, "ExChoice: cant find field", arg);
		     return(FB_ERROR);
		     }
		  j = fb_getword(line, j, arg);
		  }
	       }
	    else if (equal(word, EX_RETURN)){
               /* far fb_database has NOT had underscores removed */
	       /* fb_underscore(arg, 0); */
	       if ((exc->ex_return = fb_findfield(arg, exc->ex_db)) == NULL){
	          fb_serror(FB_MESSAGE, "ExChoice: cant find field", arg);
		  return(FB_ERROR);
		  }
	       }
	    }
	 if (exc->ex_return == NULL ||
	       exc->ex_displays[0] == NULL ||
	       exc->ex_db == NULL ){
	    fb_serror(FB_MESSAGE, "$DISPLAYS/$RETURN not set", NIL);
	    return(FB_ERROR);
	    }
	 exc->ex_first = exc->ex_ptop = 1;
         if (exc->ex_tree){
            exc->ex_first_curkey = 1;
	    exc->ex_last = exc->ex_db->b_seq->bs_reccnt;
            exc->ex_last_curkey = 3;
            }
         else
	    exc->ex_last = exc->ex_db->bsend;
	 return(FB_AOK);
      }

/*
 * nullexc - null out the extended choice strucutre
 */

   nullexc()
      {
         int i;

	 for (i = 0; i < NFILTERS; i++)
	    exc->ex_filters[i] = NULL;
	 for (i = 0; i < NDISPLAYS; i++)
	    exc->ex_displays[i] = NULL;
	 exc->ex_db = NULL;
	 exc->ex_help = NULL;
	 exc->ex_prompt = NULL;
	 exc->ex_return = NULL;
	 exc->ex_first = 0;
	 exc->ex_last = 0;
	 exc->ex_ptop = 0;
	 exc->ex_pbot = 0;
	 exc->ex_firstrow = 0;
	 exc->ex_lastrow = 0;
         /* btree fields */
	 exc->ex_tree = 0;
	 exc->ex_first_curkey = 0;
	 exc->ex_last_curkey = 0;
	 exc->ex_ptop_curkey = 0;
	 exc->ex_pbot_curkey = 0;
      }

/*
*   trace_ex()
*      {
*         int i;
*
*         fprintf(stderr, "EX CHOICE: %s %s\n", 
*	    exc->ex_db->dbase, exc->ex_db->dindex);
*	 for (i = 0; i < NFILTERS; i++)
*	    if (exc->ex_filters[i] != NULL)
*               fprintf(stderr,".. Filter %d:%s\n",i,exc->ex_filters[i]->id);
*	 for (i = 0; i < NDISPLAYS; i++)
*	    if (exc->ex_displays[i] != NULL)
*               fprintf(stderr,".. Display %d:%s\n",i,
*	          exc->ex_displays[i]->id);
*      }
*/
