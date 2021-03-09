/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: headers.c,v 9.0 2001/01/09 02:55:47 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Headers_sid[] = "@(#) $Id: headers.c,v 9.0 2001/01/09 02:55:47 john Exp $";
#endif

#include "dbsql_e.h"

static char *dash_line = NULL;
static char *title_line = NULL;
static generate_headers();
static void make_head();
static head_width();

/*
 * do_headers - generate the simple column headers for the select list
 */

   void do_headers(slist)
      node *slist;

      {
         node *n;

         for (n = slist; n != NULL; n = n->n_list)
            make_head(n);
         if (html){
            html_headers(slist);
            return;
            }
         if (emitflag || save_virtual)
            return;
         if (head_width(slist) + pageindent >= cdb_t_cols)
            print_horizontal = 0;
         else
            print_horizontal = 1;
         generate_headers(slist);
      }

/*
 * generate_headers - do the actual generation of the headers
 */

   static generate_headers(slist)
      node *slist;

      {
         node *n, *t = NULL, *nt;
         int size, lj, i;
         char *p;
         fb_field *f;

         /* initialize the title spaces */
         size = head_width(slist);
         size += countlist(slist);
         size += 10;
         title_node = NULL;
         if (dash_line != NULL){
            fb_free(dash_line);
            dash_line = NULL;
            }
         if (title_line != NULL){
            fb_free(title_line);
            title_line = NULL;
            }
         dash_line = fb_malloc(size);
         title_line = fb_malloc(size);

         /* build the dash line */
         for (p = dash_line, n = slist; n != NULL; n = n->n_list){
            for (i = 0; i < n->n_width; i++)
               *p++ = '-';
            *p++ = ' ';
            }
         *p = NULL;

         /* now build the title line */
         title_line[0] = NULL;
         for (n = slist; n != NULL; n = n->n_list){
            lj = 0;
            if (istype_fld(n)){
               f = (fb_field *) n->n_p2;
               if (!FB_OFNUMERIC(f->type))
                  lj = 1;
               }
            else if (n->n_type == F_UPPER ||
                  n->n_type == F_LOWER ||
                  n->n_type == O_CONCAT ||
                  n->n_type == F_SUBLINE ||
                  n->n_type == F_SUBSTR)
               lj = 1;
            if (lj)
               sprintf(title_line, "%s%-*s",
                  title_line, n->n_width + 1, n->n_nval);
            else
               sprintf(title_line, "%s%*s ",
                  title_line, n->n_width, n->n_nval);
            if (!print_horizontal){
               nt = makenode();
               if (title_node == NULL)
                  title_node = nt;
               else
                  t->n_list = nt;
               t = nt;
               fb_mkstr(&(t->n_nval), n->n_nval);
               }
            }
      }

/*
 * print_headers - print the column headers
 */

   void print_headers()
      {
         if (emitflag || save_virtual)
            return;
         h_indent();
         fprintf(sql_ofs, "%s", dash_line);
         h_newline();
         h_indent();
         fprintf(sql_ofs, "%s", title_line);
         h_newline();
         h_indent();
         fprintf(sql_ofs, "%s", dash_line);
         h_newline();
      }

