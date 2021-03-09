/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: initlink.c,v 9.0 2001/01/09 02:56:48 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Initlink_sid[] = "@(#) $Id: initlink.c,v 9.0 2001/01/09 02:56:48 john Exp $";
#endif

/*
 *  initialize any links to other database structures
 */

#include <fb.h>
#include <fb_ext.h>

static char *EX_DBASE = "$DBASE";
static char *EX_INDEX = "$INDEX";

#if FB_PROTOTYPES
static addlink(fb_database *dp);
static openlink(fb_database *dp, fb_field *fp);
static openexchoice(fb_database *dp, fb_field *fp);
static get_exname(fb_field *k, char *xindex, char *xdbase);
#else
static addlink();
static openlink();
static openexchoice();
static get_exname();
#endif /* FB_PROTOTYPES */

extern char *cdb_VEDIT;
extern char *cdb_pgm;
extern short int cdb_returnerror;

/*
 * initlink - initialize any linked fields. Also dbvedit extended choice/link.
 */

   fb_initlink(dp)
      fb_database *dp;
      
      {
	 int i;
	 
         for (i = 0; i < dp->nfields; i++){
	    if (dp->kp[i]->type == FB_LINK){
	       if (openlink(dp, dp->kp[i]) == FB_ERROR){
                  if (cdb_returnerror)
                     return(FB_ERROR);
	          fb_xerror(FB_LINK_ERROR, dp->ddict, dp->kp[i]->id);
                  }
	       }
	    else if (dp->kp[i]->type == FB_EXCHOICE){
	       if (equal(cdb_pgm, cdb_VEDIT)){
	          if (openexchoice(dp, dp->kp[i]) == FB_ERROR){
                     if (cdb_returnerror)
                        return(FB_ERROR);
	             fb_xerror(FB_LINK_ERROR, dp->ddict, dp->kp[i]->id);
                     }
		  }
	       else
	          dp->kp[i]->type = FB_CHOICE;
	       }
	    }
	 addlink(dp);
         return(FB_AOK);
       }

/*
 * addlink - add in the current database to linked list for searching.
 */

   static addlink(dp)
      fb_database *dp;
      
      {
         fb_link *ak, *nlk;
	 
	 /* 
	 * search for database with matching names in link list.
	 * if there, no need to add. otherwise, add to list.
	 */
	 for (nlk = cdb_linktop; nlk != NULL; nlk = nlk->f_next)
	    if (equal(nlk->f_dp->dbase, dp->dbase) &&
		   equal(nlk->f_dp->dindex, dp->dindex))
	       return(FB_AOK);

	 /* else add to list */
         ak = (fb_link *) fb_malloc(sizeof(fb_link));
         ak->f_dp = NULL;
	 ak->f_fix = ak->f_tix = ak->f_ffp = ak->f_tfp = NULL;
	 ak->f_fld = NULL;
	 ak->f_xfld = NULL;
	 
	 /* push onto link list of link structures */
	 ak->f_next = cdb_linktop;
	 cdb_linktop = ak;
	 ak->f_dp = dp;
         return(FB_AOK);
      }

