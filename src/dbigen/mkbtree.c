/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mkbtree.c,v 9.0 2001/01/09 02:55:38 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Makebtree_sid[] = "@(#) $Id: mkbtree.c,v 9.0 2001/01/09 02:55:38 john Exp $";
#endif

/*
 *  makebtree.c -
 */

#include <fb.h>
#include <fb_ext.h>
#include <igen.h>

extern fb_database *hp;
static makeseq();
static makeidx();
static long first_layer();
static long add_layer();
static setkeys_idx();

/* 
 *  makebtree - assume names are loaded into hp
 */
 
   makebtree(gidict, gindex)
      char *gidict, *gindex;
      
      {
         char seqf_buf[10];

         fb_bseq *bs, *fb_seq_alloc();
         fb_bidx *bi, *fb_idx_alloc();

	 if (!cdb_batchmode){
            fb_scrstat("Generating B+Tree");
            fb_infoline();
            fb_fmessage(NIL);
            }

         /* allocate mirror storage areas */
         bs = fb_seq_alloc(gindex, hp->irecsiz - 1);
         bi = fb_idx_alloc(gindex, hp->irecsiz - 1);
         hp->b_seqtmp = bs;
         hp->b_idxtmp = bi;

         /* allocate regular storage areas */
         bs = fb_seq_alloc(gindex, hp->irecsiz - 1);
         bi = fb_idx_alloc(gindex, hp->irecsiz - 1);
         hp->b_seq = bs;
         hp->b_idx = bi;

         hp->irec = (char *) fb_malloc(hp->irecsiz + 13);

         /* each of bidx and bseq have a header consisting of
          *    SEQF ROOT VAL MAX FREE\n ... (i.e. 4 FB_SLONG FB_SLONG FB_SLONG FB_SLONG\n)
          *    where VAL is either HEIGHT (idx) or TAIL (seq)
          */

         sprintf(seqf_buf, "%04d", fb_getseq(hp->fd));

         /* gen the sequence set - open file descriptors, get the headers */
	 close(creat(bs->bs_name, 0666));
         bs->bs_fd = fb_mustopen(bs->bs_name, READWRITE);
         write(bs->bs_fd, seqf_buf, 4);
         fb_putbxhead(bs->bs_fd, bs->bs_head, bs->bs_tail, bs->bs_reccnt,
            bs->bs_free);
         hp->ifd = fb_mustopen(gindex, READ);
         hp->ihfd = fb_mustopen(gidict, READWRITE);
         /* use the standard index to build the b+tree sequence set */
         fb_getxhead(hp->ihfd, &(hp->bsmax), &(hp->bsend));
         makeseq(hp);
         fb_putbxhead(bs->bs_fd, bs->bs_head, bs->bs_tail, bs->bs_reccnt,
            bs->bs_free);

         /* generate the index set using the sequence set */
	 close(creat(bi->bi_name, 0666));
         bi->bi_fd = fb_mustopen(bi->bi_name, READWRITE);
         write(bi->bi_fd, seqf_buf, 4);
         fb_putbxhead(bi->bi_fd, bi->bi_root, bi->bi_height,
            bi->bi_reccnt, bi->bi_free);
         if (hp->reccnt > 0L){
            makeidx(hp);
            fb_putbxhead(bi->bi_fd, bi->bi_root, bi->bi_height,
               bi->bi_reccnt, bi->bi_free);
            }

         close(bs->bs_fd);
         close(bi->bi_fd);
         close(hp->ifd);

         /* place a marker at end of idict file to signify btree */
         lseek(hp->ihfd, 0L, 2);
         write(hp->ihfd, "%\n", 2);
         close(hp->ihfd);
         hp->ifd = -1;
         hp->ihfd = -1;

         /* remove the normal index */
         unlink(gindex);

         fb_move(20, 1); fb_clrtobot();
         fb_infoline();
      }

