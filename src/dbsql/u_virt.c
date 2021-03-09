/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: u_virt.c,v 9.1 2001/02/16 19:45:36 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char U_virtual_sid[] = "@(#) $Id: u_virt.c,v 9.1 2001/02/16 19:45:36 john Exp $";
#endif

#include "dbsql_e.h"

extern long cdb_headsize;

/*
* u_virtual - routines to do virtual object creation/destruction
*/

/*
 * u_vir_index - scan the list of virtual objects for any unused (c_fval==0)
 *	if list is empty or all in use, gen another name, link in place.
 *	always return a cell * with an index name in c_nval;
 */

   node *u_vir_index()
      {
         node *n, *makenode();
         char iname[FB_MAXNAME];

         for (n = vir_index; n != NULL; n = n->n_virlist)
            if (n->n_fval == 0)
               break;
         if (n == NULL){
            n = makenode();
            n->n_tval |= T_VIR;
            n->n_virlist = vir_index;
            vir_index = n;
            sprintf(iname, "%s/FXXXXXX", sql_tempdir);
            /* the following file creation keeps the mkstemp template unique */
            close(mkstemp(iname));
            fb_mkstr(&(n->n_nval), iname);
            }
         return(n);
      }

/*
 * u_vir_dbase - scan the list of virtual objects for any unused (c_fval==0)
 *	if list is empty or all in use, gen another name, link in place.
 *	always return a cell * with an fb_database name in c_nval;
 */

   node *u_vir_dbase()
      {
         node *n, *makenode();
         char dname[FB_MAXNAME];

         for (n = vir_dbase; n != NULL; n = n->n_virlist)
            if (n->n_fval == 0)
               break;
         if (n == NULL){
            n = makenode();
            n->n_tval |= T_VIR;
            n->n_virlist = vir_dbase;
            vir_dbase = n;
            sprintf(dname, "%s/FXXXXXX", sql_tempdir);
            /* the following file creation keeps the mkstemp template unique */
            close(mkstemp(dname));
            fb_mkstr(&(n->n_nval), dname);
            }
         return(n);
      }

/*
 * u_vir_remove - loop over the virtual objects and remove them
 */

   u_vir_remove()
      {
         char tname[FB_MAXNAME], *iname, *fname;
         node *n, *nn;

         for (n = vir_index; n != NULL; n = nn){
            nn = n->n_virlist;
            iname = n->n_nval;
            unlink(iname);
	    sprintf(tname, SYSMSG[S_FMT_2S], iname, SYSMSG[S_EXT_IDX]);
            unlink(tname);
	    sprintf(tname, SYSMSG[S_FMT_2S], iname, SYSMSG[S_EXT_IDICT]);
            unlink(tname);
            /* fb_free((char *) n->n_nval); */
            /* fb_free(n); -- done via garbage collection instead */
            }
         vir_index = NULL;
         for (n = vir_dbase; n != NULL; n = nn){
            nn = n->n_virlist;
            fname = n->n_nval;
            unlink(fname);
	    sprintf(tname, SYSMSG[S_FMT_2S], fname, SYSMSG[S_EXT_CDB]);
            unlink(tname);
	    sprintf(tname, SYSMSG[S_FMT_2S], fname, SYSMSG[S_EXT_DDICT]);
            unlink(tname);
	    sprintf(tname, SYSMSG[S_FMT_2S], fname, SYSMSG[S_EXT_MAP]);
            unlink(tname);
	    sprintf(tname, SYSMSG[S_FMT_2S], fname, SYSMSG[S_EXT_LOG]);
            unlink(tname);
            /* fb_free((char *) n->n_nval); */
            /* fb_free(n); done via garbage collection instead */
            }
         vir_dbase = NULL;
      }

/*
 * init_vir_index - use the virtual index name
 * 		gen the names of the pieces within dp
 */

   init_vir_index(qn, dp)
      node *qn;
      fb_database *dp;

      {
         char iname[FB_MAXNAME];
         int i;

         strcpy(iname, qn->n_nval);
         fb_dbargs(NIL, iname, dp);
         qn->n_fval = 1;		/* turn on in-use flag */
         close(creat(dp->dindex, 0666));
         dp->ifd = open(dp->dindex, WRITE);
         if (dp->ifd < 0){
            fb_serror(FB_MESSAGE, "Could not open index:", dp->dindex);
            return(FB_ERROR);
            }
         close(creat(dp->idict, 0666));
         dp->ihfd = open(dp->idict, WRITE);
         if (dp->ihfd < 0){
            fb_serror(FB_MESSAGE, "Could not open index header:", dp->idict);
            return(FB_ERROR);
            }
	 fb_w_init(1, dp->ifd, -1);
         for (i = 0; i < FB_MAXBY; i++)
            vir_by[i] = NULL;
         vir_by[0] = dp->kp[0];
         return(FB_AOK);
      }

