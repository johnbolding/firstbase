/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: btree_in.c,v 9.0 2001/01/09 02:56:56 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Btree_insert_sid[] = "@(#) $Id: btree_in.c,v 9.0 2001/01/09 02:56:56 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#define LEFT -1
#define MIDDLE 0
#define RIGHT 1

extern short int cdb_topstack;
static char *promote_key = NULL;

#if FB_PROTOTYPES
static bi_locate(long irec, char *key, fb_bidx *bi);
static bi_insert(char *key, fb_bidx *bi, fb_bidx *bi_tmp, int path,
   fb_bseq *bs, fb_bseq *bs_tmp);
static bi_insert_ancestor(fb_bidx *bi, fb_bidx *bi_tmp, long lval, long rval);
static spread_keys(fb_bseq *ls, fb_bseq *rs, char *key);
static bs_optimize(fb_bidx *bi, fb_bseq *bs, char *key, int path, long srec,
   fb_bidx *bi_tmp, fb_bseq *bs_tmp);
static bs_insert(fb_bidx *bi, fb_bseq *bs, char *key, int path,
   fb_bidx *bi_tmp, fb_bseq *bs_tmp);
static bi_optimize(fb_bidx *bi, long lval, long rval, char *key);
static bs_rearrange(fb_bidx *bi, fb_bseq *bs, char *key,
   fb_bseq *bs_tmp);
static fb_btree_create(fb_bseq *bs, fb_bidx *bi, char *key, fb_autoindex *ix);
static bs_reclaim(fb_bseq *bs, char *key, fb_bseq *bs_tmp);
#else /* FB_PROTOTYPES */
static bi_locate();
static bi_insert();
static bi_insert_ancestor();
static spread_keys();
static bs_optimize();
static bs_insert();
static bi_optimize();
static bs_rearrange();
static bs_reclaim();
static bs_reclaim();
static fb_btree_create();
extern long fb_locate_rightmost_seq();
extern long fb_pop();
extern long fb_key_record();
#endif /* FB_PROTOTYPES */

/*
 * btree_insert - add a key to the btree auto index ix, in fb_database hp
 */

   fb_btree_insert(ix, key, hp)
      fb_autoindex *ix;
      char *key;		/* key is afld in the hp area */
      fb_database *hp;

      {
         fb_bseq *bs, *bs_tmp;
         fb_bidx *bi, *bi_tmp;
         int path, st;

         bs = ix->ix_seq;
         bs_tmp = ix->ix_seqtmp;
         bi = ix->ix_idx;
         bi_tmp = ix->ix_idxtmp;
         if (promote_key != NULL){
            fb_free(promote_key);
            promote_key = NULL;
            }
         fb_get_idx_head(bi);
         fb_get_seq_head(bs);
         /* clear the stack - used to store tree path */
         fb_initstack();
         /* add a blank to the key for the DEL marker */
         strcat(key, " ");

         /*
          * if root == 0: special case: - build a one element tree and return.
          */
         if (bi->bi_root == 0)
            return(fb_btree_create(bs, bi, key, ix));
         /* bi_locate sets bi and returns path where key needs inserting */
         path = bi_locate(bi->bi_root, key, bi);
         /* bi_insert inserts the key with bi as focal node */
         st = bi_insert(key, bi, bi_tmp, path, bs, bs_tmp);
         if (st == FB_AOK){
	    /* get bsmax,bsend from index header file */
            st = fb_getxhead(ix->hfd, &ix->ix_bsmax, &ix->ix_bsend);
	    /* redo header -- note: for btrees, both are shown as changed */
            ix->ix_bsmax++; ix->ix_bsend++;
	    st = fb_putxhead(ix->hfd, ix->ix_bsmax, ix->ix_bsend);
            }

         /* zero out dup key - if applicable  - in ix->dup_fld */
         if (st == FB_AOK && ix->dup_fld[0] != NULL)
            fb_btree_delete(ix->dup_fld, hp->rec, bi, bs);

         return(st);
      }

/*
 * bi_locate - the algorithm to locate the index node where
 *    key belongs. this was recursive, but it was tail recursion,
 *    so i removed it. this returns a path: LEFT,MIDDLE,RIGHT
 */

   static bi_locate(irec, key, bi)
      long irec;
      char *key;
      fb_bidx *bi;

      {
         int klen, path;
         long rec;
         char typ;

         /*
          * since this is insertion, the key has a record number on
          * it too ... this should not get in the way of the comparison,
          * but it needs to be there, so instead of setting 
          * klen = strlen(key), set it to bi->bi_ksize
          */

         klen = bi->bi_ksize;
         for (;;){
            /* get the index record irec */
            if (fb_idx_getrec(irec, bi) == FB_ERROR)
               return(FB_ERROR);
            fb_push(irec);
            if (strncmp(key, bi->bi_key1, klen) <= 0){
               rec = bi->bi_left;
               typ = bi->bi_ltype;
               path = LEFT;
               }
            else{
               if (bi->bi_rtype == FB_BLANK ||
                     strncmp(key, bi->bi_key2, klen) <= 0){
                  rec = bi->bi_middle;
                  typ = bi->bi_mtype;
                  path = MIDDLE;
                  }
               else{
                  rec = bi->bi_right;
                  typ = bi->bi_rtype;
                  path = RIGHT;
                  }
               }
            /*
             * at this point, rec and typ should be set
             * if typ is 1, loop with new irec == rec
             */
            if (typ != CHAR_1)
               break;
            irec = rec;
            }
         return(path);
      }

