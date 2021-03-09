/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: btalloc.c,v 9.0 2001/01/09 02:56:45 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Btalloc_sid[] = "@(#) $Id: btalloc.c,v 9.0 2001/01/09 02:56:45 john Exp $";
#endif

static char *BSEQ = ".bseq";
static char *BIDX = ".bidx";

#include <fb.h>
#include <fb_ext.h>

/*
 * seq_alloc - allocate the sequence elements
 */

   fb_bseq *fb_seq_alloc(dindex, isiz)
      char *dindex;
      int isiz;

      {
         fb_bseq *bs;
         char iname[FB_MAXNAME], tname[FB_MAXNAME];

         bs = (fb_bseq *) fb_malloc(sizeof(fb_bseq));
         bs->bs_prev = 0;
         bs->bs_next = 0;
         bs->bs_key1 = NULL;
         bs->bs_key2 = NULL;
         bs->bs_key3 = NULL;
         bs->bs_curkey = 0;
         bs->bs_lock = 0;
         bs->bs_name = NULL;
         bs->bs_fd =   0;
         bs->bs_rec =  NULL;
         bs->bs_head = 0;
         bs->bs_tail = 0;
	 bs->bs_recsiz = 0;
	 bs->bs_recno = 0;
	 bs->bs_reccnt = 0;
	 bs->bs_free = 0;

         fb_rootname(iname, dindex);
	 sprintf(tname, SYSMSG[S_FMT_2S], iname, BSEQ);
	 fb_mkstr(&(bs->bs_name), tname);

         isiz += 1;                       /* for DEL MARKER */
         bs->bs_recsiz = (isiz * 3) + 21; /* 21=2 * 10 + 1 */
         bs->bs_rec = (char *) fb_malloc((unsigned)(bs->bs_recsiz + 13));

         bs->bs_key1 = (char *) fb_malloc((unsigned)(isiz + 1)); /*1 for NULL*/
         bs->bs_key2 = (char *) fb_malloc((unsigned)(isiz + 1)); /*1 for NULL*/
         bs->bs_key3 = (char *) fb_malloc((unsigned)(isiz + 1)); /*1 for NULL*/

         bs->bs_ksize = isiz;

         fb_seq_clear(bs);
         return(bs);
      }

/*
 * idx_alloc - allocate the index elements
 */

   fb_bidx *fb_idx_alloc(dindex, isiz)
      char *dindex;
      int isiz;

      {
         fb_bidx *bi;
         char iname[FB_MAXNAME], tname[FB_MAXNAME];

         bi = (fb_bidx *) fb_malloc(sizeof(fb_bidx));
         bi->bi_left =  0;
         bi->bi_middle =0;
         bi->bi_right = 0;
         bi->bi_ltype = CHAR_BLANK;
         bi->bi_mtype = CHAR_BLANK;
         bi->bi_rtype = CHAR_BLANK;
         bi->bi_key1 = 	NULL;
         bi->bi_key2 =	NULL;
         bi->bi_lock =  0;
         bi->bi_name =  NULL;
         bi->bi_fd = 	0;
         bi->bi_rec =	NULL;
         bi->bi_root =	0;
         bi->bi_height = 0;
	 bi->bi_recsiz = 0;
	 bi->bi_recno = 0;
	 bi->bi_reccnt = 0;
	 bi->bi_free = 0;

         fb_rootname(iname, dindex);
	 sprintf(tname, SYSMSG[S_FMT_2S], iname, BIDX);
	 fb_mkstr(&(bi->bi_name), tname);
         isiz += 1;                       /* for DEL MARKER */
         bi->bi_recsiz = (isiz * 2) + 35; /* (1+10)*3 + 2 */
         bi->bi_rec = (char *) fb_malloc((unsigned)(bi->bi_recsiz + 13));

         bi->bi_key1 = (char *) fb_malloc((unsigned)(isiz + 1)); /*1 for NULL*/
         bi->bi_key2 = (char *) fb_malloc((unsigned)(isiz + 1)); /*1 for NULL*/
         bi->bi_ksize = isiz;

         fb_idx_clear(bi);
         return(bi);
      }

