/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: u_proj.c,v 9.0 2001/01/09 02:55:52 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char U_project_sid[] = "@(#) $Id: u_proj.c,v 9.0 2001/01/09 02:55:52 john Exp $";
#endif

#include "dbsql_e.h"

extern char *cdb_S_EOREC;
extern char *cdb_S_FILLC;
extern char *cdb_S_FILLE;
extern short int cdb_dbase_byteorder;
extern short int cdb_cpu_byteorder;

static do_project();
static blind_record();

/*
 * u_project - project the sel_list according to the canonical restrict tree
 *	the assumption at this point is that all requests are in a
 *	single variable state, i.e., only one dbase.
 */

   u_project(selq, sel_list, where_cform, from, group_by, having,
         order_by)
      node *selq, *sel_list, *where_cform, *from, *group_by;
      node *having, *order_by;

      {
         fb_database *dp, *fp;
         int one_project(), st, blind_project(), o_flag = 0, g_flag = 0;
         int d_flag = 0;
         node *dn, *u_vir_dbase(), *sn;
         char iname[FB_MAXNAME], gname[FB_MAXNAME];

         if (sel_list == NULL){
            fb_serror(FB_MESSAGE, "Project - Null selection list", NIL);
            return(FB_ERROR);
            }
         dn = from;
         if (dn == NULL){
            fb_serror(FB_MESSAGE, "Project - Null database node", NIL);
            return(FB_ERROR);
            }
         dp = (fb_database *) dn->n_p1;
         if (dp == NULL){
            fb_serror(FB_MESSAGE, "Project - Null database pointer", NIL);
            return(FB_ERROR);
            }
         do_headers(sel_list);
         g_slist = sel_list;
         g_restrict = where_cform;
         g_from = from;
         g_order_by = order_by;
         g_group_by = group_by;
         group_expr = 0;
         st = FB_AOK;

         if (order_by->n_type != S_NULL)
            o_flag = 1;				/* order by flag */
         if (group_by->n_type != S_NULL)
            g_flag = 1;				/* group by flag */
         if (selq->n_type == Q_DISTINCT)
            d_flag = 1;				/* distinct flag */

         if (d_flag && g_flag){
            fb_serror(FB_MESSAGE, "Cannot do Distinct And Group By", NIL);
            d_flag = 0;
            }

         /* in my implementation, groupby implies orderby - disable orderby */
         if (g_flag && o_flag)
            o_flag = 0;

         if (g_flag){
            /* create group by index for use down below, store in gname */
            st = u_order_by(gname, dp, group_by, where_cform, 0);
            if (st == FB_ERROR)
               return(FB_ERROR);
            }

         if (d_flag)
            st = u_distinct(selq, sel_list, dp, where_cform);

         /* virtual for orderby, groupby, and for create view command */
         if (o_flag || g_flag || save_virtual)
            create_virtual = 1;
         else
            create_virtual = 0;

         if (create_virtual)
            set_virtual_dbase();

         if (g_flag){
            /* group by */
            if ((st = fb_openidx(gname, dp)) != FB_ERROR)
               st = u_groupby(dp, sel_list, having, from);
            if (st == FB_ERROR)
               fb_serror(FB_MESSAGE, "Group By Failed", NIL);
            }
         else if (d_flag){
            /* distinct */
            if (st != FB_ERROR)
               if ((st = fb_openidx(selq->n_virlist->n_nval, dp)) != FB_ERROR)
                  fb_forxeach(dp, one_project);
            if (st == FB_ERROR)
               fb_serror(FB_MESSAGE, "Distinct Project - distinct set failed",
                  NIL);
            }
         else /* default path */
            whereeach(dp, one_project, where_cform);

         if (st != FB_ERROR && create_virtual){
            end_virtual_dbase();
            /* save the first virtual database into fp */
            fp = vp;
            sn = vn;
            sn->n_fval = 1;
            if (fb_opendb(fp, READWRITE, FB_ALLINDEX, FB_OPTIONAL_INDEX) ==
                  FB_ERROR){
               fb_serror(FB_MESSAGE, "Cannot open virtual database:",
                  fp->dbase);
               st = FB_ERROR;
               }
            if (st != FB_ERROR){
               if (o_flag){
                  st = u_order_by(iname, fp, order_by, (node *) NULL, 1);
                  if (st != FB_ERROR)
                     st = fb_openidx(iname, fp);
                  if (st != FB_ERROR && save_virtual)
                     set_virtual_dbase();
                  if (st != FB_ERROR)
                     fb_forxeach(fp, blind_project);
                  else
                     fb_serror(FB_MESSAGE, "Project - OrderBy failed", NIL);
                  }
               else if (g_flag){		/* g_flag - groupby */
                  if (save_virtual)
                     set_virtual_dbase();
                  fb_foreach(fp, blind_project);
                  }
               if (save_virtual && (o_flag || g_flag)){
                  end_virtual_dbase();
                  fb_closedb(fp);
                  if (fb_opendb(vp, READWRITE, FB_ALLINDEX, FB_OPTIONAL_INDEX)
                        == FB_ERROR){
                     fb_serror(FB_MESSAGE, "Cannot open virtual database:",
                        vp->dbase);
                     st = FB_ERROR;
                     }
                  }
               }
            }
         return(st);
      }

