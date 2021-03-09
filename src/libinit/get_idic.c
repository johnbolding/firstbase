/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: get_idic.c,v 9.0 2001/01/09 02:56:47 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Geti_dict_sid[] = "@(#) $Id: get_idic.c,v 9.0 2001/01/09 02:56:47 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern short cdb_datedisplay;
extern short int cdb_t_irecsiz;

/* 
 *  geti_dict - get an index dictionary using mode.
 */
 
   fb_geti_dict(mode, dhp)
      int mode;
      fb_database *dhp;
      
      {
         
	 register int nfs, p;
         int loc, isiz, i;
         char id[FB_TITLESIZE+1],buf[FB_MAXLINE], autofile[FB_MAXNAME];
         fb_autoindex *ix;

         /*
          * with the advent of btree indexes, this hook is not needed,
          * except when the index does not exist, or its a flat index.
          */

         fb_autoigen(dhp);		/* maybe regenerate new index */

	 if (access(dhp->idict, 0) != 0)
	    return(FB_ERROR);
	 
	 dhp->ihfd = -1;
	 for (i = 0; i < dhp->nfields; i++){
	    if (dhp->kp[i]->aid != NULL && 
	           dhp->kp[i]->aid->autoname != NULL &&
	           dhp->kp[i]->aid->autoname[0] != NULL){
               ix = dhp->kp[i]->aid;
	       fb_dirname(autofile, dhp->dbase);
	       strcat(autofile, ix->autoname);
	       strcat(autofile, SYSMSG[S_EXT_IDICT]);
	       if (equal(autofile, dhp->idict) && ix->hfd > 0){
	          dhp->ihfd = dhp->kp[i]->aid->hfd;
                  if (ix->ix_tree == 1)
                     dhp->b_tree = 1;
		  break;
		  }
	       }
	    }

         for (i = 0; i < dhp->b_maxauto; i++){
            ix = dhp->b_autoindex[i];
            fb_dirname(autofile, dhp->dbase);
            strcat(autofile, ix->autoname);
            strcat(autofile, SYSMSG[S_EXT_IDICT]);
            if (equal(autofile, dhp->idict) && ix->hfd > 0){
               dhp->ihfd = ix->hfd;
               break;
               }
            }

	 if (dhp->ihfd == -1)
	    if ((dhp->ihfd = open(dhp->idict, mode)) < 0){
	       fb_serror(FB_CANT_OPEN, dhp->idict, NIL);
	       return(FB_ERROR);
	       }
         fb_r_init(dhp->ihfd);
	 if (fb_getxhead(dhp->ihfd, &(dhp->bsmax), &(dhp->bsend)) == FB_ERROR){
	    fb_serror(FB_BAD_DATA, dhp->idict, NIL);
	    return(FB_ERROR);
	    }
         for(nfs = 0; fb_nextline(buf, FB_MAXLINE) != 0; nfs++)
            if (buf[0] == '%')
               break;
	    
	 /*
	  *  this should allocate enough fields for the
	  *  index. the extra is for the implicit newline.
	  */
	  
	 if (dhp->ip != NULL)
	    fb_free((char *) dhp->ip);
	 cdb_keyindx = (fb_field **)
	       fb_malloc((nfs+1) * (sizeof(fb_field *)));
         /* set up the pointer to the fb_field pointers */
         dhp->ip = cdb_ip = cdb_keyindx;
	 if (fb_getseq(dhp->ihfd) != dhp->sequence){
	    fb_serror(FB_WRONG_INDEX, dhp->dindex, NIL);
	    fb_emptyi_dict(dhp);
	    return(FB_ERROR);	/* no index - allow outer control */
	    }
         fb_r_rewind();
         fb_skipxhead(dhp->ihfd);
         for(loc = p = nfs = 0; fb_nextline(buf, FB_MAXLINE) != 0; ){
            fb_getword(buf, 1, id);
            if (equal(id, "%")){
               dhp->b_tree = 1;
               break;
               }
            nfs++;
	    if ((dhp->ip[p] = fb_findfield(id, dhp)) != NULL){
               loc += (dhp->ip[p]->size);
	       if (dhp->ip[p]->type == FB_DATE && cdb_datedisplay == 10)
	          loc += 2;		/* century space */
	       p++;
	       }
	    else{
	       fb_serror(FB_BAD_DICT, dhp->dindex, id);
               return(FB_ERROR);
               }
            }
         dhp->ip[p] = (fb_field *) fb_malloc(sizeof(fb_field));
         dhp->ip[p]->size = 10;			/* record pointer 'field' */
         dhp->ip[p]->type = CHAR_n;
         dhp->ip[p]->loc = loc;
	 dhp->ip[p]->id = NIL;
         dhp->ifields = (nfs + 1);
         isiz = (loc + dhp->ip[p]->size + 1);  /* 10 + 1 for newline */
	 dhp->irecsiz = isiz;
         /* assure that irec buffer is largest of all possible indexes */
         if (dhp->irec != NULL){
            fb_free(dhp->irec);
            dhp->irec = NULL;
            }
         cdb_t_irecsiz = MAX(cdb_t_irecsiz, isiz);
	 dhp->irec = (char *) fb_malloc((unsigned) (cdb_t_irecsiz + 13));

         if (dhp->b_tree){			/* init btree space */
            dhp->b_seq = fb_seq_alloc(dhp->dindex, isiz - 1);
            dhp->b_idx = fb_idx_alloc(dhp->dindex, isiz - 1);
            dhp->b_seqtmp = fb_seq_alloc(dhp->dindex, isiz - 1);
            dhp->b_idxtmp = fb_idx_alloc(dhp->dindex, isiz - 1);
            }

         return(FB_AOK);
      }