/*
 * bi_insert - insert key at tree focal point bi.
 *	use bi_tmp, bs, and bs_tmp as storage. path indicates where key goes.
 */

   static bi_insert(key, bi, bi_tmp, path, bs, bs_tmp)
      char *key;
      fb_bidx *bi, *bi_tmp;
      int path;
      fb_bseq *bs, *bs_tmp;

      {
         long srec = 0, irec, left_irec, right_irec;
         int fix_key = 0, fix_promote = 0;

         /* stack of bi record numbers represent the bi-path */
         if (cdb_topstack == 0)
            return(FB_ERROR);
         irec = fb_pop();
         if (bi->bi_recno != irec)
            if (fb_idx_getrec(irec, bi) == FB_ERROR)
               return(FB_ERROR);

         /* key needs inserting into bi at path (LEFT, MIDDLE, RIGHT) */
         if (path == LEFT)
            srec = bi->bi_left;
         else if (path == MIDDLE)
            srec = bi->bi_middle;
         else if (path == RIGHT)
            srec = bi->bi_right;
         else
            fb_xerror(FB_MESSAGE,
               "btree_insert - assert error. `path' unknown.", NIL);

         /*
          * optimize --- if there is room in sequence node, insert/return.
          * 	if srec is 0, gen a sequence node and use it
          */

         if (bs_optimize(bi, bs, key, path, srec, bi_tmp, bs_tmp) == FB_AOK)
            return(FB_AOK);

         if (bs->bs_recno != srec)
            if (fb_seq_getrec(srec, bs) == FB_ERROR)
               return(FB_ERROR);

         /* assert: no room in sequence node, path is LEFT/MIDDLE/RIGHT */
         fb_seq_clear(bs_tmp);
         fb_idx_clear(bi_tmp);

         /*
          * if a sequence free list exists, use its number,
          * and change the free list
          */

         fb_check_free_idx(bi_tmp);
         fb_check_free_seq(bs_tmp);

         /* spread the keys bs_tmp is now left, bs stays right */
         spread_keys(bs_tmp, bs, key);

         /*
          * now deal with the first layer of index nodes
          * at this point, the current index node, bi, is full.
          */
         if (path == LEFT){
            /* promote the left key */
            fb_mkstr(&promote_key, bi->bi_key1);

            /* set up new node - its the left side */
            bi_tmp->bi_left = bs_tmp->bs_recno;
            bi_tmp->bi_ltype = CHAR_0;
            bi_tmp->bi_middle = bs->bs_recno;
            bi_tmp->bi_mtype = CHAR_0;
            strncpy(bi_tmp->bi_key1, bs_tmp->bs_key2, bi->bi_ksize);
            bi_tmp->bi_key1[bi->bi_ksize] = NULL;
            bi_tmp->bi_right = 0;
            bi_tmp->bi_rtype = FB_BLANK;
            bi_tmp->bi_key2[0] = NULL;

            /* use the original node as the right side - slide ptrs left */
            bi->bi_left = bi->bi_middle;
            bi->bi_ltype = bi->bi_mtype;
            bi->bi_middle = bi->bi_right;
            bi->bi_mtype = bi->bi_rtype;
            /* if a key2 exists, use it ... (test via rtype) */
            if (bi->bi_rtype != FB_BLANK)
               strcpy(bi->bi_key1, bi->bi_key2);
            else
               fix_key = 1;
            /* clear the right element */
            bi->bi_right = 0;
            bi->bi_rtype = FB_BLANK;
            bi->bi_key2[0] = NULL;
            }
         else if (path == MIDDLE){
            /* promote a new key */
            fb_mkstr(&promote_key, bs_tmp->bs_key2);
            promote_key[bi->bi_ksize] = NULL;

            /* set up new node - its the left side */
            bi_tmp->bi_left = bi->bi_left;
            bi_tmp->bi_ltype = bi->bi_ltype;
            bi_tmp->bi_middle = bs_tmp->bs_recno;
            bi_tmp->bi_mtype = CHAR_0;
            strcpy(bi_tmp->bi_key1, bi->bi_key1);
            bi_tmp->bi_right = 0;
            bi_tmp->bi_rtype = FB_BLANK;
            bi_tmp->bi_key2[0] = NULL;

            /* use the original node as the right side */
            bi->bi_left = bs->bs_recno;
            bi->bi_ltype = CHAR_0;
            bi->bi_middle = bi->bi_right;
            bi->bi_mtype = bi->bi_rtype;
            /* if a key2 exists, use it ... (test via rtype) */
            if (bi->bi_rtype != FB_BLANK)
               strcpy(bi->bi_key1, bi->bi_key2);
            else
               fix_key = 1;
            /* clear the right element */
            bi->bi_right = 0;
            bi->bi_rtype = FB_BLANK;
            bi->bi_key2[0] = NULL;
            }
         else if (path == RIGHT){
            /*
             * promote the right key
             * if key2 exists, use it ... (test via rtype)
             */
            if (bi->bi_rtype != FB_BLANK)
               fb_mkstr(&promote_key, bi->bi_key2);
            else
               fix_promote = 1;

            /* set up new node - its the left side */
            bi_tmp->bi_left = bi->bi_left;
            bi_tmp->bi_ltype = bi->bi_ltype;
            bi_tmp->bi_middle = bi->bi_middle;
            bi_tmp->bi_mtype = bi->bi_mtype;
            strcpy(bi_tmp->bi_key1, bi->bi_key1);
            bi_tmp->bi_right = 0;
            bi_tmp->bi_rtype = FB_BLANK;
            bi_tmp->bi_key2[0] = NULL;

            /* use the original node as the right side */
            bi->bi_left = bs_tmp->bs_recno;
            bi->bi_ltype = CHAR_0;
            bi->bi_middle = bs->bs_recno;
            bi->bi_mtype = CHAR_0;
            strncpy(bi->bi_key1, bs_tmp->bs_key2, bi->bi_ksize);
            bi->bi_key1[bi->bi_ksize] = NULL;
            bi->bi_right = 0;
            bi->bi_rtype = FB_BLANK;
            bi->bi_key2[0] = NULL;
            }

         /* fix sequential links */
         bs_tmp->bs_prev = bs->bs_prev;
         bs->bs_prev = bs_tmp->bs_recno;
         bs_tmp->bs_next = bs->bs_recno;
         /* write out bs_tmp */
         fb_seq_putrec(bs_tmp->bs_recno, bs_tmp);

         /*
          * since the splits always occur to the left, the tail will
          * not need updating. however, the head could be off now.
          * if the seq head has changed due to this split, update it.
          */
         if (bs->bs_head == bs->bs_recno){
            fb_get_seq_head(bs);
            bs->bs_head = bs_tmp->bs_recno;
            fb_put_seq_head(bs);
            }

         /* retrieve prev to update its next */
         if (bs_tmp->bs_prev != 0){
            /* bs_tmp is now two nodes back from bs */
            fb_seq_getrec(bs_tmp->bs_prev, bs_tmp);
            bs_tmp->bs_next = bs->bs_prev;
            fb_seq_putrec(bs_tmp->bs_recno, bs_tmp);
            }

         /* put all the rest of the nodes back */
         fb_seq_putrec(bs->bs_recno, bs);
         fb_idx_putrec(bi_tmp->bi_recno, bi_tmp);

         /* special situation -- the key needs fixing */
         if (fix_key || fix_promote){
            if (bi->bi_rtype == CHAR_0)
               srec = bi->bi_right;
            else if (bi->bi_mtype == CHAR_0)
               srec = bi->bi_middle;
            else if (bi->bi_ltype == CHAR_0)
               srec = bi->bi_left;
            else
               fb_xerror(FB_MESSAGE, "btree_insert: missing level 0 seq fix.",
                  NIL);
            fb_seq_getrec(srec, bs);
            if (fix_key)
               fb_rightmost_seq(bi->bi_key1, bi->bi_ksize, bs);
            else{
               fb_rightmost_seq(bi_tmp->bi_key1, bi->bi_ksize, bs);
               fb_mkstr(&promote_key, bi_tmp->bi_key1);
               }
            }
         fb_idx_putrec(bi->bi_recno, bi);

         /* fix ancestors */
         while (promote_key != NULL){
            /* afix the left and right paths */
            left_irec = bi_tmp->bi_recno;
            right_irec = bi->bi_recno;

            if (cdb_topstack == 0){
               /* split of root idx here - gen a new node */
               fb_idx_clear(bi);
               fb_check_free_idx(bi);
               bi->bi_left = left_irec;
               bi->bi_ltype = CHAR_1;
               bi->bi_middle = right_irec;
               bi->bi_mtype = CHAR_1;
               strcpy(bi->bi_key1, promote_key);
               fb_free(promote_key);
               promote_key = NULL;
               fb_idx_putrec(bi->bi_recno, bi);
               /* now change the root ... its now bi->bi_recno */
               bi->bi_root = bi->bi_recno;
               bi->bi_height++;
               fb_put_idx_head(bi);
               }
            else{
               /* fetch irec and try and insert promote_key */
               irec = fb_pop();
               if (bi->bi_recno != irec)
                  if (fb_idx_getrec(irec, bi) == FB_ERROR)
                     return(FB_ERROR);
               /*
                * if there is room for this key, insert it
                * if full, find middle of three keys, promote it, set links
                */
               if (bi_optimize(bi, left_irec, right_irec,
                     promote_key) == FB_AOK){
                  /* free up promote_key so insertion process bottoms out */
                  fb_free(promote_key);
                  promote_key = NULL;
                  }
               else{
                  /* no space in this index node key ... promote, fix, go */
                  bi_insert_ancestor(bi, bi_tmp, left_irec, right_irec);
                  }
               }
            }
         return(FB_AOK);
      }