/*
 * one_project - first layer of projection, from foreach/whereeach
 */

   one_project(dp)
      fb_database *dp;

      {
         if (!create_virtual)
            do_project(g_slist);
         else				/* dump into a virtual dbase */
            do_record(g_slist);
         if (!full_height(g_slist))
            return(FB_ERROR);
         else
            return(FB_AOK);
      }

   full_height(n)
      node *n;

      {
         for (; n != NULL; n = n->n_list)
            if (n->n_height != 0)
               return(1);
         return(0);
      }

/*
 * do_project - project doing expr()s the whole way
 */

   static do_project(slist)
      node *slist;

      {
         node *n, *t;
         short int firstloop = 1;

         if (linenumber == 0){
            h_headers();
            if (print_horizontal)
               print_headers();
            }
         h_indent();
         t = title_node;
         if (html)
            html_row_open();
         for (n = slist; n != NULL; n = n->n_list){
            if (!print_horizontal && !emitflag){
               if (!firstloop)
                  h_indent();
               fprintf(sql_ofs, "%-10s: ", t->n_nval);
               t = t->n_list;
               }
            if (emitflag && !firstloop && !html)
               fprintf(sql_ofs, "%c", separator);
            firstloop = 0;
            p_expr(n);
            if (!print_horizontal && !emitflag && !lastchar_was_newline)
               h_newline();
            }
         if (!lastchar_was_newline && !html)
            h_newline();
         if (html)
            html_row_close();
      }

