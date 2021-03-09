/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbread.c,v 9.0 2001/01/22 18:28:07 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 * dbread.c:
 *      simple database read of a single, prompted for record.
 *      used by rectest, a simple record i/o test
 */

#include <fb.h>
#include <fb_ext.h>

   dbread(d)
      fb_database *d;

      {
         long rec;
	 int nf;
	 char num[FB_MAXLINE];
	 
	 printf("Enter record number [1-%ld]:", d->reccnt);
	 fgets(num, FB_MAXLINE, stdin);
	 rec = atol(num);
	 if (rec < 1 || rec > d->reccnt){
	    fprintf(stderr, "%ld: Out of range\n", rec);
	    return;
	    }

	 /* get record rec from fb_database d into the fb_database structure */
	 if (fb_getrec(rec, d) == FB_ERROR){
	    fprintf(stderr, "Could not get record # %f\n", rec);
	    return;
	    }

	 /* print all fields of the retrieved record */
	 printf("Record %ld\n", rec);
	 for (nf = 0; nf < d->nfields; nf++)
	    printf("   Field %d...%s\n", nf,FB_FLD(nf, d));
      }
