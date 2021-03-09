/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_stio.c,v 9.1 2001/01/16 02:46:54 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macro_stdio_sid[] = "@(#) $Id: mac_stio.c,v 9.1 2001/01/16 02:46:54 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>

/*
 * mf_print - print(string, ...)
 */

   mf_print(n, r)
      fb_mnode *n, *r;

      {
         char *sval, buf[FB_MAXLINE];

         (void) Macro_stdio_sid;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);

         /* now do the function call with all remaining arguments */
         for (; n != NULL; n = n->n_next){
            sval = tostring(n);
            if (istype_dol(n)){
               if (strrchr(sval, CHAR_DOT) != NULL)
                  fb_nodecimal(sval);
               fb_formfield(buf, sval, FB_DOLLARS, 18);
               fb_rmlead(buf);
               sval = buf;
               }
            else if (istype_dte(n)){
               fb_formfield(buf, sval,FB_DATE, 8);
               sval = buf;
               }
            fputs(sval, stdout);
            }
         fputs("\n", stdout);
         return(FB_AOK);
      }

   mf_pause(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }

   mf_error(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         fb_serror(FB_MESSAGE, tostring(n), NIL);
         return(FB_AOK);
      }

   mf_redraw(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }

   mf_header(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }

   mf_footer(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }

   mf_status(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }

   mf_move(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }

   mf_clear(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }

   mf_clrtoeol(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }

   mf_clrtobot(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }

   mf_refresh(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }

   mf_editfield(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }

   mf_rinput(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }


   mf_input(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }

   mf_gets(n, r)
      fb_mnode *n, *r;

      {
         char line[FB_MAXLINE];
         int lc, st;
         fb_mnode *t_ret, tn, *ret;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         ret = n;
         if (fgets(line, FB_MAXLINE, stdin) == NULL)
            st = 0;
         else{
            st = 1;
            lc = strlen(line) - 1;
            if (line[lc] == FB_NEWLINE)
               line[lc] = NULL;
            if (ret->n_type != V_ARRAY){
               t_ret = mnode_to_var(ret);
               if (t_ret == NULL){
                  fb_serror(FB_MESSAGE, "Illegal parameter", NIL);
                  return(FB_ERROR);
                  }
               fb_mkstr(&(t_ret->n_nval), line);
               t_ret->n_tval |= T_STR;
               }
            else{
               fb_clearnode(&tn);
               fb_mkstr(&(tn.n_nval), line);
               tn.n_tval |= T_STR;
               assign_array(ret, &tn, O_ASSIGN);
               fb_free(tn.n_nval);
               fb_free(tn.n_pval);
               }
            }
         r->n_fval = st;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

/*
 * mf_puts - puts(s)
 */

   mf_puts(n, r)
      fb_mnode *n, *r;

      {
         char *buf;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         buf = tostring(n);
         if (puts(buf) == EOF)
            return(FB_ERROR);
         r->n_fval = FB_AOK;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_printf(n, r)
      fb_mnode *n, *r;

      {
         int st;

         st = mf_sprintf(n, r);
         if (st == FB_AOK)
            fputs(r->n_nval, stdout);
         return(st);
      }

   mf_reverse(n, r)
      fb_mnode *n, *r;

      {
         return(mf_print(n, r));
      }

   mf_standout(n, r)
      fb_mnode *n, *r;

      {
         return(mf_print(n, r));
      }

   mf_bell(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }
