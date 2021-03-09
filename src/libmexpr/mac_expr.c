/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_expr.c,v 9.1 2001/01/16 02:46:53 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macro_expr_sid[] = "@(#) $Id: mac_expr.c,v 9.1 2001/01/16 02:46:53 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>

extern short int cdb_write_it;
extern short int cdb_datestyle;
extern short cdb_datedisplay;

#if !FB_PROTOTYPES
static do_op();
static testvalues();
static teststrings();
static assign();
static void assign_vid();
static assign_field();
static void assign_field_via();
static sub_assign_field();
static fb_cell *cell_to_cell();
#else /* FB_PROTOTYPES */
static do_op(int, fb_mnode *, fb_mnode *, fb_mnode *);
static testvalues(int, double, double);
static teststrings(int, char *, char *);
static assign(fb_mnode *, fb_mnode *, int);
static void assign_vid(fb_mnode *, fb_mnode *, int);
static assign_field(fb_mnode *, fb_mnode *, int);
static void assign_field_via(fb_mnode *, fb_mnode *, int, fb_database *);
static sub_assign_field(fb_mnode *, fb_mnode *, int, fb_field *,fb_database *);
static fb_cell *cell_to_cell(char *, fb_cell *);
#endif /* FB_PROTOTYPES */

/*
 * tostring - convert a fb_mnode to a string as best as possible.
 *	store result in n->n_pval, and return it too.
 */

   char *tostring(n)
      fb_mnode *n;

      {
         char buf[FB_MAXLINE];

         (void) Macro_expr_sid;

         if (istype_str(n) || istype_fld(n))
            fb_mkstr(&(n->n_pval), n->n_nval);
         else{
            sprintf(buf, "%.*f", n->n_scale, n->n_fval);
            fb_mkstr(&(n->n_pval), buf);
            }
         return(n->n_pval);
      }

/*
 * tonumber - convert a fb_mnode to a number as best as possible.
 *	store result in n->n_fval, and return it too.
 */

   double tonumber(n)
      fb_mnode *n;

      {
         if (istype_str(n))
            n->n_fval = atof(n->n_nval);
         return(n->n_fval);
      }

/*
 * is_null - return 1 if the fb_mnode is NULL, else return 0
 */

   is_null(n)
      fb_mnode *n;

      {
         int rval = 0;

         if (istype_str(n)){
            if (n->n_nval[0] == NULL)
               rval = 1;
            }
         return(rval);
      }

