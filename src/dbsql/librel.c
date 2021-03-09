/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: librel.c,v 9.0 2001/01/09 02:55:48 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Librel_sid[] = "@(#) $Id: librel.c,v 9.0 2001/01/09 02:55:48 john Exp $";
#endif

#include "dbsql_e.h"

static page_out();
static page_in();

/*
 * librel - library of relation routines
 */

/*
 * create_relation - create a relation structure for n variables
 */

   relation *create_relation(n, v, tot_n)
      int n, tot_n;
      v_matrix *v;

      {
         relation *r;
         char tmpname[FB_MAXNAME];
         int i;

         r = (relation *) fb_malloc(sizeof(relation));
         sprintf(tmpname, "%s/dbsql_XXXXXX", sql_tempdir);
         close(mkstemp(tmpname));
         r->r_tmpfile = NULL;
         fb_mkstr(&(r->r_tmpfile), tmpname);
         r->r_fd = fb_mustopen(r->r_tmpfile, READWRITE);

         r->r_dbase = (fb_database **) fb_malloc(n * sizeof(fb_database *));
         for (i = 0; i < n; i++)
            r->r_dbase[i] = NULL;
         r->r_rec = (long *) fb_malloc(n * sizeof(long));
         r->r_join_value = NULL;

         r->r_nrecs = n;
         r->r_recsiz = n * sizeof(long);
         r->r_reccnt = 0;
         r->r_curp = 0;
         r->r_npages = 0;

         /* as long as where v is pointing does not go away this is FB_AOK */
         r->r_vm = v;
         r->r_nvars = tot_n;

         r->r_used = 0;

         r->r_vars = NULL;
         r->r_next = NULL;

         for (i = 0; i < REL_PER_PAGE; i++)
            r->r_mem[i] = 0;
         r->r_page_id = 0;
         r->r_writepage = 0;

         for (i = 0; i < MAXVARIABLES; i++){
            r->r_offset[i] = 0;
            r->r_isize[i] = 0;
            }

         r->r_irecsiz = 0;
         r->r_ireccnt = 0;
         r->r_ifields = 0;
         r->r_irec = NULL;
         r->r_ibase = NULL;
         r->r_idict = NULL;
         r->r_index = NULL;
         r->r_aid = NULL;
         r->r_join_op = 0;
         r->r_join_fld = NULL;

         r->r_glink = r_ghead;
         r_ghead = r;

         return(r);
      }

/*
 * destroy_relation - destroy the relation r
 */

   static destroy_relation(r)
      relation *r;

      {
         close(r->r_fd);
         unlink(r->r_tmpfile);

         fb_free((char *) r->r_dbase);
         fb_free((char *) r->r_tmpfile);
         fb_free((char *) r->r_rec);
         fb_free((char *) r->r_join_value);
         fb_free(r->r_irec);
         fb_free(r->r_ibase);
         fb_free(r->r_idict);
         fb_free(r->r_index);
         fb_free((char *) r);
      }

/*
 * putrec_relation - fb_put the record into relation r. ZERO based. 0..n-1
 */

   putrec_relation(n, r)
      long n;
      relation *r;

      {
         long npage, *lp, nrecs;
         int j, pos;

         /* calculate the fb_page location location */
         nrecs = n * r->r_nrecs;
         npage = (nrecs) / REL_PER_PAGE;
         if (npage != r->r_page_id)
            page_in(npage, r);
         pos = (nrecs) % REL_PER_PAGE;
         for (lp = r->r_rec, j = 0; j < r->r_nrecs; pos++, lp++, j++){
            if (pos >= REL_PER_PAGE){
               page_out(r);
               page_in(r->r_page_id + 1, r);
               pos = 0;
               }
            r->r_mem[pos] = *lp;
            r->r_writepage = 1;
            }
         return(FB_AOK);
      }

/*
 * addrec_relation - add the record into relation r. ZERO based. 0..n-1
 *	go to the end, add one and putrec it.
 */

   addrec_relation(r)
      relation *r;

      {
         /* calculate the fb_page location location */
         r->r_reccnt++;
         return(putrec_relation(r->r_reccnt - 1, r));
      }

/*
 * getrec_relation - get the record into relation r
 */

   getrec_relation(n, r)
      long n;
      relation *r;

      {
         long npage, *lp, nrecs;
         int j, pos;

         /* calculate the fb_page location location */
         nrecs = n * r->r_nrecs;
         npage = (nrecs) / REL_PER_PAGE;
         if (npage != r->r_page_id)
            page_in(npage, r);
         pos = (nrecs) % REL_PER_PAGE;
         for (lp = r->r_rec, j = 0; j < r->r_nrecs; pos++, lp++, j++){
            if (pos >= REL_PER_PAGE){
               page_out(r);
               page_in(r->r_page_id + 1, r);
               pos = 0;
               }
            *lp = r->r_mem[pos];
            }
         r->r_curp = n;
         return(FB_AOK);
      }

/*
 * getrec_loadrel - load the records represented by the relation
 */

   getrec_loadrel(r)
      relation *r;

      {
         long rec;
         fb_database *dp;
         int i;
         
         for (i = 0; i < r->r_nrecs; i++){
            rec = r->r_rec[i];
            dp = r->r_dbase[i];
            if (u_getrec(rec, dp) == FB_ERROR)
               fb_xerror(FB_FATAL_GETREC, dp->dbase, (char *) &rec);
            }
      }

