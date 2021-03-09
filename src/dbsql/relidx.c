/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: relidx.c,v 9.0 2001/01/09 02:55:50 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Relidx_sid[] = "@(#) $Id: relidx.c,v 9.0 2001/01/09 02:55:50 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * init_rel_index - use the virtual index name
 * 		gen the names of the pieces within relation r
 */

   init_rel_index(qn, r)
      node *qn;
      relation *r;

      {
         char iname[FB_MAXNAME], tname[FB_MAXNAME];
         int i;

         strcpy(iname, qn->n_nval);
         fb_basename(tname, iname);
         sprintf(tname, SYSMSG[S_FMT_2S], iname, SYSMSG[S_EXT_IDX]);
         fb_mkstr(&(r->r_index), tname);
         sprintf(tname, SYSMSG[S_FMT_2S], iname, SYSMSG[S_EXT_IDICT]);
         fb_mkstr(&(r->r_idict), tname);
         qn->n_fval = 1;		/* turn on in-use flag */
         close(creat(r->r_index, 0666));
         r->r_ifd = open(r->r_index, WRITE);
         if (r->r_ifd < 0){
            fb_serror(FB_MESSAGE, "Can't open relation index:", r->r_index);
            return(FB_ERROR);
            }
         close(creat(r->r_idict, 0666));
         r->r_ihfd = open(r->r_idict, WRITE);
         if (r->r_ihfd < 0){
            fb_serror(FB_MESSAGE, "Can't open relation index header:", r->r_idict);
            return(FB_ERROR);
            }
	 fb_w_init(1, r->r_ifd, -1);
         for (i = 0; i < FB_MAXBY; i++)
            vir_by[i] = NULL;
         virtual_count = 0;
         r->r_irecsiz = 0;
         return(FB_AOK);
      }

/*
 * end_rel_index - end of gen of relation index
 *		generate the *.idict file, close write descriptors
 */

   end_rel_index(r)
      relation *r;

      {
         int rcfd, i;
         char seq[FB_SEQSIZE + 1];

         /* generate and write on the index idict file */
	 fb_wflush(1);
         close(r->r_ifd);
	 sprintf(seq, "0000");
         rcfd = r->r_ihfd;
	 lseek(rcfd, FB_SEQSTART, 0);
	 write(rcfd, seq, FB_SEQSIZE);
	 fb_putxhead(rcfd, virtual_count, virtual_count);
         r->r_ireccnt = virtual_count;
         for (i = 0; i < FB_MAXBY; i++)
            if (vir_by[i] != NULL){
	       fb_underscore(vir_by[i]->id, 1);	/* fb_put back _ before write */
	       fb_mustwrite(rcfd, vir_by[i]->id);
	       fb_mustwrite(rcfd, "\n");
               }
         close(rcfd);
      }

/* 
 *  enter_rel_index - output the vir_by fields from buf [copied from dbigen]
 *	into the relation r
 */
 
   enter_rel_index(r)
      relation *r;
      
      {
         register i;
	 char buf[FB_MAXLINE];
	 int siz, tsiz = 0;
	 
         for (i = 0; i < FB_MAXBY && vir_by[i] != NULL; i++){
	    siz = vir_by[i]->size;
	    if (vir_by[i]->type ==FB_DATE){
	       if (cdb_datedisplay == 8){
	          fb_endate(vir_by[i]->fld);
		  strcpy(cdb_bfld, vir_by[i]->fld);
		  }
	       else{
	          siz += 2;
	          fb_longdate(buf, vir_by[i]->fld);
		  fb_long_endate(buf);
		  strcpy(cdb_bfld, buf);
	          }
	       }
	    else
	       strcpy(cdb_bfld, vir_by[i]->fld);
	    if (FB_OFNUMERIC(vir_by[i]->type))
	       fb_rjustify(cdb_afld, cdb_bfld, siz, vir_by[i]->type);
	    else
	       fb_pad(cdb_afld, cdb_bfld, siz);
	    fb_nextwrite(0, cdb_afld);
            tsiz += siz;
	    }
         sprintf(cdb_afld, "%010ld", r->r_curp);
	 fb_nextwrite(0, cdb_afld);
	 fb_w_write(0, "\n");
         virtual_count++;
         if (r->r_irecsiz == 0)
            r->r_irecsiz = tsiz;
      }

/*
 * openidx_relation - this is like an openidx, but for relations.
 */

   openidx_relation(r)
      relation *r;

      {
         /* assume all names have been left intact from creation */
         r->r_ifd = open(r->r_index, READ);
         if (r->r_ifd < 0){
            fb_serror(FB_MESSAGE, "Can't open relation index:", r->r_index);
            return(FB_ERROR);
            }
         r->r_ihfd = open(r->r_idict, READ);
         if (r->r_ihfd < 0){
            fb_serror(FB_MESSAGE, "Can't open relation index header:", r->r_idict);
            return(FB_ERROR);
            }
         r->r_irecsiz += 11;
         r->r_irec = (char *) fb_malloc(r->r_irecsiz + 13);
         return(FB_AOK);
      }

/*
 * groupeach_relation - this is like whereeach except it is done for
 *	records that match the curent group_value only.
 *
 *	this is groupeach, but twisted for relations.
 */

   groupeach_relation(r, f)
      relation *r;
      int (*f)();

      {
         int isize;

	 lseek(r->r_ifd, 0L, 0);
         getxrec_relation(group_value, r);
         isize = r->r_irecsiz - 11;
         for (;;){
            /* assume record is loaded */
            if (((*f)(r)) == FB_ERROR)
               return(FB_ERROR);
            if (nextxrec_relation(r) != FB_AOK)
               break;
            if (strncmp(group_value, r->r_irec, isize) != 0)
               break;
            }
	  return(FB_AOK);
      }

/*
 *  getxrec_relation - force a relation with a particular relation index entry
 *	to be loaded into r or return FB_ERROR.
 */

   getxrec_relation(s, r)
      char *s;
      relation *r;
      
      {
         long rec, fb_megasearch();
	 int st = FB_ERROR;
	 
	 rec = fb_megasearch(r->r_ifd, s, 0, 1L, r->r_ireccnt, r->r_ireccnt,
            r->r_irecsiz, 1, r->r_irec);
	 if (rec > 0 && rec <= r->r_ireccnt){
            rec = atol((char *) 
	       (r->r_irec + r->r_irecsiz - 11));	/* FB_RECORDPTR + 1 */
	    if (rec >= 0 && rec < r->r_reccnt){
	       getrec_relation(rec, r);
               getrec_loadrel(r);
               st = FB_AOK;
               }
	    }
	 return(st);
      }

/*
 *  nextxrec_relation - force the next indexed relation to be loaded.
 */

   nextxrec_relation(r)
      relation *r;
      
      {
         long rec;
	 int st = FB_ERROR;
	 
         while (read(r->r_ifd, r->r_irec, r->r_irecsiz) == r->r_irecsiz){
            rec = atol((char *) (r->r_irec + r->r_irecsiz - 11));
            if (rec >= 0L && rec < r->r_reccnt){
	       getrec_relation(rec, r);
               getrec_loadrel(r);
	       st = FB_AOK;
	       break;
	       }
            /*
	    fb_serror(FB_MESSAGE, "Looping over relation index", NIL);
            */
	    }
	  return(st);
      }
