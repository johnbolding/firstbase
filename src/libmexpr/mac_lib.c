/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_lib.c,v 9.0 2001/01/09 02:56:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macro_lib_sid[] = "@(#) $Id: mac_lib.c,v 9.0 2001/01/09 02:56:51 john Exp $";
#endif

#include <fb.h>
#include <macro_e.h>

/*
 * macro_lib - the macro library
 */

#if !FB_PROTOTYPES
static m_typenode();
static store_string();
#else /* FB_PROTOTYPES */
static m_typenode(fb_mnode *);
static store_string(fb_mnode *, char *);
#endif /* FB_PROTOTYPES */

/*
 * m_verify_sub - recursive part of the verify sub tree routines.
 *      RESOLVE:
 *	column ids
 *	floating (double) constants
 *	integer constants
 *	add cells to all nodes for execution (and headers)
 */

   m_verify_sub(n)
      fb_mnode *n;

      {
         fb_cell *cc;
         fb_mnode *tn;
         int errors = 0, i;

         (void) Macro_lib_sid;

         if (n == NULL)
            return(FB_AOK);
         for (; n != NULL; n = n->n_next){
            cc = (fb_cell *) n->n_obj;
            if (cc == NULL){
               cc = fb_makecell();
               n->n_obj = (int) cc;
               }

            /*
             * used to be V_ID types were allocated here. now its
             * done on the fly.
             */

            for (i = 0; i < NARGS; i++)
               if (n->n_narg[i] != NULL)
                  if (m_verify_sub(n->n_narg[i]) != FB_AOK)
                     errors++;
            for (tn = n->n_list; tn != NULL; tn = tn->n_list)
               if (m_verify_sub(tn) != FB_AOK)
                  errors++;
            if (n->n_type != V_ID)
               m_typenode(n);
            }
         if (errors)
            return(FB_ERROR);
         return(FB_AOK);
      }