/*
 * macro_expr - take a tree fb_mnode and do its expressions recursively
 */

   double macro_expr(n)
      fb_mnode *n;

      {
         fb_mnode *n1, *n2, *n3, *nn, *vn, *arg;
         fb_field *f;
         char *p;
         fb_cell *bcell, **stab, *cn;
         fb_mnode *basen, *an, *subscript;
         fb_database *db;
         int on;

         n1 = n->n_narg[0];
         n2 = n->n_narg[1];
         n3 = n->n_narg[2];
         if (istype_bop(n)){
            macro_expr(n1);
            if (n->n_type != O_AND && n->n_type != O_OR)
               macro_expr(n2);
            n->n_scale = MAX(n1->n_scale, n2->n_scale);
            if (istype_dol(n1) || istype_dol(n2))
               n->n_tval |= T_DOL;
            }
         else if (istype_uop(n)){
            macro_expr(n1);
            n->n_scale = n1->n_scale;
            if (n->n_type != O_UFIELD){
               n->n_tval = 0;
               n->n_tval |= T_UOP;
               if (istype_num(n1))
                  n->n_tval |= T_NUM;
               if (istype_dol(n1))
                  n->n_tval |= T_DOL;
               if (!istype_num(n) && !istype_dol(n))
                  n->n_tval |= T_NUM;
               }
            }
         else if (istype_top(n)){
            macro_expr(n1);
            macro_expr(n2);
            macro_expr(n3);
            n->n_scale = MAX(MAX(n1->n_scale, n2->n_scale), n3->n_scale);
            }
         else if (istype_lop(n)){
            for (nn = n1; nn != NULL; nn = nn->n_list)
               macro_expr(nn);
            }
         else if (istype_fcn(n))
            return(macro_function(n));
         else if (istype_asgn(n)){
            /* just do the expr on the right side, n1 (narg[0]) is an LVAL */
            if (n2 != NULL)
               macro_expr(n2);
            }
         switch(n->n_type){
            case O_MUL:
               n->n_fval = n1->n_fval * n2->n_fval;
               break;
            case O_DIV:
               if (n2->n_fval != 0)
                  n->n_fval = n1->n_fval / n2->n_fval;
               break;
            case O_ADD:
               if (istype_str(n1) || istype_str(n2)){
                  /* handle string concatenation as an addition */
                  tostring(n1);
                  tostring(n2);
                  p = fb_malloc(strlen(n1->n_pval) + strlen(n2->n_pval) + 2);
                  sprintf(p, "%s%s", n1->n_pval, n2->n_pval);
                  fb_mkstr(&(n->n_nval), p);
                  fb_free(p);
                  n->n_tval |= T_STR;
                  n->n_fval = 0;
                  }
               else
                  n->n_fval = n1->n_fval + n2->n_fval;
               break;
            case O_SUB:
               n->n_fval = n1->n_fval - n2->n_fval;
               break;
            case O_XOR:
               n->n_fval = (double) ((int) n1->n_fval ^ (int) n2->n_fval);
               break;
            case O_XAND:
               n->n_fval = (double) ((int) n1->n_fval & (int) n2->n_fval);
               break;
            case O_IOR:
               if (istype_str(n1) || istype_str(n2)){
                  /* handle string concatenation as an addition */
                  tostring(n1);
                  tostring(n2);
                  p = fb_malloc(strlen(n1->n_pval) + strlen(n2->n_pval) + 2);
                  sprintf(p, "%s%s", n1->n_pval, n2->n_pval);
                  fb_mkstr(&(n->n_nval), p);
                  fb_free(p);
                  n->n_tval |= T_STR;
                  n->n_fval = 0;
                  }
               else
                  n->n_fval = (double) ((int) n1->n_fval | (int) n2->n_fval);
               break;
            case O_LSHFT:
               n->n_fval = (double) ((int) n1->n_fval << (int) n2->n_fval);
               break;
            case O_RSHFT:
               n->n_fval = (double) ((int) n1->n_fval >> (int) n2->n_fval);
               break;
            case O_MOD:
               if (n2->n_fval != 0)
                  n->n_fval = (double) ((int) n1->n_fval % (int) n2->n_fval);
               break;
            case O_UPLUS:
               break;
            case O_UMINUS:
               n->n_fval = -n1->n_fval;
               break;
            case O_UOR:
               n->n_fval = (double) (~ (int) n1->n_fval);
               break;
            case O_UNOT:
               if (is_null(n1))
                  n->n_fval = 0;
               else
                  n->n_fval = !(n1->n_fval);
               break;
            case O_OR:
               /* do short circuit evaluation */
               if (!is_null(n1) && n1->n_fval){	/* n1 not null and is true */
                  n->n_fval = n1->n_fval;
                  break;
                  }
               /* n1 is not a factor. either n1 is false or null. test n2 */
               macro_expr(n2);
               if (!is_null(n2) && n2->n_fval){	/* n2 not null and is true */
                  n->n_fval = n2->n_fval;
                  break;
                  }
               /* else neither side is true */
               n->n_fval = 0;
               break;
            case O_AND:
               /* do short circuit evaluation */
               if (is_null(n1)){
                  n->n_fval = 0;
                  break;
                  }
               if (!n1->n_fval){
                  n->n_fval = 0;
                  break;
                  }
               /* at this point, n1 must be true. decide on n2 */
               macro_expr(n2);
               if (is_null(n2)){
                  n->n_fval = 0;
                  break;
                  }
               n->n_fval = n1->n_fval && n2->n_fval;
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
            case V_SCON:
            case V_CCON:
               if (n->n_nval != NULL)
                  n->n_fval = (double) atof(n->n_nval);
               break;
            case V_OCON:
               if (n->n_nval != NULL){
                  sscanf(n->n_nval, "%o", &on);
                  n->n_fval = (double) on;
                  }
               break;
            case O_ASSIGN:
            case O_ADD_A:
            case O_MINUS_A:
            case O_MULT_A:
            case O_DIV_A:
            case O_LSHFT_A:
            case O_RSHFT_A:
            case O_XOR_A:
            case O_AND_A:
            case O_OR_A:
            case O_MOD_A:
            case O_INCR_A:
            case O_DECR_A:
            case O_INCR_B:
            case O_DECR_B:
               if (n2 != NULL && n2->n_type == O_CALL){
                  /* CALL do assign by OP/type using list - n1 is last one */
                  if (n2->n_list == NULL)
                     assign(n1, n2, n->n_type);
                  else{
                     for (n2 = n2->n_list; n2 != NULL; n2 = n2->n_list){
                        assign(n1, n2, n->n_type);
                        if (n1->n_list == NULL)
                           break;
                        n1 = n1->n_list;
                        }
                     }
                  }
               else{
                  /* NON CALL - do assign by OP/type - leave result in n1 */
                  assign(n1, n2, n->n_type);
                  }
               n->n_fval = n1->n_fval;
               n->n_scale = n1->n_scale;
               if (istype_dol(n1))
                  n->n_tval |= T_DOL;
               if (istype_dte(n1))
                  n->n_tval |= T_DTE;
               if (istype_num(n1))
                  n->n_tval |= T_NUM;
               if (istype_str(n1)){
                  n->n_tval |= T_STR;
                  fb_mkstr(&(n->n_nval), n1->n_nval);
                  }
               /* these are statements. dont change the tval here */
               break;

            case V_ID:
               vn = mnode_to_var(n);
               n->n_fval = vn->n_fval;
               n->n_scale = vn->n_scale;
               n->n_tval = vn->n_tval;
               if (istype_str(vn)){
                  fb_mkstr(&(n->n_nval), vn->n_nval);
                  n->n_tval |= T_STR;
                  }
               break;
            case V_ARRAY:
               /*
                * two cases here:
                * V_ARRAY with T_FLD set is a database array (istype_fld)
                * V_ARRAY without T_FLD is a normal array (istype_arr)
                */

               arg = n->n_narg[0];
               subscript = n->n_narg[1];
               macro_expr(subscript);
               bcell = (fb_cell *) arg->n_obj;
               bcell = string_to_cell(bcell->c_sval);
               basen = bcell->c_vid;  /* same as mnode_to_var(arg) */
               basen->n_tval |= T_ARR;

               if (basen != NULL && istype_fld(basen)){
                  /* evaluate the array ref as a database field */
                  /* use the n_obj inside of the ID to get at the database */
                  db = mnode_to_array_dbase(basen);
                  if (db != (fb_database *) NULL){
                     n->n_tval = 0;
                     n->n_tval |= T_FLD;
                     n->n_tval |= T_UOP;
                     f = mnode_to_field_via(subscript, db);
                     eval_field(f, db, n);
                     }
                  }
               else if (basen != NULL){
                  /* its a user defined array, _not_ a database */
                  /* bcell is the object with the symtab in it */
                  if (bcell->c_symtab == NULL){
                     bcell->c_symtab = fb_makesymtab();
                     }
                  stab = bcell->c_symtab;
                  cn = fb_u_sinstall(tostring(subscript), stab);
                  if (cn->c_vid == NULL){
                     if (istype_glob(basen))
                        cn->c_vid = fb_s_node0(V_ID);
                     else
                        cn->c_vid = fb_node0(V_ID);
                     }
                  /* array mnode - the place to store the value */
                  an = cn->c_vid;
                  n->n_fval = an->n_fval;
                  n->n_tval = an->n_tval;
                  n->n_scale = an->n_scale;
                  if (istype_str(an)){
                     fb_mkstr(&(n->n_nval), an->n_nval);
                     n->n_tval |= T_STR;
                     }
                  }
               break;
            case O_UFIELD:
               /* locate the field referenced by expr n1 */
               n->n_tval = 0;
               n->n_tval |= T_FLD;
               n->n_tval |= T_UOP;
               f = mnode_to_field(n1);
               eval_field(f, cdb_db, n);
               break;
            }
         return(n->n_fval);
      }

