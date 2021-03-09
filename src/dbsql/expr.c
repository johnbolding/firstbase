/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: expr.c,v 9.1 2001/01/16 02:46:46 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Expr_sid[] = "@(#) $Id: expr.c,v 9.1 2001/01/16 02:46:46 john Exp $";
#endif

#include "dbsql_e.h"

static char *BADPAT = "bad regular expression: ";
extern short cdb_datestyle;
extern short cdb_datedisplay;

static void do_op();
static testvalues();
static teststrings();

/*
 * p_expr - print an expression
 */

   p_expr(n)
      node *n;

      {
         fb_field *f;
         char buf1[FB_MAXLINE], buf2[FB_MAXLINE], *fb_trim(), *df, *p;
         float expr();
         int size;

         expr(n);
         lastchar_was_newline = 0;
         if (print_horizontal)
            size = n->n_width;
         else
            size = n->n_vwidth;		/* vertical width */
         if (istype_fld(n)){
            f = (fb_field *) n->n_p2;
            if (html){
               if (FB_OFNUMERIC(f->type))
                  html_cell_open(1);
               else
                  html_cell_open(0);
               }
            if (quoteflag && !(FB_OFNUMERIC(f->type)) && f->type != FB_FORMULA)
               fprintf(sql_ofs, "\"");
            if (f->type == FB_FORMULA){
               sprintf(cdb_afld, "%.*f", n->n_scale, n->n_fval);
               df = cdb_afld;
               }
            else
               df = f->fld;
            strcpy(cdb_bfld, df);
            if (!emitflag)
               fb_formfield(cdb_afld, cdb_bfld, f->type, size);
            else if (emitflag && verbose &&
                  (f->type ==FB_DATE || f->type == FB_DOLLARS)){
               fb_formfield(cdb_afld, cdb_bfld, f->type, size);
               fb_rmlead(cdb_afld);
               fb_trim(cdb_afld);
               }
            else
               strcpy(cdb_afld, cdb_bfld);
            if (newline_flag == 0){
               for (p = cdb_afld; *p; p++){ /* escape emb quotes/backg */
                  if (*p == CHAR_NEWLINE){
                     fprintf(sql_ofs, "\\n");
                     }
                  else {
                     if (*p == CHAR_QUOTE || *p == CHAR_BACKSLASH)
                        fprintf(sql_ofs, "\\");
                     fprintf(sql_ofs, "%c", *p);
                     }
                  }
               }
            else if (emitflag)
               fprintf(sql_ofs, "%s", cdb_afld);
            else if (print_horizontal)
               fprintf(sql_ofs, "%-*s ", size, cdb_afld);
            else if (strchr(cdb_afld, FB_NEWLINE) != 0)
               print_longfield(fb_trim(cdb_afld));
            else
               fprintf(sql_ofs, "%s", fb_trim(cdb_afld));
            if (quoteflag && !(FB_OFNUMERIC(f->type)) && f->type != FB_FORMULA)
               fprintf(sql_ofs, "\"");
            }
         else if (istype_str(n)){
            if (html)
               html_cell_open(0);
            fprintf(sql_ofs, "%-*s ", size, n->n_nval);
            }
         else{
            if (html)
               html_cell_open(0);
            sprintf(buf1, "%.*f", n->n_scale, n->n_fval);
            fb_rjustify(buf2, buf1, size, FB_FLOAT);
            fprintf(sql_ofs, "%-*s ", size, buf2);
            }
         if (html)
            html_cell_close();
      }

/*
 * print_longfield - print a longfield, line at a time, for formatpage stuff.
 */

   print_longfield(s)
      char *s;

      {
         char *p;
         short int padsize, col;

         h_newline();
         lastchar_was_newline = 1;
         padsize = 2 + pageindent;
         col = 1;
         for (p = s; *p; p++){
            if (lastchar_was_newline){
               fprintf(sql_ofs, "%*s", padsize, " ");
               col += padsize;
               }
            lastchar_was_newline = 0;
            if (*p == FB_NEWLINE){
               h_newline();
               lastchar_was_newline = 1;
               col = 1;
               }
            else{
               fprintf(sql_ofs, "%c", *p);
               if (++col >= cdb_t_cols){
                  h_newline();
                  lastchar_was_newline = 1;
                  col = 1;
                  }
               }
            }
      }

/*
 * s_expr - string an expression, store it in 'print val', n_pval
 */

   char *s_expr(n)
      node *n;

      {
         char *tostring();
         float expr();

         expr(n);
         return(tostring(n));
      }

/*
 * tostring - convert a node to a string as best as possible.
 *	store result in n->n_pval, and return it too.
 */

   char *tostring(n)
      node *n;

      {
         fb_field *f;
         fb_database *d;
         char buf1[FB_MAXLINE];

         if (istype_fld(n)){
            f = (fb_field *) n->n_p2;
            if (f->type == FB_FORMULA){
               d = (fb_database *) n->n_p1;
               f = (fb_field *) n->n_p2;
               fb_fetch(f, cdb_afld, d);
               fb_mkstr(&(n->n_pval), cdb_afld);
               }
            else
               fb_mkstr(&(n->n_pval), f->fld);
            }
         else if (istype_str(n)){
            sprintf(buf1, "%s", n->n_nval);
            fb_mkstr(&(n->n_pval), buf1);
            }
         else{
            sprintf(buf1, "%.*f", n->n_scale, n->n_fval);
            fb_mkstr(&(n->n_pval), buf1);
            }
         return(n->n_pval);
      }

/*
 * is_null - return 1 if the node is NULL, else return 0
 */

   is_null(n)
      node *n;

      {
         fb_field *f;
         int rval = 0;

         if (istype_fld(n)){
            f = (fb_field *) n->n_p2;
            if (f->type != FB_FORMULA && f->fld[0] == NULL)
               rval = 1;
            }
         else if (istype_str(n)){
            if (n->n_nval[0] == NULL)
               rval = 1;
            }
         return(rval);
      }

