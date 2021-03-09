/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: put.c,v 9.0 2001/01/09 02:56:40 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Put_sid[] = "@(#) $Id: put.c,v 9.0 2001/01/09 02:56:40 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static void make_irec(void);

/* 
 *  fb_put - fb_put an index entry to the named index, p.
 *	assumes current fb_database is in hp
 */
 
static char *FMT1 = 	"%010ld\n";
 
   int fb_put(p)
      char *p;

      {
         char p_iname[FB_MAXNAME], p_idict[FB_MAXNAME], seq[FB_SEQSIZE + 1];
         char buf[FB_MAXLINE];
	 int nd, id;
	 long bsmax, bsorg;
	 
         if (cdb_db->ifd <= 0 && !cdb_db->b_tree){
	    fb_serror(FB_MESSAGE, SYSMSG[S_ILLEGAL], NIL);
	    return(FB_ERROR);
	    }
	 sprintf(p_iname, SYSMSG[S_FMT_2S], p, SYSMSG[S_EXT_IDX]);
	 sprintf(p_idict, SYSMSG[S_FMT_2S], p, SYSMSG[S_EXT_IDICT]);
	 if (access(p_iname, 0) != 0){
	    close(creat(p_iname, 0666));
	    close(creat(p_idict, 0666));
	    if ((id = open(p_idict, READWRITE)) < 0){
	       fb_serror(FB_CANT_OPEN, p_idict, NIL);
	       return(FB_ERROR);
	       }
	    sprintf(seq, SYSMSG[S_FMT_04D], fb_getseq(cdb_db->fd));
	    lseek(id, FB_SEQSTART, 0);		/* write SEQF */
	    write(id, seq, FB_SEQSIZE);
	    fb_putxhead(id, 0L, 0L);		/* positions fds also */
	    fb_getxhead(cdb_db->ihfd, &bsmax, &bsorg);
            /*
             * now copy the current idxdict, but do not allow new
             * index to be a b_tree --- it must be a simple fb index
             */
            fb_r_init(cdb_db->ihfd);
            while (fb_nextline(buf, FB_MAXLINE) != 0){
               if (equal("%", buf))
                  break;
               strcat(buf, SYSMSG[S_STRING_NEWLINE]);
               write(id, buf, strlen(buf));
               }
	    close(id);
	    }
	 id = open(p_idict, READWRITE);
	 if (fb_getseq(id) != fb_getseq(cdb_db->fd)){
	    fb_serror(FB_WRONG_INDEX, p_idict, NIL);
	    close(id);
	    return(FB_ERROR);
	    }
	 if ((nd = open(p_iname, READWRITE)) < 0){
	    fb_serror(FB_CANT_OPEN, p_iname, NIL);
	    return(FB_ERROR);
	    }
	 fb_getxhead(id, &bsmax, &bsorg);		/* should error check */
	 lseek(nd, 0L, 2);			/* seek to end */
         /* always build cdb_db->irec */
         make_irec();
         write(nd, cdb_db->irec, strlen(cdb_db->irec));
         close(nd);
	 fb_putxhead(id, (long) (bsmax + 1L), bsorg);
	 close(id);
	 return(FB_AOK);
      }

   static void make_irec()
      {
         fb_field *fp;
         char lbuf[20];
         int i, lastone;

         /* generate a key fb_field */
         cdb_db->irec[0] = NULL;
         lastone = cdb_db->ifields - 1;
         for (i = 0; i < lastone; i++){
            fp = cdb_db->ip[i];
            strcpy(cdb_afld, fp->fld);
            fb_makess(cdb_afld, fp->type, fp->size);
            strcat(cdb_db->irec, cdb_afld);
            }
         sprintf(lbuf, FMT1, cdb_db->rec);
         strcat(cdb_db->irec, lbuf);
      }
