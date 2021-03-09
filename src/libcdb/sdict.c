/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: sdict.c,v 9.0 2001/01/09 02:56:29 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Sdict_sid[] = "@(#) $Id: sdict.c,v 9.0 2001/01/09 02:56:29 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char *FMT1 = "%s ";
static char *FMT1_A = " %d";
static char *FMT2 = " -d %s";
static char *FMT3 = " -c%c %s";
static char *FMT4 = " -l";
static char *FMT5 = " -h %s";
static char *FMT6 = " -a";
static char *FMT7 = " -r %s";
static char *FMT8 = " -t %s";
static char *FMT9 = " -A";
static char *FMT10 = " -m %s";

/* 
 *  sdict - output a simple dictionary field item to stream fs.
 */
 
   fb_sdict(fp, fs, doauto)
      register fb_field *fp;
      register FILE *fs;
      int doauto;
      
      {
         char c;

	 fprintf(fs, FMT1, fp->id);
         if (fp->dflink != NULL)
            c = 'L';
         else
            c = fp->type;
         fprintf(fs, "%c", c);
	 fprintf(fs, FMT1_A, fp->size);
	 if (fp->idefault != NULL)
	    fprintf(fs, FMT2, fp->idefault);
	 if (fp->comloc != FB_BLANK && fp->comment != NULL)
	    fprintf(fs, FMT3, fp->comloc, fp->comment);
	 if (fp->lock == CHAR_y)
	    fprintf(fs, FMT4);
	 if (fp->help != NULL)
	    fprintf(fs, FMT5, fp->help);
	 if (doauto && fp->aid != NULL && fp->aid->autoname != NULL) {
	    fprintf(fs, FMT6);
	    if (fp->aid->uniq > 0)
	       fputc(CHAR_u, fs);
	    fprintf(fs, SYSMSG[S_BLANK_S], fp->aid->autoname);
	    }
	 if (fp->range != NULL)
	    fprintf(fs, FMT7, fp->range);
	 if (fp->a_template != NULL)
	    fprintf(fs, FMT8, fp->a_template);
	 if (fp->choiceflag == CHAR_A)
	    fprintf(fs, FMT9);
	 if (fp->f_macro != NULL)
	    fprintf(fs, FMT10, fp->f_macro);
	 fputc(FB_NEWLINE, fs);
      }
