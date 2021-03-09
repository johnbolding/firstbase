/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: get_cdic.c,v 9.0 2001/01/09 02:56:05 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Get_cdict_sid[] = "@(#) $Id: get_cdic.c,v 9.0 2001/01/09 02:56:05 john Exp $";
#endif

#include <dbvi_ext.h>

static char *CELL_EXT = ".cdict";
extern short cdb_datedisplay;

/*
 * get_cdict - maps a vcalc screen to the fb_database db.
 */
 
   get_cdict(argc, argv)
      int argc;
      char *argv[];
      
      {
         int n, fd, j, pst, tsize;
	 char fname[FB_MAXNAME], line[FB_MAXLINE], word[FB_MAXLINE], *fb_underscore();
	 column **cp, *makecolumn();
	 fb_field *f;
	 
	 /* set the defaults */
         if ((n = fb_testargs(argc, argv, "-c")) != 0){
	    if (++n >= argc)
	       usage();
	    fb_rootname(fname, argv[n]);
	    }
	 else
	    fb_rootname(fname, cdb_db->dbase);
	 strcat(fname, CELL_EXT);
	 strcpy(cdict, fname);
	 calc_row = 4;			/* default */
	 if ((fd = open(fname, READ)) < 0)
	    return(FB_ERROR);
         fb_r_init(fd);
	 for (n = 0; fb_nextline(line, FB_MAXLINE) != 0; ){
	       fb_getword(line, 1, word);
	       if (word[0] != '#' && !equal(word, "$STARTROW"))
	          n++;
	       }
	 ncolumns = n;
	 cp = (column **) fb_malloc(ncolumns * (sizeof(column *)));
	 gcolumn = cp;
	 pst = FB_AOK;
         fb_r_rewind();
	 for (n = 0; pst == FB_AOK && fb_nextline(line, FB_MAXLINE) != 0; ){
	    j = 1; 
	    if ((j = fb_getword(line, j, word)) != 0){
	       /* fb_underscore(word, 0); not done in ddict yet */
	       if (word[0] == '#')
		  continue;
	       else if (equal(word, "$STARTROW")){
		  j = fb_getword(line, j, word);
	          calc_row = atoi(word);
		  }
	       else if ((f = fb_findfield(word, cdb_db)) == NULL){
	          fb_serror(FB_BAD_DICT, fname, word);
	          pst = FB_ERROR;
		  }
	       else{
		  cp[n] = makecolumn();
		  cp[n]->p_field = f;
		  cp[n]->p_width = 1;
		  cp[n]->p_array = n;
		  fb_mkstr(&(cp[n]->p_label), f->id);
		  for (; (j = fb_getword(line, j, word)) != 0; ){
		     if (equal(word, "-d")){
			j = fb_getword(line, j, word);
			if (strlen(word) > 0)
			   fb_mkstr(&(f->idefault), fb_underscore(word, 0));
			}
		     else if (equal(word, "-w")){
			j = fb_getword(line, j, word);
			cp[n]->p_width = atoi(word);
		        }
		     else if (equal(word, "-l")){
			j = fb_getword(line, j, word);
			if (strlen(word) > 0)
		           fb_mkstr(&(cp[n]->p_label), fb_underscore(word, 0));
		        }
		     else{
	                fb_serror(FB_BAD_DICT, fname, f->id);
	                pst = FB_ERROR;
			}
		     }
		  tsize = f->size;
		  if (f->type ==FB_DATE)
		     tsize = cdb_datedisplay;
		  cp[n]->p_width += MAX(tsize, strlen(cp[n]->p_label));
		  if (cp[n]->p_width >= cdb_t_cols){
		     fb_serror(FB_BAD_DICT, fname, f->id);
		     pst = FB_ERROR;
		     }
		  n++;
		  }
	       }
	    }
	 close(fd);
	 if (pst != FB_AOK)
	    return(FB_ERROR);
	 if (ncolumns > 0){		/* set up links */
	    cp[0]->p_prev = NULL;
	    cp[ncolumns - 1]->p_next = NULL;
	    for (n = 0; n < ncolumns; n++){
	       if (n > 0)
	          cp[n]->p_prev = cp[n - 1];
	       if (n < ncolumns - 1)
	          cp[n]->p_next = cp[n + 1];
	       }
	    }
	 return(FB_AOK);
      }
