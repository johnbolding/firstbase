/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: relatets.c,v 9.0 2001/01/09 02:55:50 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Relateeach_sid[] = "@(#) $Id: relatets.c,v 9.0 2001/01/09 02:55:50 john Exp $";
#endif

#include "dbsql_e.h"

extern char cdb_EOREC;

static locate_join_value();

/*
 * relatetest - foreach relation value (tr), copy in values into
 * 	rel_val[dpos] & test if it passes the c_root test, then enter
 * 	this combination into current r
 */

   void relatetest(r, c_root, depth)
      relation *r;
      node *c_root;
      int depth;

      {
         relation *tr;
         int dpos, i, j, vloc, vlen, recsiz, recadj, sortedpass;
         long rec, relrec, irec, fb_megasearch(), bsmax, bsend, overflow;
         long start, stop, fb_btree_search();
         fb_database *dp = NULL;
         char buf[FB_MAXLINE];
         fb_autoindex *ix;

         dpos = depth - 1;
         tr = rel_depth[dpos];			/* iteration relation */
         vloc = r->r_offset[dpos];
         vlen = r->r_isize[dpos];
         if (tr->r_nrecs != vlen)
            fb_xerror(FB_MESSAGE, "releateeach: bad relation/vector length", NIL);
         if (tr->r_reccnt == -1){
            /* use whole database */
            dp = r->r_dbase[vloc];
            for (rec = 1; rec <= dp->reccnt; rec++){
               if (u_getrec(rec, dp) == FB_ERROR)
                  fb_xerror(FB_FATAL_GETREC, dp->dbase, (char *) &rec);
               rel_val[vloc] = dp->rec;
               if (!(FB_ISDELETED(dp)) && test_restrict(c_root, dp) == FB_AOK)
                  r_store_vals(r);
               }
            }
         else if (tr->r_reccnt == -2){
            /* optimize use of whole database via autoindex */
            dp = r->r_dbase[vloc];
#if DEBUG
            if (traceflag >= 20){
               fprintf(stderr, "relatetest: optimize: op=%d, %s: ",
                  tr->r_join_op, dp->dbase);
               }
#endif /* DEBUG */

            /* get bsend so as to know when to stop */
            fb_getxhead(tr->r_aid->hfd, &bsmax, &bsend);
            recsiz = tr->r_join_fld->size + FB_RECORDPTR + 1;
            recadj = recsiz - 11;

            /*
             * locate first one, loop till no good. then drop out.
             */            

            if (r->r_join_value == NULL){
               /* need to locate the join value depending on the query */
               locate_join_value(r, tr->r_join_fld, c_root);
               }
            
            if (r->r_join_value != NULL){
               ix = tr->r_aid;
               if (ix->ix_tree){
                  irec = fb_btree_search(r->r_join_value, ix->ix_idx,
                     ix->ix_seq);
                  }
               else{
                  irec = fb_megasearch(tr->r_aid->afd, r->r_join_value, 0, 1L,
                     bsend, bsmax, recsiz, 1, buf);
                  }
               }
            else
               irec = 1;
            if (irec <= 0)
               return;

            if (1 > bsend)
               overflow = 1;
            else
               overflow = bsend + 1;
            if (overflow <= 0)
               overflow = 1;

            /* loop through here twice, once for 1-bsend,
             * again for overflow ... bsmax
             */

            for (sortedpass = 1; sortedpass <= 2; sortedpass++){
               if (sortedpass == 1){
                  start = irec;
                  stop = bsend;
                  }
               else{
                  start = overflow;
                  stop = bsmax;
                  }
               for (irec = start; irec <= stop; irec++){
                  if (fb_fgetrec(irec, tr->r_aid->afd, recsiz, buf, 0) ==
                        FB_ERROR)
                     fb_xerror(FB_FATAL_FGETREC, NIL, (char *) &irec);
                  if (buf[recsiz - 2] == cdb_EOREC)
                     continue;			/* ignore this find */

                  /* decode the record number */
                  rel_val[vloc] = rec = atol((char *) buf + recadj);

                  /* test the index value */
                  buf[recadj] = NULL;
                  if (r->r_join_value != NULL && !equal(buf, r->r_join_value))
                     if (sortedpass == 1)
                        break;
                     else
                        continue;

                  /* store the database value */
                  fb_trim(buf);
                  fb_store(tr->r_join_fld, buf, dp);

                  /* if delcnt is > 0, gotta read record to see if deleted */
                  if (dp->delcnt > 0)
                     if (u_getrec(rec, dp) == FB_ERROR)
                        fb_xerror(FB_FATAL_GETREC, dp->dbase, (char *) &rec);

                  if ((dp->delcnt <= 0 || !(FB_ISDELETED(dp))) &&
                        test_restrict(c_root, dp) == FB_AOK)
                     r_store_vals(r);
                  }
               }
            }
         else{
            /* use relation -- copy in all vlen elements as needed */
            for (relrec = 0; relrec < tr->r_reccnt; relrec++){
               getrec_relation(relrec, tr);
               for (i = vloc, j = 0; j < vlen; j++, i++){
                  rec = rel_val[i] = tr->r_rec[j];
                  dp = r->r_dbase[i];
                  if (u_getrec(rec, dp) == FB_ERROR)
                     fb_xerror(FB_FATAL_GETREC, dp->dbase, (char *) &rec);
                  }
               if (test_restrict(c_root, dp) == FB_AOK)
                  r_store_vals(r);
               }
            }
      }

   static locate_join_value(r, f, n)
      relation *r;
      fb_field *f;
      node *n;

      {
         node *eq, *eq_L, *eq_R;
         char *p, *s_expr(), buf[FB_MAXLINE];

         /* first get to eq */
         n = n->n_narg[1];
         n = n->n_narg[0];
         eq = n;
         eq_L = n->n_narg[0];
         eq_R = n->n_narg[1];
         if (tree_contains_field(eq_L, f))
            p = s_expr(eq_R);
         else
            p = s_expr(eq_L);
         if (p != NULL && *p != NULL){
            if (FB_OFNUMERIC(f->type))
               fb_rjustify(buf, p, f->size, f->type);
            else
               fb_pad(buf, p, f->size);
            fb_mkstr(&(r->r_join_value), buf);
            }
      }

/*
 * tree_contains_field - returns 0 or 1 depending on tree containing dp
 */

   tree_contains_field(n, f)
      node *n;
      fb_field *f;

      {
         int i, st = 0;
         fb_field *tf;

         if (n == NULL)
            return(0);
         for (i = 0; i < NARGS && st != 1; i++)
            if (n->n_narg[i] != NULL)
               st = tree_contains_field(n->n_narg[i], f);
         if (st == 1)
            return(st);
         if (n->n_type == V_ID){
            tf = NULL;
            if (n->n_p2 != 0){
               tf  = (fb_field *) n->n_p2;
               if (tf == f)
                  st = 1;
               }
            }
         return(st);
      }
