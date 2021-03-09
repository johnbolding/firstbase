/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: btrec.c,v 9.0 2001/01/09 02:56:56 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Btrec_sid[] = "@(#) $Id: btrec.c,v 9.0 2001/01/09 02:56:56 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#define SKIPHEAD	((int) ((FB_SLONG * 4) + FB_HEADSTART + 1))

/*
 * btrec - contains routines for btree record handling
 */

/*
 * seq_putrec - put record routine for Btree+ sequence records
 */

   fb_seq_putrec(n, bs)
      long n;
      fb_bseq *bs;

      {
         int st = FB_ERROR;
         char *p;

         /* convert the numbers into the bs_rec area for writing */
         bs->bs_recno = n;
         p = bs->bs_rec;
         sprintf(p, "%010ld", bs->bs_prev);
         p += FB_RECORDPTR;
         if (bs->bs_key1[0] != NULL)
            sprintf(p, "%s", bs->bs_key1);
         else
            sprintf(p, "%*s", bs->bs_ksize, " ");
         p += bs->bs_ksize;
         if (bs->bs_key2[0] != NULL)
            sprintf(p, "%s", bs->bs_key2);
         else
            sprintf(p, "%*s", bs->bs_ksize, " ");
         p += bs->bs_ksize;
         if (bs->bs_key3[0] != NULL)
            sprintf(p, "%s", bs->bs_key3);
         else
            sprintf(p, "%*s", bs->bs_ksize, " ");
         p += bs->bs_ksize;
         sprintf(p, "%010ld\n", bs->bs_next);

         /* now bs_rec contains the node in ascii format. */
         st = fb_fputrec(n, bs->bs_fd, bs->bs_recsiz, bs->bs_rec, SKIPHEAD);
         return(st);
      }

/*
 * seq_getrec - get record routine for Btree+ sequence records
 */

   fb_seq_getrec(n, bs)
      long n;
      fb_bseq *bs;

      {
         int st = FB_ERROR;
         char *p, *q, buf[FB_MAXLINE];

         /* read a record from disk into bs_rec, then convert into record */
         
         st = fb_fgetrec(n, bs->bs_fd, bs->bs_recsiz, bs->bs_rec, SKIPHEAD);
         if (st == FB_ERROR)
            return(FB_ERROR);
         bs->bs_recno = n;

         p = bs->bs_rec;
         strncpy(buf, p, FB_RECORDPTR);
         q = buf + FB_RECORDPTR;
         *q = NULL;
         bs->bs_prev = atol(buf);
         p += FB_RECORDPTR;

         strncpy(bs->bs_key1, p, bs->bs_ksize);
         q = bs->bs_key1 + bs->bs_ksize;
         *q = NULL;
         p += bs->bs_ksize;

         strncpy(bs->bs_key2, p, bs->bs_ksize);
         q = bs->bs_key2 + bs->bs_ksize;
         *q = NULL;
         p += bs->bs_ksize;

         strncpy(bs->bs_key3, p, bs->bs_ksize);
         q = bs->bs_key3 + bs->bs_ksize;
         *q = NULL;
         p += bs->bs_ksize;

         strncpy(buf, p, FB_RECORDPTR);
         q = buf + FB_RECORDPTR;
         *q = NULL;
         bs->bs_next = atol(buf);
         return(st);
      }

/*
 * idx_putrec - put record routine for btree+ idx files
 */

   fb_idx_putrec(n, bi)
      long n;
      fb_bidx *bi;

      {
         int st = FB_ERROR;
         char *p;

         /* convert the numbers into the bi_rec area for writing */
         bi->bi_recno = n;
         p = bi->bi_rec;

         /* left */
         *p = bi->bi_ltype; p++;
         sprintf(p, "%010ld", bi->bi_left);
         p += FB_RECORDPTR;

         /* key1 */
         if (bi->bi_key1[0] != NULL)
            sprintf(p, "%s", bi->bi_key1);
         else
            sprintf(p, "%*s", bi->bi_ksize, " ");
         p += bi->bi_ksize;

         /* middle */
         *p = bi->bi_mtype; p++;
         sprintf(p, "%010ld", bi->bi_middle);
         p += FB_RECORDPTR;

         /* key2 */
         if (bi->bi_key2[0] != NULL)
            sprintf(p, "%s", bi->bi_key2);
         else
            sprintf(p, "%*s", bi->bi_ksize, " ");
         p += bi->bi_ksize;

         /* right */
         *p = bi->bi_rtype; p++;
         sprintf(p, "%010ld", bi->bi_right);
         p += FB_RECORDPTR;

         sprintf(p, "%1d\n", bi->bi_lock);

         /* now bi_rec contains the idx node in ascii format. */
         st = fb_fputrec(n, bi->bi_fd, bi->bi_recsiz, bi->bi_rec, SKIPHEAD);
         return(st);
      }

