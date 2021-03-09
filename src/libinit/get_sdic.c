/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: get_sdic.c,v 9.0 2001/01/09 02:56:47 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Gets_dict_sid[] = "@(#) $Id: get_sdic.c,v 9.0 2001/01/09 02:56:47 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern fb_field **cdb_sp;
extern short int cdb_sfields;
extern char *cdb_SCREEN_EXT;

/*
 * gets_dict - maps a virtual screen to the fb_database db.
 */
 
   fb_gets_dict(argc, argv)
      int argc;
      char *argv[];
      
      {
         int n, fd, j, pst;
	 char fname[FB_MAXNAME], line[FB_MAXLINE], word[FB_MAXLINE];
	 fb_field **tp, *f;
	 
	 /* set the defaults */
	 cdb_sp = cdb_db->kp;
	 cdb_sfields = cdb_db->nfields;
	 cdb_db->sdict = NIL;
         if ((n = fb_testargs(argc, argv, "-s")) == 0)
	    return(FB_ERROR);
	 if (++n >= argc)
	    usage();
	 else if (equal(argv[n], "-"))
	    return(FB_ERROR);		/* allow quiet exit and default */
	 fb_rootname(fname, argv[n]);
	 cdb_db->sdict = NULL;
	 fb_mkstr(&(cdb_db->sdict), fname);
	 strcat(fname, cdb_SCREEN_EXT);
	 if ((fd = open(fname, READ)) < 0){
	    /* this could be fb_xerror. as is, if -s fails, tool still works */
	    fb_serror(FB_CANT_OPEN, fname, NIL);
	    return(FB_ERROR);
	    }
         fb_r_init(fd);
	 for (n = 0; fb_nextline(line, FB_MAXLINE) != 0; )
	    n++;
	 tp = (fb_field **) fb_malloc((n + 1) * (sizeof(fb_field *)));
	 pst = FB_AOK;
         fb_r_rewind();
	 for (n = 0; pst == FB_AOK && fb_nextline(line, FB_MAXLINE) != 0; ){
	    if (line[0] == CHAR_PERCENT)
	       break;
	    j = 1; 
	    if ((j = fb_getword(line, j, word)) != 0){
	       fb_underscore(word, 0);
	       if ((f = fb_findfield(word, cdb_db)) == NULL){
	          fb_serror(FB_BAD_DICT, fname, word);
	          pst = FB_ERROR;
		  }
	       else{
		  tp[n++] = f;
		  j = fb_getword(line, j, word);
		  if (equal(word, "-d")){
		     j = fb_getword(line, j, word);
		     if (strlen(word) > 0)
			fb_mkstr(&(f->idefault), fb_underscore(word, 0));
		     }
		  }
	       }
	    }
	 fb_r_end();
	 if (pst != FB_AOK){
	    /* this could be fb_xerror. as is, if -s fails, tool still works */
	    fb_serror(FB_BAD_DICT, fname, word);
	    return(FB_ERROR);
	    }
	 cdb_sp = tp;
	 cdb_sfields = n;
	 if (cdb_db->ihfd < 0)
	    cdb_db->ip[0] = tp[0];
	 return(FB_AOK);
      }