/*
 * do_op - do the comparison n1 op n2
 */

   static do_op(op, n, n1, n2)
      int op;
      fb_mnode *n, *n1, *n2;

      {
         int sflag = 0, val = 0;
         char buf[FB_MAXLINE];

         n->n_fval = 0;
         
         /*
          * in sql, is_null is called here, and the 0 in place stays.
          * i think for macros we want nulls compared
          * just like in the real world.
          */

         if (istype_str(n1) || istype_str(n2)){
            sflag = 1;
            tostring(n1);
            tostring(n2);
            }

         /* if either n1 or n2 is a date, convert them both, blindly */
         if (istype_dte(n1) || istype_dte(n2)){
            if ((cdb_datestyle == FB_EUROPEAN) &&
                  !istype_fld(n1) && !istype_fcn(n1) && !istype_sfcn(n1))
               fb_ameri_date(n1->n_pval);
            /* no matter what date display is, use longdates */
            fb_longdate(buf, n1->n_pval);
            fb_mkstr(&(n1->n_pval), buf);
            fb_long_endate(n1->n_pval);

            if ((cdb_datestyle == FB_EUROPEAN) &&
                  !istype_fld(n2) && !istype_fcn(n2) && !istype_sfcn(n2))
               fb_ameri_date(n2->n_pval);
            /* no matter what date display is, use longdates */
            fb_longdate(buf, n2->n_pval);
            fb_mkstr(&(n2->n_pval), buf);
            fb_long_endate(n2->n_pval);
            }

         val = 0;

         /*
          * in sql, is_null is called here, and the element
          * is skipped. i think for macros we want nulls compared
          * just like in the real world.
          */

         if (sflag)
            val = teststrings(op, n1->n_pval, n2->n_pval);
         else
            val = testvalues(op, n1->n_fval, n2->n_fval);

         n->n_fval = val;
      }
