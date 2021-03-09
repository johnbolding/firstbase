/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: itrace.c,v 9.0 2001/01/09 02:55:37 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Itrace_sid[] = "@(#) $Id: itrace.c,v 9.0 2001/01/09 02:55:37 john Exp $";
#endif

static char *tempfile = "/tmp/dblsXXXXXX";
extern short clr_aft_fhelp;

#include <fb.h>      
#include <fb_ext.h>
#include <igen.h>
extern cflag;				/* case-insensative flag */
extern btree;

static show();

/* 
 *  itrace - trace a selection tree. fb_page it to the screen.
 */
 
   itrace(p, by, format)
      struct self *p;
      fb_field *by[];
      int format;
      
      {
         struct self *t, *r = NULL;
         int   first, i;
	 char fname[FB_MAXNAME];
	 FILE *fs;
         
	 strcpy(fname, tempfile);
	 close(mkstemp(fname));
	 fs = fb_mustfopen(fname, "w");
         first = 0;
	 fprintf(fs, "{\n");
         for (; p != NULL; p = p->andp){
            if (first == 0)
               first = 1;
            else
               fprintf(fs,"   $AND$\n");      
            if (p->orp != NULL){
               fprintf(fs,"   { #OR#\n");
               for (t = p; t != NULL; r = t, t = t->orp){
                  show(t, fs);
		  if (t->orp != NULL && t->orp == t->sandp)
		     fprintf(fs, "        &AND&\n");
		  }
               fprintf(fs,"   } #ENDOR#\n");
	       if (format == 1)
	          p = r;
               }
            else
               show(p, fs);
            }
         fprintf(fs,"}\n");
         for (i = 0; i < FB_MAXBY && by[i] != NULL; i++)
            fprintf(fs,"(by: %s)\n", (by[i])->id);
	 if (cflag)
	    fprintf(fs,"(Case Insensative)\n");
	 if (btree)
	    fprintf(fs,"(Btree+ Index)\n");
	 fclose(fs);
	 clr_aft_fhelp = format;	/* if igen, no clear after. kludge.*/
	 fb_fhelp(fname);
	 unlink(fname);
      }
      
/* 
 *  show an element 
 */
 
   static show(p, fs)
      struct self *p;
      FILE *fs;
      
      {
         if (strcmp(p->rword, "NONE") == 0)
            p->rword[0] = NULL;
         fprintf(fs,"      (%s: %s %s)\n", (p->fp)->id, p->lword, p->rword);
      }
