/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: index.c,v 9.1 2001/01/16 02:46:45 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Index_sid[] = "@(#) $Id: index.c,v 9.1 2001/01/16 02:46:45 john Exp $";
#endif

/*
 *  index.c - library for dbigen
 */

#include <fb.h>
#include <fb_ext.h>
#include <igen.h>

static long count;
static int ifd;
extern fb_field *by[];
extern struct self *top;
extern fb_database *hp;
extern int rflag;
extern int cflag;
extern int nflag;
extern int Bflag;
extern int btree;			/* gen a Btree index when done */
extern short cdb_datedisplay;
extern short cdb_datestyle;

extern char *cdb_T_ALL;
extern char *cdb_T_NOT;
extern char *cdb_T_NONE;
extern char *cdb_T_NOTEQUAL;
extern char *cdb_T_EQUAL;
extern char *cdb_T_LESSEQUAL;
extern char *cdb_T_GREATEREQUAL;
extern char *cdb_T_LESSTHAN;
extern char *cdb_T_GREATERTHAN;
extern char *cdb_T_PATTERN;
extern char *cdb_T_EMPTY;

static long ai, aj, ak;
static double fi, fj, fk;

static char *BADPAT = "bad regular expression: ";
static char *NOBLOCK = "No Blocking (-B) on unclean databases:";

static int npatterns = 0;

#define FB_AMERICAN	1
#define FB_EUROPEAN	2

static long findrecs();
static checkrec();
static enter();
static ix_select();
static ormatch();
static match();
static compare();
static ccompare();
static upper();
static nl_to_blank();

/* 
 *  mkidx - make an index according to parse tree 
 */
 
   mkidx(gidict, gindex)
      char *gidict, *gindex;
      
      {
         FILE *fb_mustfopen();
	 struct self *p, *q;
         long findrecs();
         int i, rcfd;
         char com[FB_MAXLINE], seq[FB_SEQSIZE + 1], msg[FB_MAXLINE];
         char buf[FB_MAXLINE];

	 close(creat(gidict, 0666));
         rcfd = fb_mustopen(gidict, WRITE);
	 close(creat(gindex, 0666));
         ifd =  fb_mustopen(gindex, WRITE);
	 fb_w_init(1, ifd, -1);
	 for (p = top; p != NULL; p = p->andp){
	    if (p->fp->type ==FB_DATE && p->cfp == NULL){
               if (cdb_datestyle == FB_EUROPEAN){
                  fb_ameri_date(p->lword);
                  fb_ameri_date(p->rword);
                  }
	       if (cdb_datedisplay == 8){
                  if (p->lword[0] != CHAR_DOLLAR)
		     fb_endate(p->lword);
                  if (p->rword[0] != CHAR_DOLLAR)
		     fb_endate(p->rword);
		  }
	       else{
                  if (p->lword[0] != CHAR_DOLLAR){
                     fb_longdate(buf, p->lword);
                     fb_long_endate(buf);
                     strcpy(p->lword, buf);
                     }
                  if (p->rword[0] != CHAR_DOLLAR){
                     fb_longdate(buf, p->rword);
                     fb_long_endate(buf);
                     strcpy(p->rword, buf);
                     }
	          }
	       }
	    else if (!FB_OFNUMERIC(p->fp->type) && cflag){ /* case insensat. */
	       upper(p->lword);
	       upper(p->rword);
	       }
	    if (equal(p->rword, cdb_T_PATTERN)){
	       npatterns++;
	       if (re_comp(p->lword) != NULL)
	          fb_xerror(FB_MESSAGE, BADPAT, p->lword);
               /*
                * since we use our own re_comp to regcomp package (GNU)
                * i do not think the patcomp is needed anymore. jpb 12/31/00
                */
               p->patcomp = NULL;
	       }
	    for (q = p->orp; q != NULL; q = q->orp){
	       if (q->fp->type ==FB_DATE && q->cfp == NULL){
		  if (cdb_datedisplay == 8){
                     if (p->lword[0] != CHAR_DOLLAR)
		        fb_endate(q->lword);
                     if (p->lword[0] != CHAR_DOLLAR)
		        fb_endate(q->rword);
		     }
		  else{
                     if (p->lword[0] != CHAR_DOLLAR){
                        fb_longdate(buf, q->lword);
                        fb_long_endate(buf);
                        strcpy(q->lword, buf);
                        }
                     if (p->rword[0] != CHAR_DOLLAR){
                        fb_longdate(buf, q->rword);
                        fb_long_endate(buf);
                        strcpy(q->rword, buf);
                        }
		     }
		  }
	       else if (!FB_OFNUMERIC(q->fp->type) && cflag){/* case insensat. */
		  upper(q->lword);
		  upper(q->rword);
		  }
	       if (equal(q->rword, cdb_T_PATTERN)){
		  npatterns++;
		  if (re_comp(q->lword) != NULL)
		     fb_xerror(FB_MESSAGE, BADPAT, q->lword);
                  /*
                   * since we use our own re_comp to regcomp package (GNU)
                   * i do not think the patcomp is needed anymore. jpb 12/31/00
                   */
		  q->patcomp = NULL;
		  }
	       }
	    }
         count = 0;				/* set global counter */
         findrecs();
	 fb_wflush(1);
         close(ifd);
	 sprintf(seq, "%04d", fb_getseq(hp->fd));
	 lseek(rcfd, FB_SEQSTART, 0);
	 write(rcfd, seq, FB_SEQSIZE);
	 fb_putxhead(rcfd, count, count);
         for (i = 0; i < FB_MAXBY; i++)
            if (by[i] != NULL){
	       fb_underscore(by[i]->id, 1);	/* fb_put back _ before write */
	       fb_mustwrite(rcfd, by[i]->id);
	       fb_mustwrite(rcfd, "\n");
               }
         close(rcfd);
	 if (!cdb_batchmode){
	    fb_scrstat("Sorting");
	    fb_move(4,1), fb_clrtobot();
	    fb_basename(com, gindex);
	    sprintf(msg, "Sorting %s", com);
	    fb_fmessage(msg);
	    }
	 sprintf(com, "sort -o %s %s", gindex, gindex);
	 fb_system(com, FB_WITHROOT);
         if (btree){
            makebtree(gidict, gindex);
            unlink(gindex);
            }
	 if ((!cdb_batchmode && !cdb_yesmode) || rflag){
	    if (!cdb_batchmode)
	       fb_fmessage(NIL);
	    sprintf(com, "There were %ld Records Selected", count);
	    fb_serror(FB_MESSAGE, com, NIL);
	    }
         return(FB_AOK);
      }

