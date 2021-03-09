/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: u_lib.c,v 9.1 2001/01/16 02:46:47 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char U_lib_sid[] = "@(#) $Id: u_lib.c,v 9.1 2001/01/16 02:46:47 john Exp $";
#endif

#include "dbsql_e.h"

/*
 * u_lib - the utility library
 */

static u_typenode();
static u_expand();

/*
 * u_opentables - open the tables in the list
 */

   u_opentables(t)
      node *t;

      {
         int errors = 0;
         fb_database *dp, *fb_dballoc();
         cell *c, *sinstall();
         char dname[FB_MAXNAME], bname[FB_MAXNAME];

         if (t == NULL){
            fb_serror(FB_MESSAGE, "Empty database/table list", NIL);
            errors++;
            }
         int_open = t;
         for (; t != NULL; t = t->n_list){
            c = (cell *) t->n_obj;
            if (t->n_p1 == NULL){
               dp = fb_dballoc();
               if (c->c_sval[0] == '~'){
                  strcpy(dname, user_home);
                  strcat(dname, c->c_sval + 1);
                  }
               else
                  strcpy(dname, c->c_sval);
               fb_dbargs(dname, NIL, dp);
               if (fb_opendb(dp, READWRITE, FB_ALLINDEX, FB_OPTIONAL_INDEX) == FB_AOK){
                  t->n_p1 = (int) dp;
                  if (strrchr(dname, '/') != 0){
                     fb_basename(bname, dname);
                     strcpy(dname, bname);
                     }
                  }
               else{
                  fb_serror(FB_MESSAGE, "Can't open database:", dname);
                  errors++;
                  }
               c = sinstall(dname);
               t->n_obj = (int) c;
               }
            }
         if (errors)
            return(FB_ERROR);
         return(FB_AOK);
      }

/*
 * u_closetables - close the tables in the list
 */

   u_closetables(t)
      node *t;

      {
         fb_database *dp;

         for (; t != NULL; t = t->n_list){
            if (t->n_p1 != NULL){
               dp = (fb_database *) t->n_p1;
               fb_closedb(dp);
               }
            }
         int_open = NULL;
      }

/*
 * u_verify_select - verify the list of selections using table list t
 */

   u_verify_select(sel, t)
      node *sel, *t;

      {
         int errors = 0;

         for (; sel != NULL; sel = sel->n_list){
            if (sel->n_type == Q_STAR)		/* convert * to f1,f2,f3 */
               if (u_expand(sel, t) != FB_AOK)
                  errors++;
            if (u_verify_sub(sel, t) != FB_AOK)
               errors++;
            }
         if (errors)
            return(FB_ERROR);
         return(FB_AOK);
      }

/*
 * u_verify_tables - verify the list of tables has been listed in t
 */

   u_verify_tables(tab, t)
      node *tab, *t;

      {
         cell *cc, *pc;
         node *pt;
         int errors = 0;
         char *db_name;

         for (; tab != NULL; tab = tab->n_list){
            cc = (cell *) tab->n_obj;
            db_name = cc->c_sval;
            for (pt = t; pt != NULL; pt = pt->n_list){
               pc = (cell *) pt->n_obj;
               if (equal(db_name, pc->c_sval))
                  break;
               }
            if (pt == NULL){
               fb_serror(FB_MESSAGE, "Query database not specified:", db_name);
               errors++;
               continue;
               }
            }
         if (errors)
            return(FB_ERROR);
         return(FB_AOK);
      }

/*
 * u_verify_cform - decend a cform tree, calling u_verify_sub
 */

   u_verify_cform(cform, t)
      node *cform, *t;

      {
         int errors = 0;
         node *n;

         /* follow the cannonical form, right most AND nodes */
         for (n = cform->n_narg[1]; n != NULL; n = n->n_narg[1]){
            /* and verify every sub tree there */
            if (u_verify_sub(n->n_narg[0], t) != FB_AOK)
               errors++;
            }
         if (errors)
            return(FB_ERROR);
         return(FB_AOK);
      }

/*
 * u_position - for each list of dbase elements in cform tree n,
 *	scan the t list to determine this nodes relative position.
 *	this number is used into a relation array later.
 */
 
   u_position_cform(n, t)
      node *n, *t;

      {
         node *pt, *q;
         int pos;
         cell *qc, *pc;
         char *db_name;

         for (; n != NULL; n = n->n_narg[1])
            for (q = n->n_list; q != NULL; q = q->n_list){
               qc = (cell *) q->n_obj;
               db_name = qc->c_sval;
               for (pos = 0, pt = t; pt != NULL; pt = pt->n_list, pos++){
                  pc = (cell *) pt->n_obj;
                  if (equal(db_name, pc->c_sval))
                     break;
                  }
               q->n_id = pos;
               }
      }

