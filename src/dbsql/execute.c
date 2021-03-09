/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: execute.c,v 9.0 2001/01/09 02:55:46 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Execute_sid[] = "@(#) $Id: execute.c,v 9.0 2001/01/09 02:55:46 john Exp $";
#endif

#include "dbsql_e.h"

#if DEBUG
static int level;			/* used by trace routines */
#endif

/*
 * execute - the driver part of statement execution
 */

   execute(n)
      node *n;

      {
         node *h;
         FILE *popen();

         for (; n != NULL; n = n->n_next){
            /* these flags need resetting before each executed statement */
            relation_function = 0;
            group_expr = 0;
            linenumber = 0;
            sql_ofs = NULL;
            eval_functions = 1;
#if DEBUG
            if (traceflag > 5)
               trace(n);
#endif /* DEBUG */
            switch(n->n_type){
               case S_NULL:
                  break;
               case S_QUERY:
                  if (pagerflag)
                     sql_ofs = popen(sql_pager, "w");
                  else
                     sql_ofs = stdout;
                  /* process header if there */
                  h = (node *) n->n_p1;
                  if (h != NULL && formatpage)
                     h_load(h);
                  if (html)
                     html_open();
                  query_select(n, 0);
                  if (html)
                     html_close();
                  if (formatpage && linenumber > 0)
                     h_footers();
                  if (pagerflag){
                     pclose(sql_ofs);
                     sql_ofs = NULL;
                     }
                  break;
               case S_CREATE_VIEW:
                  createview(n);
                  break;
               case S_DROP_VIEW:
                  dropview(n);
                  break;
               case S_CREATE_INDEX:
                  createindex(n);
                  break;
               case S_DROP_INDEX:
                  dropindex(n);
                  break;
               default:
                  fprintf(stderr, "UNKNOWN STATEMENT *** \n");
                  break;
               }
            u_vir_remove();
         }
      }

/*
 * query_select - main driver of SELECT statement
 */

   query_select(n, sub_flag)
      node *n;
      int sub_flag;

      {
         node *sel_q, *sel_list, *tab_exp, *from, *where, *groupby;
         node *make_canon(), *cform, *order_by, *having;
         node *x_from;
         int st = FB_ERROR;

         sel_q =     n->n_narg[0];
         sel_list =  n->n_narg[1];
         tab_exp =   n->n_narg[2];
         order_by =  n->n_narg[3];

         from =      tab_exp->n_narg[0];

         /* save the from table */
         x_from = g_from;
         g_from = from;

         where =     tab_exp->n_narg[1];
         groupby =   tab_exp->n_narg[2];
         having =    tab_exp->n_narg[3];

         cform = make_canon(where);
#if DEBUG
         if (traceflag > 5)
            trace_canon(cform);
#endif /* DEBUG */
         if (cform->n_tvarc <= 1)
            st = u_qselect(sel_q, sel_list, from, cform, groupby, having,
               sub_flag, n, order_by);
         else
            st = u_multivars(sel_q, sel_list, from, cform, groupby, having,
               sub_flag, n, order_by);
         if (st != FB_ERROR)
            u_closetables(from);
         /* restore the from table */
         g_from = x_from;
         return(st);
      }