/*
 * expr - take a tree node and do its expressions recusively
 */

   float expr(n)
      node *n;

      {
         node *n1 = NULL, *n2 = NULL, *n3 = NULL, *nn;
         fb_field *f;
         float function(), sfunction();
         char *tostring(), *p;
         short int sflag;
         fb_database *d;

         if (istype_bop(n)){
            expr(n->n_narg[0]);
            expr(n->n_narg[1]);
            n1 = n->n_narg[0];
            n2 = n->n_narg[1];
            n->n_scale = MAX(n1->n_scale, n2->n_scale);
            if (n2->n_type == S_SUBQ){
               nn = n2->n_list;
               for (; nn != NULL; nn = nn->n_list)
                  expr(nn);
               }
            }
         else if (istype_uop(n)){
            expr(n->n_narg[0]);
            n1 = n->n_narg[0];
            n->n_scale = n1->n_scale;
            }
         else if (istype_top(n)){
            expr(n->n_narg[0]);
            expr(n->n_narg[1]);
            expr(n->n_narg[2]);
            n1 = n->n_narg[0];
            n2 = n->n_narg[1];
            n3 = n->n_narg[2];
            n->n_scale = MAX(MAX(n1->n_scale, n2->n_scale), n3->n_scale);
            }
         else if (istype_lop(n)){
            expr(n->n_narg[0]);
            n1 = n->n_narg[0];
            for (nn = n->n_narg[1]; nn != NULL; nn = nn->n_list)
               expr(nn);
            }
         else if (istype_fcn(n) && eval_functions)
            return(function(n));
         else if (istype_sfcn(n))	/* math functions, etc */
            return(sfunction(n));
         switch(n->n_type){
            case O_MUL:
               n->n_fval = n1->n_fval * n2->n_fval;
               break;
            case O_DIV:
               if (n2->n_fval != 0)
                  n->n_fval = n1->n_fval / n2->n_fval;
               else
                  n->n_fval = 0;
               break;
            case O_ADD:
               n->n_fval = n1->n_fval + n2->n_fval;
               break;
            case O_SUB:
               n->n_fval = n1->n_fval - n2->n_fval;
               break;
            case O_CONCAT:
               tostring(n1);
               tostring(n2);
               p = fb_malloc(strlen(n1->n_pval) + strlen(n2->n_pval) + 2);
               sprintf(p, "%s%s", n1->n_pval, n2->n_pval);
               fb_mkstr(&(n->n_nval), p);
               fb_free(p);
               n->n_tval |= T_STR;
               n->n_fval = 0;
            case O_UPLUS:
               break;
            case O_UMINUS:
               n->n_fval = -n1->n_fval;
               break;
            case O_OR:
               if (is_null(n1)){
                  if (is_null(n2))
                     n->n_fval = 0;
                  else
                     n->n_fval = n2->n_fval;
                  }
               else if (is_null(n2))
                  n->n_fval = n1->n_fval;
               else
                  n->n_fval = n1->n_fval || n2->n_fval;
               break;
            case O_AND:
               if (is_null(n1) || is_null(n2))
                  n->n_fval = 0;
               else
                  n->n_fval = n1->n_fval && n2->n_fval;
               break;
            case O_UNOT:
               if (is_null(n1))
                  n->n_fval = 0;
               else
                  n->n_fval = !(n1->n_fval);
               break;
            case O_UEXISTS:
               /* subq only, by definition */
               nn = n1->n_list;
               if (nn == NULL || is_null(nn))
                  n->n_fval = 0;
               else
                  n->n_fval = 1;
               break;
            case O_UNION:
               break;
            case V_ID:
               f = (fb_field *) n->n_p2;
               n->n_fval = (float) atof(f->fld);
               if (f->type == FB_DOLLARS){
                  n->n_fval /= 100.0;
                  n->n_scale = 2;
                  }
               else if (f->type == FB_FORMULA){
                  d = (fb_database *) n->n_p1;
                  fb_fetch(f, cdb_afld, d);
                  n->n_fval = (float) atof(cdb_afld);
                  n->n_scale = 6;
                  if (f->idefault != NULL)
                     if ((p = strrchr(f->idefault, CHAR_COLON)) != 0)
                        n->n_scale = atoi(p+1);
                  }
               break;
            case R_EQ:
               do_op(R_EQ, n, n1, n2);
               break;
            case R_LT:
               do_op(R_LT, n, n1, n2);
               break;
            case R_GT:
               do_op(R_GT, n, n1, n2);
               break;
            case R_LE:
               do_op(R_LE, n, n1, n2);
               break;
            case R_GE:
               do_op(R_GE, n, n1, n2);
               break;
            case R_NE:
               do_op(R_NE, n, n1, n2);
               break;
            case P_BETWEEN:
            case P_NOT_BETWEEN:
               n->n_fval = 0;
               if (is_null(n1) || is_null(n2) || is_null(n3))
                  break;	/* break so as not to do the NOT BETWEEN */
               /*if (istype_str(n1) || istype_str(n2) || istype_str(n3)){*/
               if (istype_str(n1)){
                  tostring(n1);
                  tostring(n2);
                  tostring(n3);
                  if (strcmp(n1->n_pval, n2->n_pval) >= 0 &&
                      strcmp(n1->n_pval, n3->n_pval) <= 0)
                     n->n_fval = 1;
                  }
               else if (n1->n_fval >= n2->n_fval && n1->n_fval <= n3->n_fval)
                  n->n_fval = 1;
               if (n->n_type == P_NOT_BETWEEN)
                  n->n_fval = (n->n_fval ? 0 : 1);
               break;
            case P_IN:
            case P_NOT_IN:
               n->n_fval = 0;
               nn = n->n_narg[1];
               if (nn->n_type == S_SUBQ){
                  nn = nn->n_list;
                  }
               if (is_null(n1))
                  break;	/* break so as not to do the NOT IN */
               sflag = 0;
               if (istype_str(n1)){
                  tostring(n1);
                  sflag = 1;
                  }
               for (; nn != NULL && !n->n_fval; nn = nn->n_list){
                  if (is_null(nn))
                     ;			/* skip to next, - continue */
                  else if (sflag){
                     tostring(nn);
                     if (equal(n1->n_pval, nn->n_pval))
                        n->n_fval = 1;
                     }
                  else if (n1->n_fval == nn->n_fval)
                     n->n_fval = 1;
                  }
               if (n->n_type == P_NOT_IN)
                  n->n_fval = (n->n_fval ? 0 : 1);
               break;
            case P_LIKE:
            case P_NOT_LIKE:
               /* n1 is the string to match, n2 is the pattern (if needed) */
               n->n_fval = 0;
               if (is_null(n1))
                  break;	/* break so as not to do the NOT LIKE */
               tostring(n1);
               if (npatterns > 1){
#if SYS_V
                  cdb_Re_pat = (char *) n2->n_p1;
#else
                  if (re_comp(n2->n_nval) != NULL){
                     fb_serror(FB_MESSAGE, BADPAT, n2->n_nval);
                     return(FB_ERROR);
                     }
#endif /* SYS_V */
                  }
               if (re_exec(n1->n_pval))
                  n->n_fval = 1;
               if (n->n_type == P_NOT_LIKE)
                  n->n_fval = (n->n_fval ? 0 : 1);
               break;
            case P_IS:
            case P_IS_NOT:
               /* mechanism to test for NULL colum specs */
               n->n_fval = 0;
               if (is_null(n1))
                  n->n_fval = 1;
               if (n->n_type == P_IS_NOT)
                  n->n_fval = (n->n_fval ? 0 : 1);
               break;
            case V_SCON:
            case V_CCON:
               if (n->n_nval != NULL)
                  n->n_fval = (float) atof(n->n_nval);
               break;
            }
         return(n->n_fval);
      }