/*
 * makeseq - for each indexed record, lay out a sequence set that
 *	is 2/3s full, i.e. two records per fb_node.
 *	process these two at a time.
 *
 *	sequence set is linked list on disk layed out like this:
 *	      PREVSEQ Key1Rec1 Key2Rec2 Key3Rec3 NEXTSEQ
 */

   static makeseq(dp)
      fb_database *dp;

      {
         int max, nper, max_per_node;
         char *p;
         fb_bseq *bs;
         long recno, nextno, prevno, nrec;

         if (!cdb_batchmode){
            fb_scrstat2("Sequence Set");
            fb_refresh();
            }
         max = dp->irecsiz + 2;
         fb_r_init(dp->ifd);
         nper = 1;
         max_per_node = 3;
         bs = dp->b_seq;
         fb_seq_clear(bs);
         recno = 1;
         prevno = 0;
         nextno = 2;

         nrec = 0;
         while (fb_nextline(dp->irec, max) != 0){
            nrec++;
            switch (nper){
               case 1:
                  sprintf(bs->bs_key1, "%s ", dp->irec); break;
               case 2:
                  sprintf(bs->bs_key2, "%s ", dp->irec); break;
               case 3:
                  sprintf(bs->bs_key3, "%s ", dp->irec); break;
               }
            if (nper >= max_per_node){
               /* link this fb_node to others using bsrec numbers */
               bs->bs_prev = prevno;
               if (nrec < dp->bsmax)
                  bs->bs_next = nextno;
               /* write the current bs as record recno */
               fb_seq_putrec(recno, bs);
               bs->bs_reccnt++;

	       if (!cdb_batchmode)
	          fb_gcounter(bs->bs_reccnt);

               nper = 1;
               prevno = recno;
               recno = nextno;
               nextno++;
               fb_seq_clear(bs);
               }
            else
               nper++;
	    }
         
         /* if current fb_node has something in it, write it */
         if (nper > 1){
            /* link this fb_node to others using bsrec numbers */
            bs->bs_prev = prevno;
            bs->bs_next = 0;
            fb_seq_putrec(recno, bs);
            bs->bs_reccnt++;
            }
         bs->bs_head = 1;
         bs->bs_tail = bs->bs_reccnt;
         bs->bs_free = 0L;
         if (!cdb_batchmode)
            fb_scrstat2(NIL);
	 return(FB_AOK);
      }

/*
 * makeidx - generate the index set, each layer is a trie of the layer below
 *	first layer is of the sequence set
 *	second through Nth layers are of the index set itself
 */

   static makeidx(hp)
      fb_database *hp;

      {
         fb_bidx *bi;
         fb_bseq *bs;
         int layer = 1;
         long first_layer(), add_layer(), n, top, bottom;

         if (!cdb_batchmode){
            fb_scrstat2("Index Set");
            fb_refresh();
            }
         bi = hp->b_idx;
         bs = hp->b_seq;
         top = bottom = 1;
         for (layer = 1; ; layer++){
            if (layer == 1){
               if ((n = first_layer(hp)) == 0)
                  break;
               bottom = n;
               }
            else{
               n = add_layer(hp, top, bottom);
               top = bottom + 1;
               bottom = n;
               }
            if (top == bottom){
               bi->bi_root = top;
               bi->bi_height = layer;
               break;
               }
            }
         if (!cdb_batchmode)
            fb_scrstat2(NIL);
      }

/*
 * first_layer - lay out the first layer of the index set
 */

   static long first_layer(hp)
      fb_database *hp;

      {
         fb_bidx *bi;
         fb_bseq *bs;
         long bnext, irec, nseq;
         int counter, st;
         char *q;

         bi = hp->b_idx;
         bs = hp->b_seq;
         nseq = 1;
         counter = 0;
         fb_idx_clear(bi);
         for (irec = 1;;){
            st = fb_seq_getrec(nseq, bs);
            if (st == FB_ERROR)
               break;
            counter++;
            switch(counter){
               case 1:
                  bi->bi_left = nseq;
                  bi->bi_ltype = CHAR_0;
                  fb_rightmost_seq(bi->bi_key1, bi->bi_ksize, bs);
                  break;
               case 2:
                  bi->bi_middle = nseq;
                  bi->bi_mtype = CHAR_0;
                  fb_rightmost_seq(bi->bi_key2, bi->bi_ksize, bs);
                  break;
               case 3:
                  bi->bi_right = nseq;
                  bi->bi_rtype = CHAR_0;
                  break;
               }
            if (counter == 3){
               /* write out the current idx fb_node, reset and go again */
               fb_idx_putrec(irec, bi);
               irec++;
               bi->bi_reccnt++;
               counter = 0;
               fb_idx_clear(bi);
	       if (!cdb_batchmode)
	          fb_gcounter(bi->bi_reccnt);
               }
            if ((nseq = bs->bs_next) == 0)
               break;
            }
         if (counter > 0){
            /* write out any remnants */
            fb_idx_putrec(irec, bi);
            bi->bi_reccnt++;
            }
         return(bi->bi_reccnt);
      }

/*
 * add_layer - add a new layer to the index set, using the top and bottom
 *	of the previous layer
 */

   static long add_layer(hp, top, bottom)
      fb_database *hp;
      long top, bottom;

      {
         fb_bidx *bi_r, *bi_w;
         fb_bseq *bs;
         long bnext, irec, nidx;
         int counter, st;
         char *q;

         bi_r = hp->b_idx;
         bi_w = hp->b_idxtmp;
         fb_idx_copy(bi_w, bi_r);
         bs = hp->b_seq;
         counter = 0;
         fb_idx_clear(bi_w);
         irec = bi_r->bi_reccnt + 1;
         for (nidx = top; nidx <= bottom; nidx++){
            st = fb_idx_getrec(nidx, bi_r);
            if (st == FB_ERROR)
               fb_xerror(FB_MESSAGE, "b+tree idx_getrec fatal error", NIL);
            counter++;
            switch(counter){
               case 1:
                  bi_w->bi_left = nidx;
                  bi_w->bi_ltype = CHAR_1;
                  break;
               case 2:
                  bi_w->bi_middle = nidx;
                  bi_w->bi_mtype = CHAR_1;
                  break;
               case 3:
                  bi_w->bi_right = nidx;
                  bi_w->bi_rtype = CHAR_1;
                  break;
               }
            if (counter == 3){
               setkeys_idx(bi_w, bi_r, bs);
               fb_idx_putrec(irec, bi_w);
               bi_r->bi_reccnt++;
               irec++;
               counter = 0;
               fb_idx_clear(bi_w);
	       if (!cdb_batchmode)
	          fb_gcounter(bi_r->bi_reccnt);
               }
            }
         if (counter > 0){
            /* write out any remnants */
            setkeys_idx(bi_w, bi_r, bs);
            fb_idx_putrec(irec, bi_w);
            bi_r->bi_reccnt++;
            }
         return(bi_r->bi_reccnt);
      }

/*
 * setkeys_idx - for the index there are two keys. determine key set
 *	by following pointers all the way to leaves, and storing values.
 */

   static setkeys_idx(bi_w, bi_r, bs)
      fb_bidx *bi_w, *bi_r;
      fb_bseq *bs;

      {
         char *p;

         /* key1 is the largest value in the left tree */
         p = bi_w->bi_key1;
         if (bi_w->bi_ltype != FB_BLANK)
            fb_largest_key(p, bi_w->bi_ksize, bi_w->bi_left, bi_w->bi_ltype,
               bi_r, bs);

         /* key2 is the largest value in the middle tree */
         p = bi_w->bi_key2;
         if (bi_w->bi_mtype != FB_BLANK)
            fb_largest_key(p, bi_w->bi_ksize, bi_w->bi_middle, bi_w->bi_mtype,
               bi_r, bs);
      }
