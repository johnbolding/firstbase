/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: ckrange.c,v 9.1 2001/01/16 02:46:50 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char checkrange_sid[] = "@(#) $Id: ckrange.c,v 9.1 2001/01/16 02:46:50 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/*
 * generalized version of the checkrange function
 */

#if FB_PROTOTYPES
static checkalpha(char *r, char *inp, int type);
static checknum(char *r, char *inp);
#else
static checknum();
static checkalpha();
#endif /* FB_PROTOTYPES */

/* 
 *  checkrange - check the range of the field value f with range r.
 */
    fb_checkrange(fp, inp)
       fb_field *fp;
       char *inp;
   
      {
	 int st, i;
	 char r[FB_MAXLINE], p[FB_MAXLINE];
	 
	 strcpy(r, fp->range);
	 strcpy(cdb_bfld, inp);
	 fb_trim(cdb_bfld);
         for (i = 1, st = FB_ERROR; ; i++){
            if (fb_subline(p, r, i, CHAR_COMMA) == 0)
               break;
            if (FB_OFNUMERIC(fp->type))
               st = checknum(p, cdb_bfld);
            else
               st = checkalpha(p, cdb_bfld, fp->type);
            if (st == FB_AOK)
               break;
            }
	 return(st);
      }

/* 
 *  checknum - check the numeric range of an input field with r
 */
    static checknum(r, inp)
       char *r, *inp;
   
      {
         double upper, lower, value;
	 char *p;
	 
	 value = atol(inp);
	 if ((p = strchr(r, CHAR_MINUS)) == 0)
	    upper = lower = atol(r);
	 else{
	    *p = NULL;
	    lower = atol(r);
	    upper = atol(p+1);
	    }
	 if (value <= upper && value >= lower)
	    return(FB_AOK);
	 return(FB_ERROR);
      }

/* 
 *  checkalpha - check the alpha range of an input field with r
 */
    static checkalpha(r, inp, type)
       char *r, *inp;
       int type;
   
      {
         char upper[FB_MAXLINE], lower[FB_MAXLINE], *p;
	 int len, len1, len2;
	 
	 if ((p = strchr(r, CHAR_MINUS)) == 0){
	    strcpy(upper, r);
	    strcpy(lower, r);
	    }
	 else{
	    *p = NULL;
	    strcpy(lower, r);
	    strcpy(upper, p+1);
	    }
	 len = strlen(inp);
	 len1 = MIN(strlen(lower), len);
	 len2 = MIN(strlen(upper), len);
	 if (type == FB_DATE){
	    fb_endate(inp);
	    fb_endate(lower);
	    fb_endate(upper);
	    }
	 if (strncmp(lower, inp, len1) <= 0 && strncmp(upper, inp, len2) >= 0)
	    return(FB_AOK);
	 return(FB_ERROR);
      }