/* 
 *  findrecs - find all recs in data base that fit the stree. return count.
 */
 
    static long findrecs()
      
      {
	 int checkrec();
	 long avail, freep = -1;
	 
	 if (hp->dindex == NULL || hp->dindex[0] == NULL){
            if (Bflag == 0)
               fb_foreach(hp, checkrec);
            else{
	       fb_getmap(hp->mfd, 0L, &avail, &freep, (long)NULL, (long)NULL);
               if (freep != 0)
                  fb_xerror(FB_MESSAGE, NOBLOCK, hp->dbase);
	       fb_blockeach(hp, checkrec);
               }
            }
	 else
	    fb_forxeach(hp, checkrec);
      }

/*
 *  checkrec - check the record for possible entry
 */

   static checkrec(dp)
      fb_database *dp;
       
      { 
         if (ix_select() == FB_AOK){
	    enter(dp);
	    count++;
	    if (!cdb_batchmode)
	       fb_gcounter(count);
            }
	 return(FB_AOK);
      }

/* 
 *  enter - output the by fields from buf 
 */
 
   static enter(dp)
      fb_database *dp;
      
      {
         register i;
	 char buf[FB_MAXLINE], *p;
	 int siz;
	 
         for (i = 0; i < FB_MAXBY && by[i] != NULL; i++){
	    siz = by[i]->size;
	    if (by[i]->type ==FB_DATE){
	       if (cdb_datedisplay == 8){
	          fb_endate(by[i]->fld);
		  strcpy(cdb_bfld, by[i]->fld);
		  }
	       else{
	          siz += 2;
	          fb_longdate(buf, by[i]->fld);
		  fb_long_endate(buf);
		  strcpy(cdb_bfld, buf);
	          }
	       }
	    else
	       strcpy(cdb_bfld, by[i]->fld);
	    if (FB_OFNUMERIC(by[i]->type))
	       fb_rjustify(cdb_afld, cdb_bfld, siz, by[i]->type);
	    else
	       fb_pad(cdb_afld, cdb_bfld, siz);
            /* convert any NEWLINES to BLANKS since NEWLINES */
            if ((p = strchr(cdb_afld, CHAR_NEWLINE)) != 0){
               /*
                *  fb_serror(FB_MESSAGE,
                *    "Warning: Removing Newline(s) from Sort-By Field:",
                * by[i]->id);
                */
               for (; p != NULL; p = strchr(cdb_afld, CHAR_NEWLINE))
                  *p = CHAR_BLANK;
               }
	    fb_nextwrite(0, cdb_afld);
	    }
         sprintf(cdb_afld, "%010ld", dp->rec);
	 fb_nextwrite(0, cdb_afld);
	 fb_w_write(0, "\n");
      }
               
