/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: function.c,v 9.1 2001/01/12 22:51:56 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Function_sid[] = "@(#) $Id: function.c,v 9.1 2001/01/12 22:51:56 john Exp $";
#endif

#include "dbsql_e.h"
#if STDC_HEADERS
#include "float.h"
#endif

#ifndef MAXFLOAT
#ifdef FLT_MAX
#define MAXFLOAT FLT_MAX
#else /* not FLT_MAX */
#include "values.h"
#endif /* FLT_MAX */
#endif /* MAXFLOAT */

#define FB_NUMERIC_TEST	0
#define STRING_TEST	1
#define DATE_TEST	2
#define MAXSTRING	"~~~"

static node *fn;
static float val = 0;
static int vcount = 0;
static int comp_type = 0;	/* 0 = numeric, 1 = string */
static char *s_val = NULL;
static char save_date[10];

static fb_database *locate_function_dp();
static f_sum();
static f_avg();
static f_count();
static f_countall();
static f_max();
static f_min();

/*
 * function - handle the built in functions
 */

   float function(n)
      node *n;

      {
         cell *sinstall();
         fb_database *dp = NULL, *locate_function_dp();
         int f_sum(), f_avg(), f_max(), f_min(), f_count(), withindex = 0;
         int f_countall(), relation_eval;
         float expr();
         node *qn;
         unsigned tval;

         n->n_fval = 0;
         if (relation_function && r_head != NULL)
            relation_eval = 1;
         else{
            relation_eval = 0;
            dp = locate_function_dp(n);
            if (dp == NULL){
               fb_serror(FB_MESSAGE, "Could not locate function database", NIL);
               return(FB_ERROR);
               }
            }
         val = 0;
         vcount = 0;
         qn = n->n_narg[0];
         if (qn != NULL){
            fn = qn->n_narg[0];
            tval = fn->n_tval;
            if (istype_date(fn))
               comp_type = DATE_TEST;
            else if (vtype_str(tval))
               comp_type = STRING_TEST;		/* set for max/min */
            else
               comp_type = FB_NUMERIC_TEST;
            if (qn->n_type == Q_DISTINCT && !relation_eval){
               if (u_distinct(qn, fn, dp, g_restrict) == FB_ERROR){
                  fb_serror(FB_MESSAGE, "Distinct Set Function - failed build", NIL);
                  return(FB_ERROR);
                  }
               withindex = 1;
               if (fb_openidx(qn->n_virlist->n_nval, dp) == FB_ERROR){
                  fb_serror(FB_MESSAGE, "Distinct Set Function - failed open", NIL);
                  return(FB_ERROR);
                  }
               }
            }
         n->n_scale = 6;
         switch(n->n_type){
            case F_AVG:
               vcount = 0;
               if (relation_eval){
                  if (group_expr)
                     groupeach_relation(r_head, f_avg);
                  else
                     relateeach(r_head, f_avg);
                  }
               else if (group_expr)
                  groupeach(dp, f_avg);
               else if (withindex)
                  wherexeach(dp, f_avg, g_restrict);
               else
                  whereeach(dp, f_avg, g_restrict);
               if (vcount > 0)
                  val /= vcount;
               break;
            case F_MAX:
               val = 0;
               fb_mkstr(&s_val, NIL);
               if (relation_eval){
                  if (group_expr)
                     groupeach_relation(r_head, f_max);
                  else
                     relateeach(r_head, f_max);
                  }
               else if (group_expr)
                  groupeach(dp, f_max);
               else if (withindex)
                  wherexeach(dp, f_max, g_restrict);
               else
                  whereeach(dp, f_max, g_restrict);
               if (comp_type == STRING_TEST || comp_type == DATE_TEST){
                  if (comp_type == DATE_TEST){
                     fb_dedate(s_val);
                     n->n_tval |= T_DTE;
                     }
                  fb_mkstr(&(n->n_nval), s_val);
                  n->n_tval |= T_STR;
                  }
               break;
            case F_MIN:
               val = MAXFLOAT;
               fb_mkstr(&s_val, MAXSTRING);
               if (relation_eval){
                  if (group_expr)
                     groupeach_relation(r_head, f_min);
                  else
                     relateeach(r_head, f_min);
                  }
               else if (group_expr)
                  groupeach(dp, f_min);
               else if (withindex)
                  wherexeach(dp, f_min, g_restrict);
               else
                  whereeach(dp, f_min, g_restrict);
               if (comp_type == STRING_TEST || comp_type == DATE_TEST){
                  if (comp_type == DATE_TEST){
                     fb_dedate(s_val);
                     n->n_tval |= T_DTE;
                     }
                  fb_mkstr(&(n->n_nval), s_val);
                  n->n_tval |= T_STR;
                  }
               break;
            case F_SUM:
               if (relation_eval){
                  if (group_expr)
                     groupeach_relation(r_head, f_sum);
                  else
                     relateeach(r_head, f_sum);
                  }
               else if (group_expr)
                  groupeach(dp, f_sum);
               else if (withindex)
                  wherexeach(dp, f_sum, g_restrict);
               else
                  whereeach(dp, f_sum, g_restrict);
               break;
            case F_COUNT:
               vcount = 0;
               if (relation_eval){
                  if (group_expr)
                     groupeach_relation(r_head, f_count);
                  else
                     relateeach(r_head, f_count);
                  }
               else if (group_expr)
                  groupeach(dp, f_count);
               else if (withindex)
                  wherexeach(dp, f_count, g_restrict);
               else
                  whereeach(dp, f_count, g_restrict);
               val = vcount;
               break;
            case F_COUNTALL:
               vcount = 0;
               if (relation_eval){
                  if (group_expr)
                     groupeach_relation(r_head, f_countall);
                  else
                     relateeach(r_head, f_countall);
                  }
               else if (group_expr)
                  groupeach(dp, f_countall);
               else
                  whereeach(dp, f_countall, g_restrict);
               val = vcount;
               n->n_scale = 0;
               break;
            }
         n->n_fval = val;
         return(val);
      }