/*
 * end_vir_index - end of gen of distinct (virtual) index
 *		generate the *.idict file, close write descriptors
 */
   end_vir_index(dp)
      fb_database *dp;

      {
         int rcfd, i;
         char seq[FB_SEQSIZE + 1];

         /* generate and write on the index idict file */
	 fb_wflush(1);
         close(dp->ifd);
	 sprintf(seq, "%04d", fb_getseq(dp->fd));
         rcfd = dp->ihfd;
	 lseek(rcfd, FB_SEQSTART, 0);
	 write(rcfd, seq, FB_SEQSIZE);
	 fb_putxhead(rcfd, virtual_count, virtual_count);
         for (i = 0; i < FB_MAXBY; i++)
            if (vir_by[i] != NULL){
	       fb_underscore(vir_by[i]->id, 1);	/* fb_put back _ before write */
	       fb_mustwrite(rcfd, vir_by[i]->id);
	       fb_mustwrite(rcfd, "\n");
               }
         close(rcfd);
      }

/* 
 *  vir_enter - output the vir_by fields from buf [copied from dbigen]
 */
 
   vir_enter(dp)
      fb_database *dp;
      
      {
         register i;
	 char buf[FB_MAXLINE];
	 int siz;
	 
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
	    }
         sprintf(cdb_afld, "%010ld", dp->rec);
	 fb_nextwrite(0, cdb_afld);
	 fb_w_write(0, "\n");
         virtual_count++;
      }

/*
 * init_vir_dbase - use the virtual dbase name
 * 		gen the names of the pieces
 */

   static fb_database *init_vir_dbase(dn)
      node *dn;

      {
         char dname[FB_MAXNAME];
         fb_database *dvp, *fb_dballoc();

         strcpy(dname, dn->n_nval);
         dvp = fb_dballoc();
         fb_dbargs(dname, NIL, dvp);
         dn->n_fval = 1;			/* turn on in-use flag */
         
	 close(creat(dvp->dbase, 0666));	/* create the dbase */
         dvp->fd = fb_mustopen(dvp->dbase, 1);
	 fb_putseq(dvp->fd);
	 fb_puthead(dvp->fd, 0L, 0L);

	 close(creat(dvp->dmap, 0666));		/* make the map */
	 dvp->mfd = fb_mustopen(dvp->dmap, READWRITE);
	 if (fb_bootmap(dvp->mfd) == FB_ERROR)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_MAP], dvp->dmap);

	 fb_w_init(2, dvp->fd, dvp->mfd);	/* init the buffer system */
         dn->n_p1 = (int) dvp;
         return(dvp);
      }

/*
 * set_virtual_dbase - initialize the variables needed for virtual dbases
 */

   set_virtual_dbase()
      {
         create_virtual = 1;
         vn = u_vir_dbase();
         vp = init_vir_dbase(vn);
         create_ddict = 1;
         rpos = (long) cdb_headsize;
         wcount = 0;
      }

/*
 * end_virtual_dbase - flush the buffers, write headers for virtual dbases
 */

   end_virtual_dbase()
      {
         int fd, fnum, t;
         node *n;
         fb_field *f;
         char *p, buf[FB_MAXLINE];
         
         fb_wflush(2);
         fb_puthead(vp->fd, wcount, 0L);
         fb_putmap(vp->mfd, 0L, rpos, 0L, 0L);
         fb_w_end(2);
         g_order_by = NULL;
         create_virtual = 0;

         if (wcount == 0){
            /*
             * special case - the ddict file for vitual has not been
             * output since no records were written.
             * So, try and output it now.
             */
	    close(creat(vp->ddict, 0666));	/* create the ddict */
            fd = fb_mustopen(vp->ddict, 1);
            for (n = g_slist, fnum = 1; n != NULL; n = n->n_list, fnum++){
               expr(n);
               if (istype_fld(n)){
                  f = (fb_field *) n->n_p2;
                  if (f->type == FB_FORMULA)
                     t = FB_FLOAT;
                  else if (f->type == FB_LINK)
                     t = FB_ALPHA;
                  else
                     t = f->type;
                  }
               else if (istype_str(n))
                  t = FB_ALPHA;
               else
                  t = FB_FLOAT;
               strcpy(buf, n->n_nval);
               if (strlen(buf) > 10)
                  buf[10] = NULL;
               for (p = buf; *p; p++)
                  if (*p == FB_BLANK)
                     *p = FB_UNDERSCORE;
               if (buf[0] == NULL)
                  sprintf(buf, "Field%d", fnum);
               sprintf(buf, "%s %c %d", buf, t, n->n_width);
               if (n->n_scale > 0)
                  sprintf(buf, "%s -d :%d", buf, n->n_scale);
               strcat(buf, "\n");
               write(fd, buf, strlen(buf));
               }
            close(fd);
            }
      }
