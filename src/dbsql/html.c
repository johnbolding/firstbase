/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: html.c,v 9.0 2001/01/09 02:55:47 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char U_html_sid[] = "@(#) $Id: html.c,v 9.0 2001/01/09 02:55:47 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * html series of functions.
 */

   html_open()

      {
         fprintf(sql_ofs, "<HTML>\n");
         fprintf(sql_ofs, "<HEAD>\n");
         fprintf(sql_ofs, "<TITLE>FirstBase: dbsql output</TITLE>\n");
         fprintf(sql_ofs, "</HEAD>\n");
         fprintf(sql_ofs, "<BODY>\n");
         fprintf(sql_ofs, "<TABLE");
         if (html_border)
            fprintf(sql_ofs, " BORDER=%d", html_border);
         if (html_cellpadding)
            fprintf(sql_ofs, " CELLPADDING=%d", html_cellpadding);
         fprintf(sql_ofs, ">\n");
      }

   html_close()

      {
         fprintf(sql_ofs, "</TABLE>\n");
         fprintf(sql_ofs, "</BODY>\n");
         fprintf(sql_ofs, "</HTML>\n");
      }

   html_row_open()
      {
         fprintf(sql_ofs, "<TR>\n");
      }

   html_row_close()
      {
         fprintf(sql_ofs, "</TR>\n");
      }

   html_cell_open(r)
      int r;

      {
         fprintf(sql_ofs, "   <TD NOWRAP");
         if (r)
            fprintf(sql_ofs, " ALIGN=RIGHT");
         fprintf(sql_ofs, ">");
      }

   html_cell_close()
      {
         fprintf(sql_ofs, "</TD>\n");
      }

   html_headers(s)
      node *s;

      {
         node *n;

         html_row_open();
         for (n = s; n != NULL; n = n->n_list){
            html_header_open();
            fprintf(sql_ofs, "%s", n->n_nval);
            html_header_close();
            }
         html_row_close();
      }

   html_header_open()
      {
         fprintf(sql_ofs, "   <TH>");
      }

   html_header_close()
      {
         fprintf(sql_ofs, "   </TH>\n");
      }