/*
 * fb_seq_free - free up a bseq node
 */

   void fb_seq_free(bs)
      fb_bseq *bs;

      {
         if (bs == NULL)
            return;
         fb_free(bs->bs_key1);
         fb_free(bs->bs_key2);
         fb_free(bs->bs_key3);
	 fb_free(bs->bs_rec);
	 fb_free(bs->bs_name);
	 fb_free((char *) (bs));
      }

/*
 * seq_copy - copy a sequence node from bf to bt
 */

   void fb_seq_copy(bt, bf)
      fb_bseq *bt, *bf;

      {
         if (bf == NULL || bt == NULL)
            return;
         bt->bs_prev = bf->bs_prev;
         bt->bs_next = bf->bs_next;
         bt->bs_curkey = bf->bs_curkey;
         bt->bs_lock = bf->bs_lock;
         bt->bs_fd = bf->bs_fd;
         bt->bs_ksize = bf->bs_ksize;
         bt->bs_head = bf->bs_head;
         bt->bs_tail = bf->bs_tail;
         bt->bs_recsiz = bf->bs_recsiz;
         bt->bs_recno = bf->bs_recno;
         bt->bs_reccnt = bf->bs_reccnt;
         bt->bs_free = bf->bs_free;
         strcpy(bt->bs_key1, bf->bs_key1);
         strcpy(bt->bs_key2, bf->bs_key2);
         strcpy(bt->bs_key3, bf->bs_key3);
         fb_mkstr(&(bt->bs_name), bf->bs_name);
      }

/*
 * idx_free - free up an idx node
 */

   void fb_idx_free(bi)
      fb_bidx *bi;
   
      {
         if (bi == NULL)
            return;
         fb_free(bi->bi_key1);
         fb_free(bi->bi_key2);
	 fb_free(bi->bi_rec);
	 fb_free(bi->bi_name);
	 fb_free((char *) (bi));
      }

/*
 * idx_copy - copy an index node from bf to bt
 */

   void fb_idx_copy(bt, bf)
      fb_bidx *bt, *bf;

      {
         if (bf == NULL || bt == NULL)
            return;
         bt->bi_left = bf->bi_left;
         bt->bi_ltype = bf->bi_ltype;
         bt->bi_middle = bf->bi_middle;
         bt->bi_mtype = bf->bi_mtype;
         bt->bi_right = bf->bi_right;
         bt->bi_rtype = bf->bi_rtype;
         bt->bi_lock = bf->bi_lock;
         bt->bi_fd = bf->bi_fd;
         bt->bi_ksize = bf->bi_ksize;
         bt->bi_root = bf->bi_root;
         bt->bi_height = bf->bi_height;
         bt->bi_recsiz = bf->bi_recsiz;
         bt->bi_recno = bf->bi_recno;
         bt->bi_reccnt = bf->bi_reccnt;
         bt->bi_free = bf->bi_free;
         strcpy(bt->bi_key1, bf->bi_key1);
         strcpy(bt->bi_key2, bf->bi_key2);
         fb_mkstr(&(bt->bi_name), bf->bi_name);
      }

/*
 * seq_clear - clear the contents of bs. assumes allocation has occured.
 */

   void fb_seq_clear(bs)
      fb_bseq *bs;

      {
         if (bs == NULL)
            return;
         bs->bs_prev = 0;
         bs->bs_next = 0;
         bs->bs_key1[0] = NULL;
         bs->bs_key2[0] = NULL;
         bs->bs_key3[0] = NULL;
         bs->bs_curkey = 0;
         bs->bs_lock = 0;
         bs->bs_rec[0] = NULL;
      }

/*
 * idx_clear - clear the contents of bi. assumes allocation has occured.
 */

   void fb_idx_clear(bi)
      fb_bidx *bi;

      {
         if (bi == NULL)
            return;
         bi->bi_left = 0;
         bi->bi_middle = 0;
         bi->bi_right = 0;
         bi->bi_key1[0] = NULL;
         bi->bi_key2[0] = NULL;
         bi->bi_ltype = CHAR_BLANK;
         bi->bi_mtype = CHAR_BLANK;
         bi->bi_rtype = CHAR_BLANK;
         bi->bi_lock = 0;
         bi->bi_rec[0] = NULL;
      }