/*
 * u_verify_sub - recursive part of the verify sub tree routines.
 *      RESOLVE:
 *	column ids
 *	floating constants
 *	integer constants
 *	add cells to all nodes for execution (and headers)
 */

   u_verify_sub(n, t)
      node *n, *t;

      {
         cell *cc, *pc = NULL, *dc, *makecell();
         node *dn, *pt, *nn, *ng;
         int errors = 0, i;
         char *fld_name, *db_name, errbuf[FB_MAXLINE];
         fb_database *dp;
         fb_field *f, *fb_findfield();

         if (n == NULL)
            return(FB_AOK);
         /* if its a sub query, ignore the rest of this tree */
         if (n->n_type == S_SUBQ)
            return(FB_AOK);
         cc = (cell *) n->n_obj;
         if (cc == NULL){
            cc = makecell();
            n->n_obj = (int) cc;
            }
         if (n->n_label != NULL)
            u_typenode(n->n_label);
         if (n->n_type == V_ID){
            cc = (cell *) n->n_obj;
            fld_name = cc->c_sval;
            dn = n->n_narg[0];
            db_name = NULL;
            if (dn != NULL){
               dc = (cell *) dn->n_obj;
               db_name = dc->c_sval;
               }
            for (pt = t; pt != NULL; pt = pt->n_list){
               pc = (cell *) pt->n_obj;
               if (db_name == NULL || equal(db_name, pc->c_sval))
                  break;
               }

            /* now pt has the pointer to the node with the dbase symbol */
            if (pt == NULL){
               fb_serror(FB_MESSAGE, "No matching database for field:", fld_name);
               return(FB_ERROR);
               }

            dp = (fb_database *) pt->n_p1;
            if (dp == NULL){
               fb_serror(FB_MESSAGE, "Null database for field:", fld_name);
               return(FB_ERROR);
               }

            f = fb_findfield(fld_name, dp);
            if (f == NULL){
               sprintf(errbuf, "Can't find field `%s' in database `%s'",
                  fld_name, pc->c_sval);
               fb_serror(FB_MESSAGE, errbuf, NIL);
               return(FB_ERROR);
               }

            n->n_p1 = (int) dp;		/* store the database pointer */
            n->n_p2 = (int) f;		/* store the field pointer */
            n->n_tval |= T_FLD;
            if (FB_OFNUMERIC(f->type))
               n->n_tval |= T_NUM;
            else
               n->n_tval |= T_STR;
            if (f->type ==FB_DATE)
               n->n_tval |= T_DTE;
            fb_mkstr(&(n->n_nval), f->id);/* store id for the label */
            n->n_height = -1;		/* -1 flag means full database */
            if (n->n_narg[0] == NULL)
               n->n_narg[0] = pt;
            return(FB_AOK);
            }
         for (i = 0; i < NARGS; i++)
            if (n->n_narg[i] != NULL)
               if (u_verify_sub(n->n_narg[i], t) != FB_AOK)
                  errors++;
         u_typenode(n);
         for (nn = n->n_list; nn != NULL; nn = nn->n_list){
            u_typenode(nn);
            for (ng = nn->n_group; ng != NULL; ng = ng->n_group)
               u_typenode(ng);
            }
         if (errors)
            return(FB_ERROR);
         return(FB_AOK);
      }

