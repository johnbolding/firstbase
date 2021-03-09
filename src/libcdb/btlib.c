/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: btlib.c,v 9.0 2001/01/09 02:56:24 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Btlib_sid[] = "@(#) $Id: btlib.c,v 9.0 2001/01/09 02:56:24 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern char cdb_EOREC;

/*
 * btlib - btree+ library routines. general in nature.
 */

   long fb_key_record(s, len)
      char *s;
      int len;

      {
         char buf[20];
         long rec = 0L;
         int siz;

         if (s == NULL || *s == NULL)
            return(0L);
         siz = FB_RECORDPTR + 1;
         memcpy(buf, s + len - siz, (unsigned) siz);
         buf[siz] = NULL;
         if (buf[siz - 1] == CHAR_STAR)
            rec = -1;
         else
            rec = atol(buf);
         return(rec);
      }

   long fb_key_eval(bs)
      fb_bseq *bs;

      {
         long rec = 0;

         switch(bs->bs_curkey){
            case 1:
               rec = fb_key_record(bs->bs_key1, bs->bs_ksize);
               break;
            case 2:
               rec = fb_key_record(bs->bs_key2, bs->bs_ksize);
               break;
            case 3:
               rec = fb_key_record(bs->bs_key3, bs->bs_ksize);
               break;
            }
         return(rec);
      }

   char *fb_key_ptr(bs)
      fb_bseq *bs;

      {
         char *p = NIL;

         switch(bs->bs_curkey){
            case 1:
               p = bs->bs_key1;
               break;
            case 2:
               p = bs->bs_key2;
               break;
            case 3:
               p = bs->bs_key3;
               break;
            }
         return(p);
      }

/*
 * rightmost_seq - return the rightmost key from a sequence node
 */

   fb_rightmost_seq(p, ksize, bs)
      char *p;
      fb_bseq *bs;
      int ksize;

      {
         char *q = NULL;

         if (fb_key_record(bs->bs_key3, bs->bs_ksize) != 0)
            q = bs->bs_key3;

         if (q == NULL){
            if (fb_key_record(bs->bs_key2, bs->bs_ksize) != 0)
               q = bs->bs_key2;
            }

         if (q == NULL){
            if (fb_key_record(bs->bs_key1, bs->bs_ksize) != 0)
               q = bs->bs_key1;
            }

         if (q == NULL)
            fb_xerror(FB_MESSAGE, "B+Tree Underflow should not occur.", NIL);

         strncpy(p, q, ksize);
         q = p + ksize;
         *q = NULL;
      }

/*
 * locate_rightmost_seq - locate the rightmost seq for use outside.
 */

   long fb_locate_rightmost_seq(tree_p, tree_t, bi, bs)
      long tree_p;
      char tree_t;
      fb_bidx *bi;
      fb_bseq *bs;

      {
         long new_p = 0;
         char new_t = NULL;
         int st;

         if (tree_t == CHAR_1){
            st = fb_idx_getrec(tree_p, bi);
            if (st == FB_ERROR)
               fb_xerror(FB_MESSAGE, "loc_rightmost_seq: idx_getrec error",
                  NIL);
            if (bi->bi_rtype != FB_BLANK){
               new_p = bi->bi_right;
               new_t = bi->bi_rtype;
               }
            else if (bi->bi_mtype != FB_BLANK){
               new_p = bi->bi_middle;
               new_t = bi->bi_mtype;
               }
            else if (bi->bi_ltype != FB_BLANK){
               new_p = bi->bi_left;
               new_t = bi->bi_ltype;
               }
            else{
               fb_xerror(FB_MESSAGE, "loc_rightmost_seq passed a null tree",
                  NIL);
               }
            /* new_p and new_t are set now */
            return(fb_locate_rightmost_seq(new_p, new_t, bi, bs));
            }
         else if (tree_t == CHAR_0)
            return(tree_p);
         else{
            fb_xerror(FB_MESSAGE, "loc_rightmost_seq passed a null tree",
               NIL);
            }
         return(0L);
      }

/*
 * largest_key - determine the largest key in tree_p
 *	p is storage, ksize is key size, tree_p is tree pointer
 *	tree_t is the tree type, bi and bs are bidx and bseq pointers.
 */

   fb_largest_key(p, ksize, tree_p, tree_t, bi, bs)
      char *p;
      int ksize;
      long tree_p;
      char tree_t;
      fb_bidx *bi;
      fb_bseq *bs;

      {
         long new_p = 0;
         char new_t = NULL;
         int st;

         if (tree_t == CHAR_1){
            st = fb_idx_getrec(tree_p, bi);
            if (st == FB_ERROR)
               fb_xerror(FB_MESSAGE, "largest_key - idx_getrec error", NIL);
            if (bi->bi_rtype != FB_BLANK){
               new_p = bi->bi_right;
               new_t = bi->bi_rtype;
               }
            else if (bi->bi_mtype != FB_BLANK){
               new_p = bi->bi_middle;
               new_t = bi->bi_mtype;
               }
            else if (bi->bi_ltype != FB_BLANK){
               new_p = bi->bi_left;
               new_t = bi->bi_ltype;
               }
            else{
               fb_xerror(FB_MESSAGE, "largest_key passed a null tree", NIL);
               }
            /* new_p and new_t are set now */
            fb_largest_key(p, ksize, new_p, new_t, bi, bs);
            }
         else if (tree_t == CHAR_0){
            st = fb_seq_getrec(tree_p, bs);
            if (st == FB_AOK)
               fb_rightmost_seq(p, ksize, bs);
            }
         else{
            fb_xerror(FB_MESSAGE, "largest_key passed a null tree", NIL);
            }
      }

