/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: fixflds.c,v 9.0 2001/01/09 02:56:34 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Fixfields_sid[] = "@(#) $Id: fixflds.c,v 9.0 2001/01/09 02:56:34 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

extern char cdb_EOREC;

/*
 * fb_fixfields - fix the fb_field pointers after an r_getrec() call.
 */

   fb_fixfields(hp, rv)
      fb_database *hp;
      fb_varvec *rv;
   
      {
         register int i;
	 register char *buf, *p, *q;
	 fb_field **kf, *lf;
         int nargs;
	 
         /*
          * results back from getxrec|getrec are:
          *    - nargs
          *    - status
          *    - recno
          *    - fb_field1
          *    - fb_field2
          *    - ...
          *    - fieldN
          */
         nargs = atoi(fb_argvec(rv, 0));
         hp->rec = atol(fb_argvec(rv, 2));
         p = hp->orec;
         for (i = 3; i < nargs; i++){
            for (q = fb_argvec(rv, i); ; q++){
               *p++ = *q;
               if (*q == NULL)
                  break;
               }
            *p = cdb_EOREC;
            }
	 FB_FLD(0,hp) = buf = hp->orec;		/* set FB_FLD 0 and buf */
	 i = 1;
         kf = hp->kp;
         lf = hp->kp[hp->nfields];
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
            /*
             * if (i > rlen)
	     *    return(FB_ERROR);
             */
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

#endif /* RPC */
