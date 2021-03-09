/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: getauto.c,v 9.0 2001/01/09 02:56:48 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Getauto_sid[] = "@(#) $Id: getauto.c,v 9.0 2001/01/09 02:56:48 john Exp $";
#endif

/*
 *  initialize and/or create autoindex files
 */

#include <fb.h>
#include <fb_ext.h>

static char *FMT = "$\n%s $ALL\n%%\n%s\n";
extern short int cdb_autobtree;
extern short int cdb_datedisplay;
extern short int cdb_returnerror;

#if FB_PROTOTYPES
static get_ixauto(fb_database *dp, char *seq, int mode);
static getd_auto(fb_database *dp, char *fname);
#else
static get_ixauto();
static getd_auto();
#endif /* FB_PROTOTYPES */

   fb_getauto(dp, mode)
      fb_database *dp;
      int mode;
      
      {
         int i, isiz, st;
	 char autofile[FB_MAXNAME], aname[FB_MAXNAME], seq[FB_SEQSIZE+1];
         char buf[FB_MAXLINE];
	 FILE *ifs;
         fb_autoindex *ix;
         fb_bseq *bs;
         fb_bidx *bi;
	 
         sprintf(seq, SYSMSG[S_FMT_04D], dp->sequence);
	 for (i = 0; i < dp->nfields; i++){
            ix = dp->kp[i]->aid;
	    if (ix != NULL && ix->autoname != NULL && ix->autoname[0] != NULL){
               fb_dirname(autofile, dp->dbase);
               strcat(autofile, ix->autoname);
               strcpy(aname, autofile);

	       /* duplicate fb_field area */
               isiz = dp->kp[i]->size + 13;
	       ix->dup_fld = (char *) fb_malloc((unsigned) isiz); /*recnum*/
	       ix->dup_fld[0] = NULL;

	       /* open/create autoindex idicti file if needed */
	       strcpy(autofile, aname);
	       strcat(autofile, SYSMSG[S_EXT_IDICTI]);
	       if (access(autofile, 0) != 0){
                  if (mode == READ)
                     return(FB_ERROR);
	          ifs = fb_mustfopen(autofile, FB_F_WRITE);
                  if (ifs == (FILE *) 0)
                     return(FB_ERROR);
		  fprintf(ifs, FMT, dp->kp[i]->id, dp->kp[i]->id);
                  if (cdb_autobtree){
		     fprintf(ifs, "%%\n");
                     ix->ix_tree = 1;
                     }
		  fclose(ifs);
		  }

	       /* open/create autoindex idict file */
	       strcpy(autofile, aname);
	       strcat(autofile, SYSMSG[S_EXT_IDICT]);
	       ix->hfd = open(autofile, mode);
	       if (ix->hfd < 0){
                  if (access(autofile, 0) == 0)
                     return(FB_ERROR);
                  if (mode == READ)
                     return(FB_ERROR);
		  close(creat(autofile, 0666));
		  ix->hfd = fb_mustopen(autofile,READWRITE);
                  if (ix->hfd < 0)
                     return(FB_ERROR);
		  lseek(ix->hfd, FB_SEQSTART, 0);
		  write(ix->hfd, seq, FB_SEQSIZE);
		  fb_putxhead(ix->hfd, 0L, 0L);
		  fb_mustwrite(ix->hfd, dp->kp[i]->id);
		  fb_mustwrite(ix->hfd, SYSMSG[S_STRING_NEWLINE]);
                  if (cdb_autobtree){
		     fb_mustwrite(ix->hfd, "%\n");
                     ix->ix_tree = 1;
                     }
		  /* to help with mutual exclusion ... */
		  close(ix->hfd);
		  ix->hfd = fb_mustopen(autofile,READWRITE);
		  }
	       /* otherwise check for proper SEQF on autoindex */
	       else if (dp->sequence != fb_getseq(ix->hfd)){
		  fb_free(ix->dup_fld);
		  fb_free((char *) (ix));
		  ix = NULL;
		  fb_serror(FB_WRONG_INDEX, autofile, NIL);
		  continue;
		  }
               else{
                  /* test for btree-ness */
                  fb_r_init(ix->hfd);
                  fb_getxhead(ix->hfd, &(ix->ix_bsmax), &(ix->ix_bsend));
                  for(; fb_nextline(buf, FB_MAXLINE) != 0; )
                     if (buf[0] == CHAR_PERCENT)
                        ix->ix_tree = 1;
                  }

               /*
                * test for flat index existence by looking for .idx file
                */
               strcpy(autofile, aname);
               strcat(autofile, SYSMSG[S_EXT_IDX]);

	       /* open/create autoindex data files */
               if (access(autofile, 0) != 0 &&
                     (cdb_autobtree || ix->ix_tree)){ /* btree */
                  /* open/create btree idx and seq files - alloc structs */
                  ix->ix_seq = fb_seq_alloc(aname, dp->kp[i]->size +
                     FB_RECORDPTR);
                  ix->ix_idx = fb_idx_alloc(aname, dp->kp[i]->size +
                     FB_RECORDPTR);
                  ix->ix_seqtmp = fb_seq_alloc(aname, dp->kp[i]->size +
                     FB_RECORDPTR);
                  ix->ix_idxtmp = fb_idx_alloc(aname, dp->kp[i]->size +
                     FB_RECORDPTR);
                  /* sequence set */
                  bs = ix->ix_seq;
                  bs->bs_fd = open(bs->bs_name, mode);
                  if (bs->bs_fd < 0){
                     if (access(bs->bs_name, 0) == 0)
                        return(FB_ERROR);
                     if (mode == READ)
                        return(FB_ERROR);
                     close(creat(bs->bs_name, 0666));
                     bs->bs_fd = fb_mustopen(bs->bs_name, READWRITE);
                     if (bs->bs_fd < 0)
                        return(FB_ERROR);
                     write(bs->bs_fd, seq, 4);
                     fb_putbxhead(bs->bs_fd, bs->bs_head, bs->bs_tail,
                        bs->bs_reccnt, bs->bs_free);
                     }
                  fb_getbxhead(bs->bs_fd, &(bs->bs_head), &(bs->bs_tail),
                     &(bs->bs_reccnt), &(bs->bs_free));

                  /* index set */
                  bi = ix->ix_idx;
                  bi->bi_fd = open(bi->bi_name, mode);
                  if (bi->bi_fd < 0){
                     if (access(bi->bi_name, 0) == 0)
                        return(FB_ERROR);
                     if (mode == READ)
                        return(FB_ERROR);
                     close(creat(bi->bi_name, 0666));
                     bi->bi_fd = fb_mustopen(bi->bi_name, READWRITE);
                     if (bi->bi_fd < 0)
                        return(FB_ERROR);
                     write(bi->bi_fd, seq, 4);
                     fb_putbxhead(bi->bi_fd, bi->bi_root, bi->bi_height,
                        bi->bi_reccnt, bi->bi_free);
                     }
                  fb_getbxhead(bi->bi_fd, &(bi->bi_root), &(bi->bi_height),
                     &(bi->bi_reccnt), &(bi->bi_free));
                  ix->ix_tree = 1;
                  fb_seq_copy(ix->ix_seqtmp, ix->ix_seq);
                  fb_idx_copy(ix->ix_idxtmp, ix->ix_idx);
                  ix->ix_ip =
                     (fb_field **) fb_malloc((2) * (sizeof(fb_field *)));
                  ix->ix_ip[0] = dp->kp[i];
                  ix->ix_ip[1] = (fb_field *) fb_malloc(sizeof(fb_field));
                  ix->ix_ip[1]->size = 10;
                  ix->ix_ip[1]->type = CHAR_n;
                  ix->ix_ip[1]->loc = dp->kp[i]->size;
                  ix->ix_ip[1]->id = NIL;
                  ix->ix_ifields = 1;
                  /* additional duplicate fb_field area for building keys */
							/* 13 here is fudge*/
                  ix->ix_key_fld = (char *) fb_malloc((unsigned) isiz + 13);
                  ix->ix_key_fld[0] = NULL;
                  }
               else{					/* normal FB index */
                  /* open/create autoindex index file */
	          strcpy(autofile, aname);
                  strcat(autofile, SYSMSG[S_EXT_IDX]);
                  ix->afd = open(autofile, mode);
                  if (ix->afd < 0){
                     if (access(autofile, 0) == 0)
                        return(FB_ERROR);
                     if (mode == READ)
                        return(FB_ERROR);
                     close(creat(autofile, 0666));
                     ix->afd = fb_mustopen(autofile, READWRITE);
                     if (ix->afd < 0)
                        return(FB_ERROR);
                     }
                  }
	       }
            }
         st = get_ixauto(dp, seq, mode);
         if (st == FB_ERROR){
	    fb_rootname(aname, dp->ddict);
            strcat(aname, ".auto");
            if (cdb_returnerror)
               fb_lerror(FB_BAD_DICT, aname, NIL);
            else
               fb_serror(FB_BAD_DICT, aname, NIL);
            }
         return(st);
      }