/*
 * do_op - do the comparison n1 op n2 where n2 might be a sub query list
 */

   static void do_op(op, n, n1, n2)
      int op;
      node *n, *n1, *n2;

      {
         int sflag = 0, subq = 0, val = 0, all_flag;
         node *nq, *nn;
         char buf[12];

         n->n_fval = 0;
         if (is_null(n1))
            return;
         if (istype_str(n1)){
            tostring(n1);
            sflag = 1;
            }
         if (istype_date(n1) || istype_date(n2)){
            if ((cdb_datestyle == FB_EUROPEAN) &&
                  !istype_fld(n1) && !istype_fcn(n1) && !istype_sfcn(n1))
               fb_ameri_date(n1->n_pval);
            if (cdb_datedisplay == 8)
               fb_endate(n1->n_pval);
            else{
               fb_longdate(buf, n1->n_pval);
               fb_mkstr(&(n1->n_pval), buf);
               fb_long_endate(n1->n_pval);
               }
            }
         all_flag = 1;
         if (n2->n_type == S_SUBQ){
            nn = n2->n_list;
            subq = 1;
            nq = n2->n_narg[3];
            if (nq != NULL)
               if (nq->n_type == Q_ANY)
                  all_flag = 0;
            }
         else
            nn = n2;
         val = 0;
         for (; nn != NULL; nn = nn->n_list){
            if (is_null(nn)){
               if (!subq)
                  continue;		/* skip to next */
               }
            else if (sflag){
               tostring(nn);
               if (istype_date(nn) || istype_date(n1)){
                  if ((cdb_datestyle == FB_EUROPEAN) &&
                        !istype_fld(nn) && !istype_fcn(nn) && !istype_sfcn(nn))
                     fb_ameri_date(nn->n_pval);
                  if (cdb_datedisplay == 8)
                     fb_endate(nn->n_pval);
                  else{
                     fb_longdate(buf, nn->n_pval);
                     fb_mkstr(&(nn->n_pval), buf);
                     fb_long_endate(nn->n_pval);
                     }
                  }
               val = teststrings(op, n1->n_pval, nn->n_pval);
               }
            else
               val = testvalues(op, n1->n_fval, nn->n_fval);
            if (all_flag && val == 0)
               break;
            if (all_flag == 0 && val == 1)
               break;
            if (!subq)			/* if not a sub query, get out */
               break;
            }
         n->n_fval = val;
      }
/*
 * testvalues - test the values a,b against the op
 */

   static testvalues(op, a, b)
      int op;
      float a, b;

      {
         switch(op){
            case R_EQ:
               return((a == b) ? 1 : 0);
            case R_LT:
               return((a < b) ? 1 : 0);
            case R_GT:
               return((a > b) ? 1 : 0);
            case R_LE:
               return((a <= b) ? 1 : 0);
            case R_GE:
               return((a >= b) ? 1 : 0);
            case R_NE:
               return((a != b) ? 1 : 0);
            default:
               fb_serror(FB_MESSAGE, "Unknown R_OP in testvalue expr", NIL);
               return(0);
            }
      }

/*
 * teststrings - test the strings a,b against the op
 */

   static teststrings(op, a, b)
      int op;
      char *a, *b;

      {
         switch(op){
            case R_EQ:
               return((strcmp(a, b) == 0) ? 1 : 0);
            case R_LT:
               return((strcmp(a, b) < 0) ? 1 : 0);
            case R_GT:
               return((strcmp(a, b) > 0) ? 1 : 0);
            case R_LE:
               return((strcmp(a, b) <= 0) ? 1 : 0);
            case R_GE:
               return((strcmp(a, b) >= 0) ? 1 : 0);
            case R_NE:
               return((strcmp(a, b) != 0) ? 1 : 0);
            default:
               fb_serror(FB_MESSAGE, "Unknown R_OP in teststring expr", NIL);
               return(0);
            }
      }
