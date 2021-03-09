/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: get_ddic.c,v 9.1 2001/01/16 02:46:52 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getd_dict_sid[] = "@(#) $Id: get_ddic.c,v 9.1 2001/01/16 02:46:52 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char *FMTDEL = "]]]del";

extern short int cdb_t_irecsiz;
extern short int cdb_secure;
extern short int cdb_returnerror;
extern short int cdb_error;
extern char *cdb_e_buf;			/* editable input variables */

/* 
 *  getd_dict - attempt to open a fb_database dictionary.
 *     load the fields into the newly calculated storage area.
 */
 
   fb_getd_dict(hdb)
      fb_database *hdb;
      
      {
	 int n, nfs, loc, j, v, fd;
         char word[FB_MAXLINE], buf[FB_MAXLINE], aword[FB_MAXLINE];
	 char tname[FB_MAXNAME], hdir[FB_MAXNAME], *p;
         fb_field *f;

         if ((fd = open(hdb->ddict, READ)) < 0){ /* always readonly */
            if (!cdb_returnerror)
	       fb_serror(FB_CANT_OPEN, hdb->ddict, NIL);
            cdb_error = FB_CANT_OPEN;
            return(FB_ERROR);
	    }
	 fb_dirname(hdir, hdb->ddict);
	 cdb_t_irecsiz = 0;
         /* use loadlines mechanism for speed */
         n = fb_ln_init(fd);

	 /*
	  *  this should allocate enough fields for the
	  *  database. the extra is for the del field. such a pain.
	  */

	 cdb_keymap = (fb_field **) 
	       (fb_malloc((n+1) * (sizeof(fb_field *))));
	 cdb_kp = cdb_keymap;	/* set up the ptr to the fb_field pointers */
	 hdb->kp = cdb_keymap; 	/* set headers pointer to fb_field pointers */
         for (loc = nfs = 0; ;){
            cdb_kp[nfs] = f = fb_makefield();
            if (fb_ln_load(buf, FB_MAXLINE) == 0)
               break;
            for (n = 0, j = 1; (j = fb_getword(buf, j, word)) != 0; ){
               switch(++n) {
                  case 1: 
		     if ((v = strlen(word)) > FB_TITLESIZE){
		        fb_serror(FB_BAD_DICT, hdb->ddict, word);
			word[FB_TITLESIZE] = NULL;
			v = FB_TITLESIZE;
			}
                     f->id = (char *) fb_malloc((unsigned)v+1);
		     strcpy(f->id, word); 
		     break;
                  case 2: f->type = word[0]; break;
                  case 3: 
                     f->size = atoi(word);
                     if (f->size <= 0 && f->type != FB_LINK){
                        if (cdb_returnerror)
                           return(FB_ERROR);
                        fb_xerror(FB_BAD_DICT, hdb->ddict, f->id);
                        }
                     else if (f->type == FB_DATE && f->size != 6)
                        f->size = 6;
		     cdb_fieldlength = MAX(f->size, cdb_fieldlength);
                     break;
                  default:
                     if (word[0] != CHAR_MINUS){
                        if (cdb_returnerror)
                           return(FB_ERROR);
                        fb_xerror(FB_BAD_DICT, hdb->ddict, f->id);
                        }
                     if (word[1] == CHAR_l){
                        f->lock = CHAR_y;
                        break;
                        }
                     else if (word[1] == CHAR_A){
                        f->choiceflag = CHAR_A;
			break;
			}
                     if ((j = fb_getword(buf, j, aword)) == 0){
                        if (cdb_returnerror)
                           return(FB_ERROR);
                        fb_xerror(FB_BAD_DICT, hdb->ddict, f->id);
                        }
		     switch(word[1]){
                        case 'c':
			   if (word[2] == NULL)
			      word[2] = CHAR_b;
			   f->comloc = word[2];
			   f->comment=(char *)
                              fb_malloc(strlen(aword)+2);
			   strcpy(f->comment, aword);
			   break;
                        case 'M':
			   fb_mkstr(&(f->mode), aword);
			   break;
                        case 'd':
			   fb_mkstr(&(f->idefault), aword);
	                   if ((p = (strrchr(aword, CHAR_COLON))) != 0)
                              f->f_prec = atoi(p + 1);
			   break;
                        case 'r':
			   fb_mkstr(&(f->range), aword);
			   break;
                        case 't':
			   fb_mkstr(&(f->a_template), aword);
			   break;
			case 'm':
			   if (aword[0] == CHAR_SLASH || 
			      aword[0] == CHAR_DOT || hdir[0] == NULL)
			      strcpy(tname, aword);
			   else{
			      strcpy(tname, hdir);
			      strcat(tname, aword);
			      }
			   fb_mkstr(&(f->f_macro), tname);
			   break;
			case 'h':
			   if (aword[0] == CHAR_SLASH || 
			      aword[0] == CHAR_DOT || hdir[0] == NULL)
			      strcpy(tname, aword);
			   else{
			      strcpy(tname, hdir);
			      strcat(tname, aword);
			      }
			   fb_mkstr(&(f->help), tname);
			   break;
			case 'a':
			   if (f->size > cdb_t_irecsiz)
			      cdb_t_irecsiz = f->size;
			   f->aid = fb_ixalloc();
			   f->aid->uniq = (word[2] == CHAR_u) ? 1:-1;
			   f->aid->autoname = 
			      (char *) fb_malloc((unsigned) (strlen(aword)+1));
			   strcpy(f->aid->autoname, aword);
			   break;
			   
			default:
                           if (cdb_returnerror)
                              return(FB_ERROR);
                           fb_xerror(FB_BAD_DICT, hdb->ddict, f->id);
			}
                  }
               }
            /*
             * if you get here and an id, type, or size is not set, ERROR out.
             */
            if (f->id == NULL || f->type == NULL || f->size < 0){
               cdb_error = FB_BAD_DICT;
               return(FB_ERROR);
               }
            f->loc = loc;
	    if (f->type != FB_FORMULA && f->type != FB_LINK)
               loc += (f->size + 1);	/* 1 for the null character */
            if (f->comment != NULL)	/* also checked in dbdbas */
               v = strlen(f->comment);
            else
               v = 0;
            if (f->size <= FB_SCREENFIELD)
               if (v + f->size > FB_SCREENFIELD){
                  if (cdb_returnerror)
                     return(FB_ERROR);
                  fb_xerror(FB_BAD_DICT, hdb->ddict, f->id);
                  }
            if (cdb_secure)
               if (f->mode == NULL)
                  fb_mkstr(&(f->mode), "666");
            nfs++;
            }
         if (cdb_secure)
            f->size = 11;       	/* del uid gid mode */
         else
            f->size = 1;		/* for the deletion marker */
	 fb_mkstr(&(f->id), FMTDEL);
         f->type = CHAR_a; 
	 f->loc = loc; 
	 f->comloc = FB_BLANK;
         f->lock = CHAR_n;
	 f->fld = f->idefault = f->comment =NULL;
         hdb->nfields = nfs;		/* cdb_kp[hdb->nfields] == del */
         if (cdb_secure)
            hdb->recsiz = loc + 12;    	/* plus del uid gid mode */
         else
            hdb->recsiz = loc + 2;    	/* plus del marker */
         /* free and close the loadline stuff */
	 fb_ln_end();
         if (hdb->nfields <= 0 || hdb->recsiz <= 0){
            if (cdb_returnerror)
               return(FB_ERROR);
            fb_xerror(FB_BAD_DICT, hdb->ddict, NIL);
            }
	 hdb->orec = (char *) fb_malloc((unsigned) (hdb->recsiz+10));
	 hdb->arec = (char *) fb_malloc((unsigned) (hdb->recsiz+10));
         cdb_fieldlength += 3;	/* was in MAX statement above */
	 /* fb_free(afld);         don't free anymore since could be used */
	 cdb_afld = (char *) fb_malloc((unsigned) (cdb_fieldlength+11));
	 /* fb_free(bfld);         don't free anymore since could be used */
	 cdb_bfld = (char *) fb_malloc((unsigned) (cdb_fieldlength+11));
	 hdb->afld = cdb_afld;
	 hdb->bfld = cdb_bfld;
	 hdb->orec[0] = hdb->arec[0] = cdb_afld[0] = cdb_bfld[0] = NULL;

         if (cdb_e_buf != NULL)
            fb_free(cdb_e_buf);
	 cdb_e_buf = (char *) fb_malloc((unsigned) (cdb_fieldlength+11));
         cdb_e_buf[0] = NULL;
	 
	 /* allow override of default settings - returns t_irecsiz number */
	 j = fb_getdef(hdb);
         cdb_t_irecsiz = MAX(cdb_t_irecsiz, j);

         /* irec needs to be largest of all indexes (see also geti_dict) */
	 if (cdb_t_irecsiz > hdb->irecsiz){
            if (hdb->irec != NULL)
               fb_free(hdb->irec);
	    hdb->irec = (char *) fb_malloc((unsigned) (cdb_t_irecsiz + 13));
            }

         return(FB_AOK);
      }