/*
 * do_record - this is do_project except it writes to vp, the virtual dbase
 *	also, write the map as it goes, for speed
 *	and, during the first pass, generate the ddict file for vp. whew.
 */

   do_record(slist)
      node *slist;

      {
         node *n;
         char *p, buf[FB_MAXLINE], t;
         fb_database *d;
         fb_field *f;
         int fd = -1, fnum = 1;
         long w_rlen, w_rpos;

         if (create_ddict){
	    close(creat(vp->ddict, 0666));	/* create the ddict */
            fd = fb_mustopen(vp->ddict, 1);
            }
         for (rlen = 0, n = slist; n != NULL; n = n->n_list, fnum++){
            expr(n);
            if (create_ddict){
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

            /*
             * this does raw output, no padding, no forming.
             */
            if (istype_fld(n)){
               f = (fb_field *) n->n_p2;
               if (f->type == FB_FORMULA){
                  d = (fb_database *) n->n_p1;
                  fb_fetch(f, cdb_afld, d);
                  p = cdb_afld;
                  }
               else
                  p = f->fld;
               }
            else if (istype_str(n))
               p = n->n_nval;
            else{
               sprintf(buf, "%.*f", n->n_scale, n->n_fval);
               p = buf;
               }

            /* write database to chanel 0 */
            fb_nextwrite(0, p);
            fb_w_write(0, "\000");

            rlen += (strlen(p) + 1);
            }

         /* write end of record stuff, deletion marker, NULL, and cdb_EOREC */
         fb_nextwrite(0, " ");      	/* deletion place holder */
         fb_w_write(0, "\000");
         fb_w_write(0, cdb_S_EOREC);	/* end of record marker */
         rlen += 3;

	 if (rlen < MIN_RLEN){			/* fill hole if needed */
	    while (++rlen < MIN_RLEN)
	       fb_nextwrite(0, cdb_S_FILLC);
	    fb_nextwrite(0, cdb_S_FILLE);		/* rec[rlen] = fille */
	    }

         w_rpos = rpos;
         w_rlen = rlen;
         if (cdb_dbase_byteorder != cdb_cpu_byteorder){
            M_32_SWAP(w_rpos);
            M_32_SWAP(w_rlen);
            }
         /* write map to chanel 1 - map for this record */
	 fb_w_writen(1, (char *) &w_rpos, FB_SLONG);
	 fb_w_writen(1, (char *) &w_rlen, FB_SLONG);
	 rpos += rlen;
         wcount++;
         if (create_ddict){
            create_ddict = 0;
            close(fd);
            }
      }

/*
 * blind_project - this does not evaluate any expressions -- just projects
 */

   blind_project(dp)
      fb_database *dp;

      {
         int i, size;
         fb_field *f;
         char *fb_trim(), *p;
         short int firstloop = 1;
         node *n, *t;

         if (create_virtual)
            return(blind_record(dp));
         if (linenumber == 0){
            h_headers();
            if (print_horizontal)
               print_headers();
            }
         h_indent();
         t = title_node;
         if (html)
            html_row_open();
         for (n = g_slist, i = 0; i < dp->nfields; i++){
            f = dp->kp[i];
            if (n != NULL && n->n_width > 0 && print_horizontal)
               size = n->n_width;
            else
               size = n->n_vwidth;
            if (!print_horizontal && !emitflag){
               if (!firstloop)
                  h_indent();
               fprintf(sql_ofs, "%-10s: ", t->n_nval);
               t = t->n_list;
               }
            if (html){
               if (FB_OFNUMERIC(f->type))
                  html_cell_open(1);
               else
                  html_cell_open(0);
               }
            if (emitflag && !firstloop && !html)
               fprintf(sql_ofs, "%c", separator);
            firstloop = 0;
            if (quoteflag && !(FB_OFNUMERIC(f->type)) && f->type != FB_FORMULA)
               fprintf(sql_ofs, "\"");
            fb_fetch(f, cdb_bfld, dp);		/* enables FB_FORMULAS */
            if (!emitflag)
               fb_formfield(cdb_afld, cdb_bfld, f->type, size);
            else if (emitflag && verbose &&
                  (f->type ==FB_DATE || f->type == FB_DOLLARS)){
               fb_formfield(cdb_afld, cdb_bfld, f->type, size);
               fb_rmlead(cdb_afld);
               fb_trim(cdb_afld);
               }
            else
               strcpy(cdb_afld, cdb_bfld);
            if (newline_flag == 0){
               for (p = cdb_afld; *p; p++){ /* escape emb quotes/backg */
                  if (*p == CHAR_NEWLINE)
                     fprintf(sql_ofs, "\\n");
                  else {
                     if (*p == CHAR_QUOTE || *p == CHAR_BACKSLASH)
                        fprintf(sql_ofs, "\\");
                     fprintf(sql_ofs, "%c", *p);
                     }
                  }
               }
            else if (emitflag)
               fprintf(sql_ofs, "%s", cdb_afld);
            else if (print_horizontal)
               fprintf(sql_ofs, "%-*s ", size, cdb_afld);
            else
               fprintf(sql_ofs, "%s", fb_trim(cdb_afld));
            if (quoteflag && !(FB_OFNUMERIC(f->type)) && f->type != FB_FORMULA)
               fprintf(sql_ofs, "\"");
            if (!print_horizontal && !emitflag && !lastchar_was_newline)
               h_newline();
            if (n != NULL)
               n = n->n_list;
            if (html)
               html_cell_close();
            }
         if (!lastchar_was_newline && !html)
            h_newline();
         if (html)
            html_row_close();
         return(FB_AOK);
      }

/*
 * blind_record - this is blind_project except it writes to vp
 *	also, write the map as it goes, for speed
 *	and, during the first pass, generate the ddict file for vp. whew.
 *	model copied from do_record
 */

   static blind_record(dp)
      fb_database *dp;

      {
         char *p, buf[FB_MAXLINE], t;
         fb_field *f;
         int fd = -1, i, size;
         node *n;
         long w_rlen, w_rpos;

         if (create_ddict){
	    close(creat(vp->ddict, 0666));	/* create the ddict */
            fd = fb_mustopen(vp->ddict, 1);
            }
         for (n = g_slist, rlen = 0, i = 0; i < dp->nfields; i++){
            f = dp->kp[i];
            if (n != NULL && n->n_width > 0)
               size = n->n_width;
            else
               size = f->size;
            if (f->type == FB_FORMULA){
               t = FB_FLOAT;
               fb_fetch(f, cdb_afld, dp);
               p = cdb_afld;
               }
            else{
               p = f->fld;
               t = f->type;
               }
               
            if (create_ddict){
               sprintf(buf, "%s %c %d", f->id, t, size);
               if (f->idefault != NULL)
                  sprintf(buf, "%s -d %s", buf, f->idefault);
               strcat(buf, "\n");
               write(fd, buf, strlen(buf));
               }

            /*
             * this does raw output, no padding, no forming.
             */
            /* write database to chanel 0 */
            fb_nextwrite(0, p);
            fb_w_write(0, "\000");

            rlen += (strlen(p) + 1);
            if (n != NULL)
               n = n->n_list;
            }

         /* write end of record stuff, deletion marker, NULL, and cdb_EOREC */
         fb_nextwrite(0, " ");      	/* deletion place holder */
         fb_w_write(0, "\000");
         fb_w_write(0, cdb_S_EOREC);	/* end of record marker */
         rlen += 3;

	 if (rlen < MIN_RLEN){			/* fill hole if needed */
	    while (++rlen < MIN_RLEN)
	       fb_nextwrite(0, cdb_S_FILLC);
	    fb_nextwrite(0, cdb_S_FILLE);		/* rec[rlen] = fille */
	    }

         w_rpos = rpos;
         w_rlen = rlen;
         if (cdb_dbase_byteorder != cdb_cpu_byteorder){
            M_32_SWAP(w_rpos);
            M_32_SWAP(w_rlen);
            }
         /* write map to chanel 1 - map for this record */
	 fb_w_writen(1, (char *) &w_rpos, FB_SLONG);
	 fb_w_writen(1, (char *) &w_rlen, FB_SLONG);
	 rpos += rlen;
         wcount++;
         if (create_ddict){
            create_ddict = 0;
            close(fd);
            }
         return(FB_AOK);
      }
