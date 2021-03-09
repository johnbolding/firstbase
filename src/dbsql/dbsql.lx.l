%{

/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbsql.lx.l,v 9.1 2001/01/12 22:51:55 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 *
 * lexical analyzer stuff for dbsql
 */

#include "dbsql_e.h"

#undef NULL
#undef END
#undef FLOAT
#undef INTEGER 
#undef INTO
#undef MAX
#undef MIN
#undef NUMERIC

#include "dbsql_df.h"

#undef input
#undef unput

extern int yylval;
extern int yychar;
extern int lineno;			/* for tracing line number */
extern int i_cur;			/* current input location */
extern char *i_ptr;

extern FILE *infile;
extern int interactive;

int save_val;

static int INPUT();
/*static unput();*/
#define YY_INPUT(buf,result,max_size) \
	{ \
		int c = INPUT(); \
		result = (c == 0) ? YY_NULL : (buf[0] = c, 1); \
	}
%}

DIGIT	[0-9]
F_ALPHA	[a-zA-Z_/]
ALPHA	[a-zA-Z/~]
DIRS	("../"|"./")
CHAR	[a-zA-Z0-9<>()[\]{}.,;&!~_*/%+\-=^|?:@`$# ]
WS	[ \t]
%p 5000
%e 2000
%a 2500
%S COMMENT
%%
<COMMENT>"\n"					{lineno++; BEGIN INITIAL;}
<COMMENT>.					;
<INITIAL>#					BEGIN COMMENT;
<INITIAL>ALL|all				return(ALL);
<INITIAL>AND|and				return(AND);
<INITIAL>ANY|any				return(ANY);
<INITIAL>AS|as					return(AS);
<INITIAL>ASC|asc				return(ASC);
<INITIAL>AVG|avg				return(AVG);
<INITIAL>BETWEEN|between			return(BETWEEN);
<INITIAL>BY|by					return(BY);
<INITIAL>COUNT|count				return(COUNT);
<INITIAL>CREATE|create				return(CREATE);
<INITIAL>DESC|desc				return(DESC);
<INITIAL>DISTINCT|distinct			return(DISTINCT);
<INITIAL>DROP|drop				return(DROP);
<INITIAL>EXISTS|exists				return(EXISTS);
<INITIAL>FROM|from				return(FROM);
<INITIAL>GROUP|group				return(GROUP);
<INITIAL>HAVING|having				return(HAVING);
<INITIAL>IN|in					return(IN);
<INITIAL>INDEX|index				return(INDEX);
<INITIAL>INTO|into				return(INTO);
<INITIAL>IS|is					return(IS);
<INITIAL>LIKE|like				return(LIKE);
<INITIAL>MAX|max				return(MAX);
<INITIAL>MIN|min				return(MIN);
<INITIAL>NOT|not				return(NOT);
<INITIAL>NULL|null				return(NULL);
<INITIAL>OF|of					return(OF);
<INITIAL>ON|on					return(ON);
<INITIAL>OR|or					return(OR);
<INITIAL>ORDER|order				return(ORDER);
<INITIAL>SELECT|select				return(SELECT);
<INITIAL>SOME|some				return(SOME);
<INITIAL>SUM|sum				return(SUM);
<INITIAL>TO|to					return(TO);
<INITIAL>VIEW|view				return(VIEW);
<INITIAL>WHERE|where				return(WHERE);
<INITIAL>POWER|power				return(POWER);
<INITIAL>ROUND|round				return(ROUND);
<INITIAL>TRUNC|trunc				return(TRUNC);
<INITIAL>ABS|abs				return(ABS);
<INITIAL>LABEL|label				return(LABEL);
<INITIAL>LENGTH|length				return(LENGTH);
<INITIAL>SUBSTR|substr				return(SUBSTR);
<INITIAL>UPPER|upper				return(UPPER);
<INITIAL>LOWER|lower				return(LOWER);
<INITIAL>SUBLINE|subline			return(SUBLINE);
<INITIAL>FORMFIELD|formfield			return(FORMFIELD);
<INITIAL>HEADER|header				return(HEADER);
<INITIAL>FOOTER|footer				return(FOOTER);
<INITIAL>ODD|odd				return(ODD);
<INITIAL>EVEN|even				return(EVEN);
<INITIAL>SYSDATE|sysdate			return(SYSDATE);
<INITIAL>OWNER|owner				return(OWNER);
<INITIAL>UID|uid				return(UID);
<INITIAL>GID|gid				return(GID);
<INITIAL>MODE|mode				return(MODE);
<INITIAL>"/"					return(yytext[0]);
<INITIAL>{DIGIT}+				return(linstall(CON));
<INITIAL>{DIGIT}+"."{DIGIT}*			return(linstall(FCON));
<INITIAL>"."{DIGIT}+{DIGIT}*			return(linstall(FCON));
<INITIAL>({ALPHA}|{DIRS})({F_ALPHA}|{DIGIT}|{DIRS})* return(linstall(ID));
<INITIAL>\"(\\?(({CHAR})|')|\\\"|\\\\)*\"	return(linstall(SCON));
<INITIAL>\'(\\?(({CHAR})|\")|\\\'|\\\\)*\'	return(linstall(SCON));
<INITIAL>"&&"					return(AND);
<INITIAL>"!="					return(NEQ);
<INITIAL>"<>"					return(NEQ);
<INITIAL>"<="					return(LEQ);
<INITIAL>">="					return(GEQ);
<INITIAL>"||"					return(OR);
<INITIAL>"("|")"|","|";"|"["|"]"|"."		|
<INITIAL>"{"|"}"|"~"|"-"|"!"|"&"|">"|"<"	|
<INITIAL>"|"|"^"|"+"|"-"|"%"|"*"|"="		return(yytext[0]);
<INITIAL>"\n"					lineno++;
<INITIAL>[ \t]					;
<INITIAL>.					fprintf(stderr,"%s?\n",yytext);
%%

/*
 * linstall - lex install of constants into the symtable
 */

   linstall(t)
      int t;

      {
         if ((yylval = (int) lookup(yytext)) == 0)
            yylval = (int) sym_install(yytext);
         return(t);
      }

/*
 * input - the lex input function - reads from mem for speed
 */

#if FLEX
   int INPUT()
#else
   int input()
#endif /* FLEX */
      {
         char c;

         c = *i_ptr;
         if (c == '\0')
            return(0);
         i_ptr++;
         i_cur++;
         /*
         if (c == '\n')
            yylineno++;
         */
         return(c);
      }

/*
 * unput - the lex unput function - reads from mem for interactive, or file
 *	for script driven.
 */

   unput(c)
      char c;

      {
         i_cur--;
         if (i_cur >= 0)
            i_ptr--;
         *i_ptr = c;
         if (c == '\n')
            lineno--;
      }