/*
 * bi_insert_ancestor - bi is full, two new trees from below
 *	are in lval and rval
 */

   static bi_insert_ancestor(bi, bi_tmp, lval, rval)
      fb_bidx *bi, *bi_tmp;
      long lval, rval;

      {
         int path = 0;

         /*
          * determine path by examining rval --- rval used to be
          * the only node at some pointer inside bi ... now a split
          * is needed, and the path determines how to split.
          */
         if (rval == bi->bi_left)
            path = LEFT;
         else if (rval == bi->bi_middle)
            path = MIDDLE;
         else if (rval == bi->bi_right)
            path = RIGHT;
         else
            fb_xerror(FB_MESSAGE,
               "btree_insert_ancestor: assert error: `path' unknown.", NIL);

         /* no room in index node, path is LEFT/MIDDLE/RIGHT */
         fb_idx_clear(bi_tmp);

         /*
          * if an index free list exists, use its number,
          * and change the free list 
          */
         fb_check_free_idx(bi_tmp);

         /*
          * now deal with *this* layer of index nodes
          * at this point, the current index node, bi, is full.
          */
         if (path == LEFT){
            /* move promote key to left side, key1 to promote, key2 to key1 */
            strcpy(bi_tmp->bi_key1, promote_key);
            fb_mkstr(&(promote_key), bi->bi_key1);

            /* set up new node - its the left side */
            bi_tmp->bi_left = lval;
            bi_tmp->bi_ltype = CHAR_1;
            bi_tmp->bi_middle = rval;
            bi_tmp->bi_mtype = CHAR_1;
            bi_tmp->bi_right = 0;
            bi_tmp->bi_rtype = FB_BLANK;
            bi_tmp->bi_key2[0] = NULL;

            /* use the original node as the right side - slide ptrs left */
            bi->bi_left = bi->bi_middle;
            bi->bi_ltype = bi->bi_mtype;
            bi->bi_middle = bi->bi_right;
            bi->bi_mtype = bi->bi_rtype;
            strcpy(bi->bi_key1, bi->bi_key2);
            bi->bi_right = 0;
            bi->bi_rtype = FB_BLANK;
            bi->bi_key2[0] = NULL;
            }
         else if (path == MIDDLE){
            /* promote a new key - the promote_key */

            /* nothing to copy - promote_key is being promoted */

            /* set up new node - its the left side */
            bi_tmp->bi_left = bi->bi_left;
            bi_tmp->bi_ltype = bi->bi_ltype;
            bi_tmp->bi_middle = lval;
            bi_tmp->bi_mtype = CHAR_1;
            strcpy(bi_tmp->bi_key1, bi->bi_key1);
            bi_tmp->bi_right = 0;
            bi_tmp->bi_rtype = FB_BLANK;
            bi_tmp->bi_key2[0] = NULL;

            /* use the original node as the right side */
            bi->bi_left = rval;
            bi->bi_ltype = CHAR_1;
            bi->bi_middle = bi->bi_right;
            bi->bi_mtype = bi->bi_rtype;
            strcpy(bi->bi_key1, bi->bi_key2);
            bi->bi_right = 0;
            bi->bi_rtype = FB_BLANK;
            bi->bi_key2[0] = NULL;
            }
         else if (path == RIGHT){
            /* get this now due to promote_key swap */
            strcpy(bi_tmp->bi_key1, bi->bi_key1);
            /* promote the right side value, now at bi->bi_key2 */
            strcpy(bi->bi_key1, promote_key);
            fb_mkstr(&(promote_key), bi->bi_key2);

            /* set up new node - its the left side */
            bi_tmp->bi_left = bi->bi_left;
            bi_tmp->bi_ltype = bi->bi_ltype;
            bi_tmp->bi_middle = bi->bi_middle;
            bi_tmp->bi_mtype = bi->bi_mtype;
            bi_tmp->bi_right = 0;
            bi_tmp->bi_rtype = FB_BLANK;
            bi_tmp->bi_key2[0] = NULL;

            /* use the original node as the right side */
            bi->bi_left = lval;
            bi->bi_ltype = CHAR_1;
            bi->bi_middle = rval;
            bi->bi_mtype = CHAR_1;
            bi->bi_right = 0;
            bi->bi_rtype = FB_BLANK;
            bi->bi_key2[0] = NULL;
            }

         /* put both nodes back */
         fb_idx_putrec(bi->bi_recno, bi);
         fb_idx_putrec(bi_tmp->bi_recno, bi_tmp);

         return(FB_AOK);
      }

/*
 * spread_keys - keys are in rs and are are sorted.
 * 	find the two leftmost key values from the set (rs,key)
 *	and move them to the new node ls (bs_tmp).
 */

   static spread_keys(ls, rs, key)
      fb_bseq *ls, *rs;
      char *key;

      {
         int len, st;
         char *p, *q;

         /* use key2 as the dividing point test in spliting */
         len = strlen(key);
         st = strncmp(key, rs->bs_key2, len);
         if (st <= 0){
            /* use bs_key1 and key las left side, key2 and key3 as right */
            if (strncmp(key, rs->bs_key1, len) <= 0){
               p = key;
               q = rs->bs_key1;
               }
            else{
               q = key;
               p = rs->bs_key1;
               }
            strcpy(ls->bs_key1, p);
            strcpy(ls->bs_key2, q);
            /* now fix right side */
            strcpy(rs->bs_key1, rs->bs_key2);
            strcpy(rs->bs_key2, rs->bs_key3);
            }
         else{
            /* use bs_key1 and bs_key2 on left side, key & bskey_3 on right */
            strcpy(ls->bs_key1, rs->bs_key1);
            strcpy(ls->bs_key2, rs->bs_key2);
            /* now do right side */
            if (strncmp(key, rs->bs_key3, len) <= 0){
               p = key;
               q = rs->bs_key3;
               }
            else{
               q = key;
               p = rs->bs_key3;
               }
            strcpy(rs->bs_key1, p);
            strcpy(rs->bs_key2, q);
            }
         rs->bs_key3[0] = NULL;
         ls->bs_key3[0] = NULL;
      }

