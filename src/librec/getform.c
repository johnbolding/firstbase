/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getform.c,v 9.0 2001/01/09 02:56:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getformula_sid[] = "@(#) $Id: getform.c,v 9.0 2001/01/09 02:56:57 john Exp $";
#endif

/* 
 *  formula.c - formula routines. 
 */

#include <fb.h>
#include <fb_ext.h>

static int ec = 0;		/* for error count */
static char *FMT1 = "%*.0d";
static char *FMT2 = "%.*f";
static char *TWO_ZERO = "00";

#if !FB_PROTOTYPES
static double value();
#else
static double value(char *id, int parse_only, fb_database *dp);
#endif

/* 
 *  getformula - calculate a formula fb_field and store into s,
 *     or merely parse for errors (if parse_only == 1)
 */
 
   fb_getformula(k, f, s, parse_only, dp)
      fb_field *k;
      char *f, *s;
      int parse_only;		/* 1 = parse only (ie, bad fb_field n ok) */
      fb_database *dp;
      
      {
         double val, tval = 0;
	 register int i;
	 int prec, mustbe, bad;
	 char op, id[FB_MAXNAME];

	 ec = 0;
	 mustbe = 0;
	 if (f == NULL){
	    fb_serror(FB_MESSAGE, SYSMSG[S_BAD_FORMULA], k->id);
	    if (!parse_only)
	       sprintf(s, FMT1, k->size, 0);
	    return(FB_ERROR);
	    }
	 fb_underscore(fb_trim(f), 0);	/* trim and replace '_' with BLANK */
	 bad = 0;
	 val = 0.0;			/* prime the simplistic parsing */
	 prec = 2;
	 op = CHAR_PLUS;
	 for (i = 1; i != 0; ){
            /* skip blanks */
	    for (id[0] = FB_BLANK; i != 0 && id[0] == FB_BLANK; )
	       i = fb_gettoken(f, i, id, CHAR_DOT);
	    if (i == 0){
	       if (mustbe == 1){		/* missing operand */
	          fb_serror(FB_MESSAGE, SYSMSG[S_BAD_FORMULA], f);
		  ec++;
		  val = 0.0;
		  }
	       break;
	       }
	    if (op != CHAR_COLON){
	       if (!parse_only)
	          tval = value(id, parse_only, dp);
	       else
	          tval = 0;
	       }
	    switch (op){
	       case '+': val += tval; break;
	       case '-': val = (val - tval); break;
	       case '*': val *= tval; break;
	       case '/': if (tval != 0)
	                    val /= tval; 
			 break;
	       case ':': prec = atoi(id); break;	/* precision */
	       default:				/* illegal operand */
	          fb_serror(FB_MESSAGE, SYSMSG[S_BAD_FORMULA], f);
		  ec++;
		  val = 0.0;
		  bad = 1;
		  break;
	       }
	    if (bad == 1)
	       break;

	    /* skip blanks */
	    for (id[0] = FB_BLANK; i != 0 && id[0] == FB_BLANK; )
	       i = fb_gettoken(f, i, id, FB_BLANK);

	    if (i == 0)
	       break;
	    op = id[0];
	    mustbe = 1;
	    }
	 if (!parse_only){
	    sprintf(s, FMT2, prec, val); 
	    if ((int) strlen(s) > k->size)
	       s[k->size] = 0;
	    }
	 if (ec == 0)
	    return(FB_AOK);
	 else
	    return(FB_ERROR);
       }

/* 
 *  value - get value of reference to field, or just parse if parse_only.
 */
 
   static double value(id, parse_only, dp)
      char *id;
      int parse_only;
      fb_database *dp;
      
      {
         int n;
	 char fld[FB_SCREENFIELD];

	 if (id[0] == CHAR_c || id[0] == CHAR_C)	/* a constant */
	    return((double) atof(id+1));
	 n = atoi(id);
	 if (n < 1 || n > dp->nfields){
	    fb_serror(FB_MESSAGE, SYSMSG[S_BAD_FORMULA], id);
	    ec++;
	    return(0);
	    }
	 n--;	/* scale to 0 based kp array */
	 if (!parse_only)
	    strcpy(fld, FB_FLD(n,dp));
	 else
	    strcpy(fld, TWO_ZERO);
	 /* blindly assume that choice is numeric. else atof will do 0 */
	 if (FB_OFNUMERIC(FB_TYP(n,dp)) || FB_TYP(n,dp) == FB_CHOICE)
	    return(atof(fld));
	 else{
	    fb_serror(FB_MESSAGE, SYSMSG[S_BAD_FORMULA], dp->kp[n]->id);
	    ec++;
	    }
	 return(0.0);
      }