/* 
 *  ix_select - walk stree to see if rec fits  FB_AOK = yes, FB_ERROR = no 
 */
 
   static ix_select()
      
      {
         register int st;
	 register struct self *mtop;
        
         st = FB_AOK; 
         for (mtop = top; mtop != NULL; mtop = mtop->andp){
            if (mtop->orp != NULL)
               st = ormatch(mtop);
            else
               st = match(mtop);
            if (st != FB_AOK)
               return(FB_ERROR);
            }
         return(FB_AOK);
      }
      
/* 
 *  ormatch - match an ortree - return FB_AOK or FB_ERROR 
 */
 
   static ormatch(mtop)
      struct self *mtop;

      {
         int st;
	 
         for (; mtop != NULL; mtop = mtop->orp){
	    /* try and catch the embedded and */
	    if (mtop->orp != NULL && mtop->orp == mtop->sandp){
	       for (st = FB_AOK; ; mtop = mtop->orp){    /* sandp */
	          if (st == FB_AOK && match(mtop) != FB_AOK)
		     st = FB_ERROR;
	          if (mtop->orp == NULL || mtop->orp != mtop->sandp)
		     break;
		  }
	       if (st == FB_AOK)
	          return(FB_AOK);
	       }
	    else if (match(mtop) == FB_AOK)
	       return(FB_AOK);
	    }
         return(FB_ERROR);
      }
      
/* 
 *  match - match a single self tree fb_node - return FB_AOK or FB_ERROR 
 */
 
   static match(mtop)
      struct self *mtop;

      {
         int s, rev = 0, rt;
	 char *cword, *cfld;
	 
	 cfld = mtop->fp->fld;
         if (equal(cdb_T_ALL, mtop->lword))
            return(FB_AOK);
         else if (equal(cdb_T_EMPTY, mtop->lword)){
            if (cfld[0] == NULL)
               return(FB_AOK);
            else
               return(FB_ERROR);
            }
	 if (cflag)
	    upper(cfld);		/* does nflag also if needed */
	 else if (nflag)
	    nl_to_blank(cfld);		/* for SYS_V, change NL to FB_BLANK */
	 if (equal(cdb_T_NOT, mtop->lword)){
	    cword = mtop->rword;
	    rev = 1;
	    }
         else if (equal(cdb_T_NONE, mtop->rword))
	    cword = mtop->lword;
         else if (equal(cdb_T_PATTERN, mtop->rword)){
	    if (npatterns > 1){
               /*
                * since we use our own re_comp to regcomp package (GNU)
                * i do not think the patcomp is needed anymore. jpb 12/31/00
                * BUT, something could be done to optomize this by
                * saving old compiled patterns for re-use when npatterns > 1
                */
	       re_comp(mtop->lword);
               }
	    if (re_exec(cfld))
	       return(FB_AOK);
	    return(FB_ERROR);
	    }
	 else if (equal(mtop->lword, cdb_T_LESSTHAN) ||
	        equal(mtop->lword, cdb_T_GREATERTHAN) ||
	        equal(mtop->lword, cdb_T_LESSEQUAL) ||
	        equal(mtop->lword, cdb_T_GREATEREQUAL) ||
	        equal(mtop->lword, cdb_T_NOTEQUAL) ||
	        equal(mtop->lword, cdb_T_EQUAL))
	    return(ccompare(mtop));
	 else
            return(compare(mtop));
         if (equal(cdb_T_EMPTY, mtop->rword)){
            /* rev is guaranteed here since only NOT EMPTY is accepted */
            if (cfld[0] == NULL)
               return(FB_ERROR);
            else
               return(FB_AOK);
            }
	 if (mtop->fp->type ==FB_DATE){
	    strcpy(cdb_afld, cfld);
	    if (cdb_datedisplay == 8)
	       fb_endate(cdb_afld);
	    else{
	       fb_longdate(cdb_bfld, cdb_afld);
	       strcpy(cdb_afld, cdb_bfld);
	       fb_long_endate(cdb_afld);
	       }
	    cfld = cdb_afld;
	    }
	 s = strlen(cword) - 1;
	 /* for wildstar, compare only s chars, else compare all chars */
	 if (cword[s] != '*')
	    s = (MAX(strlen(cfld), s + 1));
	 if (strncmp(cword, cfld, s) == 0)
	    rt = FB_AOK;
	 else
	    rt = FB_ERROR;
	 if (rev == 0)
	    return(rt);
	 if (rt == FB_AOK)	/* reverse the return value */
	    return(FB_ERROR);
	 else
	    return(FB_AOK);
      }