#if DEBUG

   trace(n)
      node *n;

      {
         for (; n != NULL; n = n->n_next)
            s_trace(n);
      }

   s_trace(n)
      node *n;

      {
         node *h;

         /* trace header if there */
         h = (node *) n->n_p1;
         if (h != NULL){
            fprintf(stderr, "heading:\n");
            a_trace(h, 66);
            }
         fprintf(stderr, "statment: ");
         switch(n->n_type){
            case S_NULL:   fprintf(stderr, "NULL\n"); break;
            case S_QUERY:  fprintf(stderr, "QUERY SELECT\n");
                           break;
            case S_SUBQ:   fprintf(stderr, "SUBQ SELECT\n");
/*
               if ((a = n->n_list) != NULL){
                  for (; a != NULL; a = a->n_list){
                     a_trace(a, 77);
                     if ((b = a->n_group) != NULL){
                        for (; b != NULL; b = b->n_group)
                           a_trace(b, 99);
                        }
                     }
                  }
               fprintf(stderr, " ... [END SUBQ SELECT statement]\n");
*/
               break;
            case S_CREATE_VIEW: fprintf(stderr, "CREATE VIEW\n"); break;
            case S_DROP_VIEW: fprintf(stderr, "DROP VIEW\n"); break;
            case S_CREATE_INDEX: fprintf(stderr, "CREATE INDEX\n"); break;
            case S_DROP_INDEX: fprintf(stderr, "DROP INDEX\n"); break;
            default:  fprintf(stderr, "UNKNOWN <*** \n"); break;
            }
         traceargs(n);
      }

   traceargs(n)
      node *n;

      {
         int i;
         node *a, *p, *b;

         level++;
         for (i = 0; n != NULL && i < NARGS; i++){
            p = n->n_narg[i];
            if (p == NULL)
               break;
            /* trace p */
            a_trace(p, i);
            /* trace p group */
            if ((b = p->n_group) != NULL){
               for (; b != NULL; b = b->n_group)
                  a_trace(b, 99);
               }
            if ((a = p->n_list) != NULL){
               for (; a != NULL; a = a->n_list){
                  a_trace(a, 77);
                  if ((b = a->n_group) != NULL){
                     for (; b != NULL; b = b->n_group)
                        a_trace(b, 99);
                     }
                  }
               }
            }
         level--;
      }

   a_trace(n, i)
      node *n;
      int i;

      {
         int skip = 0;
         int x;

         if (n == NULL)
            return;
         x = (int) n;
         if (n->n_label != NULL && n->n_label != (node *) NULL){
            fprintf(stderr, "%*s (label ", level*3, " ");
            tracecell(n->n_label);
            fprintf(stderr, " )\n");
            }
         fprintf(stderr, "%*s narg[%d ... addr:%x] - ", level*3, " ", i, x);
         switch(n->n_type){
            case S_NULL:   fprintf(stderr, "NULL");
                           break;
            case S_SUBQ:   fprintf(stderr, "SUBQ\n");
                           trace(n);
                           skip = 1;
                           break;
            case S_QUERY:  fprintf(stderr, "QUERY\n");
                           trace(n);
                           skip = 1;
                           break;
            case Q_ALL:    fprintf(stderr, "Q_ALL\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case Q_ANY:    fprintf(stderr, "Q_ANY\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case Q_SOME:    fprintf(stderr, "Q_SOME\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case Q_DISTINCT:fprintf(stderr, "Q_DISTINCT\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case Q_STAR:   fprintf(stderr, "Q_STAR");
                           skip = tracecell(n);
                           break;
            case Q_USER:   fprintf(stderr, "Q_USER"); break;
            case Q_ASC:    fprintf(stderr, "Q_ASC"); break;
            case Q_DESC:   fprintf(stderr, "Q_DESC"); break;
            case E_TABLE:  fprintf(stderr, "E_TABLE (FROM)\n");
                           traceargs(n);
                           break;
            case E_SORT:   fprintf(stderr, "E_SORT (ORDER BY)\n");
                           traceargs(n);
                           break;
            case V_ID:     fprintf(stderr, "V_ID");
                           if (n->n_id != -1)
                              fprintf(stderr, " (%d)", n->n_id);
                           skip = tracecell(n);
                           break;
            case V_CON:    fprintf(stderr, "V_CON");
                           skip = tracecell(n);
                           break;
            case V_FCON:   fprintf(stderr, "V_FCON");
                           skip = tracecell(n);
                           break;
            case V_CCON:   fprintf(stderr, "V_CCON");
                           skip = tracecell(n);
                           break;
            case V_SCON:   fprintf(stderr, "V_SCON");
                           skip = tracecell(n);
                           break;
            case O_ADD:    fprintf(stderr, "O_ADD\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_SUB:    fprintf(stderr, "O_SUB\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_MUL:    fprintf(stderr, "O_MUL\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_DIV:    fprintf(stderr, "O_DIV\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_UPLUS:  fprintf(stderr, "O_UPLUS\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_UMINUS: fprintf(stderr, "O_UMINUS\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_UEXISTS: fprintf(stderr, "O_UEXISTS\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_OR: fprintf(stderr, "O_OR\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_AND: fprintf(stderr, "O_AND\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_UNOT: fprintf(stderr, "O_UNOT\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_UNION: fprintf(stderr, "O_UNION\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case R_EQ:     fprintf(stderr, "R_EQ\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case R_LT:     fprintf(stderr, "R_LT\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case R_GT:     fprintf(stderr, "R_GT\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case R_LE:     fprintf(stderr, "R_LE\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case R_GE:     fprintf(stderr, "R_GE\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case R_NE:     fprintf(stderr, "R_NE\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case P_BETWEEN:fprintf(stderr, "P_BETWEEN\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case P_NOT_BETWEEN:fprintf(stderr, "P_NOT_BETWEEN\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case P_IN:fprintf(stderr, "P_IN\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case P_NOT_IN:fprintf(stderr, "P_NOT_IN\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case P_LIKE:fprintf(stderr, "P_LIKE\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case P_NOT_LIKE:fprintf(stderr, "P_NOT_LIKE\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case P_IS:fprintf(stderr, "P_IS\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case P_IS_NOT:fprintf(stderr, "P_IS_NOT\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case F_AVG:fprintf(stderr, "F_AVG %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_MAX:fprintf(stderr, "F_MAX %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_MIN:fprintf(stderr, "F_MIN %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_SUM:fprintf(stderr, "F_SUM %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_COUNT:fprintf(stderr, "F_COUNT %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_COUNTALL:fprintf(stderr, "F_COUNTALL %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_POWER:fprintf(stderr, "F_POWER %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_ROUND:fprintf(stderr, "F_ROUND %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_TRUNC:fprintf(stderr, "F_TRUNC %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_ABS:fprintf(stderr, "F_ABS %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_LENGTH:fprintf(stderr, "F_LENGTH %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_OWNER:fprintf(stderr, "F_OWNER %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_GROUP:fprintf(stderr, "F_GROUP %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_UID:fprintf(stderr, "F_UID %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_GID:fprintf(stderr, "F_GID %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_MODE:fprintf(stderr, "F_MODE %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_SUBSTR:fprintf(stderr, "F_SUBSTR %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_UPPER:fprintf(stderr, "F_UPPER %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_LOWER:fprintf(stderr, "F_LOWER %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_SUBLINE:fprintf(stderr, "F_SUBLINE %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case F_FORMFIELD:fprintf(stderr, "F_FORMFIELD %f\n", n->n_fval);
                           traceargs(n);
                           skip = 1;
                           break;
            case H_MASTER: fprintf(stderr, "H_MASTER\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case H_HEADER: fprintf(stderr, "H_HEADER\n");
                           level++;
                           a_trace(n->n_narg[0], 0);
                           a_trace(n->n_narg[1], 1);
                           a_trace(n->n_narg[2], 2);
                           a_trace(n->n_narg[3], 3);
                           level--;
                           skip = 1;
                           break;
            case H_FOOTER: fprintf(stderr, "H_FOOTER\n");
                           level++;
                           a_trace(n->n_narg[0], 0);
                           a_trace(n->n_narg[1], 1);
                           a_trace(n->n_narg[2], 2);
                           a_trace(n->n_narg[3], 3);
                           level--;
                           skip = 1;
                           break;
            case H_ODD:    fprintf(stderr, "H_ODD"); break;
            case H_EVEN:   fprintf(stderr, "H_EVEN"); break;
            default:       fprintf(stderr, "UNKNOWN ARG"); break;
            }
         if (!skip)
            fprintf(stderr, "\n");
      }

   tracecell(n)
      node *n;

      {
         cell *c;
         int skip = 0;

         if (n == NULL)
            return(skip);
         c = (cell *) n->n_obj;
         if (c != NULL){
            fprintf(stderr, " ... %s", c->c_sval);
            if (n->n_narg[0] != NULL){
               fprintf(stderr, "\n");
               traceargs(n);
               skip = 1;
               }
            }
         return(skip);
      }

   tracelist(n)
      node *n;
      {
         cell *c;

         for (; n != NULL; n = n->n_list){
            c = (cell *) n->n_obj;
            fprintf(stderr, "%s(%d) ", c->c_sval, n->n_id);
            }
         fprintf(stderr, "\n");
      }

   trace_fulllist(n)
      node *n;

      {
         if (n == NULL)
            return;
         for (; n != NULL; n = n->n_list)
            tracecell(n);
      }

   trace_canon(n)
      node *n;

      {
         for (; n != NULL; n = n->n_narg[1]){
            switch(n->n_type){
               case C_ROOT:
                  fprintf(stderr, "C_ROOT %d %d (id:%d):\n",
                     n->n_lvarc, n->n_tvarc, n->n_id);
                  if (n->n_list != NULL){
                     fprintf(stderr, "      ");
                     tracelist(n->n_list);
                     }
                  break;
               case C_AND:
                  fprintf(stderr, "C_AND %d %d (id:%d):\n",
                     n->n_lvarc, n->n_tvarc, n->n_id);
                  if (n->n_list != NULL){
                     fprintf(stderr, "      ");
                     tracelist(n->n_list);
                     }
                  break;
               default:
                  fprintf(stderr, "UNKNOWN CANONICAL NODE:\n");
                  break;
               }
            a_trace(n->n_narg[0], 0);
            }
      }

#endif /* DEBUG */
