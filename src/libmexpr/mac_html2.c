/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_html2.c,v 9.0 2001/01/09 02:56:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Mac_html2_sid[] = "@(#) $Id: mac_html2.c,v 9.0 2001/01/09 02:56:51 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>

#define HTML_MAXLINE	1024
#define HTML_TEXTLINE	5120

/*
 * mf_html_dl_open - html_dl_open() - <DL>
 * mf_html_dl_close - html_dl_close() - </DL>
 */

   mf_html_dl_open(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         (void) Mac_html2_sid;

         strcpy(buf, "<DL COMPACT>");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_dl_close(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         strcpy(buf, "</DL>");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_dt - html_dt(s) - <DT> s
 */

   mf_html_dt(n, r)
      fb_mnode *n, *r;

      {
         char buf[HTML_MAXLINE];
         int len;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         len = strlen(n->n_pval);
         if (len + 10 > HTML_MAXLINE)
            n->n_pval[HTML_MAXLINE - 10] = NULL;
         sprintf(buf, "   <DT>%s", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_dd - html_dd(s) - <DD> s
 */

   mf_html_dd(n, r)
      fb_mnode *n, *r;

      {
         char buf[HTML_MAXLINE];
         int len;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         len = strlen(n->n_pval);
         if (len + 10 > HTML_MAXLINE)
            n->n_pval[HTML_MAXLINE - 10] = NULL;
         sprintf(buf, "   <DD>%s", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_ul_open - html_ul_open() - <UL>
 * mf_html_ul_close - html_ul_close() - </UL>
 */

   mf_html_ul_open(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         strcpy(buf, "<UL>");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_ul_close(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         strcpy(buf, "</UL>");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_ol_open - html_ul_open() - <OL>
 * mf_html_ol_close - html_ul_close() - </OL>
 */

   mf_html_ol_open(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE], sbuf[FB_MAXLINE];

         sbuf[0] = NULL;
         if (fb_realnodes(n) >= 1){
            sprintf(sbuf, " START=%.0f", n->n_fval);
            }
            
         sprintf(buf, "<OL%s>", sbuf);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_ol_close(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         strcpy(buf, "</OL>");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_li - html_li(s) - <LI> s
 */

   mf_html_li(n, r)
      fb_mnode *n, *r;

      {
         char buf[HTML_MAXLINE];
         int len;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         len = strlen(n->n_pval);
         if (len + 10 > HTML_MAXLINE)
            n->n_pval[HTML_MAXLINE - 10] = NULL;
         sprintf(buf, "   <LI>%s", n->n_pval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_blockquote_open - html_blockquote_open() - <BLOCKQUOTE>
 * mf_html_blockquote_close - html_blockquote_close() - </BLOCKQUOTE>
 */

   mf_html_blockquote_open(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         strcpy(buf, "<BLOCKQUOTE>");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_html_blockquote_close(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE];

         strcpy(buf, "</BLOCKQUOTE>");
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

/*
 * mf_html_filter_lt - filter_lt(addr, s)
 */

   mf_html_filter_lt(n, r)
      fb_mnode *n, *r;

      {
         int len;
         char *buf, *str, *p, *q;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         tostring(n);
         str = n->n_pval;
         len = strlen(str);
         buf = fb_malloc((unsigned) (len + 120));

         for (p = str, q = buf; *p; p++){
            if (*p == '<'){
               *q = NULL;
               strcpy(q, "&lt;");
               q += 4;
               }
            else
               *q++ = *p;
            }
         *q = NULL;
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         fb_free(buf);
         return(FB_AOK);
      }

