/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: sfunc.c,v 9.1 2001/01/16 02:46:46 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Sfunction_sid[] = "@(#) $Id: sfunc.c,v 9.1 2001/01/16 02:46:46 john Exp $";
#endif

#include "dbsql_e.h"
#include <math.h>
#include <dbpwd.h>

static char *RMODE = "%c%04d%03d%s";

extern short int cdb_secure;

/*
 * sfunction - handle the extra, simple functions on a single record/relation
 */

   float sfunction(n)
      node *n;

      {
         node *n0, *n1, *n2;
         float val, v0, v1, v2, expr(), len;
         char *p, *q, type;
         char buf[FB_MAXLINE], buf2[FB_MAXLINE], m[6], c[2];
         double fabs();
         int u, g;
         fb_passwd *pwd;
         fb_database *dp;

         n->n_fval = val = 0;
         n0 = n->n_narg[0];
         n1 = n->n_narg[1];
         n2 = n->n_narg[2];
         n->n_scale = 6;
         if (n0 != NULL)
            n->n_height = n0->n_height;
         switch(n->n_type){
            case F_POWER:
               v0 = expr(n0);
               v1 = expr(n1);
               val = (float) pow(v0, v1);
               break;
            case F_ROUND:
               v0 = expr(n0);
               v1 = expr(n1);
               val = v0;
               n->n_scale = v1;
               break;
            case F_TRUNC:
               v0 = expr(n0);
               v1 = expr(n1);
               if (v1 < 0)
                  v1 = 0;
               sprintf(buf, "%.11f", v0);
               p = strchr(buf, '.');
               p += ((int) v1 + 1);
               *p = NULL;
               val = (float) atof(buf);
               n->n_scale = v1;
               break;
            case F_ABS:
               v0 = expr(n0);
               val = (float) fabs((double) v0);
               n->n_scale = 0;
               break;
            case F_LENGTH:
               s_expr(n0);
               val = (float) strlen(n0->n_pval);
               n->n_scale = 0;
               break;
            case F_SUBSTR:
               s_expr(n0);
               v1 = expr(n1);
               v2 = (float) fabs((double) expr(n2));
               len = strlen(n0->n_pval);
               if (fabs((double) v1) > (double) len)
                  v1 = 0;
               p = n0->n_pval;
               if (len > 0){
                  if (v1 >= 0)
                     p = p + (int) v1 - 1;
                  else
                     p = p + strlen(n0->n_pval) + (int) v1;
                  for (len = v2, q = p; *q && len > 0; q++, len--)
                     ;
                  if (*q != NULL)
                     *q = NULL;
                  }
               fb_mkstr(&(n->n_nval), p);
               n->n_tval |= T_STR;
               break;
            case F_UPPER:
               s_expr(n0);
               for (p = n0->n_pval; *p; p++)
                  if (islower(*p))
                     *p = toupper(*p);
               fb_mkstr(&(n->n_nval), n0->n_pval);
               n->n_tval |= T_STR;
               break;
            case F_LOWER:
               s_expr(n0);
               for (p = n0->n_pval; *p; p++)
                  if (isupper(*p))
                     *p = tolower(*p);
               fb_mkstr(&(n->n_nval), n0->n_pval);
               n->n_tval |= T_STR;
               break;
            case F_SUBLINE:
               s_expr(n0);
               v1 = (float) fabs(expr(n1));
               fb_subline(buf, n0->n_pval, (int) v1, FB_NEWLINE);
               fb_mkstr(&(n->n_nval), buf);
               n->n_tval |= T_STR;
               break;
            case F_FORMFIELD:
               s_expr(n0);
               strcpy(buf2, n0->n_pval);
               s_expr(n1);
               type = *(n1->n_pval);
               v2 = expr(n2);
               fb_formfield(buf, buf2, type, (int) n->n_width);
               fb_mkstr(&(n->n_nval), buf);
               n->n_tval |= T_STR;
               break;
            case F_SYSDATE:
               fb_simpledate(buf, 1);
               fb_mkstr(&(n->n_nval), buf);
               n->n_tval |= T_STR;
               n->n_height = 0;
               break;
            case F_OWNER:
               if (cdb_secure){
                  dp = (fb_database *) g_from->n_p1;
                  fb_fetch(dp->kp[dp->nfields], buf, dp);
                  sscanf(buf, RMODE, c, &u, &g, m);
                  pwd = fb_getpwuid(u);
                  fb_mkstr(&(n->n_nval), pwd->dbpw_name);
                  }
               else
                  fb_mkstr(&(n->n_nval), NIL);
               n->n_tval |= T_STR;
               break;
            case F_UID:
               if (cdb_secure){
                  dp = (fb_database *) g_from->n_p1;
                  fb_fetch(dp->kp[dp->nfields], buf, dp);
                  sscanf(buf, RMODE, c, &u, &g, m);
                  }
               else
                  u = -1;
               val = u;
               n->n_scale = 0;
               break;
            case F_GROUP:
            case F_GID:
               if (cdb_secure){
                  dp = (fb_database *) g_from->n_p1;
                  fb_fetch(dp->kp[dp->nfields], buf, dp);
                  sscanf(buf, RMODE, c, &u, &g, m);
                  }
               else
                  g = -1;
               val = g;
               n->n_scale = 0;
               break;
            case F_MODE:
               if (cdb_secure){
                  dp = (fb_database *) g_from->n_p1;
                  fb_fetch(dp->kp[dp->nfields], buf, dp);
                  sscanf(buf, RMODE, c, &u, &g, m);
                  fb_mkstr(&(n->n_nval), m);
                  }
               else
                  fb_mkstr(&(n->n_nval), NIL);
               n->n_tval |= T_STR;
               break;
            }
         n->n_fval = val;
         return(val);
      }