/*
 * testvalues - test the values a,b against the op
 */

   static testvalues(op, a, b)
      int op;
      double a, b;

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

/*
 * assign - assign n2 to n1 - using assignment operator op.
 */

   static assign(n1, n2, op)
      fb_mnode *n1, *n2;
      int op;

      {
         fb_mnode *arg, *subscript, *basen;
         fb_cell *bcell;
         fb_database *db;
         fb_field *f;

         /* simple case - just in the sym table */
         if (n1->n_type == V_ARRAY){
            /*
             * two cases here:
             * V_ARRAY with T_FLD set is a database array (istype_fld)
             * V_ARRAY without T_FLD is a normal array (istype_arr)
             */
            arg = n1->n_narg[0];
            subscript = n1->n_narg[1];
            macro_expr(subscript);
            bcell = (fb_cell *) arg->n_obj;
            bcell = string_to_cell(bcell->c_sval);
            basen = bcell->c_vid;  /* same as mnode_to_var(arg) */
            if (istype_fld(basen)){
               db = mnode_to_array_dbase(basen);
               if (db != (fb_database *) NULL){
                  f = mnode_to_field_via(subscript, db);
                  if (f != NULL)
                     sub_assign_field(n1, n2, op, f, db);
                  }
               }
            else
               assign_array(n1, n2, op);
            }
         else if (n1->n_type == V_ID)
            assign_vid(n1, n2, op);
         else
            assign_field(n1, n2, op);
      }

