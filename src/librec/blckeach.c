/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: blckeach.c,v 9.0 2001/01/09 02:56:55 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char sccsid[] = "%W% %G% FB";
#endif

/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: blckeach.c,v 9.0 2001/01/09 02:56:55 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Blockeach_sid[] = "@(#) $Id: blckeach.c,v 9.0 2001/01/09 02:56:55 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char *buffer;			/* for buffered read */
static int curp = -1;			/* current pointer into buffer */
static int maxp = 0;			/* maximum file pointer */
static int extrap = 0;			/* extra bytes at block boundry */
static fb_database *hp;			/* pointer to current database */
static long nrec;			/* for tracking record number */
static char *WARNING = "Warning: Unexpected EOF in blockeach()";

int cdb_blockread;			/* block characters read */

extern long cdb_blocksize;		/* defined in fb_setup() */
extern short int cdb_secure;
extern long cdb_headsize;		/* defined in fb_setup() */

extern char cdb_FREEC;
extern char cdb_FILLC;
extern char cdb_EOREC;
extern char cdb_FILLE;

#if FB_PROTOTYPES
static blockrec(int c, int fd);
static nextread(int fd, char *buf);
static skipbyte(int fd);
#else
static blockrec();
static nextread();
static skipbyte();
#endif /* FB_PROTOTYPES */

/*
 * blockeach - this is basically foreach but it uses blockrec, not getrec.
 *
 *      foreach record in dp database, get record and call f(dp)
 *	as long as f(dp) returns non FB_ERROR code or until reccnt is reached.
 *	return FB_AOK or FB_ERROR.
 */

   fb_blockeach(dp, f)
      fb_database *dp;
      int (*f)(fb_database *dp);
      
      {
         
         int fd, n, freeval;
         char c, *fb_malloc(unsigned s);

         hp = dp;
         fd = dp->fd;
	 nrec = 0L;
         buffer = fb_malloc((unsigned) (cdb_blocksize + 1));
	 if (lseek(fd, (long) cdb_headsize, 0) < 0L)
	    fb_xerror(FB_SEEK_ERROR, dp->dbase, NIL);
	 freeval = FB_SLONG*2;
	 for (;;){
	    if ((n = nextread(fd, &c)) == 0)
	       break;
	    else if (n != 1)
	       fb_xerror(FB_READ_ERROR, dp->dbase, NIL);
            if (c == cdb_FREEC){			/* free list area */
               curp += freeval;
               if (curp > maxp)
                  extrap = curp - maxp;
               }
            else if (c == cdb_FILLC)			/* skip region */
               skipbyte(fd);
	    else if (c == cdb_EOREC)			/* no - op */
               ;
	    else if (c == cdb_FILLE)			/* no - op */
               ;
	    else{					/* must be good */
               ++nrec;
               if ((cdb_blockread = blockrec(c, fd)) == FB_ERROR)
                  fb_xerror(FB_FATAL_GETREC, dp->dbase, (char *) &nrec);
               dp->rec = nrec;
               if (!(fb_isdeleted(dp))){
                  if (cdb_secure &&
                        fb_record_permission(dp, READ) == FB_ERROR)
                     continue;
                  fb_getlink(dp);
                  if (((*f)(dp)) == FB_ERROR)
                     return(FB_ERROR);
                  }
	       }
            }
         fb_free(buffer);
	 return(FB_AOK);
      }

/*
 *  blockrec - process a good record - store in hp like getrec
 */

   static blockrec(c, fd)
      int c;
      register int fd;
      
      {
	 char *buf, *p, cc;
         int i, n, j;
	 fb_field **kf, *lf;
	 
	 FB_FLD(0,hp) = buf = hp->orec;		/* set FB_FLD 0 and buf */
         /* read record and set the fb_field pointers all at once */
	 i = 1, kf = hp->kp, lf = hp->kp[hp->nfields];
	 for (; ; buf++, i++){
            *buf = c;
            if ((*kf)->type == FB_BINARY){
               for (j = (*kf)->size; j > 0; j--){
                  *buf = c;
                  if ((n = nextread(fd, &cc)) == 0)
                     fb_xerror(FB_MESSAGE, WARNING, NIL);
                  else if (n != 1)
                     fb_xerror(FB_READ_ERROR, hp->dbase, NIL);
                  c = cc;
                  }
               i += ((*kf)->size);
               if (*buf == cdb_EOREC)		/* could be last fb_field */
                  break;
               buf--;				/* sub one for ++ at top */
               i--;
	       if (*kf != lf)
	          (*++kf)->fld = buf + 1;	/* set for next FB_FLD */
               continue;
               }
            if (*buf == cdb_EOREC)
               break;
	    if (*buf == NULL){
	       /* skip any FB_FORMULA or FB_LINK fields */
	       for (; *kf != lf && 
	             ((*kf)->type == FB_FORMULA || (*kf)->dflink != NULL ||
                      (*kf)->type == FB_LINK); ){
	          p = (*kf)->fld;		/* save fld pointer in p*/
		  (*kf)->fld = buf;		/* make F point to NULL */
		  (*++kf)->fld = p;		/* make next FB_FLD use p */
		  }
	       if (*kf != lf)
	          (*++kf)->fld = buf + 1;	/* set for next FB_FLD */
	       }
	    if ((n = nextread(fd, &cc)) == 0)
	       fb_xerror(FB_MESSAGE, WARNING, NIL);
	    else if (n != 1)
	       fb_xerror(FB_READ_ERROR, hp->dbase, NIL);
            c = cc;
	    }
         return(i);
      }

/*
 * nextread - semi buffered read - read a block if needed.
 */
   static nextread(fd, buf)
      int fd;
      char *buf;
      
      {
         if (curp < 0 || curp >= maxp){
	    maxp = read(fd, buffer, (unsigned) cdb_blocksize);
	    if (maxp <= 0)
	       return(0);
	    curp = extrap;
	    extrap = 0;
	    }
	 *buf = buffer[curp++];
         return(1);
      }

/*
 *  skipbyte - all FILLC's to EOREC or FILLE
 */

   static skipbyte(fd)
      int fd;
      
      {
         int n;
	 char c;
	 
	 /* checking for eorec is paranoia since all fillc's end in fille */
         for(c = NULL ; c != cdb_FILLE && c != cdb_EOREC ; ){
	    if ((n = nextread(fd, &c)) == 0)
	       break;
	    else if (n != 1)
	       fb_xerror(FB_READ_ERROR, hp->dbase, NIL);
	    }
      }