/*
 * init/open the dbase.auto set of autoindex structures
 */

   static get_ixauto(dp, seq, mode)
      fb_database *dp;
      char *seq;
      int mode;

      {
         char fname[FB_MAXNAME];
         int i;

	 fb_rootname(fname, dp->ddict);
         strcat(fname, ".auto");
         if (access(fname, 0) < 0)
            return(FB_AOK);

         if (getd_auto(dp, fname) == FB_ERROR)
            return(FB_ERROR);
         /*
          * names are in place ... now init each index
          * --- idicti must exist
          * --- index is a btree only
          */
         for (i = 0; i < dp->b_maxauto; i++)
            if (fb_open_auto(dp, dp->b_autoindex[i], seq, mode) == FB_ERROR)
               return(FB_ERROR);
         return(FB_AOK);
      }

/*
 * getd_auto - get a dbase.auto file of names for autoindexes
 */

   static getd_auto(dp, fname)
      fb_database *dp;
      char *fname;

      {
         char buf[FB_MAXLINE], word1[FB_MAXLINE], word2[FB_MAXLINE];
         int fd, i, j;
         fb_autoindex *ax;

         fd = open(fname, READ);
         if (fd <= 0){
            if (cdb_returnerror)
               fb_lerror(FB_CANT_OPEN, fname, NIL);
            else
               fb_serror(FB_CANT_OPEN, fname, NIL);
            return(FB_ERROR);
            }

         if (fd > 0){
            i = fb_ln_init(fd);
            dp->b_maxauto = i;
            dp->b_autoindex = (fb_autoindex **)
               fb_malloc((dp->b_maxauto + 1) * sizeof(fb_autoindex *));
            for (i = 0; i < dp->b_maxauto; i++){
               if (fb_ln_load(buf, FB_MAXLINE) == 0)
                  break;
               word1[0] = word2[0] = NULL;
               j = fb_getword(buf, 1, word1);
               fb_getword(buf, j, word2);
               dp->b_autoindex[i] = fb_ixalloc();
               ax = dp->b_autoindex[i];
	       fb_mkstr(&(ax->autoname), word1);
               if (word2[0] == CHAR_u ||
                     (word2[0] == CHAR_MINUS && word2[1] == CHAR_u))
                  ax->uniq = 1;
               }
            fb_ln_end();
            }
         return(FB_AOK);
      }