/*
 * assign_vid - assign n2 to n1 - using assignment operator op. V_ID only.
 *	leave results in n1.
 */

   static void assign_vid(n1, n2, op)
      fb_mnode *n1, *n2;
      int op;

      {
         fb_mnode *vn;

         /* simple case - just in the sym table */
         vn = mnode_to_var(n1);
         if (vn == NULL){
            fb_serror(FB_MESSAGE, "Illegal variable", NIL);
            return;
            }
         if (n2 != NULL && op == O_ASSIGN && istype_arr(n2) &&
               n2->n_narg[0] == NULL){
            assign_copy_array(n1, n2, symtab);
            return;
            }
         switch(op){
            case O_ASSIGN:	vn->n_fval  = n2->n_fval; break;
            case O_ADD_A:	vn->n_fval += n2->n_fval; break;
            case O_MINUS_A:	vn->n_fval -= n2->n_fval; break;
            case O_MULT_A:	vn->n_fval *= n2->n_fval; break;
            case O_DIV_A:	if (n2->n_fval != 0)
				   vn->n_fval /= n2->n_fval;
                                break;
            case O_LSHFT_A:
               vn->n_fval = (double) ((int) vn->n_fval << (int) n2->n_fval);
               break;
            case O_RSHFT_A:
               vn->n_fval = (double) ((int) vn->n_fval >> (int) n2->n_fval);
               break;
            case O_XOR_A:
               vn->n_fval = (double) ((int) vn->n_fval ^ (int) n2->n_fval);
               break;
            case O_AND_A:
               vn->n_fval = (double) ((int) vn->n_fval & (int) n2->n_fval);
               break;
            case O_OR_A:
               vn->n_fval = (double) ((int) vn->n_fval | (int) n2->n_fval);
               break;
            case O_MOD_A:
               if (n2->n_fval != 0)
                  vn->n_fval = (double) ((int) vn->n_fval % (int) n2->n_fval);
               break;

            /* the _A series need special linkage to after effect stuff */
            case O_INCR_A:	break;
            case O_DECR_A:	break;

            /* the _B series is FB_AOK */
            case O_INCR_B:	++vn->n_fval; break;
            case O_DECR_B:	--vn->n_fval; break;
            }

         /* if its a string, store this too, and covert all to strings */
         if (n2 != NULL){
            vn->n_tval = 0;
            if (istype_str(n2)){
               tostring(n2);
               fb_mkstr(&(vn->n_nval), n2->n_pval);
               fb_mkstr(&(n1->n_nval), n2->n_pval);
               vn->n_tval |= T_STR;
               /* turn off NUM if set */
               if (istype_num(vn))
                  vn->n_tval &= ~T_NUM;
               }
            else if (istype_num(n2)){
               vn->n_tval |= T_NUM;
               /* turn off STR if set */
               if (istype_str(vn))
                  vn->n_tval &= ~T_STR;
               }
            vn->n_scale = n2->n_scale;
            if (istype_dol(n2))
               vn->n_tval |= T_DOL;
            if (istype_dte(n2))
               vn->n_tval |= T_DTE;
            }
         else { /* n2 is NULL - must be an INCR of some kind */
            /* turn off STR if set */
            if (istype_str(vn))
               vn->n_tval &= ~T_STR;
            }

         /* store result in n1 --- the actual lval node */
         n1->n_fval = vn->n_fval;
         n1->n_tval = vn->n_tval;
         n1->n_scale = vn->n_scale;

         /* now patch up any _A series that were done */
         if (op == O_INCR_A)
            vn->n_fval++;
         else if (op == O_DECR_A)
            vn->n_fval--;
      }