/*
 * bs_optimize - if room in bs with adjustments to bi, do so, return FB_AOK.
 *	else, return 0
 *	also, if bs does not exist yet, insert it.
 */

   static bs_optimize(bi, bs, key, path, srec, bi_tmp, bs_tmp)
      fb_bidx *bi, *bi_tmp;
      fb_bseq *bs, *bs_tmp;
      char *key;
      int path;
      long srec;

      {
         if (srec == 0){
            /* if srec is 0, gen a sequence node and use it */
            return(bs_insert(bi, bs, key, path, bi_tmp, bs_tmp));
            }

         /* at this point, srec might have room in it */
         if (fb_seq_getrec(srec, bs) == FB_ERROR)
            return(FB_ERROR);

         if (bs_reclaim(bs, key, bs_tmp) == FB_AOK)
            return(FB_AOK);

         if (fb_key_record(bs->bs_key3, bs->bs_ksize) != 0){
            /*
             * optimze # 3:
             *    there is no room in srec, but there might be room in
             *    the bi record itself ... test and insert if possible.
             */
            return(bs_rearrange(bi, bs, key, bs_tmp));
            }

         /* there are 0, 1 or 2 keys here */
         if (fb_key_record(bs->bs_key1, bs->bs_ksize) == 0){
            /* no keys are here - must be from deletions */
            strcpy(bs->bs_key1, key);
            }
         else if (fb_key_record(bs->bs_key2, bs->bs_ksize) == 0){
            /* one key exists already */
            if (strcmp(bs->bs_key1, key) > 0){
               strcpy(bs->bs_key2, bs->bs_key1);
               strcpy(bs->bs_key1, key);
               }
            else
               strcpy(bs->bs_key2, key);
            }
         else{
            /* two keys exist already - put key at key3 or ... */
            if (strcmp(key, bs->bs_key2) >= 0)
               strcpy(bs->bs_key3, key);
            else{
               /* ... slide key2 over and figure where to insert key: 1 or 2 */
               strcpy(bs->bs_key3, bs->bs_key2);
               if (strcmp(key, bs->bs_key1) >= 0)
                  strcpy(bs->bs_key2, key);
               else{
                  strcpy(bs->bs_key2, bs->bs_key1);
                  strcpy(bs->bs_key1, key);
                  }
               }
            }
         fb_seq_putrec(bs->bs_recno, bs);
         return(FB_AOK);
      }

