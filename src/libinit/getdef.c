/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getdef.c,v 9.1 2001/01/16 02:46:52 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getdef_sid[] = "@(#) $Id: getdef.c,v 9.1 2001/01/16 02:46:52 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char *FB_DEFAULTS_EXT = "_rc";
static char *MSG = "Defaults File error:";

/*
 * getdef - allow override defaults to the Cdb system.
 *	search along path looking for .dbase_rc
 *	process this file, reassigning defaults if needed.
 */
 
   fb_getdef(hdb)
      fb_database *hdb;
      
      {
	 char line[FB_MAXLINE], word[FB_MAXLINE], fname[FB_MAXNAME];
	 char aword[FB_MAXLINE], hdir[FB_MAXNAME], *p, tname[FB_MAXNAME];
         int fd, j, t_irecsiz = 0;
	 fb_field *f;

	 /* generate .dbase_rc file name */	 
	 tname[0] = CHAR_DOT;
	 tname[1] = NULL;
	 fb_basename(fname, hdb->dbase);
	 strcat(tname, fname);
	 strcat(tname, FB_DEFAULTS_EXT);
	 fb_pathname(fname, tname);
	 fb_dirname(hdir, hdb->ddict);
	 if (strlen(fname) == 0)
	    return(0);
	 if ((fd = open(fname, READ)) < 0)
	    return(0);
         fb_r_init(fd);
	 for (; fb_nextline(line, FB_MAXLINE) != 0; ){
            j = fb_getword(line, 1, word);
	    if (j == 0 || word[0] == '#')
	       continue;
	    if ((f = fb_findfield(word, hdb)) == NULL){
	       fb_serror(FB_MESSAGE, MSG, word);
	       continue;
	       }
            for (; (j = fb_getword(line, j, word)) != 0; ){
	       if (word[1] == CHAR_l){
		  f->lock = CHAR_y;
		  continue;
		  }
	       else if (word[1] == CHAR_A){
		  f->choiceflag = CHAR_A;
		  continue;
		  }
	       if (word[0] != CHAR_MINUS){
		  fb_serror(FB_MESSAGE, MSG, f->id);
		  break;
		  }
	       if ((j = fb_getword(line, j, aword)) == 0){
		  fb_serror(FB_MESSAGE, MSG, f->id);
		  break;
		  }
	       switch(word[1]){
		  case 'c':
		     if (word[2] == NULL)
			word[2] = CHAR_b;
		     f->comloc = word[2];
		     fb_mkstr(&(f->comment), aword);
		     break;
		  case 'M':
		     fb_mkstr(&(f->mode), aword);
		     break;
		  case 'd':
		     fb_mkstr(&(f->idefault), aword);
                     if ((p = (strrchr(aword, CHAR_COLON))) != 0)
                        f->f_prec = atoi(p + 1);
		     break;
		  case 'r':
		     fb_mkstr(&(f->range), aword);
		     break;
		  case 't':
		     fb_mkstr(&(f->a_template), aword);
		     break;
                  case 'm':
                     if (aword[0] == CHAR_SLASH || 
                        aword[0] == CHAR_DOT || hdir[0] == NULL)
                        strcpy(tname, aword);
                     else{
                        strcpy(tname, hdir);
                        strcat(tname, aword);
                        }
                     fb_mkstr(&(f->f_macro), tname);
                     break;
		  case 'h':
		     if (aword[0] == CHAR_SLASH || 
			aword[0] == CHAR_DOT || hdir[0] == NULL)
			strcpy(tname, aword);
		     else{
			strcpy(tname, hdir);
			strcat(tname, aword);
			}
		     fb_mkstr(&(f->help), tname);
		     break;
		  case 'a':
		     break;
/*
*                 removed this so that end users do not munge btrees.
*
*		     if (f->size > t_irecsiz)
*			t_irecsiz = f->size;
*		     f->aid = fb_ixalloc();
*		     f->aid->uniq = (word[2] == CHAR_u) ? 1 : -1;
*		     f->aid->autoname = (char *) fb_malloc(strlen(aword) + 1);
*		     strcpy(f->aid->autoname, aword);
*		     break;
*/
		  default:
		     fb_serror(FB_MESSAGE, MSG, f->id);
		     break;
		  }
	       }
	    }
	 fb_r_end();
         return(t_irecsiz);
      }