/*
 * assign_field - assign n2 to n1 - using assignment operator op. field only.
 *	leave results in n1.
 */

   static assign_field(n1, n2, op)
      fb_mnode *n1, *n2;
      int op;

      {
         if (cdb_db != NULL){
            assign_field_via(n1, n2, op, cdb_db);
            cdb_write_it = 1;
            return(FB_AOK);
            }
         else
            return(FB_ERROR);
      }

/*
 * assign_field - assign n2 to n1 - using assignment operator op. field only.
 *	leave results in n1.
 */

   static void assign_field_via(n1, n2, op, db)
      fb_mnode *n1, *n2;
      int op;
      fb_database *db;

      {
         fb_cell *cn;
         fb_mnode *fn;
         fb_field *f;
         int i;
         char fid[FB_MAXNAME];

         /* locate the fb_field referenced by expr below O_UFIELD */
         f = NULL;
         fn = n1->n_narg[0];
         macro_expr(fn);

         /* reset type to empty */
         if (istype_str(fn)){
            /* locate the field as a string */
            strcpy(fid, fn->n_nval);
            fb_underscore(fid, 0);
            if (db != (fb_database *) NULL)
               f = fb_findfield(fid, db);
            }
         else if (istype_num(fn)){
            i = (int) fn->n_fval - 1;
            if (db != (fb_database *) NULL && i >= 0 && i < db->nfields)
               f = db->kp[i];
            }
         else if (fn->n_type == V_ID){
            cn = (fb_cell *) fn->n_obj;
            if (cn != NULL){
               strcpy(fid, cn->c_sval);
               fb_underscore(fid, 0);
               if (db != (fb_database *) NULL)
                  f = fb_findfield(fid, db);
               }
            }
         else{
            /* locate the field as a number */
            i = (int) fn->n_fval - 1;
            if (db != (fb_database *) NULL && i >= 0 && i < db->nfields)
               f = db->kp[i];
            }
         if (f == NULL)
            return;
         if (f->type == FB_FORMULA || f->dflink != NULL || f->type==FB_BINARY)
            return;
         sub_assign_field(n1, n2, op, f, db);
      }

