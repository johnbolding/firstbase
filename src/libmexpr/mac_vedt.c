/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_vedt.c,v 9.0 2001/01/09 02:56:53 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macro_vedit_sid[] = "@(#) $Id: mac_vedt.c,v 9.0 2001/01/09 02:56:53 john Exp $";
#endif

#include <dbve_ext.h>
#include <macro_e.h>

extern short int st_up;
extern short int cdb_create_it, cdb_write_it;
extern short int cdb_edit_input;
extern char *cdb_e_buf;
extern short int cdb_e_st;

static mf_do_print();
static mf_edit_one_field();
static mf_sub_input();

extern short int cdb_m_fld; /* this is kind of local to macros, in macroinput */

   mf_print(n, r)
      fb_mnode *n, *r;

      {
         return(mf_do_print(n, r, 1));
      }

   mf_standout(n, r)
      fb_mnode *n, *r;

      {
         return(mf_do_print(n, r, 2));
      }

   mf_reverse(n, r)
      fb_mnode *n, *r;

      {
         return(mf_do_print(n, r, 3));
      }

   mf_bell(n, r)
      fb_mnode *n, *r;

      {
         fb_bell();
         return(FB_AOK);
      }

   static mf_do_print(n, r, ptype)
      fb_mnode *n, *r;
      int ptype;

      {
         char *tostring(), *sval, buf[FB_MAXLINE];

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);

         for (; n != NULL; n = n->n_next){
            sval = tostring(n);
            if (istype_dol(n)){
               fb_nodecimal(sval);
               fb_formfield(buf, sval, FB_DOLLARS, 18);
               fb_rmlead(buf);
               sval = buf;
               }
            else if (istype_dte(n)){
               fb_formfield(buf, sval,FB_DATE, 8);
               sval = buf;
               }
            switch(ptype){
               case 1: fb_prints(sval); break;
               case 2: fb_stand(sval); break;
               case 3: fb_reverse(sval); break;
               }
            }
         return(FB_AOK);
      }

   mf_pause(n, r)
      fb_mnode *n, *r;

      {
         fb_serror(FB_MESSAGE, NIL, NIL);
         return(FB_AOK);
      }

   mf_error(n, r)
      fb_mnode *n, *r;

      {
         char *tostring();

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         fb_serror(FB_MESSAGE, tostring(n), NIL);
         return(FB_AOK);
      }

   mf_redraw(n, r)
      fb_mnode *n, *r;

      {
         fb_checkformula(cdb_m_fld);
         fb_scrlbl(cdb_db->sdict);
         fb_scrstat2(msg);		/* individual record status */
         fb_display(1);
         return(FB_AOK);
      }

   mf_header(n, r)
      fb_mnode *n, *r;

      {
         fb_scrhdr(cdb_db, "Auto Field Level");
         return(FB_AOK);
      }

   mf_footer(n, r)
      fb_mnode *n, *r;

      {
         fb_infoline();
         return(FB_AOK);
      }

   mf_status(n, r)
      fb_mnode *n, *r;

      {
         char *tostring();

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         fb_scrstat(tostring(n));
         return(FB_AOK);
      }

   mf_move(n, r)
      fb_mnode *n, *r;

      {
         int row, col;

         if (fb_realnodes(n) < 2 )
            return(FB_ERROR);
         /* interpret arguments */
         row = (int) n->n_fval;
         n = n->n_next;
         col = (int) n->n_fval;
         fb_move(row, col);
         return(FB_AOK);
      }

   mf_clear(n, r)
      fb_mnode *n, *r;

      {
         fb_clear();
         return(FB_AOK);
      }

   mf_clrtoeol(n, r)
      fb_mnode *n, *r;

      {
         fb_clrtoeol();
         return(FB_AOK);
      }

   mf_clrtobot(n, r)
      fb_mnode *n, *r;

      {
         fb_clrtobot();
         return(FB_AOK);
      }

   mf_refresh(n, r)
      fb_mnode *n, *r;

      {
         fb_refresh();
         return(FB_AOK);
      }

   mf_editfield(n, r)
      fb_mnode *n, *r;

      {
         int st = 0;
         fb_mnode *q, *pq;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);

         /* make a reverse list in the n_virlist area */

         for (pq = n, q = n->n_next; q != NULL; pq = q, q = q->n_next)
            q->n_virlist = pq;

         if (st_up)
            q = pq;
         else
            q = n;
         for (; q != NULL; ){
            if (cdb_edit_input && cdb_e_st != 0){
               st = cdb_e_st;
               cdb_e_st = 0;
               }
            else
               st = mf_edit_one_field(q);
            if (st == FB_ABORT || st == FB_END)
               break;
            if (cdb_edit_input && cdb_e_st != 0)
               continue;		/* to do the saved command */
            if (st == FB_YSIGNAL){
               q = q->n_virlist;
               if (q == NULL)
                  break;
               }
            else{
               q = q->n_next;
               if (q == NULL)
                  break;
               }
            }
         r->n_fval = st;
         r->n_tval |= T_NUM;
         return(st);
      }

   static mf_edit_one_field(n)
      fb_mnode *n;

      {
         fb_field *f, *mnode_to_field();
         int st, fld, rd_flag;
         fb_node *rn = NULL;

         f = mnode_to_field(n);
         if (f == NULL)
            return(FB_ERROR);

         if (mf_search_stack(f)){
            fb_serror(FB_MESSAGE, "Infinite loop detected on editfield: ",
               f->id);
            return(FB_ERROR);
            }

         /*
          * f is now the target fb_field ... locate it in pcur->p_nedit[]
          * fld is now needed for the call to edit_field ...
          */

         /* keep this 0 based for the moment */
         for (fld = 0; fld < pcur->p_maxedit; fld++){
	    rn = pcur->p_nedit[fld];
            if (rn->n_fp != NULL && rn->n_fp == f)
               break;
            }
         if (fld < 0 || fld >= pcur->p_maxedit)
            return(FB_ERROR);

         rd_flag = 0;
         if (rn->n_readonly > 0){
            rn->n_readonly = 0;
            rd_flag = 1;
            }
         /* switch to one based on the way in */
         st = edit_field(fld + 1, (fb_node *) NULL, m_addf_flag);
         fb_checkformula(cdb_m_fld);
         if (st == FB_AOK)
            cdb_write_it = 1;
         else{
            fb_fetch(f, cdb_afld, cdb_db);
            fb_putfield(rn, f, cdb_afld);
            }
         if (rd_flag)
            rn->n_readonly = 1;
         return(st);
      }


   mf_rinput(n, r)
      fb_mnode *n, *r;

      {
         int row, col;
         fb_mnode *string_to_var(), *rn, *cn;

         if (fb_realnodes(n) < 4)
            return(FB_ERROR);
         rn = string_to_var("ROW");
         cn = string_to_var("COL");
         row = (int) rn->n_fval;
         col = (int) cn->n_fval;
         return(mf_sub_input(row, col, n, r));
      }


   mf_input(n, r)
      fb_mnode *n, *r;

      {

         int row, col;

         if (fb_realnodes(n) < 6 )
            return(FB_ERROR);

         /* interpret arguments */
         row = (int) n->n_fval;		 n = n->n_next;
         col = (int) n->n_fval;		 n = n->n_next;
         return(mf_sub_input(row, col, n, r));
      }

   static mf_sub_input(row, col, n, r)
      int row, col;
      fb_mnode *n, *r;

      {
         int st, max, min;
         char *fmt, *tostring(), buf[FB_MAXLINE];
         fb_mnode *addr, *mnode_to_var(), *ne_st;

         max = (int) n->n_fval;		 n = n->n_next;
         min = (int) n->n_fval;		 n = n->n_next;
         fmt = tostring(n);		 n = n->n_next;
         addr = mnode_to_var(n);	 n = n->n_next;
         if (n != NULL){
            ne_st = mnode_to_var(n);
            if (ne_st == NULL)
               fb_serror(FB_MESSAGE, "Illegal variable - E_ST", NIL);
            }
         else
            ne_st = NULL;
         if (addr == NULL){
            fb_serror(FB_MESSAGE, "Illegal variable - addr", NIL);
            return(FB_ERROR);
            }
         if (*fmt != FB_ALPHA && *fmt !=FB_DATE && *fmt != FB_DOLLARS &&
                *fmt != FB_FLOAT && *fmt != FB_INTEGER && *fmt != FB_NUMERIC &&
                *fmt != FB_POS_NUM && *fmt != FB_STRICTALPHA && *fmt != FB_UPPERCASE){
            fb_serror(FB_MESSAGE, "Illegal format: ", fmt);
            return(FB_ERROR);
            }
         if (ne_st != NULL)
            ne_st->n_tval = 0;
         if (cdb_edit_input){
            cdb_e_st = 0;
            buf[0] = NULL;
            if (istype_str(addr) || istype_num(addr)){
               tostring(addr);
               if (addr->n_pval != NULL)
                  strcpy(buf, addr->n_pval);
               }
            strcpy(cdb_e_buf, buf);
            }
         st = fb_input(row, col, -max, min, *fmt, cdb_afld, FB_ECHO, FB_OKEND, FB_CONFIRM);
         if (st == FB_AOK){
            fb_trim(cdb_afld);
            addr->n_tval = 0;
            if (FB_OFNUMERIC(*fmt)){
               addr->n_tval |= T_NUM;
               if (*fmt == FB_DOLLARS){
                  fb_putdecimal(cdb_afld);
                  addr->n_tval |= T_DOL;
                  addr->n_scale = 2;
                  }
               addr->n_fval = atof(cdb_afld);
               }
            else{
               addr->n_tval |= T_STR;
               fb_mkstr(&(addr->n_nval), cdb_afld);
               }
            if (cdb_edit_input){
               if (ne_st != NULL){
                  ne_st->n_tval |= T_NUM;
                  ne_st->n_fval = cdb_e_st;
                  }
               cdb_e_st = 0;
               }
            }
         r->n_fval = st;
         r->n_tval |= T_NUM;
         return(st);
      }

   mf_gets(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }

   mf_puts(n, r)
      fb_mnode *n, *r;

      {
         return(FB_AOK);
      }

   mf_printf(n, r)
      fb_mnode *n, *r;

      {
         int st;

         st = mf_sprintf(n, r);
         if (st == FB_AOK)
            fb_prints(r->n_nval);
         return(st);
      }
