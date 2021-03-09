/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_trac.c,v 9.0 2001/01/09 02:56:54 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macrotrace_sid[] = "@(#) $Id: mac_trac.c,v 9.0 2001/01/09 02:56:54 john Exp $";
#endif

#include <fb.h>
#include <macro_e.h>

static int level = 0;			/* used by trace routines */
static FILE *mf;

extern char macrotrace_filename[];

#if !FB_PROTOTYPES
static s_trace();
static traceargs();
static void a_trace();
static tracecell();
#else
static s_trace(fb_mnode *);
static traceargs(fb_mnode *);
static void a_trace(fb_mnode *, int);
static tracecell(fb_mnode *);
#endif

#ifdef FB_MAC_TRAC_DEBUG
static trace_expr_list(fb_mnode *, int i);
#endif /*  FB_MAC_TRAC_DEBUG */

   void fb_macrotrace(n, fname)
      fb_mnode *n;
      char *fname;

      {
         (void) Macrotrace_sid;

         level = 0;
         if (macrotrace_filename[0] != NULL){
            mf = fopen(macrotrace_filename, "w");
            if (mf == NULL)
               mf = stderr;
            }
         else
            mf = stderr;
         fprintf(stderr, "------------------------------------------\n");
         fb_tracesource(mf, fname);
         r_macrotrace(n);
         if (mf != stderr)
            fclose(mf);
      }

   void fb_tracesource(fs, fname)
      FILE *fs;
      char *fname;

      {
         int fd, i;
         char line[FB_MAXLINE];

         fd = open(fname, READ);
         if (fd <= 0)
            return;
         fb_r_init(fd);
         fprintf(fs, "----------------- begin macro code ----------\n");
         for (i = 1; ; i++){
            if (fb_nextline(line, FB_MAXLINE) == 0)
               break;
            fprintf(fs, "%2d %s\n", i, line);
            }
         fprintf(fs, "----------------- end macro code ----------\n");
         fb_r_end();			/* closes fd */
      }

   void r_macrotrace(n)
      fb_mnode *n;

      {
         if (mf == NULL)
            mf = stderr;
         for (; n != NULL; n = n->n_next)
            s_trace(n);
      }

   static s_trace(n)
      fb_mnode *n;

      {
         fprintf(mf, "%*s statement - ", level*3, " ");
         switch(n->n_type){
            case S_NULL:  fprintf(mf, "NULL\n"); break;
            case S_EXPR:  fprintf(mf, "EXPR\n"); break;
            case S_IF:    fprintf(mf, "IF\n"); break;
            case S_IFELSE:    fprintf(mf, "IFELSE\n"); break;
            case S_WHILE:    fprintf(mf, "WHILE\n"); break;
            case S_FOR:    fprintf(mf, "FOR\n"); break;
            case S_BREAK:    fprintf(mf, "BREAK\n"); break;
            case S_CONTINUE:    fprintf(mf, "CONTINUE\n"); break;
            case S_LIST:    fprintf(mf, "LIST\n"); break;
            case S_RETURN:    fprintf(mf, "RETURN\n"); break;
            case S_EXIT:    fprintf(mf, "EXIT\n"); break;
            case S_BEGIN:    fprintf(mf, "BEGIN\n"); break;
            case S_BODY:    fprintf(mf, "BODY\n"); break;
            case S_END:    fprintf(mf, "END\n"); break;
            case S_FUNCTION:    fprintf(mf, "FUNCTION\n"); break;
            case S_LOCAL:    fprintf(mf, "LOCAL\n"); break;
            case S_SWITCH:    fprintf(mf, "SWITCH\n"); break;
            case S_CASE:    fprintf(mf, "CASE\n"); break;
            case S_DEFAULT:    fprintf(mf, "DEFAULT\n"); break;
            default:  fprintf(mf, "UNKNOWN <*** \n"); break;
            }
         traceargs(n);
      }

   static traceargs(n)
      fb_mnode *n;

      {
         int i;
         fb_mnode *a, *p;

         level++;
         for (i = 0; n != NULL && i < NARGS; i++){
            p = n->n_narg[i];
            if (p == NULL)
               break;
            /* trace p */
            a_trace(p, i);
            if ((a = p->n_list) != NULL){
               for (; a != NULL; a = a->n_list){
                  a_trace(a, 77);
                  }
               }
            }
         level--;
      }

   static void a_trace(n, i)
      fb_mnode *n;
      int i;

      {
         int skip = 0;
         int x, k;
         fb_mnode *q;

         if (n == NULL)
            return;
         x = (int) n;
         fprintf(mf, "%*s narg[%d ... addr:%x:%d:%s] - ", level*3, " ", i, x,
            n->n_lineno, n->n_fname);
         switch(n->n_type){
            case S_BEGIN:  fprintf(mf, "BEGIN\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_BODY:   fprintf(mf, "BODY\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_END:    fprintf(mf, "END\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_FUNCTION:
                           fprintf(mf, "FUNCTION\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_NULL:   fprintf(mf, "NULL");
                           break;
            case S_EXPR:   fprintf(mf, "EXPR\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_IF:     fprintf(mf, "IF\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_IFELSE: fprintf(mf, "IFELSE\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_WHILE:  fprintf(mf, "WHILE\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_FOR:    fprintf(mf, "FOR\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_BREAK:  fprintf(mf, "BREAK\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_CONTINUE:fprintf(mf, "CONTINUE\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_LIST:   fprintf(mf, "sLIST\n");
                           s_trace(n);
                           skip = 1;
                           break;
            case S_RETURN: fprintf(mf, "RETURN\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_EXIT:   fprintf(mf, "EXIT\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_LOCAL: fprintf(mf, "LOCAL\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_SWITCH: fprintf(mf, "SWITCH\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_CASE: fprintf(mf, "CASE\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case S_DEFAULT: fprintf(mf, "DEFAULT\n");
                           r_macrotrace(n);
                           skip = 1;
                           break;
            case V_ARRAY:  fprintf(mf, "V_ARRAY\n");
                           traceargs(n); skip = 1; break;
            case V_ID:     fprintf(mf, "V_ID");
                           skip = tracecell(n);
                           break;
            case V_CON:    fprintf(mf, "V_CON");
                           skip = tracecell(n);
                           break;
            case V_OCON:    fprintf(mf, "V_OCON");
                           skip = tracecell(n);
                           break;
            case V_FCON:   fprintf(mf, "V_FCON");
                           skip = tracecell(n);
                           break;
            case V_CCON:   fprintf(mf, "V_CCON");
                           skip = tracecell(n);
                           break;
            case V_SCON:   fprintf(mf, "V_SCON");
                           skip = tracecell(n);
                           break;
            case O_ASSIGN: fprintf(mf, "O_ASSIGN\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_LSHFT_A:
                           fprintf(mf, "O_LSHFT_A\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_RSHFT_A:
                           fprintf(mf, "O_RSHFT_A\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_XOR_A:
                           fprintf(mf, "O_XOR_A\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_AND_A:
                           fprintf(mf, "O_AND_A\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_OR_A:
                           fprintf(mf, "O_OR_A\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_ADD_A:
                           fprintf(mf, "O_ADD_A\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_MINUS_A:
                           fprintf(mf, "O_MINUS_A\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_MULT_A:
                           fprintf(mf, "O_MULT_A\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_DIV_A:
                           fprintf(mf, "O_DIV_A\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_MOD_A:
                           fprintf(mf, "O_MOD_A\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_ADD:    fprintf(mf, "O_ADD\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_SUB:    fprintf(mf, "O_SUB\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_MUL:    fprintf(mf, "O_MUL\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_DIV:    fprintf(mf, "O_DIV\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_CONCAT: fprintf(mf, "O_CONCAT\n");traceargs(n);skip=1;break;
            case O_UPLUS:  fprintf(mf, "O_UPLUS\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_UMINUS: fprintf(mf, "O_UMINUS\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_OR: fprintf(mf, "O_OR\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_AND: fprintf(mf, "O_AND\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case O_UNOT: fprintf(mf, "O_UNOT\n");
                           traceargs(n);
                           skip = 1;
                           break;

            case O_XOR: fprintf(mf, "O_XOR\n");
                           traceargs(n); skip = 1; break;
            case O_XAND: fprintf(mf, "O_XAND\n");
                           traceargs(n); skip = 1; break;
            case O_IOR: fprintf(mf, "O_IOR\n");
                           traceargs(n); skip = 1; break;
            case O_LSHFT: fprintf(mf, "O_LSHFT\n");
                           traceargs(n); skip = 1; break;
            case O_RSHFT: fprintf(mf, "O_RSHFT\n");
                           traceargs(n); skip = 1; break;
            case O_MOD: fprintf(mf, "O_MOD\n");
                           traceargs(n); skip = 1; break;
            case O_CALL: fprintf(mf, "O_CALL");
                           tracecell(n->n_narg[0]);
                           fprintf(mf, "\n");
                           k = 1;
                           for (q = n->n_narg[1]; q != NULL; q = q->n_next){
                              fprintf(mf, "%*s ** argument %d\n",
                                 level*3, " ",k++);
                              a_trace(q, i);
                              }
                           skip = 1;
                           break;
            case O_INCR_A: fprintf(mf, "O_INCR_A\n");
                           traceargs(n); skip = 1; break;
            case O_DECR_A: fprintf(mf, "O_DECR_A\n");
                           traceargs(n); skip = 1; break;
            case O_INCR_B: fprintf(mf, "O_INCR_B\n");
                           traceargs(n); skip = 1; break;
            case O_DECR_B: fprintf(mf, "O_DECR_B\n");
                           traceargs(n); skip = 1; break;
            case O_UOR: fprintf(mf, "O_UOR\n");
                           traceargs(n); skip = 1; break;
            case O_UFIELD: fprintf(mf, "O_UFIELD\n");
                           traceargs(n); skip = 1; break;

            case R_EQ:     fprintf(mf, "R_EQ\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case R_LT:     fprintf(mf, "R_LT\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case R_GT:     fprintf(mf, "R_GT\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case R_LE:     fprintf(mf, "R_LE\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case R_GE:     fprintf(mf, "R_GE\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case R_NE:     fprintf(mf, "R_NE\n");
                           traceargs(n);
                           skip = 1;
                           break;
            case R_END:    fprintf(mf, "R_END\n");
                           skip = 1; break;
            case R_ERROR:  fprintf(mf, "R_ERROR\n");
                           skip = 1; break;
            case R_ABORT:  fprintf(mf, "R_ABORT\n");
                           skip = 1; break;
            case R_NEXT:   fprintf(mf, "R_NEXT\n");
                           skip = 1; break;
            case R_PREV:   fprintf(mf, "R_PREV\n");
                           skip = 1; break;
            default:       fprintf(mf, "UNKNOWN ARG"); break;
            }
         if (!skip)
            fprintf(mf, "\n");
      }

   static tracecell(n)
      fb_mnode *n;

      {
         fb_cell *c;
         int skip = 0;

         if (n == NULL)
            return(skip);
         c = (fb_cell *) n->n_obj;
         if (c != NULL){
            fprintf(mf, " ... %s", c->c_sval);
            if (n->n_narg[0] != NULL){
               fprintf(mf, "\n");
               traceargs(n);
               skip = 1;
               }
            }
         return(skip);
      }

#ifdef FB_MAC_TRAC_DEBUG
   static trace_expr_list(n, i)
      fb_mnode *n;
      int i;

      {
         for (n = n->n_narg[0]; n != NULL; n = n->n_next)
            a_trace(n, i);
         fprintf(mf, "\n");
      }
#endif /* FB_MAC_TRAC_DEBUG */