/*
 * sub_assign - the shared part of the assign field command
 *	shared by normal field assign and by dbase array field assigns.
 */

   static sub_assign_field(n1, n2, op, f, db)
      fb_field *f;
      fb_mnode *n1, *n2;
      int op;
      fb_database *db;

      {
         fb_mnode *vn, *sn;
         char *str;
         int len;

         /* now set vn to n1 so code is same as prev, and load a val into vn */
         vn = n1;
         vn->n_tval = 0;

         /*
          * precision here is based on the field, not on object
          * except for type float, which can do any of these.
          */
         if (FB_OFNUMERIC(f->type)){
            vn->n_tval |= T_NUM;
            if (f->type == FB_FLOAT){
               if (f->f_prec > 0)
                  vn->n_scale = f->f_prec;
               else if (n2 != NULL && n2->n_scale >= 0)
                  vn->n_scale = n2->n_scale;
               else
                  vn->n_scale = 6;
               }
            }
         else
            vn->n_tval |= T_STR;
         if (f->type == FB_DATE)
            vn->n_tval |= T_DTE;
         vn->n_fval = (double) atof(f->fld);
         str = f->fld;
         if (f->type == FB_DOLLARS){
            vn->n_fval /= 100.0;
            vn->n_scale = 2;
            }
         if (istype_str(vn))
            fb_mkstr(&(vn->n_nval), str);

         switch(op){
            case O_ASSIGN:	vn->n_fval  = n2->n_fval; break;
            case O_ADD_A:	vn->n_fval += n2->n_fval; break;
            case O_MINUS_A:	vn->n_fval -= n2->n_fval; break;
            case O_MULT_A:	vn->n_fval *= n2->n_fval; break;
            case O_DIV_A:	if (n2->n_fval != 0)
				   vn->n_fval /= n2->n_fval;
                                break;
            case O_LSHFT_A:
               vn->n_fval = (double) ((int) vn->n_fval << (int) n2->n_fval);
               break;
            case O_RSHFT_A:
               vn->n_fval = (double) ((int) vn->n_fval >> (int) n2->n_fval);
               break;
            case O_XOR_A:
               vn->n_fval = (double) ((int) vn->n_fval ^ (int) n2->n_fval);
               break;
            case O_AND_A:
               vn->n_fval = (double) ((int) vn->n_fval & (int) n2->n_fval);
               break;
            case O_OR_A:
               vn->n_fval = (double) ((int) vn->n_fval | (int) n2->n_fval);
               break;
            case O_MOD_A:
               if (n2->n_fval != 0)
                  vn->n_fval = (double) ((int) vn->n_fval % (int) n2->n_fval);
               break;

            /* the _A series need special linkage to after effect stuff */
            case O_INCR_A:	break;
            case O_DECR_A:	break;

            /* the _B series is FB_AOK */
            case O_INCR_B:	++vn->n_fval; break;
            case O_DECR_B:	--vn->n_fval; break;
            }

         /* if its a string, store this too, and covert all to strings */
         if (n2 != NULL && (istype_str(n2) || istype_str(vn))){
            tostring(n2);
            fb_mkstr(&(vn->n_nval), n2->n_pval);
            vn->n_tval |= T_STR;
            }
         /*
          * vn == n1 -- so no storage necessary
          *    n1->n_fval = vn->n_fval;
          */

         if (op == O_INCR_A || op == O_DECR_A){
            /* this node is freed at the bottom of this routine */
            sn = fb_s_makenode();
            fb_copynode(sn, vn);
            if (op == O_INCR_A)
               sn->n_fval++;
            else if (op == O_DECR_A)
               sn->n_fval--;
            }
         else
            sn = vn;

         /* store the string back into the fb_database fb_field */
         if (f->type == FB_DOLLARS)
            sn->n_scale = 2;
         tostring(sn);
         fb_trim(sn->n_pval);
         if (f->type == FB_DOLLARS)
            fb_nodecimal(sn->n_pval);
         if (strlen(sn->n_pval) > f->size)
            sn->n_pval[f->size] = NULL;
         len = strlen(sn->n_pval);
         if (f->type != FB_DATE || (len <= 6))
            fb_store(f, sn->n_pval, db);
         if (op == O_INCR_A || op == O_DECR_A)
            fb_freenode(sn);
      }

   fb_field *mnode_to_field(n)
      fb_mnode *n;

      {
         if (cdb_db != (fb_database *) NULL)
            return(mnode_to_field_via(n, cdb_db));
         else
            return((fb_field *) NULL);
      }

   fb_field *mnode_to_field_via(n, db)
      fb_mnode *n;
      fb_database *db;

      {
         fb_field *f;
         int i;
         fb_cell *cn;
         char fid[FB_MAXNAME];

         f = (fb_field *) NULL;
         if (istype_str(n)){
            strcpy(fid, n->n_nval);
            fb_underscore(fid, 0);
            if (db != (fb_database *) NULL)
               f = fb_findfield(fid, db);
            /* locate the fb_field as a string */
            }
         else if (istype_num(n)){
            i = (int) n->n_fval - 1;
            if (db != (fb_database *) NULL && i >= 0 && i < db->nfields)
               f = db->kp[i];
            }
         else if (n->n_type == V_ID){
            cn = (fb_cell *) n->n_obj;
            if (cn != NULL){
               strcpy(fid, cn->c_sval);
               fb_underscore(fid, 0);
               if (db != (fb_database *) NULL)
                  f = fb_findfield(fid, db);
               }
            }
         else{
            /* locate the fb_field as a number */
            i = (int) n->n_fval - 1;
            if (db != (fb_database *) NULL && i >= 0 && i < db->nfields)
               f = db->kp[i];
            }
         return(f);
      }

   fb_mnode *mnode_to_var(n)
      fb_mnode *n;

      {
         fb_cell *cn;
         fb_mnode *vn;

         cn = (fb_cell *) n->n_obj;
         vn = string_to_var(cn->c_sval);
         return(vn);
      }

   fb_mnode *mnode_to_local_var(n)
      fb_mnode *n;

      {
         fb_cell *cn;
         fb_mnode *vn;

         cn = (fb_cell *) n->n_obj;
         vn = string_to_local_var(cn->c_sval);
         return(vn);
      }

   fb_mnode *string_to_var(s)
      char *s;

      {
         fb_cell *c;

         c = string_to_cell(s);
         return((fb_mnode *) c->c_vid);
      }

   fb_mnode *string_to_local_var(s)
      char *s;

      {
         fb_cell *c;

         c = string_to_local_cell(s);
         return((fb_mnode *) c->c_vid);
      }

