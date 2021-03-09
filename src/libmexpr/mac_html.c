/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_html.c,v 9.0 2001/01/09 02:56:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Mac_html_sid[] = "@(#) $Id: mac_html.c,v 9.0 2001/01/09 02:56:51 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>

#define HTML_MAXLINE	1024
#define HTML_TEXTLINE	5120

/*
 * html components
 */

/*
 * mf_html_open - return string with beginnings of HTML document
 *	html_open(title, [basehref, bodytag, extratags])
 */

   mf_html_open(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_PRINTLINE], hbuf[FB_PRINTLINE], bbuf[FB_PRINTLINE];
         char bodytag[FB_PRINTLINE], excesstag[HTML_MAXLINE * 10];
         int maxlen, rn;

         (void) Mac_html_sid;

         rn = fb_realnodes(n);
         if (rn < 1)
            return(FB_ERROR);
         tostring(n);
         maxlen = FB_PRINTLINE - 30;
         if (strlen(n->n_pval) > maxlen)
            n->n_pval[maxlen] = NULL;
         strcpy(hbuf, n->n_pval);
         bbuf[0] = NULL;
         bodytag[0] = NULL;
         excesstag[0] = NULL;
         if (rn >= 2){
            n = n->n_next;
            tostring(n);
            if (strlen(n->n_pval) > 0)
               sprintf(bbuf, "<BASE HREF=\"%s\">\n", n->n_pval);
            }
         if (rn >= 3){
            n = n->n_next;
            tostring(n);
            strcpy(bodytag, n->n_pval);
            }
         if (rn >= 4){
            n = n->n_next;
            tostring(n);
            strcpy(excesstag, n->n_pval);
            }
         sprintf(buf,
            "<HTML>\n<HEAD>\n%s<TITLE>\n%s</TITLE>\n%s</HEAD>\n<BODY %s>\n",
            bbuf, hbuf, excesstag, bodytag);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_close(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         sprintf(buf, "</BODY>\n</HTML>\n");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_table_open - returns string to open HTML table
 *	html_table_open(border, cellpadding, directive ...)
 *		border and cellpadding are optional, sort of.
 *		to specify a cellpadding, you gotta use a border arg too.
 */

   mf_html_table_open(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE], obuf[FB_MAXLINE];
         int border = 0, cellpadding = 0, rn;
         fb_mnode *n1, *n2;

         rn = fb_realnodes(n);
         if (rn >= 1){
            n1 = n;   n = n->n_next;
            n2 = n;
            if (n1 != NULL)
               border = n1->n_fval;
            if (n2 != NULL)
               cellpadding = n2->n_fval;
            }
         strcpy(buf, "<TABLE");
         if (border > 0)
            sprintf(buf, "%s BORDER=%d", buf, border);
         if (cellpadding > 0)
            sprintf(buf, "%s CELLPADDING=%d", buf, cellpadding);

         obuf[0] = NULL;
         if (n != NULL)
            for (n = n->n_next; n != NULL; n = n->n_next){
               tostring(n);
               sprintf(obuf, "%s %s", obuf, n->n_pval);
               }

         strcat(buf, obuf);
         strcat(buf, ">\n");

         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_table_close(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         sprintf(buf, "</TABLE>\n");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_row(n, r)
      fb_mnode *n, *r;

      {
         char buf[HTML_MAXLINE], align[FB_MAXLINE], rowspan[FB_MAXLINE];
         char width[FB_MAXLINE], nowrap[FB_MAXLINE];
         int cc, nlen = 0;

         sprintf(buf, "<TR>\n");
         cc = 4;
         /* for each n arg, tostring it and place it in a column */
         for (; n != NULL; n = n->n_next){
            tostring(n);
            strcpy(align, "ALIGN=LEFT");
            strcpy(rowspan, "");
            strcpy(width, "");
            strcpy(nowrap, "");
            nlen = 0;
            if (strncmp(n->n_pval, "ALIGN=", 6) == 0 ||
                strncmp(n->n_pval, "Align=", 6) == 0 ||
                strncmp(n->n_pval, "align=", 6) == 0){
               strcpy(align, n->n_pval);
               nlen += strlen(align);
               n = n->n_next;
               if (n == NULL)
                  return(FB_ERROR);
               tostring(n);
               }
            if (strncmp(n->n_pval, "NOWRAP", 6) == 0 ||
                strncmp(n->n_pval, "Nowrap", 6) == 0 ||
                strncmp(n->n_pval, "nowrap", 6) == 0){
               strcpy(nowrap, n->n_pval);
               nlen += strlen(nowrap);
               n = n->n_next;
               if (n == NULL)
                  return(FB_ERROR);
               tostring(n);
               }
            if (strncmp(n->n_pval, "ROWSPAN=", 8) == 0 ||
                strncmp(n->n_pval, "Rowspan=", 8) == 0 ||
                strncmp(n->n_pval, "rowspan=", 8) == 0){
               strcpy(rowspan, n->n_pval);
               nlen += strlen(rowspan);
               n = n->n_next;
               if (n == NULL)
                  return(FB_ERROR);
               tostring(n);
               }
            if (strncmp(n->n_pval, "WIDTH=", 6) == 0 ||
                strncmp(n->n_pval, "Width=", 6) == 0 ||
                strncmp(n->n_pval, "width=", 6) == 0){
               strcpy(width, n->n_pval);
               nlen += strlen(width);
               n = n->n_next;
               if (n == NULL)
                  return(FB_ERROR);
               tostring(n);
               }
            nlen += (12 + strlen(n->n_pval));
            if (cc + nlen >= HTML_MAXLINE)
               return(FB_ERROR);
            cc += nlen;
            sprintf(buf, "%s   <TD %s %s %s %s>%s</TD>\n", buf, align,
               rowspan, width, nowrap, n->n_pval);
            }
         strcat(buf, "</TR>\n");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_row_open - html_row_open(fs1, ...)
 *	opens a row, uses any passed in strings as formatting strings
 *	<TR %s %s ...>
 */

   mf_html_row_open(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];
         int cc, len;

         sprintf(buf, "   <TR");
         cc = strlen(buf);
         if (fb_realnodes(n) >= 1){
            for (; n != NULL; n = n->n_next){
               tostring(n);
               len = strlen(n->n_pval);
               if (cc + len >= FB_MAXLINE - 1)
                  return(FB_ERROR);
               sprintf(buf, "%s %s", buf, n->n_pval);
               cc += len;
               }
            }
         strcat(buf, ">");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_row_close(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         sprintf(buf, "</TR>\n");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_cell_open - html_cell_open(fs1, ...)
 *	opens a cell, uses any passed in strings as formatting strings
 *	<TD %s %s ...>
 */

   mf_html_cell_open(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];
         int cc, len;

         sprintf(buf, "   <TD");
         cc = strlen(buf);
         if (fb_realnodes(n) >= 1){
            for (; n != NULL; n = n->n_next){
               tostring(n);
               len = strlen(n->n_pval);
               if (cc + len >= FB_MAXLINE - 1)
                  return(FB_ERROR);
               sprintf(buf, "%s %s", buf, n->n_pval);
               cc += len;
               }
            }
         strcat(buf, ">");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_cell_close(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         sprintf(buf, "</TD>\n");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_table_headers - N arguments, all table headers
 */

   mf_html_table_headers(n, r)
      fb_mnode *n, *r;

      {
         char buf[1024];
         int len, nlen;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         strcpy(buf, "<TR>\n");
         len = 5;
         for (; n != NULL; n = n->n_next){
            tostring(n);
            nlen = strlen(n->n_pval);
            if (len + nlen + 26 >= 1024)
               break;
            sprintf(buf, "%s  <TH>\n%s  </TH>\n", buf, n->n_pval);
            }
         strcat(buf, "</TR>\n");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_h_open - header [sub level] open
 *	html_h_open(level)
 */

   mf_html_h_open(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         sprintf(buf, "<H%d>", atoi(n->n_pval));
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_h_close - header [sub level] close
 *	html_h_close(level)
 */

   mf_html_h_close(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         sprintf(buf, "</H%d>", atoi(n->n_pval));
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_h1 - header1 string
 *	html_h1(header)
 */

   mf_html_h1(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];
         int maxlen;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         maxlen = FB_MAXLINE - 20;
         if (strlen(n->n_pval) > maxlen)
            n->n_pval[maxlen] = NULL;
         sprintf(buf, "<H1>%s</H1>", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_h2(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];
         int maxlen;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         maxlen = FB_MAXLINE - 20;
         if (strlen(n->n_pval) > maxlen)
            n->n_pval[maxlen] = NULL;
         sprintf(buf, "<H2>%s</H2>", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_h3(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];
         int maxlen;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         maxlen = FB_MAXLINE - 20;
         if (strlen(n->n_pval) > maxlen)
            n->n_pval[maxlen] = NULL;
         sprintf(buf, "<H3>%s</H3>", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_h4(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];
         int maxlen;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         maxlen = FB_MAXLINE - 20;
         if (strlen(n->n_pval) > maxlen)
            n->n_pval[maxlen] = NULL;
         sprintf(buf, "<H4>%s</H4>", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_h5(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];
         int maxlen;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         maxlen = FB_MAXLINE - 20;
         if (strlen(n->n_pval) > maxlen)
            n->n_pval[maxlen] = NULL;
         sprintf(buf, "<H5>%s</H5>", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_h6(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];
         int maxlen;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         maxlen = FB_MAXLINE - 20;
         if (strlen(n->n_pval) > maxlen)
            n->n_pval[maxlen] = NULL;
         sprintf(buf, "<H6>%s</H6>", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_center(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];
         int maxlen;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         maxlen = FB_MAXLINE - 18;
         if (strlen(n->n_pval) > maxlen)
            n->n_pval[maxlen] = NULL;
         sprintf(buf, "<CENTER>%s</CENTER>", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_center_open(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "<CENTER>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_center_close(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "</CENTER>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_strong_open(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "<STRONG>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_strong_close(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "</STRONG>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_em_open(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "<EM>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_em_close(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "</EM>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_bold(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];
         int maxlen;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         maxlen = FB_MAXLINE - 10;
         if (strlen(n->n_pval) > maxlen)
            n->n_pval[maxlen] = NULL;
         sprintf(buf, "<B>%s</B>", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_bold_open(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "<B>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_bold_close(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "</B>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_italics(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];
         int maxlen;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         maxlen = FB_MAXLINE - 10;
         if (strlen(n->n_pval) > maxlen)
            n->n_pval[maxlen] = NULL;
         sprintf(buf, "<I>%s</I>", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_italics_open(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "<I>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_italics_close(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "</I>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_p_open(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "<P>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_p_close(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "</P>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_pre_open(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "<PRE>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_pre_close(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "</PRE>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_form_open - html_form_open(action, enctype)
 */

   mf_html_form_open(n, r)
      fb_mnode *n, *r;

      {
         char buf[HTML_MAXLINE], ebuf[FB_MAXLINE];
         int maxlen, rn;
         fb_mnode *n_action;

         rn = fb_realnodes(n);
         if (rn < 1)
            return(FB_ERROR);
         maxlen = HTML_MAXLINE - 30;
         tostring(n);
         if (strlen(n->n_pval) > maxlen)
            n->n_pval[maxlen] = NULL;
         ebuf[0] = NULL;
         n_action = n; n = n->n_next;
         if (n != NULL){
            tostring(n);
            if (strncmp(n->n_pval, "MULTI", 5) == 0 ||
                strncmp(n->n_pval, "multi", 5) == 0)
               strcpy(ebuf, "ENCTYPE=\"multipart/form-data\"");
            }
         sprintf(buf, "<FORM METHOD=\"POST\" %s \n   ACTION=\"%s\">",
            ebuf, n_action->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_form_close(n, r)
      fb_mnode *n, *r;

      {
         fb_mkstr(&(r->n_nval), "</FORM>");
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_input - html_input(type, name, size, maxsize, value)
 */

   mf_html_input(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n_type, *n_name, *n_size, *n_maxlength, *n_value;
         fb_mnode *n_checked;
         char *buf;
         int rn, cc;

         rn = fb_realnodes(n);
         if (rn < 2)
            return(FB_ERROR);
         n_type = n;		n = n->n_next;
         n_name = n;		n = n->n_next;
         tostring(n_type);
         tostring(n_name);
         n_size = n_maxlength = n_value = n_checked = NULL;
         if (n != NULL){
            n_size = n;		n = n->n_next;
            tostring(n_size);
            }
         if (n != NULL){
            n_maxlength = n;	n = n->n_next;
            tostring(n_maxlength);
            }
         if (n != NULL){
            n_value = n;	n = n->n_next;
            tostring(n_value);
            }
         if (n != NULL){
            n_checked = n;	n = n->n_next;
            tostring(n_checked);
            }
         cc = 0;
         if (n_maxlength != NULL)
            cc = n_maxlength->n_fval;
         if (n_value != NULL)
            cc = MAX(cc, strlen(n_value->n_pval));
         cc = MAX(cc, HTML_MAXLINE);
         buf = (char *) fb_malloc((unsigned) cc + FB_MAXLINE);
         sprintf(buf, "<INPUT TYPE=%s NAME=%s",
            n_type->n_pval, n_name->n_pval);
         if (n_size != NULL && n_size->n_fval > 0)
            sprintf(buf, "%s SIZE=%d", buf, atoi(n_size->n_pval));
         if (n_maxlength != NULL && n_maxlength->n_fval > 0)
            sprintf(buf, "%s MAXLENGTH=%d", buf, atoi(n_maxlength->n_pval));
         if (n_value != NULL){
            if ((equal(n_value->n_pval, "CHECKED") ||
                   equal(n_value->n_pval, "checked")) &&
                  (equal(n_type->n_pval, "CHECKBOX") ||
                   equal(n_type->n_pval, "checkbox") ||
                   equal(n_type->n_pval, "RADIO") ||
                   equal(n_type->n_pval, "radio")))
               strcat(buf, " CHECKED");
            else if (n_checked == NULL)
               sprintf(buf, "%s VALUE=\"%s\"", buf, n_value->n_pval);
            else
               sprintf(buf, "%s VALUE=\"%s\" %s", buf, n_value->n_pval,
                  n_checked->n_pval);
            }
         strcat(buf, ">");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         fb_free(buf);
         return(FB_AOK);
      }

/*
 * mf_html_select - html_select(name, size, option, ...)
 */

   mf_html_select(n, r)
      fb_mnode *n, *r;

      {
         char buf[HTML_MAXLINE], select[FB_MAXLINE];
         int cc, nlen, rn, size = 1;
         fb_mnode *n_name, *n_size;

         /* for each n arg, tostring it and place it in a column */
         rn = fb_realnodes(n);
         if (rn < 3)
            return(FB_ERROR);
         n_name = n;  n = n->n_next;
         tostring(n_name);
         n_size = n;  n = n->n_next;
         tostring(n_size);
         if ((size = atoi(n_size->n_pval)) < 1)
            size = 1;
         sprintf(buf, "   <SELECT NAME=%s SIZE=%d>\n", n_name->n_pval, size);
         cc = strlen(buf);
         
         for (; n != NULL; n = n->n_next){
            tostring(n);
            strcpy(select, "");
            if (equal(n->n_pval, "SELECTED") || equal(n->n_pval, "Selected") ||
                  equal(n->n_pval, "selected")){
               strcpy(select, n->n_pval);
               cc += strlen(select);
               n = n->n_next;
               if (n == NULL)
                  return(FB_ERROR);
               tostring(n);
               }
            if (n->n_pval != NULL && n->n_pval[0] == NULL)
               continue;
            nlen = 24 + strlen(n->n_pval);
            cc += nlen;
            if (cc >= HTML_MAXLINE)
               return(FB_ERROR);
            sprintf(buf, "%s   <OPTION %s>%s\n", buf, select, n->n_pval);
            }
         strcat(buf, "</SELECT>\n");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_select_open - html_select_start(name, size, directive ...)
 *	only do the start portion of the select code.
 */

   mf_html_select_open(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE], obuf[FB_MAXLINE];
         fb_mnode *n_name;
         int size = 1;

         /* for each n arg, tostring it and place it in a column */
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         n_name = n;    n = n->n_next;
         tostring(n_name);
         if (n != NULL){
            tostring(n);
            if ((size = atoi(n->n_pval)) < 1)
               size = 1;
            }
         obuf[0] = NULL;
         for (n = n->n_next; n != NULL; n = n->n_next){
            tostring(n);
            sprintf(obuf, "%s %s", obuf, n->n_pval);
            }

         sprintf(buf, "   <SELECT NAME=%s SIZE=%d%s>", n_name->n_pval, size,
            obuf);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_select_option - html_select_option(name, option ...)
 *	do as many options as are passed in, watching for SELECTED
 */

   mf_html_select_option(n, r)
      fb_mnode *n, *r;

      {
         char buf[HTML_MAXLINE], select[FB_MAXLINE], value[FB_MAXLINE];
         int cc, nlen;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         cc = 0;
         buf[0] = NULL;
         for (; n != NULL; n = n->n_next){
            tostring(n);
            select[0] = NULL;
            value[0] = NULL;
            if (equal(n->n_pval, "SELECTED") || equal(n->n_pval, "Selected") ||
                  equal(n->n_pval, "selected")){
               strcpy(select, n->n_pval);
               cc += strlen(select);
               n = n->n_next;
               if (n == NULL)
                  return(FB_ERROR);
               tostring(n);
               }
            if ((strncmp(n->n_pval, "VALUE=", 6) == 0) ||
                 (strncmp(n->n_pval, "value=", 6) == 0)){
               strcpy(value, n->n_pval);
               cc += strlen(value);
               n = n->n_next;
               if (n == NULL)
                  return(FB_ERROR);
               tostring(n);
               }
            if (n->n_pval != NULL && n->n_pval[0] == NULL)
               continue;
            nlen = 24 + strlen(n->n_pval);
            cc += nlen;
            if (cc >= HTML_MAXLINE)
               return(FB_ERROR);
            sprintf(buf, "%s   <OPTION %s %s>%s", buf, select, value,
               n->n_pval);
            }
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_select_close - html_select_close()
 */

   mf_html_select_close(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         strcpy(buf, "</SELECT>");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_textarea - html_textarea(name, row, col, value)
 */

   mf_html_textarea(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n_name, *n_row, *n_col, *n_value = NULL;
         char *buf;
         int rn, cc;

         rn = fb_realnodes(n);
         if (rn < 3)
            return(FB_ERROR);
         n_name = n;		n = n->n_next;
         n_row = n;		n = n->n_next;
         n_col = n;		n = n->n_next;
         if (n != NULL){
            n_value = n;
            tostring(n_value);
            }
         tostring(n_name);
         tostring(n_row);
         tostring(n_col);
         if (n_value != NULL)
            cc = strlen(n_value->n_pval);
         else
            cc = FB_MAXLINE;
         buf = (char *) fb_malloc((unsigned) cc + FB_MAXLINE);
         sprintf(buf, "<TEXTAREA NAME=\"%s\" ROWS=%d COLS=%d>",
            n_name->n_pval, atoi(n_row->n_pval), atoi(n_col->n_pval));
         if (n_value != NULL)
            strcat(buf, n_value->n_pval);
         strcat(buf, "</TEXTAREA>");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         fb_free((char *) buf);
         return(FB_AOK);
      }

   mf_html_href(n, r)
      fb_mnode *n, *r;

      {
         char buf[HTML_MAXLINE], tbuf[FB_MAXLINE];
         int maxlen;
         fb_mnode *n_ref, *n_str, *n_tar;

         if (fb_realnodes(n) < 2 )
            return(FB_ERROR);
         n_ref = n;   n = n->n_next;
         n_str = n;   n = n->n_next;
         n_tar = n;
         tostring(n_ref);
         tostring(n_str);
         tbuf[0] = NULL;
         if (n_tar != NULL){
            tostring(n_tar);
            sprintf(tbuf, "target=%s", n_tar->n_pval);
            }
         maxlen = HTML_MAXLINE - 20;
         if (strlen(n_ref->n_pval) + strlen(n_str->n_pval) > maxlen)
            return(FB_ERROR);
         sprintf(buf, "<A HREF=\"%s\" %s>\n   %s</A>", n_ref->n_pval,
            tbuf, n_str->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_imgsrc - html_imgsrc(ref, [align, hspace, border, alt])
 */

   mf_html_imgsrc(n, r)
      fb_mnode *n, *r;

      {
         char buf[HTML_MAXLINE];
         int maxlen, cc;
         fb_mnode *n_ref = NULL, *n_align = NULL, *n_hspace = NULL;
         fb_mnode *n_border = NULL, *n_alt = NULL;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         n_ref = n;   n = n->n_next;
         tostring(n_ref);
         if (n != NULL){
            n_align = n;
            n = n->n_next;
            tostring(n_align);
            }
         if (n != NULL){
            n_hspace = n;
            n = n->n_next;
            tostring(n_hspace);
            }
         if (n != NULL){
            n_border = n;
            n = n->n_next;
            tostring(n_border);
            }
         if (n != NULL){
            n_alt = n;
            n = n->n_next;
            tostring(n_alt);
            }
         maxlen = HTML_MAXLINE - 40;
         cc = strlen(n_ref->n_pval);
         if (n_align != NULL)
            cc += strlen(n_align->n_pval);
         if (cc > maxlen)
            return(FB_ERROR);
         sprintf(buf, "<IMG SRC=\"%s\"", n_ref->n_pval);
         if (n_align != NULL)
            sprintf(buf, "%s ALIGN=%s", buf, n_align->n_pval);
         if (n_hspace != NULL)
            sprintf(buf, "%s HSPACE=%d", buf, atoi(n_hspace->n_pval));
         if (n_border != NULL)
            sprintf(buf, "%s BORDER=%d", buf, atoi(n_border->n_pval));
         if (n_alt != NULL)
            sprintf(buf, "%s ALT=\"%s\"", buf, n_alt->n_pval);
         strcat(buf, ">");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_hr(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];
         int val = 4;

         if (fb_realnodes(n) > 0){
            tostring(n);
            val = atoi(n->n_pval);
            }
         sprintf(buf, "<HR SIZE=%d>", val);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_comment - generate an html comment <!-- %s -->\n
 */

   mf_html_comment(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];
         int maxlen;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         maxlen = FB_MAXLINE - 10;
         if (strlen(n->n_pval) > maxlen)
            n->n_pval[maxlen] = NULL;
         sprintf(buf, "<!-- %s -->", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_br - html_br()
 */

   mf_html_br(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         strcpy(buf, "<BR>");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_fontsize_open - html_fontsize_open(n)
 */

   mf_html_fontsize_open(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         sprintf(buf, "<FONT SIZE=%s>", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_fontsize_close - fontsize_close()
 */

   mf_html_fontsize_close(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         strcpy(buf, "</FONT>");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_font_open - html_font_open(typeface, size)
 */

   mf_html_font_open(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE], typeface[FB_MAXLINE], typesize[FB_MAXLINE];
         int rn;

         rn = fb_realnodes(n);
         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         strcpy(typeface, n->n_pval);
         typesize[0] = NULL;
         if (rn >= 2){
            n = n->n_next;
            tostring(n);
            sprintf(typesize, " SIZE=%s", n->n_pval);
            }
         sprintf(buf, "<FONT FACE=\"%s\"%s>", typeface, typesize);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_font_color - html_font_color(color)
 */

   mf_html_font_color(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE], typecolor[FB_MAXLINE], typesize[FB_MAXLINE];
         int rn;

         rn = fb_realnodes(n);
         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         strcpy(typecolor, n->n_pval);
         typesize[0] = NULL;
         if (rn >= 2){
            n = n->n_next;
            tostring(n);
            sprintf(typesize, " SIZE=%s", n->n_pval);
            }
         sprintf(buf, "<FONT COLOR=\"%s\"%s>", typecolor, typesize);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_font_close - font_close()
 */

   mf_html_font_close(n, r)
      fb_mnode *n, *r;

      {
         return(mf_html_fontsize_close(n, r));
      }

/*
 * mf_html_meta - html_meta(t1, t2, t3 ...)
 */

   mf_html_meta(n, r)
      fb_mnode *n, *r;

      {
         char buf[HTML_MAXLINE];
         int rn;

         rn = fb_realnodes(n);
         if (rn < 1)
            return(FB_ERROR);
         strcpy(buf, "<META ");
         for (; n != NULL; n = n->n_next){
            tostring(n);
            sprintf(buf, "%s %s", buf, n->n_pval);
            }
         strcat(buf, ">");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_link - html_link(t1, t2, t3 ...)
 */

   mf_html_link(n, r)
      fb_mnode *n, *r;

      {
         char buf[HTML_MAXLINE];
         int rn, cc;

         rn = fb_realnodes(n);
         if (rn < 2)
            return(FB_ERROR);
         strcpy(buf, "<LINK ");
         for (; n != NULL; n = n->n_next){
            tostring(n);
            sprintf(buf, "%s %s", buf, n->n_pval);
            }
         strcat(buf, ">");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_script_open - html_script_open(type)
 */

   mf_html_script_open(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         sprintf(buf, "<SCRIPT TYPE=\"%s\">", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_script_close(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         sprintf(buf, "</SCRIPT>\n");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }
