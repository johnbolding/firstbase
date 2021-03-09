/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getrec.c,v 9.0 2001/01/09 02:56:58 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getrec_sid[] = "@(#) $Id: getrec.c,v 9.0 2001/01/09 02:56:58 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static long min_rlen = FB_MAPREC_SIZE + 2;
extern short int cdb_allow_links;
extern short int cdb_use_rpc;
extern short int cdb_locklevel;
extern short int cdb_dbase_byteorder;
extern short int cdb_cpu_byteorder;

#if FB_PROTOTYPES
static sread(fb_database *hp, long rlen);
#else
static sread();
#endif /* FB_PROTOTYPES */

extern char cdb_EOREC;

/* 
 * getrec - get logical record n from fd into hp->orec using hp->mfd as map.
 *	this map provides the position and the length of the record.
 * 	the fb_field pointers are loaded into hp->kp.
 * 	the Actual number of characters in the record is returned.
 *	note: the assumption is that sizeof(int) is plenty for max reclen,
 *	-- thus comparison of int i to long rlen in sread().
 */

   fb_getrec(n, hp)
      long n;
      register fb_database *hp;
   
      {
         long rpos, rlen, fend;
         char buf[FB_SLONG+FB_SLONG+3];
	 int status;
         int map_rlen = FB_SLONG + FB_SLONG;

#if RPC
         if (cdb_use_rpc)
            return(fb_getrec_clnt(n, hp));
#endif /* RPC */
	 hp->rec = n;			/* store this rec num */
	 
	 /* 
	  *  seek past map record n since first record is header, meaning
	  *  the point of interest is past the n'th physical record.
	  */

	 if (cdb_locklevel > 1)
	    fb_lock_head(hp);
         if (lseek(hp->mfd, n * (long) FB_MAPREC_SIZE, 0) < 0L){
	    fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], hp->dmap);
	    return(FB_ERROR);
	    }
	 if (read(hp->mfd, buf, (unsigned) map_rlen) != map_rlen){
	    fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_MAP], hp->dmap);
	    return(FB_ERROR);
	    }
         memcpy((char *) &rpos, buf, FB_SLONG);
         memcpy((char *) &rlen, buf + FB_SLONG, FB_SLONG);
         if (cdb_dbase_byteorder != cdb_cpu_byteorder){
            M_32_SWAP(rpos);
            M_32_SWAP(rlen);
            }

	 if (rlen < min_rlen || 
	       (rlen > hp->recsiz + 1 && hp->recsiz > min_rlen)){
	    fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_MAP], hp->dmap);
	    return(FB_ERROR);
	    }
	 if (lseek(hp->fd, rpos, 0) < 0L){
	    fb_lerror(FB_SEEK_ERROR, SYSMSG[S_BAD_DATA], hp->dbase);
	    return(FB_ERROR);
	    }
	 if ((status = sread(hp, rlen)) == FB_ERROR){
            /* test rpos against end of file. if >, its a map prob */
	    fend = lseek(hp->fd, 0L, 2);
            if (rpos >= fend)
	       fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_MAP], hp->dmap);
            else
	       fb_lerror(FB_READ_ERROR, SYSMSG[S_BAD_DATA], hp->dbase);
	    return(FB_ERROR);
	    }
	 if (cdb_locklevel > 1)
	    fb_unlock_head(hp);
         if (cdb_allow_links)
	    fb_getlink(hp);
	 return(status);
      }

/*
 *  sread - do a simple read up to rlen characters or eorec char.
 *	return number of chars actually in the record.
 */
 
   static sread(hp, rlen)
      fb_database *hp;
      long rlen;
   
      {
         register int i;
	 register char *buf, *p;
	 fb_field **kf, *lf;
	 
	 FB_FLD(0,hp) = buf = hp->orec;		/* set FB_FLD 0 and buf */
	 if (read(hp->fd, buf, (unsigned) rlen) != rlen)
	    return(FB_ERROR);
	 i = 1, kf = hp->kp, lf = hp->kp[hp->nfields];
	 for (; ; buf++, i++){
            if ((*kf)->type == FB_BINARY){
               buf += ((*kf)->size);
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
	    if (i > rlen)
	       return(FB_ERROR);
	    if (*buf == NULL){
	       /* skip any FB_FORMULA or FB_LINK fields */
	       for (; *kf != lf && 
	             ((*kf)->type == FB_FORMULA ||
                      (*kf)->dflink != NULL ||
                      (*kf)->type == FB_LINK) ; ){
	          p = (*kf)->fld;		/* save fld pointer in p*/
		  (*kf)->fld = buf;		/* make F point to NULL */
		  (*++kf)->fld = p;		/* make next FB_FLD use p */
		  }
	       if (*kf != lf)
	          (*++kf)->fld = buf + 1;	/* set for next FB_FLD */
	       }
	    }
	 return(i);
      }
