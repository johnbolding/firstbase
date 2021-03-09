/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_stmt.c,v 9.0 2001/01/09 02:56:53 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macro_stmt_sid[] = "@(#) $Id: mac_stmt.c,v 9.0 2001/01/09 02:56:53 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>


#if !FB_PROTOTYPES
static list_expr();
#else /* FB_PROTOTYPES */
static list_expr(fb_mnode *n);
#endif /* FB_PROTOTYPES */

extern short int cdb_usrlog;
extern short int cdb_cgi_flag;

/*
 * macro_statement - the driver for the macro fb_field parse trees
 *	uses s as this local symbol table to simulate execution frames.
 */

   double macro_statement(n, sv)
      fb_mnode *n;
      fb_stack_vars *sv;

      {
         fb_mnode *n0, *n1, *n2, *n3, *sn, *psn, *nn;
         double v;

         (void) Macro_stmt_sid;

         n->n_fval = 0;
         v = 0;

         for (; n != NULL; n = n->n_next){
            /* these flags need resetting before each executed statement */
            n0 = n->n_narg[0];
            n1 = n->n_narg[1];
            n2 = n->n_narg[2];
            n3 = n->n_narg[3];
            switch(n->n_type){
               case S_NULL:
                  break;
               case S_EXPR:
                  n->n_fval = macro_expr(n0);
                  n->n_scale = n0->n_scale;
                  n->n_tval = n0->n_tval;
                  v = n->n_fval;
                  break;
               case S_IF:
                  if (macro_expr(n0)){
                     n->n_fval = (double) macro_statement(n1, sv);
                     n->n_scale = n1->n_scale;
                     n->n_tval = n1->n_tval;
                     v = n->n_fval;
                     }
                  break;
               case S_IFELSE:
                  if (macro_expr(n0)){
                     n->n_fval = (double) macro_statement(n1, sv);
                     n->n_scale = n1->n_scale;
                     n->n_tval = n1->n_tval;
                     }
                  else{
                     n->n_fval = (double) macro_statement(n2, sv);
                     n->n_scale = n2->n_scale;
                     n->n_tval = n2->n_tval;
                     }
                  v = n->n_fval;
                  break;
               case S_SWITCH:
                  macro_expr(n0);
                  tostring(n0);
                  /*
                   * now walk the n1 list looking for a match.
                   * the first match found is the first one executed.
                   * execute all below it (unless a break is done)
                   * if no match execute n2 default
                   */
                  for (; n1 != NULL; n1 = n1->n_next){
                     nn = n1->n_narg[0];
                     tostring(nn);
                     if ((istype_str(n0) || istype_str(nn))){
                        if (equal(n0->n_pval, nn->n_pval))
                           break;
                        }
                     else if (n0->n_fval == nn->n_fval)
                        break;
                     }
                  if (n1 != NULL){
                     for (; n1 != NULL; n1 = n1->n_next){
                        nn = n1->n_narg[1];
                        n->n_fval = (double) macro_statement(nn, sv);
                        n->n_scale = nn->n_scale;
                        n->n_tval = nn->n_tval;
                        if (sv->break_flag)
                           break;
                        }
                     }
                  else if (n2 != NULL && n2->n_narg[0] != NULL){
                     /* DEFAULT clause of SWITCH statement */
                     nn = n2->n_narg[0];
                     n->n_fval = (double) macro_statement(nn, sv);
                     n->n_scale = nn->n_scale;
                     n->n_tval = nn->n_tval;
                     }
                  if (sv->break_flag){
                     sv->break_flag = 0;
                     }
                  break;
               case S_WHILE:
                  while (macro_expr(n0) != 0){
                     n->n_fval = (double) macro_statement(n1, sv);
                     n->n_scale = n1->n_scale;
                     n->n_tval = n1->n_tval;
                     v = n->n_fval;
                     if (sv->break_flag || sv->return_flag){
                        sv->break_flag = 0;
                        break;
                        }
                     else if (sv->continue_flag){
                        sv->continue_flag = 0;
                        continue;
                        }
                     }
                  break;
               case S_FOR:
                  for (list_expr(n0);
                        n1->n_type == S_NULL || macro_expr(n1) != 0;
                        list_expr(n2)){
                     n->n_fval = (double) macro_statement(n3, sv);
                     n->n_scale = n3->n_scale;
                     n->n_tval = n3->n_tval;
                     v = n->n_fval;
                     if (sv->break_flag || sv->return_flag){
                        sv->break_flag = 0;
                        break;
                        }
                     else if (sv->continue_flag){
                        sv->continue_flag = 0;
                        continue;
                        }
                     }
                  break;
               case S_LIST:
                  n->n_fval = (double) macro_statement(n0, sv);
                  n->n_scale = n0->n_scale;
                  n->n_tval = n0->n_tval;
                  break;
               case S_RETURN:
                  psn = NULL;
                  for (nn = n0->n_narg[0]; nn != NULL; nn = nn->n_list){
                     macro_expr(nn);
                     sn = fb_s_makenode();
                     fb_copynode(sn, nn);
                     if (istype_arr(nn) && !istype_fld(nn))
                        sn->n_next = array_to_list(nn);
                     if (sv->s_return_list == NULL)
                        sv->s_return_list = sn;
                     if (psn != NULL)
                        psn->n_list = sn;
                     psn = sn;
                     }
                  nn = n0->n_narg[0];
                  if (nn != NULL)
                     sv->return_value = nn->n_fval;
                  sv->return_flag = 1;
                  break;
               case S_EXIT:
                  nn = n0->n_narg[0];
                  if (nn != NULL)
                     sv->return_value = macro_expr(nn);
                  else
                     sv->return_value = 0;
                  sv->exit_flag = 1;
                  fb_exit(sv->return_value);
                  break;
               case S_BREAK:
                  sv->break_flag = 1;
                  break;
               case S_CONTINUE:
                  sv->continue_flag = 1;
                  break;
               case S_LOCAL:
                  mf_local(n0);
                  break;
               default:
                  fb_serror(FB_MESSAGE, "CANNOT EXECUTE: UNKNOWN STATEMENT ",
                     NIL);
                  break;
               }
            if (sv->break_flag || sv->continue_flag || sv->return_flag ||
                  sv->exit_flag)
               break;
            }
         return(v);
      }

   static list_expr(n)
      fb_mnode *n;

      {
         for (n = n->n_narg[0]; n != NULL; n = n->n_list)
            macro_expr(n);
      }

   void mf_set_constant(s, v)
      char *s;
      int v;

      {
         fb_cell *c;

         c = fb_ginstall(s);
         c->c_vid = fb_node0(V_ID);
         c->c_vid->n_fval = v;
         c->c_vid->n_tval |= T_NUM;
      }

   int mf_set_value(s, p)
      char *s, *p;

      {
         fb_cell *c;

         c = fb_ginstall(s);
         c->c_vid = fb_node0(V_ID);
         fb_mkstr(&(c->c_vid->n_nval), p);
         c->c_vid->n_tval |= T_STR;
         /*
          * removing this code forces all incoming variables to be
          * treated as strings. then as needed, they can be converted.
          */
         /*
         c->c_vid->n_fval = atof(p);
         if (c->c_vid->n_fval != 0){
            c->c_vid->n_tval = T_NUM;
            c->c_vid->n_scale = 6;
            }
         */
         return(FB_AOK);
      }

   void mf_push_field(f)
      fb_field *f;

      {
         m_fstack[m_ftop] = f;
         if (++m_ftop >= MAX_FSTACK)
            fb_xerror(FB_MESSAGE, "Too many depths of macro field editing", NIL);
      }

   void mf_pop_field()
      {
         if (--m_ftop < 0)
            fb_xerror(FB_MESSAGE, "Tried to pop on empty macro field stack", NIL);
         m_fstack[m_ftop] = NULL;
      }

   int mf_search_stack(f)
      fb_field *f;

      {
         int i;

         for (i = 0; i < m_ftop; i++)
            if (f == m_fstack[i])
               return(1);
         return(0);
      }


   void mf_push_symtab(s)
      fb_cell **s;

      {
         m_symstack[m_sstop] = s;
         if (++m_sstop >= MAX_SYMSTACK)
            fb_xerror(FB_MESSAGE, "Too many symbol table depths of macros", NIL);
      }

   void mf_pop_symtab()
      {
         if (--m_sstop < 0)
            fb_xerror(FB_MESSAGE,
               "Tried to pop on empty symbol table stack", NIL);
      }

   fb_cell **mf_current_symtab()
      {
         /*
         if (m_sstop == 0)
            fb_xerror(FB_MESSAGE, "Tried to use empty current symbol table",
               NIL);
         return(m_symstack[m_sstop-1]);
         */
         return(symtab);
      }

   void mf_init_stack(sv)
      fb_stack_vars *sv;

      {
         sv->break_flag = 0;
         sv->continue_flag = 0;
         sv->return_flag = 0;
         sv->return_value = 0;
         sv->exit_flag = 0;
         sv->s_symtab = NULL;
         sv->s_n_ghead = NULL;
         sv->s_c_ghead = NULL;
         sv->s_return_list = NULL;
      }

   void mf_make_frame(sv)
      fb_stack_vars *sv;

      {
         sv->s_symtab = symtab;
         symtab = fb_makesymtab();
         /* mnode garbage */
         sv->s_n_ghead = n_ghead;
         n_ghead = NULL;
         /* cell garbage */
         sv->s_c_ghead = c_ghead;
         c_ghead = NULL;
      }

   void mf_destroy_frame(sv)
      fb_stack_vars *sv;

      {
         /* garbage collection */
         fb_gcollect(n_ghead, c_ghead);
         /* restore previous garbage pointers */
         n_ghead = sv->s_n_ghead;
         c_ghead = sv->s_c_ghead;
         /* expunge and restore symtable */
         fb_expunge_symtab(symtab);
         symtab = sv->s_symtab;
      }

   void mf_local(n)
      fb_mnode *n;

      {
         fb_mnode *ln;
         fb_cell *c;

         if (n == NULL)
            return;
         for (ln = n; ln != NULL; ln = ln->n_list){
            if (ln->n_type != V_ID)
               fb_serror(FB_MESSAGE,
                  "local declaration: bad variable type", NIL);
            else{
               c = (fb_cell *) ln->n_obj;
               if (c != NULL && c->c_sval != NULL)
                  fb_sinstall(c->c_sval);
               }
            }
      }

   void mf_perror(s1, s2, n)
      char *s1, *s2;
      fb_mnode *n;

      {
         char buffer[FB_MAXLINE];

         sprintf(buffer, "%s (near line %d of file %s)", s2,
            n->n_lineno, n->n_fname);
         fb_serror(FB_MESSAGE, s1, buffer);
      }