/*
 * bs_insert - the idx node exists and sequence node does not: insert new bs
 */

   static bs_insert(bi, bs, key, path, bi_tmp, bs_tmp)
      fb_bidx *bi, *bi_tmp;
      fb_bseq *bs, *bs_tmp;
      char *key;
      int path;

      {
         long left_bs = 0;
         char left_type = NULL;

         fb_seq_clear(bs);
         fb_check_free_seq(bs);
         /* store key */
         strcpy(bs->bs_key1, key);
         /* fix pointers in bi */
         switch(path){
            case LEFT:
               fb_xerror(FB_MESSAGE, "bs_insert: assert: LEFT",NIL);
               break;
            case MIDDLE:
               bi->bi_middle = bs->bs_recno;
               bi->bi_mtype = CHAR_0;
               /* set this for fixing sequential links */
               left_bs = bi->bi_left;
               left_type = bi->bi_ltype;
               break;
            case RIGHT:
               bi->bi_right = bs->bs_recno;
               bi->bi_rtype = CHAR_0;
               /* set this for fixing sequential links */
               left_bs = bi->bi_middle;
               left_type = bi->bi_mtype;
               break;
            default:
               fb_xerror(FB_MESSAGE,
                  "btree_insert: assert: optimize: bad path",NIL);
               break;
            }
         /* fix sequential links in bs! */
         if (left_bs <= 0)
            fb_xerror(FB_MESSAGE,
               "btree_insert: assert: optimize - left_bs<=0", NIL);
         if (left_type != CHAR_0){
            left_bs = fb_locate_rightmost_seq(left_bs,left_type,bi_tmp,bs_tmp);
            if (left_bs <= 0)
               fb_xerror(FB_MESSAGE,
                  "btree_insert: assert:optimize-locate_seq", NIL);
            }
         if (fb_seq_getrec(left_bs, bs_tmp) == FB_ERROR)
            fb_xerror(FB_MESSAGE, "btree_insert: assert: optimize: seq_getrec",
               NIL);
         bs->bs_prev = bs_tmp->bs_recno;
         bs->bs_next = bs_tmp->bs_next;
         bs_tmp->bs_next = bs->bs_recno;
         /* write out bs_tmp */
         fb_seq_putrec(bs_tmp->bs_recno, bs_tmp);

         /* get next node for change to its prev fb_field */
         if (bs->bs_next > 0){
            fb_seq_getrec(bs->bs_next, bs_tmp);
            bs_tmp->bs_prev = bs->bs_recno;
            fb_seq_putrec(bs_tmp->bs_recno, bs_tmp);
            }
         else {
            /* bs_tmp must have been the tail */
            bs->bs_tail = bs->bs_recno;
            fb_put_seq_head(bs);
            }

         /* write bs */
         fb_seq_putrec(bs->bs_recno, bs);

         /* bi needs a new seperator key if path is RIGHT */
         if (path == RIGHT)
            fb_largest_key(bi->bi_key2, bi->bi_ksize, bi->bi_middle,
               bi->bi_mtype, bi_tmp, bs_tmp);

         /* write bi */
         fb_idx_putrec(bi->bi_recno, bi);

         return(FB_AOK);
      }

   static bi_optimize(bi, lval, rval, key)
      fb_bidx *bi;
      long lval, rval;
      char *key;

      {
         if (bi->bi_mtype != FB_BLANK && bi->bi_rtype != FB_BLANK)
            return(0);
         /* space somewhere here - either right or both middle and right */
         if (bi->bi_mtype == FB_BLANK){
            /*
             * only one pointer is in use, the left one.
             * now its two pieces, lval and rval.
             * cancel the old key, use the new prmote_key since
             * it divides the tree into left and right parts
             */
            strcpy(bi->bi_key1, key);
            bi->bi_left = lval;
            bi->bi_ltype = CHAR_1;
            bi->bi_middle = rval;
            bi->bi_mtype = CHAR_1;
            }
         else{
            /*
             * two pointers are in use
             * two cases - right_irec matches bi left OR middle
             */
            if (rval == bi->bi_left){
               strcpy(bi->bi_key2, bi->bi_key1);
               bi->bi_right = bi->bi_middle;
               bi->bi_rtype = bi->bi_mtype;
               bi->bi_middle = bi->bi_left;
               bi->bi_mtype = bi->bi_ltype;
               bi->bi_left = lval;
               bi->bi_ltype = CHAR_1;
               strcpy(bi->bi_key1, key);
               }
            else if (rval == bi->bi_middle){
               bi->bi_right = bi->bi_middle;
               bi->bi_rtype = bi->bi_mtype;
               bi->bi_middle = lval;
               bi->bi_mtype = CHAR_1;
               strcpy(bi->bi_key2, key);
               }
            else
               fb_xerror(FB_MESSAGE,
                  "bi_optimize: assert: rval unmatched", NIL);
            }
         fb_idx_putrec(bi->bi_recno, bi);
         return(FB_AOK);
      }

/*
 * bs_rearrange - the sequence node is full, the idx node is not.
 *	split the sequence node and place all pointers and keys into bi
 */

   static bs_rearrange(bi, bs, key, bs_tmp)
      fb_bidx *bi;
      fb_bseq *bs, *bs_tmp;
      char *key;

      {
         long lval, rval;

         if (bi->bi_mtype != FB_BLANK && bi->bi_rtype != FB_BLANK)
            return(0);

         /* assert: no room in sequence node, path is LEFT/MIDDLE/RIGHT */
         fb_seq_clear(bs_tmp);
         fb_check_free_seq(bs_tmp);

         /* spread the keys bs_tmp is now left, bs stays right */
         spread_keys(bs_tmp, bs, key);

         lval = bs_tmp->bs_recno;
         rval = bs->bs_recno;

         /* space somewhere here - either right or both middle and right */
         if (bi->bi_mtype == FB_BLANK){
            /*
             * only one pointer is in use, the left one.
             * now its two pieces, lval and rval.
             * gen a new key, from rightmost element in bs_tmp
             */
            fb_rightmost_seq(bi->bi_key1, bi->bi_ksize, bs_tmp);
            bi->bi_left = lval;
            bi->bi_ltype = CHAR_0;
            bi->bi_middle = rval;
            bi->bi_mtype = CHAR_0;
            }
         else{
            /*
             * two pointers are in use
             * two cases - right_irec matches bi left OR middle
             */
            if (rval == bi->bi_left){
               strcpy(bi->bi_key2, bi->bi_key1);
               bi->bi_right = bi->bi_middle;
               bi->bi_rtype = bi->bi_mtype;
               bi->bi_middle = bi->bi_left;
               bi->bi_mtype = bi->bi_ltype;
               bi->bi_left = lval;
               bi->bi_ltype = CHAR_0;
               fb_rightmost_seq(bi->bi_key1, bi->bi_ksize, bs_tmp);
               }
            else if (rval == bi->bi_middle){
               bi->bi_right = bi->bi_middle;
               bi->bi_rtype = bi->bi_mtype;
               bi->bi_middle = lval;
               bi->bi_mtype = CHAR_0;
               fb_rightmost_seq(bi->bi_key2, bi->bi_ksize, bs_tmp);
               }
            else
               fb_xerror(FB_MESSAGE, "bi_optimize: assert:rval unmatched",NIL);
            }

         /* fix sequential links */
         bs_tmp->bs_prev = bs->bs_prev;
         bs->bs_prev = bs_tmp->bs_recno;
         bs_tmp->bs_next = bs->bs_recno;

         /* put all three of the nodes back - bs, bs_tmp, bi */
         fb_seq_putrec(bs_tmp->bs_recno, bs_tmp);
         fb_seq_putrec(bs->bs_recno, bs);
         fb_idx_putrec(bi->bi_recno, bi);

         /*
          * since the splits always occur to the left, the tail will
          * not need updating. however, the head could be off now.
          * if the seq head has changed due to this split, update it.
          */
         if (bs->bs_head == bs->bs_recno){
            fb_get_seq_head(bs);
            bs->bs_head = bs_tmp->bs_recno;
            fb_put_seq_head(bs);
            }

         /* retrieve prev to update its next */
         if (bs_tmp->bs_prev != 0){
            /* bs_tmp is now two nodes back from bs */
            fb_seq_getrec(bs_tmp->bs_prev, bs_tmp);
            bs_tmp->bs_next = bs->bs_prev;
            fb_seq_putrec(bs_tmp->bs_recno, bs_tmp);
            }
         return(FB_AOK);
      }

/*
 * btree_delete - delete key from index ... the one with this rec #.
 */

   fb_btree_delete(delkey, rec, bi, bs)
      char *delkey;
      long rec;
      fb_bidx *bi;
      fb_bseq *bs;

      {
         int path, klen, ilen;
         long srec, irec;
         char lbuf[11];

         /* clear the stack - used to store tree path */
         fb_initstack();
         sprintf(delkey, "%s%010ld", delkey, rec);
         /* bi_locate sets bi and returns path where key exists */
         path = bi_locate(bi->bi_root, delkey, bi);

         /* stack of bi record numbers represent the bi-path */
         if (cdb_topstack == 0)
            return(FB_ERROR);
         irec = fb_pop();
         if (bi->bi_recno != irec)
            if (fb_idx_getrec(irec, bi) == FB_ERROR)
               return(FB_ERROR);
         /* delkey needs deleting from bi at path (LEFT, MIDDLE, RIGHT) */
         srec = 0;
         if (path == LEFT)
            srec = bi->bi_left;
         else if (path == MIDDLE)
            srec = bi->bi_middle;
         else if (path == RIGHT)
            srec = bi->bi_right;
         if (srec == 0){
            fb_serror(FB_MESSAGE,
               "btree_delete - assert error. path or srec not set.", NIL);
            return(FB_ERROR);
            }
         /* at this point, srec should contain the delkey to delete */
         if (fb_seq_getrec(srec, bs) == FB_ERROR)
            return(FB_ERROR);
         klen = strlen(delkey);
         ilen = klen;
         strcpy(lbuf, "*");
         if (strncmp(delkey, bs->bs_key1, klen) == 0)
            strncpy(bs->bs_key1 + ilen, lbuf, 1);
         else if (strncmp(delkey, bs->bs_key2, klen) == 0)
            strncpy(bs->bs_key2 + ilen, lbuf, 1);
         else if (strncmp(delkey, bs->bs_key3, klen) == 0)
            strncpy(bs->bs_key3 + ilen, lbuf, 1);
         fb_seq_putrec(bs->bs_recno, bs);
         return(FB_AOK);
      }

