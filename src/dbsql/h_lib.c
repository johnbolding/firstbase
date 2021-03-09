/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: h_lib.c,v 9.1 2001/01/12 22:51:56 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char H_lib_sid[] = "@(#) $Id: h_lib.c,v 9.1 2001/01/12 22:51:56 john Exp $";
#endif

#include "dbsql_e.h"

extern struct tm *localtime();
extern char *cdb_user;
extern char *cdb_coname;

static char *h_buf1 = NULL;
static char *h_buf2 = NULL;

static char *DAYS[] = {
   "Sunday", "Monday", "Tuesday", "Wednesday",
   "Thursday", "Friday", "Saturday"};

static char *MONTHS[] = {
   "January", "February", "March", "April", "May", "June",
   "July", "August", "September", "October", "November", "December"};

static h_decode();
static h_metasub();
void h_footers();

/*
 * h_lib - libraries for the H_MASTER and header stuff
 */

/*
 * h_load - load in the headers from the control node, m
 */

   h_load(m)
      node *m;

      {
         node *h, *a, *f, *n;

         header1[0] = header2[0] = NULL;
         footer1[0] = footer2[0] = NULL;

         u_verify_sub(m, 0);
         n = m->n_narg[0];		/* master argument 0 - header */
         if (n->n_type != S_NULL){
            /* first header */
            a = n->n_narg[0];
            h = n->n_narg[1];
            if (a != NULL && h != NULL){
               if (a->n_type == H_ODD || a->n_type == S_NULL)
                  strcpy(header1, h->n_nval);
               else
                  strcpy(header2, h->n_nval);
               }

            /* second header */
            a = n->n_narg[2];
            h = n->n_narg[3];
            if (a != NULL && h != NULL){
               if (a->n_type == H_ODD || a->n_type == S_NULL)
                  strcpy(header1, h->n_nval);
               else
                  strcpy(header2, h->n_nval);
               }
            }

         n = m->n_narg[1];		/* master argument 1 - footer*/
         if (n->n_type != S_NULL){
            /* first footer */
            a = n->n_narg[0];
            f = n->n_narg[1];
            if (a != NULL && f != NULL){
               if (a->n_type == H_ODD || a->n_type == S_NULL)
                  strcpy(footer1, f->n_nval);
               else
                  strcpy(footer2, f->n_nval);
               }

            /* second footer */
            a = n->n_narg[2];
            f = n->n_narg[3];
            if (a != NULL && f != NULL){
               if (a->n_type == H_ODD || a->n_type == S_NULL)
                  strcpy(footer1, f->n_nval);
               else
                  strcpy(footer2, f->n_nval);
               }
            }
         if (header1[0] != NULL && header2[0] == NULL)
            strcpy(header2, header1);
         if (footer1[0] != NULL && footer2[0] == NULL)
            strcpy(footer2, footer1);
         if (h_buf1 == NULL){
            h_buf1 = fb_malloc(cdb_t_cols + FB_MAXLINE);
            h_buf1[0] = NULL;
            h_buf2 = fb_malloc(cdb_t_cols + FB_MAXLINE);
            h_buf2[0] = NULL;
            }
         last_printline = pagelength - (margin[3] + margin[4]) - 1;
      }

/*
 * h_newline - provide the newline function for formatting pages
 */

   void h_newline()
      {
         fprintf(sql_ofs, "\n");
         linenumber++;
         if (!formatpage)
            return;
         if (linenumber >= last_printline)
            h_footers();
      }

/*
 * h_indent - provide the indent function for formatting pages
 */

   h_indent()
      {
         if (formatpage)
            fprintf(sql_ofs, "%*s", pageindent, " ");
      }

/*
 * h_headers - print the formatpage headers
 */

   void h_headers()
      {
         int rem, i;
         char *p;

         if (!formatpage)
            return;
         pagenumber++;
         linenumber = 0;
         rem = pagenumber % 2;
         /* use header 2 for even, header 1 for odd */
         if (rem == 0)
            p = header2;
         else
            p = header1;
         for (i = 0; i < margin[1]; i++, linenumber++)
            fprintf(sql_ofs, "\n");
         if (p[0] != NULL){
            h_decode(h_buf1, p);
            /* fully formatted result is now in h_buf1 */
            h_indent();
            fprintf(sql_ofs, "%s\n", h_buf1);
            }
         linenumber++;
         for (i = 0; i < margin[2]; i++, linenumber++)
            fprintf(sql_ofs, "\n");
      }