/*
 * u_typenode - label the node according to its node type
 */

   static u_typenode(n)
      node *n;

      {
         char *p;
         cell *c;
         int lflag;

         lflag = 1;	/* set the labels -- was an argument, now forced */
         c = (cell *) n->n_obj;
         switch(n->n_type){
            case V_SCON:
            case V_CCON:
               n->n_tval |= T_STR;
               fb_mkstr(&(n->n_nval), c->c_sval + 1);	/* past quote mark */
               for (p = n->n_nval; *p && *(p+1); p++)
                  ;
               *p = NULL;				/* del other quote */
               break;
            case O_ADD:
               n->n_tval |= T_BOP;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "+");
               break;
            case O_SUB:
               n->n_tval |= T_BOP;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "-");
               break;
            case O_MUL:
               n->n_tval |= T_BOP;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "*");
               break;
            case O_DIV:
               n->n_tval |= T_BOP;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "/");
               break;
            case O_CONCAT:
               n->n_tval |= T_BOP;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "|");
               break;
            case V_FCON:
               n->n_scale = 6;			/* 6 is precision */
               /* fall through to V_CON here */
            case V_CON:
               n->n_tval |= T_CON;
               n->n_tval |= T_NUM;
               n->n_fval = (float) atof(c->c_sval);
               if (lflag)
                  fb_mkstr(&(n->n_nval), c->c_sval);
               break;
            case F_AVG:
               n->n_tval |= T_FCN;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "AVG");
               break;
            case F_MAX:
               n->n_tval |= T_FCN;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "MAX");
               break;
            case F_MIN:
               n->n_tval |= T_FCN;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "MIN");
               break;
            case F_SUM:
               n->n_tval |= T_FCN;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "SUM");
               break;
            case F_COUNT:
               n->n_tval |= T_FCN;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "COUNT");
               break;
            case F_COUNTALL:
               n->n_tval |= T_FCN;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "COUNT(*)");
               break;
            /* no labels kept for the following simple functions */
            case F_POWER:
            case F_ROUND:
            case F_TRUNC:
            case F_ABS:
            case F_LENGTH:
            case F_SUBSTR:
            case F_UPPER:
            case F_LOWER:
            case F_SUBLINE:
            case F_FORMFIELD:
               n->n_tval |= T_SFCN;
               break;
            case F_UID:
               n->n_tval |= T_SFCN;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "UID ");
               break;
            case F_GID:
               n->n_tval |= T_SFCN;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "GID ");
               break;
            case F_OWNER:
               n->n_tval |= T_SFCN;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "OWNER     ");
               break;
            case F_GROUP:
               n->n_tval |= T_SFCN;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "GROUP");
               break;
            case F_MODE:
               n->n_tval |= T_SFCN;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "MODE");
               break;
            case F_SYSDATE:
               n->n_tval |= T_SFCN;
               n->n_tval |= T_DTE;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "system Date");
               break;
            case Q_STAR:
               n->n_tval |= T_FCN;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "*");
               break;
            case Q_ALL:
            case Q_ANY:
            case Q_SOME:
            case Q_DISTINCT:
               n->n_tval |= T_FCN;
               break;
            case Q_USER:
            case O_UPLUS:
               n->n_tval |= T_UOP;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "+");
               break;
            case O_UMINUS:
               n->n_tval |= T_UOP;
               if (lflag)
                  fb_mkstr(&(n->n_nval), "-");
               break;

            /* all the R_ series are T_BOP */
            case R_EQ:
            case R_LT:
            case R_GT:
            case R_LE:
            case R_GE:
            case R_NE:
               n->n_tval |= T_BOP;
               break;

            /* all these O_ series are T_BOP */
            case O_UEXISTS:
            case O_UNOT:
               n->n_tval |= T_UOP;
               break;

            /* all these O_ series are T_BOP */
            case O_OR:
            case O_AND:
            case O_UNION:
               n->n_tval |= T_BOP;
               break;

            /* this P_ series is T_TOP (trinary) */
            case P_BETWEEN:
            case P_NOT_BETWEEN:
               n->n_tval |= T_TOP;
               break;

            /* this P_ series is T_LOP (N-Ary - List Op) */
            case P_IN:
            case P_NOT_IN:
               n->n_tval |= T_LOP;
               break;

            /* this P_ series is T_BOP (Binary Op) */
            case P_LIKE:
            case P_NOT_LIKE:
            case P_IS:
            case P_IS_NOT:
               n->n_tval |= T_BOP;
               break;

            case E_TABLE:
               /* dont think i need anything here - these are in SUB_Q
               *  so fall thru to default, NULL
               */
            default:
               if (lflag)
                  fb_mkstr(&(n->n_nval), "NULL");
               break;
            }
         if (n->n_nval != NULL)
            n->n_width = strlen(n->n_nval);
      }

/*
 * u_expand - expand select node into all fields from first table t
 *	or from matching dbase if argument within sel is there.
 */

   static u_expand(sel, t)
      node *sel, *t;

      {
         node *n, *nn, *sn, *dn, *makenode();
         cell *c, *sinstall();
         fb_database *d;
         fb_field *f;
         int i;
         char dname[FB_MAXNAME];

         sn = sel->n_list;
         c = (cell *) sel->n_obj;
         if (c != NULL){
            /* locate proper database */
            strcpy(dname, c->c_sval);
            for (; t != NULL; t = t->n_list){
               c = (cell *) t->n_obj;
               if (equal(dname, c->c_sval))
                  break;
               }
            }
         else{
            /* default - use first dbase in the list */
            c = (cell *) t->n_obj;
            strcpy(dname, c->c_sval);
            }
         if (t == NULL){
            fb_serror(FB_MESSAGE, "Cannot locate database for * request:",
               dname);
            return(FB_ERROR);
            }
         d = (fb_database *) t->n_p1;
         for (n = sel, i = 0;;){
            f = d->kp[i];
            c = sinstall(f->id);
            n->n_type = V_ID;
            n->n_obj = (int) c;
            /* make database node here too */
            dn = makenode();
            dn->n_type = V_ID;
            c = sinstall(dname);
            dn->n_obj = (int) c;
            n->n_narg[0] = dn;
            i++;
            if (i >= d->nfields)
               break;
            /* get set for next field/node */
            nn = makenode();
            n->n_list = nn;
            n = nn;
            }

         /* patch up original select node list */
         n->n_list = sn;
         return(FB_AOK);
      }

/*
 * u_getrec - basic getrec routine, checks to see if record is already loaded.
 */

   u_getrec(rec, dp)
      long rec;
      fb_database *dp;

      {
         if (dp->rec == rec)
            return(FB_AOK);
         return(fb_getrec(rec, dp));
      }
