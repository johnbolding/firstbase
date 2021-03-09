/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: yyerror.c,v 9.0 2001/01/09 02:55:53 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Yyerror_sid[] = "@(#) $Id: yyerror.c,v 9.0 2001/01/09 02:55:53 john Exp $";
#endif

#include "dbsql_e.h"

#undef NULL
#undef END
#undef FB_FLOAT
#undef FB_INTEGER 
#undef INTO
#undef MAX
#undef MIN
#undef FB_NUMERIC

#include "dbsql_df.h"

extern int yychar, yylval;
static make_et();

/*
* yerror - error routine called by yyparse() on grammer errors
*/
   yyerror()
   
      {
         char et[FB_MAXLINE];
         cell *c;

         switch(yychar){
            case CCON:
            case CON:
            case FCON:
            case ID:
            case SCON:
               c = (cell *) yylval;
               if (c != 0)
                  strcpy(et, c->c_sval);
               break;
            default:
               make_et(et, yychar);
            }
	 fprintf(stderr, "syntax error in line %d, near token `%s'.\n",
            lineno + 1, et);
      }

/*
 * make_et - make an error token using the token and most recent char read.
 */

   static make_et(s, y)
      char *s;
      int y;

      {
         char *tokname(), *p;

         if (y <= 256){
            s[0] = y;
            s[1] = 0;
            }
         else{
            strcpy(s, tokname(y));
            for (p = s; *p; p++)
               if (isupper(*p))
                  *p = tolower(*p);
            }
      }