/*
 * h_footers - print the formatpage footers
 */

   void h_footers()
      {
         int rem, i;
         char *p;

         if (!formatpage)
            return;
         /* pad if needed */
         for (; linenumber < last_printline; linenumber++)
            fprintf(sql_ofs, "\n");
         /* use footer 2 for even, footer 1 for odd */
         rem = pagenumber % 2;
         if (rem == 0)
            p = footer2;
         else
            p = footer1;
         for (i = 0; i < margin[3]; i++, linenumber++)
            fprintf(sql_ofs, "\n");
         if (p[0] != NULL){
            h_decode(h_buf1, p);
            /* fully formatted result is now in h_buf1 */
            h_indent();
            fprintf(sql_ofs, "%s\n", h_buf1);
            }
         linenumber++;
         for (i = 0; i < margin[4]; i++, linenumber++)
            fprintf(sql_ofs, "\n");
         /* now reset linenumber */
         linenumber = 0;
      }

/*
 * h_decode - decode p into a formatted header, replete with
 *	all tokesn resolved, three part title format handled, etc.
 *	build result into h.
 */

   static h_decode(h, p)
      char *h, *p;

      {
         char p1[FB_MAXLINE], p2[FB_MAXLINE], p3[FB_MAXLINE];
         int a1, a2, a3, b2, e2, b3, j;

         p1[0] = p2[0] = p3[0] = NULL;
         fb_subline(p1, p, 2, '\'');
         fb_subline(p2, p, 3, '\'');
         fb_subline(p3, p, 4, '\'');
         /* now sub out meta tokens for each piece */
         h_metasub(p1);
         h_metasub(p2);
         h_metasub(p3);
         /* now build h from left to right */
         strcpy(h, p1);
         a1 = strlen(p1);
         a2 = strlen(p2);
         a3 = strlen(p3);
         /* begin point for string p2 */
         b2 = (linelength / 2) - (a2 / 2);
         if (b2 < 0)
            b2 = 0;
         /* now b2 is the place to start string p2 -- blank pad h */
         for (j = a1; j < b2; j++)
            h[j] = FB_BLANK;
         h[b2] = NULL;
         strcat(h, p2);
         /* end point of current string */
         e2 = strlen(h);
         /* begin point for p3 */
         b3 = linelength - a3;
         if (b3 < 0)
            b3 = 0;
         for (j = e2; j < b3; j++)
            h[j] = FB_BLANK;
         h[b3] = NULL;
         strcat(h, p3);
      }

/*
 * h_metasub - handle the meta substitutes
 * 	$fb_page $date $user $day $month $dd $yyyy
 */

   static h_metasub(s)
      char *s;

      {
         char out[FB_MAXLINE], word[FB_MAXLINE];
         int i;
         time_t tme, *clock = &tme;
         struct tm xtm, *t = &xtm;
   
         time(clock);
         t = (struct tm *) localtime(clock);

         strcpy(out, s);
         s[0] = NULL;
         for (i = 1; (i = fb_gettoken(out, i, word, '$')) != 0; ){
            if (equal(word, "$page") || equal(word, "$PAGE")){
               sprintf(word, "%d", pagenumber);
               strcat(s, word);
               }
            else if (equal(word, "$user") || equal(word, "$USER"))
               strcat(s, cdb_user);
            else if (equal(word, "$coname") || equal(word, "$CONAME"))
               strcat(s, cdb_coname);
            else if (equal(word, "$date") || equal(word, "$DATE")){
               fb_simpledate(word, 1);
               strcat(s, word);
               }
            else if (equal(word, "$time") || equal(word, "$TIME")){
               fb_simpledate(word, 0);
               strcat(s, word);
               }
            else if (equal(word, "$miltime") || equal(word, "$MILTIME")){
               fb_simpledate(word, 2);
               strcat(s, word);
               }
            else if (equal(word, "$day") || equal(word, "$DAY"))
               strcat(s, DAYS[t->tm_wday]);
            else if (equal(word, "$month") || equal(word, "$MONTH"))
               strcat(s, MONTHS[t->tm_mon]);
            else if (equal(word, "$mm") || equal(word, "$MM")){
               sprintf(word, "%d", t->tm_mon + 1);
               strcat(s, word);
               }
            else if (equal(word, "$dd") || equal(word, "$DD")){
               sprintf(word, "%d", t->tm_mday);
               strcat(s, word);
               }
            else if (equal(word, "$yy") || equal(word, "$YY")){
               sprintf(word, "%d", t->tm_year);
               strcat(s, word);
               }
            else if (equal(word, "$yyyy") || equal(word, "$YYYY")){
               sprintf(word, "%d", t->tm_year + 1900);
               strcat(s, word);
               }
            else
               strcat(s, word);
            }
      }