/*
 * idx_getrec - get record routine for btree+ idx files
 */

   fb_idx_getrec(n, bi)
      long n;
      fb_bidx *bi;

      {
         int st = FB_ERROR;
         char *p, *q, buf[FB_MAXLINE];

         /* read a record from disk into bi_rec, then convert into record */
         
         st = fb_fgetrec(n, bi->bi_fd, bi->bi_recsiz, bi->bi_rec, SKIPHEAD);
         if (st == FB_ERROR)
            return(FB_ERROR);
         bi->bi_recno = n;
         p = bi->bi_rec;

         /* ltype and pointer */
         bi->bi_ltype = *p++;
         strncpy(buf, p, FB_RECORDPTR);
         q = buf + FB_RECORDPTR;
         *q = NULL;
         bi->bi_left = atol(buf);
         p += FB_RECORDPTR;

         /* key1 */
         strncpy(bi->bi_key1, p, bi->bi_ksize);
         q = bi->bi_key1 + bi->bi_ksize;
         *q = NULL;
         p += bi->bi_ksize;


         /* middle type and pointer */
         bi->bi_mtype = *p++;
         strncpy(buf, p, FB_RECORDPTR);
         q = buf + FB_RECORDPTR;
         *q = NULL;
         bi->bi_middle = atol(buf);
         p += FB_RECORDPTR;

         /* key2 */
         strncpy(bi->bi_key2, p, bi->bi_ksize);
         q = bi->bi_key2 + bi->bi_ksize;
         *q = NULL;
         p += bi->bi_ksize;

         /* right type and pointer */
         bi->bi_rtype = *p++;
         strncpy(buf, p, FB_RECORDPTR);
         q = buf + FB_RECORDPTR;
         *q = NULL;
         bi->bi_right = atol(buf);
         p += FB_RECORDPTR;

         /* lock fb_field */
         strncpy(buf, p, 1);
         q = buf + 1;
         *q = NULL;
         bi->bi_lock = atoi(buf);
         return(st);
      }

/*
 * check_free_idx - could implement free list for this if needed.
 *	this is the place to look at it.
 *	for this one, generate the idx node.
 */

   fb_check_free_idx(bi)
      fb_bidx *bi;

      {
         fb_getbxhead(bi->bi_fd, &(bi->bi_root), &(bi->bi_height),
            &(bi->bi_reccnt), &(bi->bi_free));
         bi->bi_reccnt++;
         bi->bi_recno = bi->bi_reccnt;
         fb_putbxhead(bi->bi_fd, bi->bi_root, bi->bi_height,
            bi->bi_reccnt, bi->bi_free);
      }

/*
 * check_free_seq - check the free list for a seq node, or gen one.
 */

   fb_check_free_seq(bs)
      fb_bseq *bs;

      {
         if (fb_get_seq_head(bs) == FB_ERROR)
            fb_xerror(FB_MESSAGE, "check_free_seq: error in getbxhead", NIL);
         if (bs->bs_free > 0){
            fb_seq_getrec(bs->bs_free, bs);
            bs->bs_free = bs->bs_next;
            bs->bs_next = 0;
            }
         else{
            bs->bs_reccnt += 1L;
            bs->bs_recno = bs->bs_reccnt;
            }
         if (fb_put_seq_head(bs) == FB_ERROR)
            fb_xerror(FB_MESSAGE, "check_free_seq: error in getbxhead", NIL);
      }

   fb_put_seq_head(bs)
      fb_bseq *bs;

      {
         return(fb_putbxhead(bs->bs_fd, bs->bs_head, bs->bs_tail,
            bs->bs_reccnt, bs->bs_free));
      }

   fb_get_seq_head(bs)
      fb_bseq *bs;

      {
         return(fb_getbxhead(bs->bs_fd, &(bs->bs_head), &(bs->bs_tail),
            &(bs->bs_reccnt), &(bs->bs_free)));
      }

   fb_put_idx_head(bi)
      fb_bidx *bi;

      {
         return(fb_putbxhead(bi->bi_fd, bi->bi_root, bi->bi_height,
            bi->bi_reccnt, bi->bi_free));
      }

   fb_get_idx_head(bi)
      fb_bidx *bi;

      {
         return(fb_getbxhead(bi->bi_fd, &(bi->bi_root), &(bi->bi_height),
            &(bi->bi_reccnt), &(bi->bi_free)));
      }

   fb_put_free_bs(bs)
      fb_bseq *bs;

      {
         fb_get_seq_head(bs);
         bs->bs_next = bs->bs_free;
         bs->bs_prev = 0;
         bs->bs_free = bs->bs_recno;
         fb_put_seq_head(bs);
         fb_seq_putrec(bs->bs_recno, bs);
      }