/* 
 *  compare - compare the loaded values, mtop->lw,mtop->rw and mtop->fp->fld.
 */
 
   static compare(mtop)
      struct self *mtop;

      {
         switch (mtop->fp->type){
	    case 'd':			/*FB_DATE */
            case 'c':			/* FB_CHOICE */
            case 'a':			/* FB_ALPHA */
            case 'A':			/* FB_STRICTALPHA */
            case 'U':			/* FB_UPPERCASE */
            case 'C':			/* SILENT FB_CHOICE */
            case 'X':			/* EXTENDED FB_CHOICE */
	       strcpy(cdb_afld, mtop->fp->fld);
	       if (mtop->fp->type ==FB_DATE){
		  if (cdb_datedisplay == 8)
		     fb_endate(cdb_afld);
		  else{
		     fb_longdate(cdb_bfld, cdb_afld);
		     strcpy(cdb_afld, cdb_bfld);
		     fb_long_endate(cdb_afld);
		     }
		  }
	       cdb_afld[strlen(mtop->lword)] = NULL;
               if (equal(mtop->lword, cdb_afld) || equal(mtop->rword, cdb_afld))
                  return(FB_AOK);
               if ((strcmp(mtop->lword, cdb_afld) <= 0) && 
	           (strcmp(mtop->rword, cdb_afld) >= 0))
                  return(FB_AOK);
               break;
            case 'f':			/* FB_FLOAT */
	    case 'F':			/* FB_FORMULA */
               fi = atof(mtop->lword);
               fk = atof(mtop->rword);
	       if (mtop->fp->type != 'F')
                  fj = atof(mtop->fp->fld);
	       else{
	          fb_fetch(mtop->fp, cdb_bfld, cdb_db);	/* in case FORMULA */
		  fj = atof(cdb_bfld);
	          }
               if (fi <= fj && fj <= fk)
                  return(FB_AOK);
               break;
            /* FB_NUMERIC:  FB_DOLLARS:  FB_INTEGER:  FB_LONG:  FB_POS_NUM */
            case 'n': case '$': case 'i': case 'l': case 'N':
               ai = atol(mtop->lword);
               ak = atol(mtop->rword);
               aj = atol(mtop->fp->fld);
               if (ai <= aj && aj <= ak)
                  return(FB_AOK);
               break;
            default:
               fb_xerror(FB_BAD_DATA, SYSMSG[S_FIELD], NIL);
            }
         return(FB_ERROR);
      }