/*
 * m_typenode - label the node according to its node type
 */

   static m_typenode(n)
      fb_mnode *n;

      {
         fb_cell *c;
         int on;

         c = (fb_cell *) n->n_obj;
         n->n_tval = 0;
         switch(n->n_type){
            case V_SCON:
            case V_CCON:
               n->n_tval |= T_STR;
               store_string(n, c->c_sval);
               break;
            case V_FCON:
               n->n_scale = 6;			/* 6 is precision */
               /* fall through to V_CON here */
            case V_CON:
               n->n_tval |= T_CON;
               n->n_tval |= T_NUM;
               n->n_fval = (double) atof(c->c_sval);
               break;
            case V_OCON:
               n->n_tval |= T_CON;
               n->n_tval |= T_NUM;
               sscanf(c->c_sval, "%o", &on);
               n->n_fval = (double) on;
               break;
/*
            case F_SYSDATE:
               n->n_tval |= T_SFCN;
               n->n_tval |= T_DTE;
               break;
*/
            case O_CALL:
               n->n_tval |= T_FCN;
               break;
            case V_ARRAY:
               n->n_tval |= T_ARR;
               break;

            /* O_UFIELDs have a very late bindings - all we know is type fld */
            case O_UFIELD:
               n->n_tval |= T_FLD;
               n->n_tval |= T_UOP;
               break;

            /* all these O_ series are T_UOP */
            case O_UPLUS:
            case O_UMINUS:
            case O_UOR:
            case O_UNOT:
               n->n_tval |= T_UOP;
               break;

            /* all of these are binary ops - T_BOP */

            case R_EQ:
            case R_LT:
            case R_GT:
            case R_LE:
            case R_GE:
            case R_NE:

            case O_OR:
            case O_AND:
            case O_XOR:
            case O_XAND:
            case O_IOR:

            case O_ADD:
            case O_SUB:
            case O_MUL:
            case O_DIV:
            case O_MOD:
            case O_LSHFT:
            case O_RSHFT:

            case O_CONCAT:
               n->n_tval |= T_BOP;
               break;

            /* the assignment series - T_ASGN */
            case O_ASSIGN:
            case O_ADD_A:
            case O_LSHFT_A:
            case O_RSHFT_A:
            case O_XOR_A:
            case O_AND_A:
            case O_OR_A:
            case O_MINUS_A:
            case O_MULT_A:
            case O_DIV_A:
            case O_MOD_A:
            case O_INCR_A:
            case O_DECR_A:
            case O_INCR_B:
            case O_DECR_B:
               n->n_tval |= T_ASGN;
               break;

            case R_END:
               n->n_tval |= T_CON; n->n_tval |= T_NUM;
               n->n_fval = FB_END;
               break;
            case R_ERROR:
               n->n_tval |= T_CON; n->n_tval |= T_NUM;
               n->n_fval = FB_ERROR;
               break;
            case R_ABORT:
               n->n_tval |= T_CON; n->n_tval |= T_NUM;
               n->n_fval = FB_ABORT;
               break;
            case R_NEXT:
               n->n_tval |= T_CON; n->n_tval |= T_NUM;
               n->n_fval = FB_ESIGNAL;
               break;
            case R_PREV:
               n->n_tval |= T_CON; n->n_tval |= T_NUM;
               n->n_fval = FB_YSIGNAL;
               break;

            default:
               break;
            }
         if (n->n_nval != NULL)
            n->n_width = strlen(n->n_nval);
      }

   static store_string(n, s)
      fb_mnode *n;
      char *s;

      {
         char buf[FB_MAXLINE], *p, *q;
         long o;

         p = s + 1;				/* past quote mark */
         buf[0] = NULL;
         q = buf;
         for (; *p && *(p+1); ){
            /* provide for standard UNIX conversion of escaped characters */
            if (*p == '\\'){
               switch(*++p){
                  case 'n':
                     *q++ = '\n';
                     break;
                  case 't':
                     *q++ = '\t';
                     break;
                  case '0':
                     o = strtol(p, &p, 8);
                     *q++ = o;
                     continue;
                     break;
                  default:
                     *q++ = *p;
                     break;
                  }
               p++;
               continue;
               }
            *q++ = *p++;
            }
         *q = NULL;				/* no copy other quote */
         fb_mkstr(&(n->n_nval), buf);
      }

/*
 * locate_section - locate section s from the fb_mnode *e_winner.
 */

   fb_mnode *locate_section(s)
      int s;

      {
         fb_mnode *n;

         for (n = e_winner; n != NULL; n = n->n_next)
            if (n->n_type == s)
               return((fb_mnode *) n->n_narg[0]);
         return((fb_mnode *) NULL);
      }

/*
 * locate_function - locate function s from the fb_mnode *e_winner.
 */

   fb_mnode *locate_function(func)
      char *func;

      {
         fb_mnode *n, *cn = NULL, *vn = NULL;
         fb_cell *cc = NULL;

         for (n = e_winner; n != NULL; n = n->n_next){
            if (n->n_type == S_FUNCTION){
               cn = n->n_narg[0];	/* call node */
               if (cn != NULL)
                  vn = cn->n_narg[0];	/* vid node within call node */
               if (vn != NULL)
                  cc = (fb_cell *) vn->n_obj; /* call cell */
               if (cc != NULL && equal(func, cc->c_sval))
                  return((fb_mnode *) n);
               }
            }
         return((fb_mnode *) NULL);
      }

   void m_destroy_call_lists(n)
      fb_mnode *n;

      {
         /* fb_mnode *tn; */
         int i;

         if (n == NULL)
            return;
         for (; n != NULL; n = n->n_next){
            for (i = 0; i < NARGS; i++)
               if (n->n_narg[i] != NULL)
                  m_destroy_call_lists(n->n_narg[i]);
            /*
             * for (tn = n->n_list; tn != NULL; tn = tn->n_list)
             *   m_destroy_call_lists(tn);
             */

            if (n->n_type == O_CALL)
               destroy_list(n);
            }
      }
