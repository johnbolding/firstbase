/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbvform.c,v 9.0 2001/01/09 02:56:03 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "@(#)dbvform.c	8.2 04 Aug 1997 FB";
#endif

/*  
 *  dbvform.c - vform records using a standard vdict (view(5)) file.
 */

#include <dbvf_v.h>

static char *USAGE = 
"usage: dbvform [-w width] [-l length] [-f] [-d dbase] [-i index] [-v view]";

static Bflag = 0;				/* Blocking flag */
static sform();
static onenode();

extern short int cdb_returnerror;

/*
 *  dbvform - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
	 int j;

	 cdb_batchmode = 1;
	 cdb_returnerror = 1;
         fb_getargs(argc, argv, FB_ALLOCDB);
	 p_cols = cdb_t_cols;
	 p_lines = cdb_t_lines;
	 if (fb_testargs(argc, argv, "-f") > 0)
	    formfeed_flag = 1;
	 if (fb_testargs(argc, argv, "-B") > 0)
	    Bflag = 1;
	 if ((j = fb_testargs(argc, argv, "-w")) > 0)
	    p_cols = atoi(argv[j + 1]);
	 if ((j = fb_testargs(argc, argv, "-l")) > 0)
	    p_lines = atoi(argv[j + 1]);
	 if (p_cols <= 0)
	    p_cols = cdb_t_cols;
	 if (p_lines <= 0)
	    p_lines = cdb_t_lines;
	 if (p_cols <= 0)
	    p_cols = 80;
	 if (p_lines <= 0)
	    p_lines = 24;
	 if (p_lines > F_PROW || p_cols > F_PCOL)
	    usage();
	 F_fb_initscreen();
	 fb_allow_int();		/* allows instant piping */	
	 vform(argc, argv);
	 fb_ender();
      }

/*
 *  vform - emit loop for a single dbase.
 */

   vform(argc, argv)
      int argc;
      char *argv[];
   
      
      {
         int sform(), st;

	 if (fb_opendb(cdb_db, READ, FB_WITHINDEX, FB_OPTIONAL_INDEX)!=FB_AOK){
	    fb_serror(FB_MESSAGE, SYSMSG[S_BAD_DATA], cdb_db->dbase);
	    return(FB_ERROR);
	    }
	 if (fb_get_vdict(argc, argv) == FB_ERROR)
	    fb_xerror(FB_BAD_DICT, vdict, NIL);
	 if (cdb_db->dindex == NULL || cdb_db->dindex[0] == NULL ||
               cdb_db->ifd < 0){
            if (Bflag == 0)
	       st = fb_foreach(cdb_db, sform);
            else
	       st = fb_blockeach(cdb_db, sform);
            }
	 else
	    st = fb_forxeach(cdb_db, sform);
	 fb_closedb(cdb_db);
	 return(st);
      }

/* 
 *  sform - sform a single record.
 */

   static sform(db)
      fb_database *db;
      
      {
         fb_page *p;
	 fb_node *n;
	 
	 for (p = phead; p; p = p->p_next){
	    for (n = p->p_nhead; n; n = n->n_next)
	       onenode(n);
	    screendump();
	    }
         return(FB_AOK);
      }

/*
 * onenode - vform a single node
 */

   static onenode(n)
      fb_node *n;
      
      {
         fb_field *f;

	 F_fb_move(n->n_row, n->n_col);
	 if (n->n_fp == NULL){		/* i.e., if its not a fb_field */
	    F_fb_printw(FB_FSTRING, n->n_text);
	    }
	 else{
	    f = n->n_fp;
	    fb_fetch(f, cdb_afld, cdb_db);
	    frm_putfield(n, f, cdb_afld);
	    }
      }

/* 
 * usage message 
 */

   usage()
      {
         fb_xerror(FB_MESSAGE, USAGE, NIL);
      }