/*
 * fb_btree_create - create (bootstrap) a btree with a single key.
 */

   static fb_btree_create(bs, bi, key, ix)
      fb_bseq *bs;
      fb_bidx *bi;
      char *key;
      fb_autoindex *ix;

      {
         fb_seq_clear(bs);
         bs->bs_reccnt++;
         bs->bs_recno = bs->bs_reccnt;
         strcpy(bs->bs_key1, key);
         fb_seq_putrec(bs->bs_recno, bs);
         bs->bs_head = bs->bs_tail = bs->bs_recno;
         fb_put_seq_head(bs);

         fb_idx_clear(bi);
         bi->bi_reccnt++;
         bi->bi_recno = bi->bi_reccnt;
         strcpy(bi->bi_key1, key);
         bi->bi_left = bs->bs_recno;
         bi->bi_ltype = CHAR_0;
         fb_idx_putrec(bi->bi_recno, bi);

         bi->bi_root = bi->bi_recno;
         bi->bi_height = 1;
         fb_put_idx_head(bi);
	 fb_putxhead(ix->hfd, 1L, 1L);
         return(FB_AOK);
      }

/*
 * bs_reclaim - try and reclaim a deleted slot in this seq node.
 */

   static bs_reclaim(bs, key, bs_tmp)
      fb_bseq *bs, *bs_tmp;
      char *key;

      {
         long r1, r2, r3, rt;

         if (fb_key_record(bs->bs_key1, bs->bs_ksize) == -1)
            strcpy(bs->bs_key1, key);
         else if (fb_key_record(bs->bs_key2, bs->bs_ksize) == -1)
            strcpy(bs->bs_key2, key);
         else if (fb_key_record(bs->bs_key3, bs->bs_ksize) == -1)
            strcpy(bs->bs_key3, key);
         else
            return(FB_ERROR);	/* no deletions here */

         /* assert: at this point, there was a deleted key, now replaced.
          *         could be the insertion left things in the wrong order.
          *         since btree index set the focal point here, this key can
          *         be placed here, as long as seq is re-sorted
          *
          *         SO, do a mini-bubble sort - smallest keys to the left
          *         by comparing nodes 1 vs 2, then 1 vs 3, then 2 vs 3.
          */

         r1 = fb_key_record(bs->bs_key1, bs->bs_ksize);
         r2 = fb_key_record(bs->bs_key2, bs->bs_ksize);
         r3 = fb_key_record(bs->bs_key3, bs->bs_ksize);

         /* compare nodes 1 vs 2 */
         if (r1 != 0 && r2 != 0 && strcmp(bs->bs_key1, bs->bs_key2) > 0){
            /* swap key1 with key2 */
            strcpy(bs_tmp->bs_key1, bs->bs_key1);
            strcpy(bs->bs_key1, bs->bs_key2);
            strcpy(bs->bs_key2, bs_tmp->bs_key1);
            rt = r1;
            r1 = r2;
            r2 = rt;
            }

         /* compare nodes 1 vs 3 */
         if (r1 != 0 && r3 != 0 && strcmp(bs->bs_key1, bs->bs_key3) > 0){
            /* swap key1 with key3 */
            strcpy(bs_tmp->bs_key1, bs->bs_key1);
            strcpy(bs->bs_key1, bs->bs_key3);
            strcpy(bs->bs_key3, bs_tmp->bs_key1);
            rt = r1;
            r1 = r3;
            r3 = rt;
            }

         /* compare nodes 2 vs 3 */
         if (r2 != 0 && r3 != 0 && strcmp(bs->bs_key2, bs->bs_key3) > 0){
            /* swap key2 with key3 */
            strcpy(bs_tmp->bs_key2, bs->bs_key2);
            strcpy(bs->bs_key2, bs->bs_key3);
            strcpy(bs->bs_key3, bs_tmp->bs_key2);
            }

         fb_seq_putrec(bs->bs_recno, bs);
         return(FB_AOK);
      }
