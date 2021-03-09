/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: yyerror.c,v 9.1 2001/02/16 19:31:18 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Yyerror_sid[] = "@(#) $Id: yyerror.c,v 9.1 2001/02/16 19:31:18 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>

#undef NULL
#undef END
#undef FLOAT
#undef INTEGER 
#undef INTO
#undef MAX
#undef MIN
#undef NUMERIC

#include "macro_df.h"

extern int yychar, yylval;
extern short int cdb_batchmode;

#if !FB_PROTOTYPES
static void make_et();
#else /* FB_PROTOTYPES */
static void make_et(char *, int);
#endif /* FB_PROTOTYPES */

extern short int cdb_cgi_flag;

/*
* yerror - error routine called by yyparse() on grammer errors
*/
   void yyerror(msg)
      char *msg;
   
      {
         char et[FB_MAXLINE], buf[FB_MAXLINE];
         fb_cell *c;

         (void) Yyerror_sid;
         (void) msg;

         switch(yychar){
            case CCON:
            case CON:
            case FCON:
            case ID:
            case SCON:
               c = (fb_cell *) yylval;
               if (c != 0)
                  strcpy(et, c->c_sval);
               break;
            default:
               make_et(et, yychar);
            }
         
	 sprintf(buf, "syntax error in line %d, near token `%s'", lineno, et);
         if (parsename != 0 && *parsename != 0)
	    sprintf(buf, "%s of file `%s'", buf, parsename);
	 strcat(buf, ".\n");
         if (cdb_batchmode){
            if (cdb_cgi_flag)
               fprintf(stdout, "%s<BR>\n", buf);
            else
               fprintf(stderr, buf);
            }
         else{
            fb_move(cdb_t_lines, 1);
            fb_clrtoeol();
            fb_prints(buf);
            fb_refresh();
            }
         macro_errors++;
      }

/*
 * make_et - make an error token using the token and most recent char read.
 */

   static void make_et(s, y)
      char *s;
      int y;

      {
         char *p;

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