/*
 * openlink - open the linked databases for field fp in fb_database dp.
 */

   static openlink(dp, fp)
      fb_database *dp;
      fb_field *fp;
      
      {
         fb_link *ak, *nlk;
	 char cix[FB_MAXNAME], cdp[FB_MAXNAME], tname[FB_MAXNAME];
	 char stix[FB_MAXNAME], sfdb[FB_MAXNAME], hdir[FB_MAXNAME];
	 int ntix, nfdb, st;
	 long nrec = 0L;
	 
         ak = fp->dflink = (fb_link *) fb_malloc(sizeof(fb_link));
         ak->f_dp = NULL;
	 ak->f_fix = ak->f_tix = ak->f_ffp = ak->f_tfp = NULL;
	 ak->f_fld = NULL;
	 ak->f_xfld = NULL;
	 ak->f_basedp = dp;
	 
	 fb_dirname(hdir, dp->dbase);	/* get default dir name */
	 
	 /* push onto link list of fb_link structures */
	 ak->f_next = cdb_linktop;
	 cdb_linktop = ak;

	 ak->f_tfp = fp;	/* make this link point to this 'to' db */
	 if (fb_parselink(fp->idefault, stix, cix, cdp, sfdb, &nrec)==FB_ERROR)
	    return(FB_ERROR);
	 /* find to index fb_field */
	 if (isdigit(*stix))
	    ntix = atoi(stix);
	 else
	    ntix = fb_findnfield(stix, dp) + 1;
	 if (ntix < 1 || ntix > dp->nfields)
	    return(FB_ERROR);
	 ak->f_tix = dp->kp[ntix - 1];
	 ak->f_dp = fb_dballoc();
	 
	 /* prepend the link index and dbase with real db home dir */
	 if (cix[0] == CHAR_SLASH || cix[0] == CHAR_DOT || hdir[0] == NULL)
	    strcpy(tname, cix);
	 else {
	    strcpy(tname, hdir);
	    strcat(tname, cix);
	    }
	 strcpy(cix, tname);
	 
	 if (cdp[0] == CHAR_SLASH || cdp[0] == CHAR_DOT || hdir[0] == NULL)
	    strcpy(tname, cdp);
	 else {
	    strcpy(tname, hdir);
	    strcat(tname, cdp);
	    }
	 strcpy(cdp, tname);
	 
	 fb_dbargs(cdp, cix, ak->f_dp);
	 /* 
	 * search here for fb_database with matching names.
	 * start at cdb_linktop + 1
	 */
	 nlk = NULL;
	 if (cdb_linktop != NULL){
	    for (nlk = cdb_linktop->f_next; nlk != NULL; nlk = nlk->f_next)
	       if (equal(nlk->f_dp->dbase, ak->f_dp->dbase) &&
	              equal(nlk->f_dp->dindex, ak->f_dp->dindex))
	          break;
	    }
	 if (nlk != NULL){
	    /* must have found fb_database already opened -- in link list */
	    fb_free(ak->f_dp->dbase);	ak->f_dp->dbase = NULL;
	    fb_free(ak->f_dp->ddict);	ak->f_dp->ddict = NULL;
	    fb_free(ak->f_dp->dmap);	ak->f_dp->dmap = NULL;
	    fb_free(ak->f_dp->dlog);	ak->f_dp->dlog = NULL;
	    fb_free(ak->f_dp->dindex);	ak->f_dp->dindex = NULL;
	    fb_free(ak->f_dp->idict);	ak->f_dp->idict = NULL;
	    fb_free((char *) ak->f_dp);
	    ak->f_dp = nlk->f_dp;
	    ak->f_dp->refcnt++;		/* bump reference counter */
	    }
	 else{
	    st = fb_opendb(ak->f_dp, READ, FB_WITHINDEX, FB_MUST_INDEX);
            if (st == FB_ERROR)
               return(st);
            }

	 /* save index fb_field */
	 ak->f_fix = ak->f_dp->ip[0];

	 /* find from fb_database fb_field */
	 if (isdigit(*sfdb))
	    nfdb = atoi(sfdb);
	 else
	    nfdb = fb_findnfield(sfdb, ak->f_dp) + 1;
	 if (nfdb < 1 || nfdb > ak->f_dp->nfields)
	    return(FB_ERROR);
	 ak->f_ffp = ak->f_dp->kp[nfdb - 1];

	 ak->f_fld = (char *) fb_malloc((unsigned) (ak->f_ffp->size + 1));
	 ak->f_fld[0] = NULL;
	 ak->f_xfld = (char *) fb_malloc((unsigned) (ak->f_tix->size + 1));
	 ak->f_xfld[0] = NULL;
	 /* now walk on type and size in original fb_database */
	 ak->f_tfp->type = ak->f_ffp->type;
	 ak->f_tfp->size = ak->f_ffp->size;
	 ak->f_absrec = nrec;
         return(FB_AOK);
      }