/*
 * flush_relation - flush the relation r and NULL it out
 */

   flush_relation(r)
      relation *r;

      {
         int i;

         if (r->r_writepage)
            page_out(r);
         for (i = 0 ; i < REL_PER_PAGE; i++)
            r->r_mem[i] = 0;
         r->r_page_id = -1;
      }

/*
 * page_out - write fb_page mechanism for relation getrec/putrec
 */

   static page_out(r)
      relation *r;

      {
         long npos;

         npos = r->r_page_id * (DBSQL_PAGESIZE * FB_SLONG);
	 if (lseek(r->r_fd, npos, 0) < 0L)
            fb_xerror(FB_SEEK_ERROR, SYSMSG[S_BAD_DATA], NIL);
	 if (write(r->r_fd, (char *) r->r_mem, DBSQL_PAGESIZE) !=
               DBSQL_PAGESIZE)
	    fb_xerror(FB_WRITE_ERROR, SYSMSG[S_IOERROR], NIL);
         r->r_writepage = 0;
         if (r->r_npages < r->r_page_id)
            r->r_npages = r->r_page_id;
         return(FB_AOK);
      }

/*
 * page_in - read fb_page mechanism for relation getrec/putrec
 */

   static page_in(n, r)
      long n;
      relation *r;

      {
         int i;
         long npos;

         /* check if current fb_page needs writing */
         if (r->r_writepage)
            page_out(r);
         /* set current fb_page and read it into mem */
         r->r_page_id = n;
         if (n > r->r_npages){
            for (i = 0; i < REL_PER_PAGE; i++)
               r->r_mem[i] = 0;
            return(FB_AOK);
            }
         npos = r->r_page_id * (DBSQL_PAGESIZE * FB_SLONG);
	 if (lseek(r->r_fd, npos, 0) < 0L)
            fb_xerror(FB_SEEK_ERROR, SYSMSG[S_BAD_DATA], NIL);
	 if (read(r->r_fd, (char *) r->r_mem, DBSQL_PAGESIZE) !=
               DBSQL_PAGESIZE)
	    fb_xerror(FB_READ_ERROR, SYSMSG[S_IOERROR], NIL);
         return(FB_AOK);
      }

/*
 * var_in_relation - test whether the variable in n is in the list of r
 *	return 1 if yes, 0 if no.
 */

   var_in_relation(n, r)
      node *n;
      relation *r;

      {
         cell *c;
         char *dname;
         node *q;

         c = (cell *) n->n_obj;
         dname = c->c_sval;
         for (q = r->r_vars; q != NULL; q = q->n_list){
            c = (cell *) q->n_obj;
            if (equal(c->c_sval, dname))
               return(1);
            }
         return(0);
      }

/*
 * r_clear_relations - clear the fixed relations kept inbetween subQ's.
 */

   r_clear_relations()
      {
         int i;

         for (i = 0; i < MAXVARIABLES; i++){
            rel_depth[i] = NULL;
            rel_single[i] = NULL;
            }
      }

/*
 * r_store_vals - store the values in rel_val into the record and addrec it
 */

   r_store_vals(r)
      relation *r;

      {
         int i;

         for (i = 0; i < r->r_nrecs; i++)
            r->r_rec[i] = rel_val[i];
         addrec_relation(r);
      }

/*
 * rel_enter - store into vr the relations values from r
 */

   rel_enter(vr, r)
      relation *vr, *r;

      {
         int i;

         for (i = 0; i < r->r_nrecs; i++)
            vr->r_rec[i] = r->r_rec[i];
         addrec_relation(vr);
      }

/*
 * gcollect_relation - garbage collect all of the relations that have been made
 */

   gcollect_relation()
      {
         relation *r, *rr;

         for (r = r_ghead; r != NULL; r = rr){
            rr = r->r_glink;
            destroy_relation(r);
            }
         r_ghead = NULL;
      }

#if DEBUG

   trace_relation(r, s)
      relation *r;
      char *s;

      {
         long rec;
         int i;

         fprintf(stderr, "%s\n", s);
         if (r == NULL){
            fprintf(stderr, "EMPTY (NULL) Relation\n");
            return;
            }
         fprintf(stderr, "   relations variables: ");
         tracelist(r->r_vars);

         fprintf(stderr, "   relations dbases: ");
         for (i = 0; i < r->r_nrecs; i++)
            if (r->r_dbase[i] != NULL)
               fprintf(stderr, "%s ", r->r_dbase[i]->dbase);
         fprintf(stderr, "\n");
         
         if (r->r_vm != NULL){
            fprintf(stderr, "   relations matrix components: ");
            for (i = 0; i < r->r_nvars; i++)
               fprintf(stderr, "%d ", r->r_vm->v_array[i]);
            fprintf(stderr, "\n");
            }
         fprintf(stderr, "   Relation reccnt: %ld\n", r->r_reccnt);
         fprintf(stderr, "   Relation join op: %d\n", r->r_join_op);
         fprintf(stderr, "   Actual relation values:\n");
         for (rec = 0; rec < r->r_reccnt; rec++){
            getrec_relation(rec, r);
            fprintf(stderr, "      ");
            for (i = 0; i < r->r_nrecs; i++)
               fprintf(stderr, "%10ld", r->r_rec[i]);
            fprintf(stderr, "\n");
            }
      }

#endif /* DEBUG */
