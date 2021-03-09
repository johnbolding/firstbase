/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getlink.c,v 9.1 2001/01/16 02:46:52 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getlink_sid[] = "@(#) $Id: getlink.c,v 9.1 2001/01/16 02:46:52 john Exp $";
#endif

/*
 *  routine to access links to other fb_database structures
 */

#include <fb.h>
#include <fb_ext.h>

#if FB_PROTOTYPES
static link_getrec(fb_link *ak);
static nulllink(fb_link *ak);
#else
static link_getrec();
static nulllink();
#endif /* FB_PROTOTYPES */

extern short int cdb_secure;
extern short int cdb_regexp;

/* 
 *  getlink - for each linked fb_field in fb_database dp, call s_getlink.
 */

   fb_getlink(dp)
      fb_database *dp;
      
      {
         int i;

	 for (i = 0; i < dp->nfields; i++)
	    if (dp->kp[i]->dflink != NULL)
	       fb_s_getlink(dp->kp[i]->dflink);
      }

/* 
 *  s_getlink - try and load the fb_field pointed to by ak
 *	ak is used since l is so close to 1. use ak, think lk.
 */

   fb_s_getlink(ak)
      fb_link *ak;
      
      {
         int save_cdb_regexp;

         /* no regexp for links */
         save_cdb_regexp = cdb_regexp;
         cdb_regexp = 0;

	 ak->f_tfp->fld = NIL;
	 /*
	  * read all headers for the far database.
	  * then read record. fb_field is already null if getxrec fails.
	  */
	 if (link_getrec(ak) == FB_AOK){
	    /* cross referenced record is loaded */
	    if (ak->f_ffp->type != FB_FORMULA)
	       strcpy(ak->f_fld, ak->f_ffp->fld);
	    else if (fb_getformula(ak->f_ffp, ak->f_ffp->idefault,
	             ak->f_fld, 0, ak->f_dp) != FB_AOK)
	       ak->f_fld[0] = NULL;
	    ak->f_tfp->fld = ak->f_fld;
	    }
	 else{
            /* the link failed --- flag recno of the far dbase for nulllink */
            ak->f_dp->rec = -1;
	    nulllink(ak);
            }
	 /* always save the fb_field value that is causing this link */
         strcpy(ak->f_xfld, ak->f_tix->fld);

         /* restore cdb_regexp */
         cdb_regexp = save_cdb_regexp;
       }

/*
 * link_getrec - getrec only if not already loaded. return FB_AOK || FB_ERROR.
 *	ie, if the index value in the to (home) db matches the index val
 *	    in the from db, then do not load.
 */

   static link_getrec(ak)
      fb_link *ak;
      
      {
         int st;

         if (ak->f_dp == NULL)
	    return(FB_ERROR);
	 if (ak->f_tix->fld[0] == NULL && ak->f_absrec == 0)
	    return(FB_ERROR);
	 if (equal(ak->f_tix->fld, ak->f_fix->fld) && ak->f_absrec == 0)
	    return(FB_AOK);
	 if (ak->f_absrec > 0 && ak->f_absrec == ak->f_dp->rec)
	    return(FB_AOK);
	 /* 
	  * should be able to detect if this value was tried on this database
	  * so as not to attempt another read. The following attempt at code
	  * is not good enough...since xfld appears in lots of flinks.
	  * if (equal(ak->f_xfld, ak->f_tix->fld))
	  *    return(FB_AOK);
	  */
	 /* convert tix fb_field into index compatible form. place in bfld */
	 if (FB_OFNUMERIC(ak->f_tix->type))
	    sprintf(cdb_bfld, "%*s", ak->f_tix->size, ak->f_tix->fld);
	 else
	    sprintf(cdb_bfld, "%-*s", ak->f_tix->size, ak->f_tix->fld);
	 if (ak->f_tix->type == FB_DATE && strlen(cdb_bfld) == 6)
	    fb_endate(cdb_bfld);
	 if (fb_getxhead(ak->f_dp->ihfd,
		&(ak->f_dp->bsmax), &(ak->f_dp->bsend)) == FB_ERROR)
	    return(FB_ERROR);
	 fb_lock_head(ak->f_dp);
	 st = fb_gethead(ak->f_dp);
	 fb_unlock_head(ak->f_dp);
	 if (st == FB_ERROR){
	    /* fb_xerror(READ_FB_ERROR, SYSMSG[S_BAD_HEADER],ak->f_dp->dbase);*/
	    return(FB_ERROR);
	    }
	 if (ak->f_absrec > 0)
	    st = fb_getrec(ak->f_absrec, ak->f_dp);
	 else 
	    st = fb_getxrec(cdb_bfld, ak->f_dp);
         if (st == FB_AOK && cdb_secure &&
               fb_record_permission(ak->f_dp, READ) == FB_ERROR)
            st = FB_ERROR;
	 if (st == FB_ERROR)
	    return(FB_ERROR);
	 else
	    return(FB_AOK);
      }

/*
 * parselink - parse a definition of a link. check for plausibleness.
 *	address must be of the form [R@]N!index!dbase!N
 *	where R@ is an optional absolute record reference.
 */

   fb_parselink(addr, stix, cix, cdp, sfdb, nrec)
      char *addr, *cix, *cdp;
      char *stix, *sfdb;
      long *nrec;

      {
         int i;
	 char laddr[FB_MAXLINE], *p, *q;

         if (addr == NULL || addr[0] == NULL)
	    return(FB_ERROR);
	 *nrec = 0L;
	 strcpy(laddr, addr);
	 p = laddr;
	 if ((q = strchr(p, '@')) != 0){
	    *q = NULL;
	    *nrec = atol(p);
	    p = q + 1;
	    }
	 for (i = 1; i <= 3; i++){
	    if ((q = strchr(p, CHAR_BANG)) == 0)
	       return(FB_ERROR);
	    *q = NULL;
	    switch (i){
	       case 1: strcpy(stix, p); break;
	       case 2: strcpy(cix, p); break;
	       case 3: strcpy(cdp, p); break;
	       }
	    p = q + 1;
	    }
	 strcpy(sfdb, p);
	 return(FB_AOK);
      }

/*
 * nullall - null out the fb_field values of all links in the system.
 *	also, null out all fields of the far databases - use db->rec as flag.
 */

   fb_nullall()

      {
         fb_link *ak;
	 
	 for (ak = cdb_linktop; ak != NULL; ak = ak->f_next)
	    if (ak->f_fld != NULL)	/* or its a name only structure */
	       nulllink(ak);
       }

/*
 * nulllink - null out the fb_field values of a single link in fb_database dp.
 *	also, null out all fields of the far databases - use db->rec as flag.
 */
   static nulllink(ak)
      fb_link *ak;

      {
	 fb_database *dp;
         int i;
	 
	 ak->f_tfp->fld = NIL;
	 ak->f_ffp->fld = NIL;
	 ak->f_fld[0] = NULL;
	 ak->f_xfld[0] = NULL;
         /*
          * if set to 0 already by a previous pass, then this next code
          * is not done, thereby not thrashing dbases with LOTS of links.
          * if set to -1 outside of here, its still done!
          */
	 if (ak->f_dp->rec != 0){
	    dp = ak->f_dp;
	    dp->rec = 0;
	    for (i = 0; i < dp->nfields; i++)
	       dp->kp[i]->fld = NIL;
	    }
       }
