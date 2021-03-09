/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: putrec.c,v 9.0 2001/01/09 02:57:01 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Putrec_sid[] = "@(#) $Id: putrec.c,v 9.0 2001/01/09 02:57:01 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#if FB_PROTOTYPES
static long countrec(fb_database *hp);
static long p_putrec(fb_database *hp, long size, long rlen);
#else /* FB_PROTOTYPES */
static long countrec();
static long p_putrec();
#endif /* FB_PROTOTYPES */

extern short int cdb_use_rpc;
extern short int cdb_returnerror;
extern short int cdb_error;
extern char cdb_EOREC;
extern char cdb_FILLE;
extern char cdb_FILLC;

extern long fb_getfree(int fd, int mfd, long size, long *rlen);
static long countrec(fb_database *hp);

/* 
 * putrec - put hp->orec/hp->arec to logical record n of hp->fd.
 *
 */

   fb_putrec(n, hp)
      long n;
      fb_database *hp;
   
      {
         long avail, freep, rpos, rlen, npos, frlen;
         long wlen, plen, size;

#if RPC
         if (cdb_use_rpc)
            return(fb_putrec_clnt(n, hp));
#endif /* RPC */
         if (n <= 0 || n > hp->reccnt + 1)
            return(FB_ERROR);
         hp->rec = n;
	 size = countrec(hp);
         /* set projected length */
         if (!hp->fixedwidth)
	    plen = size;
         else
            plen = hp->recsiz + 1;
	 rlen = 0L;
	 if (fb_getmap(hp->mfd, n, &avail, &freep, &rpos, &rlen) == FB_ERROR){
	    fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_MAP], hp->dmap);
	    return(FB_ERROR);
	    }
	 if (size <= rlen && plen <= rlen){ /* replace record - it fits here */
	    if (lseek(hp->fd, rpos, 0) < 0L){
	       fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_DATA], hp->dbase);
	       return(FB_ERROR);
	       }

	    /* no reclaiming here -- once rlen, always rlen, or new! */

	    if (p_putrec(hp, size, rlen) == FB_ERROR)
               return(FB_ERROR);
	    }
	 else {		/* must find new place for record */
	    /* 
	     *  the idea here is to find a free space, or use avail.
	     *  if a free space exists, do not changes its rlen.
	     */
	    
	    frlen = rlen;
            npos = fb_getfree(hp->fd, hp->mfd, size, &rlen);
	    if (lseek(hp->fd, npos, 0) < 0L){	/* put the record there! */
	       fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_DATA], NIL);
	       return(FB_ERROR);
	       }
	    wlen = p_putrec(hp, size, rlen);	/* rlen may change */
            if (wlen == FB_ERROR)
               return(FB_ERROR);
	    if (rlen == 0L)
	       avail += wlen;			/* bump avail to past cur. */
            if (rpos > 0)			/* free up old space */
	       if (fb_putfree(hp->fd, hp->mfd, rpos, frlen, hp) == FB_ERROR){
                  cdb_error = FB_WRITE_ERROR;
	          fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_DATA],
                     SYSMSG[S_BAD_MAP]);
		  return(FB_ERROR);
		  }
	    if (fb_putmap(hp->mfd, n, avail, npos, wlen)==FB_ERROR){
               cdb_error = FB_WRITE_ERROR;
	       fb_lerror(FB_WRITE_ERROR, SYSMSG[S_BAD_DATA],SYSMSG[S_BAD_MAP]);
	       return(FB_ERROR);
	       }
	    }
	 return(FB_AOK);
      }

/*
 * countrec - count the characters in the record by looking at all
 *	fields. include the NULL markers. 
 *	assume record length can fit in sizeof(long).
 */
 
   static long countrec(hp)
      fb_database *hp;
      
      {
	 register int i;
	 fb_field **kf;
	 long size;
	 
	 kf = hp->kp;
	 for (size = 0L, i = 0; i <= hp->nfields; i++, kf++){
            if ((*kf)->type != FB_BINARY){
               if ((*kf)->fld == NULL)
                  (*kf)->fld = NIL;			/* just for safety */
               if ((*kf)->type != FB_FORMULA && (*kf)->dflink == NULL &&
                     (*kf)->type != FB_LINK)
                  size += strlen((*kf)->fld) + 1L;	/* one for NULL */
               }
            else
	       size += (*kf)->size;			/* FB_BINARY only */
	    }
	 return(size+1L);				/* one for cdb_EOREC */
      }

/*
 * p_putrec - put the record out to hp->fd. hp->fd is already set.
 *           follow the fb_field pointers, add FILLCs and an cdb_EOREC.
 */
 
   static long p_putrec(hp, size, rlen)
      fb_database *hp;
      long rlen, size;
      
      {
	 register char *bufp, *fldp;
	 fb_field **kf;
         register int i;
	 char *startp;
	 
	 kf = hp->kp;
	 bufp = hp->arec;
	 /* copy each fb_field and its NULL pointer to arec */
	 for (i = 0; i <= hp->nfields; i++, kf++)
	    if ((*kf)->type != FB_FORMULA && (*kf)->dflink == NULL &&
                   (*kf)->type != FB_LINK){
	       /* save beginning of fb_field position */
	       startp = bufp;
               if ((*kf)->type != FB_BINARY){	/* NULL terminated copy */
                  for (fldp = (*kf)->fld; ; fldp++){
                     *bufp++ = *fldp;
                     if (!*fldp)
                        break;
                     }
                  }
               else{				/* FB_BINARY byte copy */
                  memcpy(bufp, (*kf)->fld, (unsigned) (*kf)->size);
                  bufp += (*kf)->size;
                  }
	       (*kf)->fld = startp;
	       }
	 *bufp++ = cdb_EOREC;		 	/* mark record end */
	 rlen = MAX( size, MAX(rlen, (long) (FB_MAPREC_SIZE + 2L)));
         if (hp->fixedwidth)
            rlen = MAX(rlen, hp->recsiz + 1);
	 if (size < rlen){			/* fill hole if needed */
	    while (++size < rlen)
	       *bufp++ = cdb_FILLC;
	    *bufp++ = cdb_FILLE;		/* rec[rlen] = fille */
	    }
	 if (write(hp->fd, hp->arec, (unsigned) rlen) != rlen){
            if (cdb_returnerror){
               cdb_error = FB_WRITE_ERROR;
	       fb_lerror(FB_FATAL_PUTREC,
                  SYSMSG[S_BAD_DATA],(char *) &(hp->rec));
               return(FB_ERROR);
               }
	    fb_xerror(FB_FATAL_PUTREC, SYSMSG[S_BAD_DATA],(char *) &(hp->rec));
            }
	 /* swap arec and orec -- so multiple fb_putrec() calls will work */
	 bufp = hp->arec;
	 hp->arec = hp->orec;
	 hp->orec = bufp;
	 return(rlen);
      }