/*
 * string_to_cell - lookup string, provide cell - looks in current symtab
 */

   fb_cell *string_to_cell(s)
      char *s;

      {
         fb_cell *c;

         c = fb_lookup(s);
         return((fb_cell *) cell_to_cell(s, c));
      }

/*
 * string_to_cell - lookup string, provide cell - looks in passed in stab
 */

   fb_cell *string_to_cell_via(s, stab)
      char *s;
      fb_cell **stab;

      {
         fb_cell *c;

         c = fb_u_lookup(s, stab);
         return((fb_cell *) cell_to_cell(s, c));
      }

/*
 * cell_to_cell - lookup string, provide cell - looks in current symtab
 */

   static fb_cell *cell_to_cell(s, c)
      char *s;
      fb_cell *c;

      {
         int local;

         local = 1;
         if (c == (fb_cell *) NULL){
            local = 0;
            c = fb_ginstall(s);
            }
         if (c->c_vid == NULL){
            if (local)
               c->c_vid = fb_node0(V_ID);
            else{
               c->c_vid = fb_s_node0(V_ID);
               c->c_vid->n_tval |= T_GLOB;
               }
            }
         return((fb_cell *) c);
      }

   fb_cell *string_to_local_cell(s)
      char *s;

      {
         fb_cell *c;

         c = fb_sinstall(s);
         if (c->c_vid == NULL)
            c->c_vid = fb_node0(V_ID);
         return((fb_cell *) c);
      }

/*
 * eval_field - called by macro_exp during field evaluation
 */

   void eval_field(f, db, n)
      fb_field *f;
      fb_database *db;
      fb_mnode *n;

      {
         char *p, *str;

         /* null it out if there is a field */
         if (f == NULL){
            fb_mkstr(&(n->n_nval), NIL);
            n->n_tval |= T_STR;
            return;
            }
         if (FB_OFNUMERIC(f->type)){
            n->n_tval |= T_NUM;
            if (f->type == FB_FLOAT)
               n->n_scale = 6;
            }
         else
            n->n_tval |= T_STR;
         if (f->type == FB_DATE)
            n->n_tval |= T_DTE;
         n->n_fval = (double) atof(f->fld);
         str = f->fld;
         if (f->type == FB_DOLLARS){
            n->n_fval /= 100.0;
            n->n_scale = 2;
            n->n_tval |= T_DOL;
            }
         else if (f->type == FB_FORMULA){
            fb_fetch(f, db->afld, db);
            str = db->afld;
            n->n_fval = (double) atof(db->afld);
            n->n_scale = 6;
            if (f->idefault != NULL)
               if ((p = strrchr(f->idefault, CHAR_COLON)) != 0)
                  n->n_scale = atoi(p+1);
            }
         fb_mkstr(&(n->n_nval), str);
         if (istype_str(n)){
            n->n_tval |= T_STR;
            }
      }