/*
 * openexchoice - open the EXCHOICE database for field fp in database dp.
 */

   static openexchoice(dp, fp)
      fb_database *dp;
      fb_field *fp;
      
      {
         fb_link *ak, *nlk;
	 char cix[FB_MAXNAME], cdp[FB_MAXNAME], tname[FB_MAXNAME];
         char hdir[FB_MAXNAME];
         int st;
	 
	 /* link to xflink, the extended choice/link spot */
         ak = fp->xflink = (fb_link *) fb_malloc(sizeof(fb_link));
         ak->f_dp = NULL;
	 ak->f_fix = ak->f_tix = ak->f_ffp = ak->f_tfp = NULL;
	 ak->f_fld = NULL;
	 ak->f_xfld = NULL;
	 ak->f_basedp = dp;
	 
	 fb_dirname(hdir, dp->dbase);	/* get default dir name */
	 
	 /* push onto link list of fb_link structures */
	 ak->f_next = cdb_linktop;
	 cdb_linktop = ak;

	 ak->f_tfp = fp;	/* make this link point to this 'to' db */
	 
	 			/* use fp->help to get xdbase and xindex */
	 if (get_exname(fp, cix, cdp) == FB_ERROR)
	    return(FB_ERROR);
	 
	 ak->f_dp = fb_dballoc();
	 
	 /* prepend the link index and dbase with real db home dir */
	 if (cix[0] == CHAR_SLASH || cix[0] == CHAR_DOT || hdir[0] == NULL)
	    strcpy(tname, cix);
	 else {
	    strcpy(tname, hdir);
	    strcat(tname, cix);
	    }
	 strcpy(cix, tname);
	 
	 if (cdp[0] == CHAR_SLASH || cdp[0] == CHAR_DOT || hdir[0] == NULL)
	    strcpy(tname, cdp);
	 else {
	    strcpy(tname, hdir);
	    strcat(tname, cdp);
	    }
	 strcpy(cdp, tname);
	 
	 fb_dbargs(cdp, cix, ak->f_dp);
	 /* 
	 * search here for database with matching names.
	 * start at cdb_linktop + 1
	 */
	 nlk = NULL;
	 if (cdb_linktop != NULL){
	    for (nlk = cdb_linktop->f_next; nlk != NULL; nlk = nlk->f_next)
	       if (equal(nlk->f_dp->dbase, ak->f_dp->dbase) &&
	              equal(nlk->f_dp->dindex, ak->f_dp->dindex))
	          break;
	    }
	 if (nlk != NULL){
	    /* must have found database already opened -- in link list */
	    fb_free(ak->f_dp->dbase);	ak->f_dp->dbase = NULL;
	    fb_free(ak->f_dp->ddict);	ak->f_dp->ddict = NULL;
	    fb_free(ak->f_dp->dmap);	ak->f_dp->dmap = NULL;
	    fb_free(ak->f_dp->dlog);	ak->f_dp->dlog = NULL;
	    fb_free(ak->f_dp->dindex);	ak->f_dp->dindex = NULL;
	    fb_free(ak->f_dp->idict);	ak->f_dp->idict = NULL;
	    fb_free((char *) ak->f_dp);
	    ak->f_dp = nlk->f_dp;
	    ak->f_dp->refcnt++;		/* bump reference counter */
	    }
	 else{
	    st = fb_opendb(ak->f_dp, READ, FB_WITHINDEX, FB_MUST_INDEX);
            if (st == FB_ERROR)
               return(st);
            }

         return(FB_AOK);
      }

/*
 * get_exname - initialize the extended choice/help dbase and index names
 */

   static get_exname(k, xindex, xdbase)
      fb_field *k;
      char *xindex, *xdbase;
      
      {
         char line[FB_MAXLINE], word[FB_MAXLINE], arg[FB_MAXLINE];
	 int j;
	 FILE *cfs;

         if (k->help == NULL || *k->help == NULL ||
	       (cfs = fopen(k->help, FB_F_READ)) == NULL){
	    fb_serror(FB_CANT_OPEN, k->help, NIL);
	    return(FB_ERROR);
	    }
	 for (; fgets(line, FB_MAXLINE, cfs) != NULL; ){
	    if (line[0] == '"' || line[0] == '#')
	       continue;
	    j = fb_getword(line, 1, word);
	    j = fb_getword(line, j, arg);
	    if (equal(word, EX_DBASE))
	       strcpy(xdbase, arg);
	    else if (equal(word, EX_INDEX))
	       strcpy(xindex, arg);
	    }
	 fclose(cfs);
	 return(FB_AOK);
      }
