/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: token.c,v 9.1 2003/03/29 17:59:36 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#include "dbsql_df.h"
struct tok
{	char *tnm;
	int yval;
} tok[]	= {
"FIRSTTOKEN", 258,
"ALL", 259,
"AND", 260,
"ANY", 261,
"AS", 262,
"ASC", 263,
"AVG", 264,
"T_BEGIN", 265,
"BETWEEN", 266,
"BY", 267,
"COUNT", 268,
"CREATE", 269,
"DEC", 270,
"DESC", 271,
"DROP", 272,
"DISTINCT", 273,
"EXISTS", 274,
"FROM", 275,
"GROUP", 276,
"HAVING", 277,
"IN", 278,
"INTO", 279,
"IS", 280,
"INDEX", 281,
"LIKE", 282,
"MAX", 283,
"MIN", 284,
"NOT", 285,
"NULL", 286,
"OF", 287,
"ON", 288,
"OR", 289,
"ORDER", 290,
"SELECT", 291,
"SOME", 292,
"SUM", 293,
"TABLE", 294,
"TO", 295,
"UNION", 296,
"USER", 297,
"VIEW", 298,
"WHENEVER", 299,
"WHERE", 300,
"WITH", 301,
"POWER", 302,
"ROUND", 303,
"TRUNC", 304,
"ABS", 305,
"LENGTH", 306,
"SUBSTR", 307,
"UPPER", 308,
"LOWER", 309,
"OWNER", 310,
"UID", 311,
"GID", 312,
"MODE", 313,
"SUBLINE", 314,
"FORMFIELD", 315,
"HEADER", 316,
"FOOTER", 317,
"ODD", 318,
"EVEN", 319,
"PAGELENGTH", 320,
"INDENT", 321,
"LINELENGTH", 322,
"LABEL", 323,
"SYSDATE", 324,
"CON", 325,
"FCON", 326,
"CCON", 327,
"SCON", 328,
"ID", 329,
"NEQ", 330,
"LEQ", 331,
"GEQ", 332,
"UNARY", 333,
"LASTTOKEN", 334,
};



void fb_ptoken(n)
{
	if(n<128) printf("lex: %c\n",n);
	else	if(n<=256) printf("lex:? %o\n",n);
	else	if(n<LASTTOKEN) printf("lex: %s\n",tok[n-257].tnm);
	else	printf("lex:? %o\n",n);
	return;
}

char *tokname(n)
{
	if (n<=256 || n >= LASTTOKEN)
		n = 257;
	return(tok[n-FIRSTTOKEN].tnm);
}
