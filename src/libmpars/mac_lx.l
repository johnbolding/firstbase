%{

/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_lx.l,v 9.2 2001/01/24 12:37:17 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 *
 * lexical analyzer stuff for macro language
 *
 */

#include <fb.h>
#include "macro_e.h"

#undef MAX
#undef MIN

#include "macro_df.h"

#undef input
#undef unput

extern int yylval;
extern int yychar;
extern int lineno;			/* for tracing line number */
extern int i_cur;			/* current input location */
extern char *i_ptr;
extern short int cdb_cgi_flag;
extern short int cdb_batchmode;

#if !FB_PROTOTYPES
static linstall();
static unput();
static lex_error();
#else /* ! FB_PROTOTYPES */
static linstall(int);
static unput(int);
static lex_error(char *);
#endif /* FB_PROTOTYPES */

#if FLEX
static int INPUT();
#define YY_INPUT(buf,result,max_size) \
	{ \
		int c = INPUT(); \
		result = (c == 0) ? YY_NULL : (buf[0] = c, 1); \
	}
#else

#if !FB_PROTOTYPES
int input();
#else  /* !FB_PROTOTYPES */
int input(void);
#endif /* FB_PROTOTYPES */

#endif /* FLEX */
%}
EMPTY	""
DIGIT	[0-9]
ALPHA	[a-zA-Z_]
CHAR	[a-zA-Z0-9<>()[\]{}.,;&!~_*/%+\-=^|?:@`$# \t]
WH	[\ \t]*
COM	#.*
%S COMMENT COMMENT1
%%
<INITIAL>#.*\n					lineno++;
<INITIAL>"#"					BEGIN COMMENT1;
<COMMENT1>"\n"					{lineno++; BEGIN INITIAL;}
<COMMENT1>.	 				;
<COMMENT>"*/"					BEGIN INITIAL;
<COMMENT>"\n"					lineno++;
<COMMENT>.	 				;
<INITIAL>"/*"					BEGIN COMMENT;
<INITIAL>"if"					return(IF);
<INITIAL>"else"					return(ELSE);
<INITIAL>"while"				return(WHILE);
<INITIAL>"for"					return(FOR);
<INITIAL>"break"				return(BREAK);
<INITIAL>"continue"				return(CONTINUE);
<INITIAL>"switch"				return(SWITCH);
<INITIAL>"case"					return(CASE);
<INITIAL>"default"				return(DEFAULT);
<INITIAL>"return"				return(M_RETURN);
<INITIAL>"exit"					return(M_EXIT);
<INITIAL>"local"				return(M_LOCAL);
<INITIAL>("BEGIN"|"begin")({WH}|{WH}{COM})$	return(M_BEGIN);
<INITIAL>("BODY"|"body")({WH}|{WH}{COM})$	return(M_BODY);
<INITIAL>("END"|"end")({WH}|{WH}{COM})$		return(M_END);
<INITIAL>"function"|"FUNCTION"			return(M_FUNCTION);
<INITIAL>"main"|"MAIN"				return(M_MAIN);
<INITIAL>0{DIGIT}+				return(linstall(OCON));
<INITIAL>{DIGIT}+				return(linstall(CON));
<INITIAL>{DIGIT}+"."{DIGIT}*			return(linstall(FCON));
<INITIAL>"."{DIGIT}+{DIGIT}*                    return(linstall(FCON));
<INITIAL>{ALPHA}({ALPHA}|{DIGIT})*		return(linstall(ID));
<INITIAL>\"(\\?(({CHAR})|')|\\\"|\\\\)*\"	return(linstall(SCON));
<INITIAL>\'(\\?(({CHAR})|\")|\\\'|\\\\)\'	return(linstall(CCON));
<INITIAL>"<<="					return(LSHFT_A);
<INITIAL>">>="					return(RSHFT_A);
<INITIAL>"^="					return(XOR_A);
<INITIAL>"&="					return(AND_A);
<INITIAL>"|="					return(OR_A);
<INITIAL>"+="					return(ADD_A);
<INITIAL>"-="					return(MINUS_A);
<INITIAL>"*="					return(MULT_A);
<INITIAL>"/="					return(DIV_A);
<INITIAL>"%="					return(MOD_A);
<INITIAL>"++"					return(INCR);
<INITIAL>"--"					return(DECR);
<INITIAL>"&&"					return(AND);
<INITIAL>"=="					return(EQ);
<INITIAL>"!="					return(NEQ);
<INITIAL>"<="					return(LEQ);
<INITIAL>">="					return(GEQ);
<INITIAL>"||"					return(OR);
<INITIAL>"<<"					return(LSHFT);
<INITIAL>">>"					return(RSHFT);
<INITIAL>"("|")"|","|";"|"["|"]"|"$"|":"	|
<INITIAL>"{"|"}"|"~"|"-"|"!"|"&"|">"|"<"	|
<INITIAL>"|"|"^"|"+"|"-"|"%"|"*"|"/"|"=" 	return(yytext[0]);
<INITIAL>"\n"					lineno++;
<INITIAL>[ \t]					;
<INITIAL>.				{
                                        	lex_error(yytext);
                                        }
%%

/*
 * linstall - lex install of constants into the symtable
 */

   static linstall(t)
      int t;

      {
         if ((yylval = (int) fb_lookup(yytext)) == 0)
            yylval = (int) fb_sym_install(yytext);
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
            lineno++;
         */
         return(c);
      }

/*
 * unput - the lex unput function - reads from mem for interactive, or file
 *	for script driven.
 */

   static unput(c)
      int c;

      {
         i_cur--;
         if (i_cur >= 0)
            i_ptr--;
         *i_ptr = c;
         if (c == '\n')
            lineno--;
      }

   static lex_error(p)
      char *p;

      {
         char buf[FB_MAXLINE];
         
	 sprintf(buf, "lexical error in line %d, token is `%s'",
            lineno, yytext);
         if (cdb_batchmode){
            if (cdb_cgi_flag)
               fprintf(stdout, "%s<BR>\n", buf);
            else
               fprintf(stderr, "%s\n", buf);
            }
         else{
            fb_move(24, 1);
            fb_clrtoeol();
            fb_prints(buf);
            fb_refresh();
            }
         macro_errors++;
      }