/* 
 *  ccompare - compare two fb_field values, depending on the contents of lword.
 *	this provides the comparison of two fields facility.
 */
 
   static ccompare(mtop)
      struct self *mtop;

      {
         char ctype, buf[FB_MAXLINE];
	 
	 ctype = mtop->fp->type;
	 if (mtop->fp->type != mtop->cfp->type){
	    ctype = 'a';
	    if (mtop->fp->type ==FB_DATE || mtop->cfp->type ==FB_DATE)
	       ctype = 'a';
	    else if ((FB_OFNUMERIC(mtop->fp->type)) ||
	             (FB_OFNUMERIC(mtop->cfp->type)))
	       ctype = 'f';
	    }
	 if (cflag)
	    upper(mtop->cfp->fld);
         switch (ctype){
	    case 'd':			/*FB_DATE */
            case 'c':			/* FB_CHOICE */
            case 'a':			/* FB_ALPHA */
            case 'A':			/* FB_STRICTALPHA */
            case 'U':			/* FB_UPPERCASE */
            case 'C':			/* SILENT FB_CHOICE */
            case 'X':			/* EXTENDED FB_CHOICE */
	       strcpy(cdb_afld, mtop->fp->fld);
	       if (mtop->fp->type ==FB_DATE){
		  if (cdb_datedisplay == 8){
		     fb_endate(cdb_afld);
		     }
		  else{
		     fb_longdate(buf, cdb_afld);
		     fb_long_endate(buf);
		     strcpy(cdb_afld, buf);
		     }
		  }

	       strcpy(cdb_bfld, mtop->cfp->fld);
	       if (mtop->cfp->type ==FB_DATE){
		  if (cdb_datedisplay == 8){
		     fb_endate(cdb_bfld);
		     }
		  else{
		     fb_longdate(buf, cdb_bfld);
		     fb_long_endate(buf);
		     strcpy(cdb_bfld, buf);
		     }
		  }

	       return(acompare(mtop->lword, cdb_afld, cdb_bfld));
               break;
            case 'f':			/* FB_FLOAT */
	    case 'F':			/* FB_FORMULA */
	       if (mtop->fp->type != 'F')	/* get fp */
                  fj = atof(mtop->fp->fld);
	       else{
	          fb_fetch(mtop->fp, cdb_bfld, cdb_db);	/* decode FB_FORMULA fb_field */
		  fj = atof(cdb_bfld);
	          }
	       if (mtop->cfp->type != 'F')	/* get cfp */
                  fk = atof(mtop->cfp->fld);
	       else{
	          fb_fetch(mtop->cfp, cdb_bfld, cdb_db);	/* decode FB_FORMULA fb_field */
		  fk = atof(cdb_bfld);
	          }
	       return(fcompare(mtop->lword, fj, fk));
               break;

            /* FB_NUMERIC:  FB_DOLLARS:  FB_INTEGER:  FB_LONG:  FB_POS_NUM */
            case 'n': case '$': case 'i': case 'l': case 'N':
               aj = atol(mtop->fp->fld);
               ak = atol(mtop->cfp->fld);
	       return(ncompare(mtop->lword, aj, ak));
               break;
            default:
               fb_xerror(FB_BAD_DATA, SYSMSG[S_FIELD], NIL);
            }
         return(FB_ERROR);
      }

/*
 * various compare routines - 'a' = alpha/date, 'f' = float, n = numeric.
 */

   acompare(op, cdb_afld, cdb_bfld)
      char *op, *cdb_afld, *cdb_bfld;
      
      {
         int st;
	 
	 st = strcmp(cdb_afld, cdb_bfld);
	 if (equal(op, cdb_T_EQUAL) && st == 0)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_LESSEQUAL) && st <= 0)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_GREATEREQUAL) && st >= 0)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_LESSTHAN) && st < 0)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_GREATERTHAN) && st > 0)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_NOTEQUAL) && st != 0)
	    return(FB_AOK);
	 else
	    return(FB_ERROR);
      }

   fcompare(op, a, b)
      char *op;
      double a, b;
      
      {
	 if (equal(op, cdb_T_EQUAL) && a == b)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_LESSEQUAL) && a <= b)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_GREATEREQUAL) && a >= b)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_LESSTHAN) && a < b)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_GREATERTHAN) && a > b)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_NOTEQUAL) && a != b)
	    return(FB_AOK);
	 else
	    return(FB_ERROR);
      }

   ncompare(op, a, b)
      char *op;
      long a, b;
      
      {
	 if (equal(op, cdb_T_EQUAL) && a == b)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_LESSEQUAL) && a <= b)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_GREATEREQUAL) && a >= b)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_LESSTHAN) && a < b)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_GREATERTHAN) && a > b)
	    return(FB_AOK);
	 else if (equal(op, cdb_T_NOTEQUAL) && a != b)
	    return(FB_AOK);
	 else
	    return(FB_ERROR);
      }

   static upper(s)	/* convert to upper for C.I, and/or do nflag */
      char *s;
      
      {
         char *p;
	 
	 for (p = s; *p; p++){
	    if (islower(*p))
	       *p = toupper(*p);
	    else if (nflag && *p == FB_NEWLINE)
	       *p = FB_BLANK;
	    }
      }

   static nl_to_blank(s)
      char *s;
      
      {
         char *p;
	 
	 for (p = s; *p; p++)
	    if (*p == FB_NEWLINE)
	       *p = FB_BLANK;
      }