/*
 * make_head - recursive part of column header generation
 */

   static void make_head(n)
      node *n;

      {
         char buffer[FB_MAXLINE];
         cell *nc;
         node *n0, *n1, *tn;
         int nw0, nw1, nh0, nh1;
         int nv0, nv1;
         fb_field *f;

         if (n == NULL)
            return;
         n0 = n->n_narg[0];
         n1 = n->n_narg[1];
         if (n0 == NULL && n1 == NULL && n->n_nval != NULL &&
               n->n_type != F_COUNTALL && n->n_type != F_SYSDATE &&
               n->n_type != F_OWNER    && n->n_type != F_GROUP &&
               n->n_type != F_UID      && n->n_type != F_GID &&
               n->n_type != F_MODE)
            return;
         if (n->n_type != V_ID){
            make_head(n0);
            make_head(n1);
            }
         buffer[0] = NULL;
         nw0 = nw1 = 0;
         nh0 = nh1 = 0;
         nv0 = nv1 = 0;
         if (n->n_label != NULL){
            n1 = n->n_label;
            if (n1->n_type == V_SCON || n1->n_type == V_CCON)
               strcpy(buffer, n1->n_nval);
            else{
               nc = (cell *) n1->n_obj;
               strcpy(buffer, nc->c_sval);
               }
            /* set nw0 with the width of the label */
            nw0 = nv0 = strlen(buffer);
            /* set nw1 with the MAX width of the other branches */
            if (n0 != NULL){
               nw1 = MAX(nw1, n0->n_width);
               nh0 = n0->n_height;
               nv0 = MAX(nv0, n->n_vwidth);
               }
            if (n1 != NULL){
               nw1 = MAX(nw1, n1->n_width);
               nh1 = n1->n_height;
               nv1 = n1->n_vwidth;
               }
            }
         else if (n0 == NULL && n1 == NULL){
            /* catches no op functions, like count(*) */
            strcat(buffer, n->n_nval);
            nw0 = nv0 = strlen(buffer);
            }
         else if (istype_fcn(n)){
            if (n != NULL && n->n_nval != NULL){
               strcat(buffer, n->n_nval);
               strcat(buffer, "(");
               }
            if (n0 != NULL && n0->n_nval != NULL){
               strcat(buffer, n0->n_nval);
               nw0 = n0->n_width;
               nh0 = n0->n_height;
               }
            if (n->n_nval != NULL)
               strcat(buffer, ")");
            nv0 = nw0;
            }
         else if (istype_sfcn(n)){
            if (n0 != NULL && n0->n_nval != NULL){
               strcat(buffer, n0->n_nval);
               nw0 = n0->n_width;
               nh0 = n0->n_height;
               }
            if (n1 != NULL && n1->n_nval != NULL){
               nw1 = n1->n_width;
               nh1 = n0->n_height;
               nw0 = MAX(nw0, atoi(n1->n_nval));
               }
            nv0 = nw0;
            nv1 = nw1;
            }
         else {
            /* not a function - build header left to right */
            if (n0 != NULL && n0->n_nval != NULL){
               strcat(buffer, n0->n_nval);
               nw0 = n0->n_width;
               nh0 = n0->n_height;
               }
            if (n != NULL && n->n_nval != NULL){
               strcat(buffer, n->n_nval);
               }
            if (n1 != NULL && n1->n_nval != NULL){
               strcat(buffer, n1->n_nval);
               nw1 = n1->n_width;
               nh1 = n0->n_height;
               }
            nv0 = nw0;
            nv1 = nw1;
            }
         if (n->n_type == V_ID){
            f = (fb_field *) n->n_p2;
            if (n->n_label == NULL)
               strcpy(buffer, f->id);
            nv0 = f->size;
            nw0 = MAX(f->size, strlen(buffer));
            if (nw0 < 8 && f->type ==FB_DATE && (!emitflag || verbose))
               nw0 = 8;
            }
         fb_mkstr(&(n->n_nval), buffer);
         n->n_width = strlen(n->n_nval);
         if (n->n_type == F_SUBSTR || n->n_type == F_SUBLINE ||
               n->n_type == F_FORMFIELD){
            tn = n->n_narg[2];
            if (tn->n_type == V_CON || tn->n_type == V_FCON){
               nc = (cell *) tn->n_obj;
               nw0 = MAX(atoi(nc->c_sval), n->n_width);
               nw1 = 0;
               nv0 = MAX(atoi(nc->c_sval), n->n_vwidth);
               nv1 = 0;
               }
            }
         else if (n->n_type == O_CONCAT){
            nw0 = n0->n_width + n1->n_width + 1;
            nh0 = 1;
            nw1 = 0;
            nv0 = n0->n_vwidth + n1->n_vwidth + 1;
            }
         n->n_width = MAX(MAX(n->n_width, nw0), nw1);
         n->n_vwidth = MAX(MAX(n->n_vwidth, nv0), nv1);
         if (nh0 != 0)
            n->n_height = nh0;
         if (nh1 != 0)
            n->n_height = nh1;
         if (istype_fcn(n))
            n->n_height = 0;
         else if (istype_sfcn(n))
            n->n_height = 1;
      }

/*
 * head_width - calculate the width of the nodes (column width).
 */

   static head_width(n)
      node *n;

      {
         int i = 0;

         for (; n != NULL; n = n->n_list)
            i += n->n_width;
         return(i);
      }