/*
 * fb_open_auto - open a single auto index
 *	- must have an idicti
 *	- allocate and init the ix_keyindx array
 *	- init/open the btree bseq/bidx structures
 */

   fb_open_auto(dp, ax, seq, mode)
      fb_database *dp;
      fb_autoindex *ax;
      char *seq;
      int mode;

      {
         char idicti[FB_MAXNAME], idict[FB_MAXNAME], flist[FB_MAXLINE];
         char idx[FB_MAXNAME];
         char fname[FB_MAXNAME], id[FB_MAXNAME], aname[FB_MAXNAME];
         char buf[FB_MAXLINE];
         int rcfd, isiz, nfs, loc, p;
         fb_bseq *bs;
         fb_bidx *bi;

         fb_dirname(aname, dp->dbase);
         strcat(aname, ax->autoname);
         strcpy(idx, aname);
         strcat(idx, SYSMSG[S_EXT_IDX]);
	 if (access(idx, 0) == 0){
            if (cdb_returnerror)
               fb_lerror(FB_WRONG_INDEX, aname, NIL);
            else
               fb_serror(FB_WRONG_INDEX, aname, NIL);
            return(FB_ERROR);
            }
         strcpy(idicti, aname);
         strcat(idicti, SYSMSG[S_EXT_IDICTI]);
         strcpy(idict, aname);
         strcat(idict, SYSMSG[S_EXT_IDICT]);

         if (access(idicti, READ) < 0){
            if (cdb_returnerror)
               fb_lerror(FB_BAD_DICT, idicti, NIL);
            else
               fb_serror(FB_BAD_DICT, idicti, NIL);
            return(FB_ERROR);
            }
         if (access(idict, READ) < 0){
            /*
             * must gen the idict file here using the idicti file
             * start by getting line from idicti that has all the field names
             */
            if (mode == READ)
               return(FB_ERROR);
            rcfd = fb_mustopen(idicti, READ);
            if (rcfd < 0)
               return(FB_ERROR);
            fb_r_init(rcfd);
            for (;;){
               if (fb_nextline(flist, FB_MAXLINE) == 0)
                  return(FB_ERROR);
               if (equal(flist, "%"))
                  break;
               }
            if (fb_nextline(flist, FB_MAXLINE) == 0){
               if (cdb_returnerror)
                  fb_lerror(FB_BAD_DICT, idicti, NIL);
               else
                  fb_serror(FB_BAD_DICT, idicti, NIL);
               return(FB_ERROR);
               }
            close(rcfd);

            /* generate the idict file */
            close(creat(idict, 0666));
            ax->hfd = fb_mustopen(idict, READWRITE);
            if (ax->hfd < 0)
               return(FB_ERROR);
            lseek(ax->hfd, FB_SEQSTART, 0);
            write(ax->hfd, seq, FB_SEQSIZE);
            fb_putxhead(ax->hfd, 0L, 0L);
            for (p = 1; (p = fb_getword(flist, p, fname)) > 0; ){
               strcat(fname, SYSMSG[S_STRING_NEWLINE]);
               fb_mustwrite(ax->hfd, fname);
               }
            fb_mustwrite(ax->hfd, "%\n");
            close(ax->hfd);
            }

         ax->hfd = open(idict, mode);
         if (ax->hfd < 0){
            if (cdb_returnerror)
               fb_lerror(FB_CANT_OPEN, idict, NIL);
            else
               fb_serror(FB_CANT_OPEN, idict, NIL);
            return(FB_ERROR);
            }
         if (fb_getxhead(ax->hfd, &(ax->ix_bsmax), &(ax->ix_bsend))==FB_ERROR){
            if (cdb_returnerror)
               fb_lerror(FB_BAD_DATA, idict, NIL);
            else
               fb_serror(FB_BAD_DATA, idict, NIL);
            return(FB_ERROR);
            }
         nfs = fb_ln_init(ax->hfd);
         fb_ln_get(nfs, buf, FB_MAXLINE);
         if (buf[0] == CHAR_PERCENT)
            nfs--;
         ax->ix_ifields = nfs;

         /*
          * nfs is now set to number of index fields.
          * use this count to allocate space in the ix_ip area
          * additionally, each element must be set to point in db->kp
          */

         ax->ix_ip = (fb_field **) fb_malloc((nfs+1) * (sizeof(fb_field *)));
         if (fb_getseq(ax->hfd) != dp->sequence){
            if (cdb_returnerror)
               fb_lerror(FB_WRONG_INDEX, aname, NIL);
            else
               fb_serror(FB_WRONG_INDEX, aname, NIL);
            return(FB_ERROR);
            }
         for(loc = p = nfs = 0; fb_ln_load(buf, FB_MAXLINE) != 0; ){
            nfs++;
            fb_getword(buf, 1, id);
            if ((ax->ix_ip[p] = fb_findfield(id, dp)) != NULL){
               loc += (ax->ix_ip[p]->size);
               if (ax->ix_ip[p]->type == FB_DATE && cdb_datedisplay == 10)
                  loc += 2;		/* century space */
               p++;
               }
            else if (equal(id, "%"))
               break;
            else{
               if (cdb_returnerror)
                  fb_lerror(FB_BAD_DICT, aname, id);
               else
                  fb_serror(FB_BAD_DICT, aname, id);
               return(FB_ERROR);
               }
            }

         /* record pointer 'field' - FB_RECORDPTR */
         ax->ix_ip[p] = (fb_field *) fb_malloc(sizeof(fb_field));
         ax->ix_ip[p]->size = 10;
         ax->ix_ip[p]->type = CHAR_n;
         ax->ix_ip[p]->loc = loc;
         ax->ix_ip[p]->id = NIL;
         /* isiz has index size (key size) for the btrees below */
         isiz = (loc + ax->ix_ip[p]->size);  /* 10 = FB_RECORDPTR == keysize */

         /* open/create btree idx and seq files - alloc structs */
         ax->ix_seq = fb_seq_alloc(aname, isiz);
         ax->ix_idx = fb_idx_alloc(aname, isiz);
         ax->ix_seqtmp = fb_seq_alloc(aname, isiz);
         ax->ix_idxtmp = fb_idx_alloc(aname, isiz);
         /* duplicate fb_field area */
         ax->dup_fld = (char *) fb_malloc((unsigned) isiz + 13); /*fudge*/
         ax->dup_fld[0] = NULL;
         ax->ix_key_fld = (char *) fb_malloc((unsigned) isiz + 13); /*fudge*/
         ax->ix_key_fld[0] = NULL;
         /* sequence set */
         bs = ax->ix_seq;
         bs->bs_fd = open(bs->bs_name, mode);
         if (bs->bs_fd < 0){
            if (mode == READ)
               return(FB_ERROR);
	    if (access(bs->bs_name, 0) == 0)
               return(FB_ERROR);
            close(creat(bs->bs_name, 0666));
            bs->bs_fd = fb_mustopen(bs->bs_name, READWRITE);
            if (bs->bs_fd < 0)
               return(FB_ERROR);
            write(bs->bs_fd, seq, 4);
            fb_put_seq_head(bs);
            }
         fb_get_seq_head(bs);

         /* index set */
         bi = ax->ix_idx;
         bi->bi_fd = open(bi->bi_name, mode);
         if (bi->bi_fd < 0){
            if (mode == READ)
               return(FB_ERROR);
	    if (access(bi->bi_name, 0) == 0)
               return(FB_ERROR);
            close(creat(bi->bi_name, 0666));
            bi->bi_fd = fb_mustopen(bi->bi_name, READWRITE);
            if (bi->bi_fd < 0)
               return(FB_ERROR);
            write(bi->bi_fd, seq, 4);
            fb_put_idx_head(bi);
            }
         fb_get_idx_head(bi);
         ax->ix_tree = 1;
         fb_seq_copy(ax->ix_seqtmp, ax->ix_seq);
         fb_idx_copy(ax->ix_idxtmp, ax->ix_idx);

         /* free up the loadline stuff */
         fb_ln_free();

         return(FB_AOK);
      }
