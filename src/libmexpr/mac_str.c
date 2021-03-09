/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_str.c,v 9.2 2001/09/29 18:07:58 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macro_str_sid[] = "@(#) $Id: mac_str.c,v 9.2 2001/09/29 18:07:58 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>

static char *BADPAT = "bad regular expression: ";

#if !FB_PROTOTYPES
static mf_doprintf();
#else /* FB_PROTOTYPES */
static mf_doprintf(char *, char *, fb_mnode *n);
#endif /* FB_PROTOTYPES */

extern int fb_subline_maxline;		/* for fb_subline maxline setting */

/*
 * mf_substr - provide substr(se, start, stop) function
 */

   mf_substr(n, r)
      fb_mnode *n, *r;

      {
         char *sval, *p, *q;
         double v1, v2, len, sv;

         (void) Macro_str_sid;

         if (fb_realnodes(n) < 3 )
            return(FB_ERROR);
         sval = tostring(n);		n = n->n_next;
         v1 = (double) n->n_fval;	n = n->n_next;
         v2 = fabs((double) n->n_fval);	n = n->n_next;
         len = strlen(sval);
         r->n_tval |= T_STR;
         if (len <= 0 || (v1 > 0 && v1 > len)){
            fb_mkstr(&(r->n_nval), NIL);
            return(FB_AOK);
            }

         /* sv is the real start (non-negative) v */
         sv = v1;
         if (v1 < 0){
            sv = len + v1 + 1;	/* v1 is negative, so sv is backwards */
            if (sv < 0)
               v1 = sv = 1;
            }
         if (sv <= 0)
            sv = 1;

         /* now sv is between 1 and the len of the string in question */
         p = sval + (int) sv - 1;

         /* start at sv, run for v2 characters */
         for (q = p; *q && v2 > 0; q++, v2--)
            ;
         if (*q != NULL)
            *q = NULL;

         fb_mkstr(&(r->n_nval), p);
         return(FB_AOK);
      }

   mf_length(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         r->n_fval = strlen(n->n_pval);
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_upper(n, r)
      fb_mnode *n, *r;

      {
         char *p;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         for (p = n->n_pval; *p; p++)
            if (islower(*p))
               *p = toupper(*p);
         fb_mkstr(&(r->n_nval), n->n_pval);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_lower(n, r)
      fb_mnode *n, *r;

      {
         char *p;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         for (p = n->n_pval; *p; p++)
            if (isupper(*p))
               *p = tolower(*p);
         fb_mkstr(&(r->n_nval), n->n_pval);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_subline(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2, *n3, *t_ret, *ret, tn;
         char *buf, attr;
         double v2;
         int st, nargs, len;

         nargs = fb_realnodes(n);
         if (nargs < 3)
            return(FB_ERROR);
         ret = n; n = n->n_next;
         n1 = n;  n = n->n_next;
         n2 = n;  n = n->n_next;
         n3 = n;
         if (n3 == NULL)
            attr = FB_NEWLINE;
         else{
            tostring(n3);
            attr = *(n3->n_pval);
            }
         
         v2 = (double) n2->n_fval;
         tostring(n1);
         /*
          * dont know how big the subline section will be,
          * but it wont be bigger than strlen(n1->n_pval)
          */
         len = strlen(n1->n_pval);
         fb_subline_maxline = len + 10;
         buf = fb_malloc(len + 1);
         st = fb_subline(buf, n1->n_pval, (int) v2, attr);
         if (ret->n_type != V_ARRAY){
            t_ret = mnode_to_var(ret);
            if (t_ret == NULL){
               fb_serror(FB_MESSAGE, "Illegal parameter", NIL);
               return(FB_ERROR);
               }
            fb_mkstr(&(t_ret->n_nval), buf);
            t_ret->n_tval |= T_STR;
            }
         else{
            fb_clearnode(&tn);
            fb_mkstr(&(tn.n_nval), buf);
            tn.n_tval |= T_STR;
            assign_array(ret, &tn, O_ASSIGN);
            fb_free(tn.n_nval);
            fb_free(tn.n_pval);
            }
         fb_free(buf);
         if (nargs > 2){
            r->n_fval = st;
            r->n_tval |= T_NUM;
            }
         return(FB_AOK);
      }

   mf_countlines(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2;
         char buf[FB_MAXLINE], attr;
         int st, i;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         n2 = n;
         if (n2 == NULL)
            attr = FB_NEWLINE;
         else{
            tostring(n2);
            attr = *(n2->n_pval);
            }
         tostring(n1);
         for (i = 1; ; i++){
            st = fb_subline(buf, n1->n_pval, i, attr);
            if (st == 0){
               fb_subline(buf, n1->n_pval, --i, attr);
               if (buf[0] == NULL)
                  i--;
               break;
               }
            }
         r->n_fval = i;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_strchr(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2;
         char attr, *p;
         int i;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         n2 = n;
         tostring(n1);
         tostring(n2);
         attr = n2->n_pval[0];
         if (attr == '\\')
            attr = n2->n_pval[1];
         for (p = n1->n_pval, i = 1; *p; i++, p++){
            if (*p == attr)
               break;
            }
         if (*p)
            r->n_fval = i;
         else
            r->n_fval = 0;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_strrchr(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2;
         char attr, *p;
         int i, len;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         n2 = n;
         tostring(n1);
         tostring(n2);
         attr = n2->n_pval[0];
         if (attr == '\\')
            attr = n2->n_pval[1];
         len = strlen(n1->n_pval);
         for (p = n1->n_pval + len - 1, i = len; *p && i > 0; i--, p--){
            if (*p == attr)
               break;
            }
         if (i > 0 && *p)
            r->n_fval = i;
         else
            r->n_fval = 0;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_formfield(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2, *n3;
         char buf[FB_MAXLINE], buf2[FB_MAXLINE], type;

         if (fb_realnodes(n) < 3)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         n2 = n; n = n->n_next;
         n3 = n;

         tostring(n2);
         type = *(n2->n_pval);

         if (type == FB_DOLLARS)
            n1->n_scale = 2;
         tostring(n1);
         strcpy(buf2, n1->n_pval);
         if (type == FB_DOLLARS)
            fb_nodecimal(buf2);

         if (type == FB_ALPHA || type == FB_STRICTALPHA)
            fb_pad(buf, buf2, (int) n3->n_fval);
         else
            fb_formfield(buf, buf2, type, (int) n3->n_fval);
         fb_mkstr(&(r->n_nval), buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_fmt(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2;
         char buffer[FB_MAXLINE];

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         n2 = n;
         tostring(n1);
         sprintf(buffer, n1->n_pval, n2->n_fval);
         fb_mkstr(&(r->n_nval), buffer);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_in(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *nn;
         int sflag = 0;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         nn = n;
         r->n_fval = 0;
         if (istype_str(n1)){
            tostring(n1);
            sflag = 1;
            }
         for (; nn != NULL && !r->n_fval; nn = nn->n_next){
            if (sflag){
               tostring(nn);
               if (equal(n1->n_pval, nn->n_pval))
                  r->n_fval = 1;
               }
            else if (n1->n_fval == nn->n_fval)
               r->n_fval = 1;
            }
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_pattern(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         /* n1 is the string to match, n2 is the pattern (if needed) */
         n1 = n; n = n->n_next;
         n2 = n;
         tostring(n1);
         tostring(n2);
         if (re_comp(n2->n_pval) != NULL){
            fb_serror(FB_MESSAGE, BADPAT, n2->n_pval);
            return(FB_ERROR);
            }
         r->n_fval = 0;
         if (re_exec(n1->n_pval))
            r->n_fval = 1;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_pattern_comp(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         /* n1 is the pattern that is compiled */
         n1 = n;
         tostring(n1);
         if (re_comp(n1->n_pval) != NULL){
            fb_serror(FB_MESSAGE, BADPAT, n1->n_pval);
            return(FB_ERROR);
            }
         r->n_fval = 1;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_pattern_exec(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         /* n1 is the string to match */
         n1 = n;
         tostring(n1);
         r->n_fval = 0;
         if (re_exec(n1->n_pval))
            r->n_fval = 1;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_pattern_icase(n, r)
      fb_mnode *n, *r;

      {
         int flag;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         /* n1 is the flag to pass to fb_set_regcomp_icase() */
         flag = (int) n->n_fval;
         fb_set_regcomp_icase(flag);
         r->n_fval = 0;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_pattern_eo(n, r)
      fb_mnode *n, *r;

      {
         int val, pos;

         if (fb_realnodes(n) < 1)
            pos = 0;
         else
            pos = (int) n->n_fval;
         val = fb_get_regexec_eo(pos);
         if (val != -1)
            val++;
         r->n_fval = val;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_pattern_so(n, r)
      fb_mnode *n, *r;

      {
         int val, pos;

         if (fb_realnodes(n) < 1)
            pos = 0;
         else
            pos = (int) n->n_fval;
         val = fb_get_regexec_so(pos);
         if (val != -1)
            val++;
         r->n_fval = val;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_pattern_substr(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2;
         double pos;
         char *buf, *p, *q, *ep;
         int so, eo;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         /* n1 is the string to match, n2 is the Nth matched substr */
         n1 = n; n = n->n_next;
         tostring(n1);
         n2 = n;
         pos = fabs((double) n2->n_fval);	n = n->n_next;
         if (pos < 0)
            pos = 0;
         /*
          * dont know how big the subline section will be,
          * but it wont be bigger than strlen(n1->n_pval)
          */
         buf = fb_malloc(strlen(n1->n_pval) + 1);
         so = eo = 0;
         so = fb_get_regexec_so((int) pos);
         eo = fb_get_regexec_eo((int) pos);
         p = n1->n_pval;
         ep = p + eo;
         p += so;
         for (q = buf; p < ep; p++, q++)
            *q = *p;
         *q = NULL;
         fb_mkstr(&(r->n_nval), buf);
         fb_free(buf);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_sprintf(n, r)
      fb_mnode *n, *r;

      {
         char *buf;
         fb_mnode *n1;
         int st;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         buf = fb_malloc(5120);
         n1 = n; n = n->n_next;
         tostring(n1);
         st = mf_doprintf(buf, n1->n_pval, n);
         if (st != FB_AOK)
            buf[0] = NULL;
         r->n_tval |= T_STR;
         fb_mkstr(&(r->n_nval), buf);
         fb_free(buf);
         return(FB_AOK);
      }

   static mf_doprintf(buf, s, n)
      char *buf, *s;
      fb_mnode *n;

      {
         char *p, fmt[200], *t;
         int flag = 0;
         double xf = 0;
         fb_mnode *cn;

         p = buf;
         while (*s) {
            if (*s != '%'){
               *p++ = *s++;
               continue;
               }
            if (*(s+1) == '%') {
               *p++ = '%';
               s += 2;
               continue;
               }
            for (t = fmt; (*t++ = *s) != '\0'; s++)
               if (*s >= 'a' && *s <= 'z' && *s != 'l')
                  break;
            *t = '\0';
            if (t >= fmt + sizeof(fmt)){
               fb_serror(FB_MESSAGE, "printf item is too long.", NIL);
               return(FB_ERROR);
               }
            switch (*s) {
               case 'f': case 'e': case 'g':
                  flag = 1;
                  break;
               case 'd':
                  flag = 2;
                  if(*(s-1) == 'l')
                     break;
                  *(t-1) = 'l';
                  *t = 'd';
                  *++t = '\0';
                  break;
               case 'o': case 'x':
                  flag = *(s-1)=='l' ? 2 : 3;
                  break;
               case 'c':
                  flag = 3;
                  break;
               case 's':
                  flag = 4;
                  break;
               default:
                  flag = 0;
                  break;
               }
            if (flag == 0) {
               sprintf(p, "%s", fmt);
               p += strlen(p);
               continue;
               }
            if (n == NULL){
               fb_serror(FB_MESSAGE, "printf - not enough arguments.", NIL);
               return(FB_ERROR);
               }
            cn = n;
            n = n->n_next;
            if (flag != 4)	/* watch out for converting to numbers! */
               xf = tonumber(cn);
            if (flag==1)
               sprintf(p, fmt, xf);
            else if (flag==2)
               sprintf(p, fmt, (long) xf);
            else if (flag==3)
               sprintf(p, fmt, (int) xf);
            else if (flag==4){
               tostring(cn);
               sprintf(p, fmt, cn->n_pval);
               }
            p += strlen(p);
            s++;
         }
         *p = '\0';
         return(FB_AOK);
      }

   mf_rmlead(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         fb_rmlead(n->n_pval);
         fb_mkstr(&(r->n_nval), n->n_pval);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_trim(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         fb_trim(n->n_pval);
         fb_mkstr(&(r->n_nval), n->n_pval);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_rmunderscore(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         fb_underscore(n->n_pval, 0);
         fb_mkstr(&(r->n_nval), n->n_pval);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_rmnewline(n, r)
      fb_mnode *n, *r;

      {
         char *p;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         for (p = n->n_pval; *p; p++)
            if (*p == '\015' || *p == '\012'){
               *p = NULL;
               break;
               }
         fb_mkstr(&(r->n_nval), n->n_pval);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_rmlinefeed(n, r)
      fb_mnode *n, *r;

      {
         char *p;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         tostring(n);
         for (p = n->n_pval; *p; p++)
            if (*p == '\012' || *p == '\015'){
               *p = NULL;
               break;
               }
         fb_mkstr(&(r->n_nval), n->n_pval);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_makess(n, r)
      fb_mnode *n, *r;

      {
         int len;
         fb_mnode *n1, *n2, *n3;
         char *buf, type;

         if (fb_realnodes(n) < 3 )
            return(FB_ERROR);
         /* n1 is buf, n2 is type, n3 is length */
         n1 = n; n = n->n_next;
         n2 = n; n = n->n_next;
         n3 = n; 
         tostring(n1);
         tostring(n2);
         len = n3->n_fval;
         buf = fb_malloc(len + 10 + strlen(n1->n_pval));
         strcpy(buf, n1->n_pval);
         type = *(n2->n_pval);
         fb_makess(buf, type, len);
         r->n_tval |= T_STR;
         fb_mkstr(&(r->n_nval), buf);
         fb_free(buf);
         return(FB_AOK);
      }

   mf_getword(n, r)
      fb_mnode *n, *r;

      {
         int len, st;
         fb_mnode *n1, *n2, *n3, *t_ret;
         double pos;
         char buf[FB_MAXLINE], type;

         if (fb_realnodes(n) < 3 )
            return(FB_ERROR);
         /* implement getword(in, i, out) */
         n1 = n; n = n->n_next;
         n2 = n; n = n->n_next;
         n3 = n; 
         tostring(n1);
         pos = (double) n2->n_fval;

         st = fb_getword(n1->n_pval, (int) pos, buf);

         t_ret = mnode_to_var(n3);
         if (t_ret == NULL){
            fb_serror(FB_MESSAGE, "Illegal parameter", NIL);
            return(FB_ERROR);
            }
         fb_mkstr(&(t_ret->n_nval), buf);
         t_ret->n_tval |= T_STR;
         
         r->n_fval = st;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_gettoken(n, r)
      fb_mnode *n, *r;

      {
         int len, st;
         fb_mnode *n1, *n2, *n3, *n4, *t_ret;
         double pos;
         char buf[FB_MAXLINE], type, c = NULL;

         if (fb_realnodes(n) < 4 )
            return(FB_ERROR);
         /* implement gettoken(in, i, out, c) */
         n1 = n; n = n->n_next;
         n2 = n; n = n->n_next;
         n3 = n; n = n->n_next;
         n4 = n; 
         tostring(n1);
         tostring(n4);
         pos = (double) n2->n_fval;
         if (n4->n_pval != NULL)
            c = n4->n_pval[0];

         st = fb_gettoken(n1->n_pval, (int) pos, buf, c);

         t_ret = mnode_to_var(n3);
         if (t_ret == NULL){
            fb_serror(FB_MESSAGE, "Illegal parameter", NIL);
            return(FB_ERROR);
            }
         fb_mkstr(&(t_ret->n_nval), buf);
         t_ret->n_tval |= T_STR;
         
         r->n_fval = st;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_crypt(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2;
         char *p;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         /* n1 is the string to crypt, n2 is the salt */
         n1 = n; n = n->n_next;
         tostring(n1);
         n2 = n;
         tostring(n2);
         /*
          * dont know how big the subline section will be,
          * but it wont be bigger than strlen(n1->n_pval)
          */
         p = crypt(n1->n_pval, n2->n_pval);
         fb_mkstr(&(r->n_nval), p);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }
