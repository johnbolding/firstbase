/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbwrite.c,v 9.0 2001/01/22 18:28:07 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 * dbwrite.c:
 *      simple database write of a single, prompted for record.
 *      used by rectest, a simple record i/o test
 */

#include <fb.h>
#include <fb_ext.h>

   dbwrite(d)
      fb_database *d;

      {
         long rec;
	 int nf, len;
	 char num[FB_MAXLINE], buf[FB_MAXLINE];
	 
	 /* get record number, assure number is in range */
	 printf("Enter record number [1-%ld]:", d->reccnt+1);
	 fgets(num, FB_MAXLINE, stdin);
	 rec = atol(num);
	 if (rec < 1 || rec > d->reccnt+1){
	    fprintf(stderr, "%ld: Out of range\n", rec);
	    return;
	    }

	 /* prompt for all fb_field values in fb_database d */
	 printf("Record %d\n", rec);
	 for (nf = 0; nf < d->nfields; nf++){
	    printf("   Enter Field %d:", nf);
	    if (fgets(buf, FB_MAXLINE, stdin) == NULL)
	       return;
            len = strlen(buf);
            if (buf[len-1] == '\n')
               buf[len-1] = NULL;
	    /* truncate buf if it is too long */
	    if (strlen(buf) > d->kp[nf]->size)
	       buf[d->kp[nf]->size] = NULL;
	    /* store buf into fb_field nf of fb_database d */
	    fb_store(d->kp[nf], buf, d);
	    }

	 /* now write the record from the structure d into fb_database d */

	 /* if the record already exists, merely 'fb_put' it back */
	 if (rec <= d->reccnt){
	    if (fb_putrec(rec, d) == FB_ERROR){
	       fprintf(stderr, "Could not write record # %f\n", rec);
	       return;
	       }
	    }
	 /* else, use addrec to add the record to the end of fb_database */
	 else
	    fb_addrec(d);
      }