/*
 * locate_function_dp - locate the proper database pointer for a function call
 */

   static fb_database *locate_function_dp(n)
      node *n;

      {
         fb_database *dp = NULL;
         node *qn;

         qn = n->n_narg[0];	/* ALL */
         if (qn != NULL){
            qn = qn->n_narg[0];	/* ID */
            if (qn != NULL)
               dp = (fb_database *) qn->n_p1;
            }
         else if (n->n_type == F_COUNTALL)
            dp = (fb_database *) g_from->n_p1;
         return(dp);
      }


/*
 * gcollect_function - the garbage collection part of the function area
 */

   gcollect_function()
      {
         fb_free(s_val);
         s_val = NULL;
      }

/*
 * series of functions for set function capabilities
 */

   static f_sum(dp)
      fb_database *dp;

      {
         float expr(), v;

         v = expr(fn);
         if (!testnull(fn))
            val += v;
         return(FB_AOK);
      }

   static f_avg(dp)
      fb_database *dp;

      {
         float expr(), v;

         v = expr(fn);
         if (!testnull(fn)){
            val += v;
            vcount++;
            }
         return(FB_AOK);
      }

   static f_count(dp)
      fb_database *dp;

      {
         expr(fn);
         if (!testnull(fn))
            vcount++;
         return(FB_AOK);
      }

   static f_countall(dp)
      fb_database *dp;

      {
         vcount++;
         return(FB_AOK);
      }

   static f_max(dp)
      fb_database *dp;

      {
         float expr(), v = 0;
         char *cv = NIL, *s_expr();

         if (comp_type == FB_NUMERIC_TEST)
            v = expr(fn);
         else
            cv = s_expr(fn);
         if (!testnull(fn)){
            if (comp_type == FB_NUMERIC_TEST)
               val = MAX(val, v);
            else{
               if (comp_type == DATE_TEST)
                  fb_endate(cv);
               if (strcmp(cv, s_val) > 0)
                  fb_mkstr(&(s_val), cv);
               }
            }
         return(FB_AOK);
      }

   static f_min(dp)
      fb_database *dp;

      {
         float expr(), v = 0;
         char *cv = NIL, *s_expr();

         if (comp_type == FB_NUMERIC_TEST)
            v = expr(fn);
         else
            cv = s_expr(fn);
         if (!testnull(fn)){
            if (comp_type == FB_NUMERIC_TEST)
               val = MIN(val, v);
            else {
               if (comp_type == DATE_TEST)
                  fb_endate(cv);
               if (strcmp(cv, s_val) < 0)
                  fb_mkstr(&(s_val), cv);
               }
            }
         return(FB_AOK);
      }
