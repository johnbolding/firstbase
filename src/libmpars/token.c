/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: token.c,v 9.1 2004/01/02 20:51:10 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#include "macro_df.h"
struct tok
{	char *tnm;
	int yval;
} tok[]	= {
"FIRSTTOKEN", 258,
"IF", 259,
"ELSE", 260,
"WHILE", 261,
"FOR", 262,
"BREAK", 263,
"CONTINUE", 264,
"M_RETURN", 265,
"M_EXIT", 266,
"M_LOCAL", 267,
"SWITCH", 268,
"CASE", 269,
"DEFAULT", 270,
"CON", 271,
"FCON", 272,
"CCON", 273,
"SCON", 274,
"OCON", 275,
"ID", 276,
"LSHFT_A", 277,
"RSHFT_A", 278,
"XOR_A", 279,
"AND_A", 280,
"OR_A", 281,
"ADD_A", 282,
"MINUS_A", 283,
"MULT_A", 284,
"DIV_A", 285,
"MOD_A", 286,
"INCR", 287,
"DECR", 288,
"AND", 289,
"EQ", 290,
"NEQ", 291,
"LEQ", 292,
"GEQ", 293,
"OR", 294,
"LSHFT", 295,
"RSHFT", 296,
"M_BEGIN", 297,
"M_BODY", 298,
"M_END", 299,
"M_FUNCTION", 300,
"M_MAIN", 301,
"LOW", 302,
"UNARY", 303,
"LASTTOKEN", 304,
};

void ptoken(n)
int n;

{
	if(n<128) printf("lex: %c\n",n);
	else	if(n<=256) printf("lex:? %o\n",n);
	else	if(n<LASTTOKEN) printf("lex: %s\n",tok[n-257].tnm);
	else	printf("lex:? %o\n",n);
	return;
}

char *tokname(n)
int n;

{
	if (n<=256 || n >= LASTTOKEN)
		n = 257;
	return(tok[n-FIRSTTOKEN].tnm);
}
