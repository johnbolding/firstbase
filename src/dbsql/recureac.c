/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: recureac.c,v 9.0 2001/01/09 02:55:50 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Recurseeach_sid[] = "@(#) $Id: recureac.c,v 9.0 2001/01/09 02:55:50 john Exp $";
#endif

#include "dbsql_e.h"

static generate_join_value();

extern short int cdb_secure;
extern char cdb_EOREC;

/*
 * recurseeach - foreach relation value (tr), copy in values into
 * 	rel_val[dpos...], do getrecs, and recurse back to rr_select(depth+1).
 */

   recurseeach(r, c_root, depth)
      relation *r;
      node *c_root;
      int depth;

      {
         relation *tr;
         int dpos, i, j, vloc, vlen, recsiz, recadj, st;
         long rec, relrec, bsmax, bsend, irec;
         fb_database *dp;
         char buf[FB_MAXLINE];

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
               /* disable join value since no index here --- looked up later */
               if (r->r_join_value != NULL){
                  fb_free(r->r_join_value);
                  r->r_join_value = NULL;
                  }
               if (!(FB_ISDELETED(dp))){
                  st = FB_AOK;
                  if (cdb_secure && !fb_record_permission(dp, READ))
                     st = FB_ERROR;
                  if (st == FB_AOK)
                     rr_select(r, c_root, depth + 1);
                  }
               }
            }
         else if (tr->r_reccnt == -2){
            /* optimize use of whole database via autoindex */
            dp = r->r_dbase[vloc];
#if DEBUG
            if (traceflag >= 10){
               fprintf(stderr, "RECURSEEACH: OPTIMIZE found ... op=%d, %s\n",
                  tr->r_join_op, dp->dbase);
               }
#endif /* DEBUG */
            /* get bsend so as to know when to stop */
            fb_getxhead(tr->r_aid->hfd, &bsmax, &bsend);
            recsiz = tr->r_join_fld->size + FB_RECORDPTR + 1;
            recadj = recsiz - 11;

            /*
             * loop through index using fgetrec
             * for each one, store away index value in record, and proceed
             */            
            for (irec = 1; irec <= bsmax; irec++){
               if (fb_fgetrec(irec, tr->r_aid->afd, recsiz, buf, 0) == FB_ERROR)
	          fb_xerror(FB_FATAL_FGETREC, NIL, (char *) &irec);
               if (buf[recsiz - 2] == cdb_EOREC)
                  continue;			/* ignore this find */

               /* decode the record number */
               rel_val[vloc] = rec = atol((char *) buf + recadj);

               /* store the database value */
               buf[recadj] = NULL;
               fb_trim(buf);
               fb_store(tr->r_join_fld, buf, dp);

               /* generate the index value */
               generate_join_value(r, tr->r_join_fld, c_root);

               /* if delcnt is > 0, gotta read record to see if deleted */
               if (dp->delcnt > 0)
                  if (u_getrec(rec, dp) == FB_ERROR)
                     fb_xerror(FB_FATAL_GETREC, dp->dbase, (char *) &rec);
               if (dp->delcnt <= 0 || !(FB_ISDELETED(dp))){
                  st = FB_AOK;
                  if (cdb_secure && !fb_record_permission(dp, READ))
                     st = FB_ERROR;
                  if (st == FB_AOK)
                     rr_select(r, c_root, depth + 1);
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
                  /*
                   * no need to check FB_ISDELETED since this relation was
                   * built using getrec code that did test for FB_ISDELETED.
                   *
                   */
                  }
               /* disable join value since no index here --- looked up later */
               if (r->r_join_value != NULL){
                  fb_free(r->r_join_value);
                  r->r_join_value = NULL;
                  }
               rr_select(r, c_root, depth + 1);
               }
            }
      }


   static generate_join_value(r, f, n)
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
         /* find the *current* tree and evaluate it */
         if (tree_contains_field(eq_L, f))
            p = s_expr(eq_L);
         else
            p = s_expr(eq_R);
         if (p != NULL && *p != NULL){
            if (FB_OFNUMERIC(f->type))
               fb_rjustify(buf, p, f->size, f->type);
            else
               fb_pad(buf, p, f->size);
            fb_mkstr(&(r->r_join_value), buf);
            }
      }
