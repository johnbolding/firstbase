/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dicti.c,v 9.0 2001/01/09 02:55:37 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dicti_sid[] = "@(#) $Id: dicti.c,v 9.0 2001/01/09 02:55:37 john Exp $";
#endif

/*
 *  dicti.c - library for dbigen and dbdind.
 */

#include <fb.h>
#include <fb_ext.h>
#include <igen.h>

static char line[FB_MAXLINE] = {""};		/* genaral purpose storage */
static char idicti[FB_MAXNAME];			/* .idicti file name */

extern short cdb_datedisplay;
extern int btree;
extern char *cdb_pgm;
extern char *cdb_IGEN;
extern char *cdb_T_EQUAL;
extern char *cdb_T_NOTEQUAL;
extern char *cdb_T_LESSEQUAL;
extern char *cdb_T_GREATEREQUAL;
extern char *cdb_T_LESSTHAN;
extern char *cdb_T_GREATERTHAN;

static struct self *makstree();
static struct self *andtree();
static struct self *ortree();
static struct self *elem();
static sortby();

/* 
 *  stree - build the selection tree 
 */
 
   struct self *stree(k, hp, by)
      fb_field *k[], *by[];
      fb_database *hp;
      
      {
         int gen;
	 FILE *fs;
         char ix[FB_MAXNAME], err[FB_MAXLINE];
         struct self *makstree(), *top;

	 if (equal(cdb_pgm, cdb_IGEN))
	    gen = 1;
	 else
	    gen = 0;
	 strcpy(ix, hp->dindex);
         sprintf(idicti, "%si", hp->idict);
         if ((fs = fopen(idicti, "r")) == NULL)
	    return(NULL);
         if (gen == 1 && access(ix, 0) != -1 &&
	       !cdb_batchmode && !cdb_yesmode){ /* test existance */
	    sprintf(err, 
	    	"Permission to OVERWRITE %s (y=yes, <ret>=no)? ", ix);
	    fb_bell();
	    if (fb_mustbe('y', err, 15, 10) == FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
	 top = makstree(k, hp, fs, by);
	 fclose(fs);
	 return(top);
       }

/* 
 *  makstree - make a selection tree 
 */
 
   static struct self *makstree(k, hp, fs, by)
      fb_field *k[], *by[];
      fb_database *hp;
      FILE *fs;
      
      {
         
         struct self *p, *q, *top, *andtree();
         char wd1[FB_MAXNAME];
         
         top = NULL;
	 q = NULL;
         while (fgets(line, FB_MAXLINE, fs) != NULL){
            wd1[0] = NULL;
            fb_getword(line, 1, wd1);
            switch (wd1[0]){
               case '$': 
                  p = andtree(hp, fs);
		  if (top == NULL)
		     top = p;
		  else
                     q->andp = p;
                  q = p;
                  break;
               case '%':
                  sortby(fs, by, k, hp);
                  break;
               default:
                  fb_xerror(FB_BAD_DICT, idicti, NIL);
		  break;
               }
            }   
         return(top);
      } 

/* 
 *  andtree - create andtree element | ortree 
 */
 
   static struct self *andtree(hp, fs)
      fb_database *hp;
      FILE *fs;
      
      {
         struct self *ortree(), *elem();
         
         if (fgets(line, FB_MAXLINE, fs) == NULL)
            fb_xerror(FB_BAD_DICT, idicti, NIL);
         if (line[0] == '#')
            return(ortree(hp, fs));
         return(elem(hp));
      }

/* 
 *  ortree  - elem+ '#' ...(ortree element)
 */
 
   static struct self *ortree(hp, fs)
      fb_database *hp;
      FILE *fs;
      
      {
         struct self *t, *p, *r, *elem();
	 int sand;
         
         t = NULL;
	 r = NULL;
         for(sand = 0;;){
            if (fgets(line, FB_MAXLINE, fs) == NULL)
               fb_xerror(FB_BAD_DICT, idicti, NIL);
            if (line[0] == '#')
               break;
	    else if (line[0] == '&'){
	       sand = 1;
	       continue;
	       }
            p = elem(hp);
	    if (t == NULL)
	       r = p;
	    else if (sand == 0)
	       t->orp = p;
	    else {
	       t->orp = t->sandp = p;
	       sand = 0;
	       }
	    t = p;
            }
         return(r);
      }
      
/* 
 *  elem - allocate and build a fb_cell in stree using line as fb_input 
 */
 
   static struct self *elem(hp)
      fb_database *hp;
      
      {
         char id[FB_TITLESIZE+1];
         int i;
         struct self *p;
	 fb_field *fb_findfield();
                 
         p = (struct self *) fb_malloc(sizeof(struct self));
         (p->lword)[0]  = NULL;
         (p->rword)[0] =  NULL;
         i = fb_getword(line, 1, id);
         i = fb_getword(line, i, p->lword);
         i = fb_getword(line, i, p->rword);
	 fb_underscore(p->lword, 0);		/* replace '_' with FB_BLANK */
	 fb_underscore(p->rword, 0);
	 fb_underscore(id, 0);
         if ((p->fp = fb_findfield(id, hp)) == NULL)
            fb_xerror(FB_BAD_DICT, idicti, id);
         p->orp = p->andp = p->sandp = NULL;
	 p->cfp = NULL;
	 if (equal(p->lword, cdb_T_LESSTHAN) ||
	        equal(p->lword, cdb_T_GREATERTHAN) ||
	        equal(p->lword, cdb_T_LESSEQUAL) ||
	        equal(p->lword, cdb_T_GREATEREQUAL) ||
	        equal(p->lword, cdb_T_NOTEQUAL) ||
	        equal(p->lword, cdb_T_EQUAL)){
	    if ((p->cfp = fb_findfield(p->rword, hp)) == NULL)
	       fb_xerror(FB_BAD_DICT, idicti, p->rword);
	    }
         return(p);
      }

/* 
 *  sortby - gether all elements to print in the index. mere integers. 
 */
 
   static sortby(fs, by, tkp, hp)
      FILE *fs;
      fb_field *by[], *tkp[];
      fb_database *hp;
      
      {
         int i, p, j;
         char id[FB_TITLESIZE+1];
         
         if (fgets(line, FB_MAXLINE, fs) == NULL)
            fb_xerror(FB_BAD_DICT, idicti, NIL);
         for (p = 0; p < FB_MAXBY; p++)
            by[p] = NULL;
         hp->irecsiz = 0;
         for (p = 0, i = 1; (i = fb_getword(line, i, id)) != 0; ){
	    fb_underscore(id, 0);
            for (j = 0; j < hp->nfields; j++)
               if (equal(id, tkp[j]->id))
                  break;
            if (j >= hp->nfields)
               fb_xerror(FB_BAD_DICT, idicti, NIL);
            by[p] = tkp[j];
            hp->irecsiz += by[p]->size;
	    if (by[p]->type ==FB_DATE && cdb_datedisplay == 10)
               hp->irecsiz += 2;		/* century space */
            p++;
            if (p >= FB_MAXBY)
               break;
            }
         hp->irecsiz += (FB_RECORDPTR + 1);
         if (p == 0)
            fb_xerror(FB_BAD_DICT, idicti, NIL);
         btree = 0;
         if (fgets(line, FB_MAXLINE, fs) != NULL)
            if (line[0] == '%')
               btree = 1;
      }
