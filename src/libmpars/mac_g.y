%{
   
/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_g.y,v 9.1 2002/12/26 22:12:15 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 *  grammer/parser stuff for macro language
 */

#include <fb.h>
#include "macro_e.h"

#undef NULL
#undef END
#undef FLOAT
#undef INTEGER 
#undef INTO
#undef MAX
#undef MIN
#undef NUMERIC

#define NULL 0

int c;
%}

%token FIRSTTOKEN		/* has to be first */
%token IF ELSE WHILE FOR BREAK CONTINUE M_RETURN M_EXIT M_LOCAL
%token SWITCH CASE DEFAULT
%token CON FCON CCON SCON OCON ID
%token LSHFT_A RSHFT_A XOR_A AND_A OR_A ADD_A
%token MINUS_A MULT_A DIV_A MOD_A 
%token INCR DECR AND EQ NEQ LEQ GEQ OR LSHFT RSHFT
%token M_BEGIN M_BODY M_END M_FUNCTION M_MAIN

%left LOW
%right '=' LSHFT_A RSHFT_A XOR_A AND_A OR_A ADD_A MINUS_A MULT_A DIV_A MOD_A
%right ','
%left OR
%left AND
%left '|'
%left '^'
%left '&' '$'
%left EQ NEQ
%left '<' '>' LEQ GEQ
%left LSHFT RSHFT
%left '+' '-'
%left '*' '/' '%'
%left ':'
%right UNARY INCR DECR
%start prog

%token LASTTOKEN		/* has to be last */
%%
prog	:	sect_lst
			{
                        	if (macro_errors > 0)
                                   return(-1);
                               	winner = (fb_mnode *) $1;
			}
	;

sect_lst:	sect
			{
				$$ = $1;
			}
	|	sect_lst sect
			{
				$$ = (int) fb_linknode($1, $2);
			}
	;
	
sect	:	M_BEGIN _prog
			{
				$$ = (int) fb_node1(S_BEGIN, $2);
			}
	|	M_BODY _prog
			{
				$$ = (int) fb_node1(S_BODY, $2);
			}
	|	M_END _prog
			{
				$$ = (int) fb_node1(S_END, $2);
			}
	|	M_FUNCTION call '{' _prog '}'
			{
				$$ = (int) fb_node2(S_FUNCTION, $2, $4);
			}
	|	M_FUNCTION call '{' '}'
			{
				$$ = (int)
                                   fb_node2(S_FUNCTION, $2, fb_nullnode());
			}
	|	M_MAIN '(' arg_lst ')' '{' _prog '}'
			{
				$$ = (int) fb_node1(S_BODY, $6);
			}
	|	_prog
			{
				$$ = (int) fb_node1(S_BODY, $1);
			}
	;

_prog	:	stmt_lst
			{
				$$ = $1;
			}
	;

stmt_lst:	stmt
			{
				$$ = $1;
			}
	|	stmt_lst stmt
			{
				$$ = (int) fb_linknode($1, $2);
			}
	;
	
stmt	:	expr
			{
				$$ = (int) fb_node1(S_EXPR, $1);
			}
	|  	if
			{
				$$ = $1;
			}
	|	if_else
			{
				$$ = $1;
			}
	|	while
			{
				$$ = $1;
			}
	|	for
			{
				$$ = $1;
			}
	|	switch
			{
				$$ = $1;
			}
	|	break
			{
				$$ = $1;
			}
	|	continue
			{
				$$ = $1;
			}
	|	return
			{
				$$ = $1;
			}
	|	exit
			{
				$$ = $1;
			}
	|	M_LOCAL vid_lst
			{
				$$ = (int) fb_node1(S_LOCAL, $2);
			}
	|	'{' stmt_lst '}'
			{
				$$ = (int) fb_node1(S_LIST, $2);
			}
	|	';'
			{
				$$ = (int) fb_nullnode();
			}
	;

if	:	IF '(' if_cond ')' stmt
			{
				$$ = (int) fb_node2(S_IF, $3, $5);
			}
	;

opcond	:	cond
			{
				$$ = $1;
			}
	|	/* empty */
			{
				$$ = (int) fb_nullnode();
			}
	;

if_cond	:	if_expr
			{
				$$ = $1;
			}
	;

cond	:	expr
			{
				$$ = $1;
			}
	;

if_else	:	IF '(' if_cond ')' stmt ELSE stmt
			{
				$$ = (int) fb_node3(S_IFELSE, $3, $5, $7);
			}
	;
	
while	:	WHILE '(' cond ')' stmt
			{
				$$ = (int) fb_node2(S_WHILE, $3, $5);
			}
	;
	
for	:	FOR '(' opexpr_lst ';' opcond ';' opexpr_lst ')' stmt
			{
				$$ = (int) fb_node4(S_FOR, $3, $5, $7, $9);
			}
		
	;
	
switch	:	SWITCH '(' expr ')' '{' case_lst default '}'
			{
				$$ = (int) fb_node3(S_SWITCH, $3, $6, $7);
			}
	;

case_lst:	case
			{
				$$ = $1;
			}
	|	case_lst case
			{
				$$ = (int) fb_linknode($1, $2);
			}
	;

case:		CASE case_term ':' stmt_lst
			{
				$$ = (int) fb_node2(S_CASE, $2, $4);
			}
	|	CASE case_term ':' /* empty */
			{
				$$ = (int) fb_node2(S_CASE, $2, fb_nullnode());
			}
	;

case_term:	vid
			{
                                $$ = $1;
			}
	|	CON
			{
                                $$ = (int) fb_vnode(V_CON, $1);
			}
	|	OCON
			{
                                $$ = (int) fb_vnode(V_OCON, $1);
			}
	|	FCON
			{
				$$ = (int) fb_vnode(V_FCON, $1);
			}
	|	CCON
			{
				$$ = (int) fb_vnode(V_CCON, $1);
			}
	|	SCON
			{
				$$ = (int) fb_vnode(V_SCON, $1);
			}
	;

default:	DEFAULT ':' stmt_lst
			{
				$$ = (int) fb_node1(S_DEFAULT, $3);
			}
	|	/* empty */
			{
				$$ = (int) fb_nullnode();
			}
	;

break	:	BREAK
			{
				$$ = (int) fb_node0(S_BREAK);
			}
	;
	
continue:	CONTINUE
			{
				$$ = (int) fb_node0(S_CONTINUE);
			}
	;

return:		M_RETURN ret_expr
                        {       
                                $$ = (int) fb_node1(S_RETURN, $2);
                        }
	|
		M_RETURN /* empty */
                        {       
                                $$ = (int) fb_node1(S_RETURN, fb_nullnode());
                        }
        ;

exit:		M_EXIT ret_expr
                        {       
                                $$ = (int) fb_node1(S_EXIT, $2);
                        }
	|
		M_EXIT /* empty */
                        {       
                                $$ = (int) fb_node1(S_EXIT, fb_nullnode());
                        }
        ;

ret_expr:	'(' opexpr_lst ')'
			{
                        	$$ = $2;
			}
        ;

opexpr_lst:	_opexpr_lst
			{
				$$ = (int) fb_node1(S_LIST, $1);
			}
	|	/* empty */
			{
				$$ = (int) fb_nullnode();
			}
	;

_opexpr_lst:	expr
			{
				$$ = $1;
			}
	|	_opexpr_lst ',' expr
			{
				$$ = (int) fb_listnode($1, $3);
			}
	;

expr	:	term
			{
				$$ = $1;
			}
	|	asgn
			{
				$$ = $1;
			}
	|	relxpr
			{
				$$ = $1;
			}
	|	combxpr
			{
				$$ = $1;
			}
	|	error
	;
	
if_expr	:	term
			{
				$$ = $1;
			}
	|	relxpr
			{
				$$ = $1;
			}
	|	combxpr
			{
				$$ = $1;
			}
	|	error
	;
	
asgn	:	lval LSHFT_A expr
			{
				$$ = (int) fb_node2(O_LSHFT_A, $1, $3);
	 		}
	|	lval RSHFT_A expr
			{
				$$ = (int) fb_node2(O_RSHFT_A, $1, $3);
	 		}
	|	lval XOR_A expr		
			{
				$$ = (int) fb_node2(O_XOR_A, $1, $3);
	 		}
	|	lval AND_A expr		
			{
				$$ = (int) fb_node2(O_AND_A, $1, $3);
	 		}
	|	lval OR_A expr 		
			{
				$$ = (int) fb_node2(O_OR_A, $1, $3);
	 		}
	|	lval ADD_A expr
			{
				$$ = (int) fb_node2(O_ADD_A, $1, $3);
	 		}
	|	lval MINUS_A expr
			{
				$$ = (int) fb_node2(O_MINUS_A, $1, $3);
	 		}
	|	lval MULT_A expr
			{
				$$ = (int) fb_node2(O_MULT_A, $1, $3);
	 		}
	|	lval DIV_A expr
			{
				$$ = (int) fb_node2(O_DIV_A, $1, $3);
	 		}
	|	lval MOD_A expr
			{
				$$ = (int) fb_node2(O_MOD_A, $1, $3);
	 		}
	|	lval '=' expr
			{
				$$ = (int) fb_node2(O_ASSIGN, $1, $3);
			}
	|	'(' lval_lst ')' '=' call
			{
				$$ = (int) fb_node2(O_ASSIGN, $2, $5);
			}
	;

relxpr	:	expr EQ expr
			{
				$$ = (int) fb_node2(R_EQ, $1, $3);
			}	
	|	expr NEQ expr
			{
				$$ = (int) fb_node2(R_NE, $1, $3);
			}	
	|	expr LEQ expr
			{
				$$ = (int) fb_node2(R_LE, $1, $3);
			}	
	|	expr GEQ expr
			{
				$$ = (int) fb_node2(R_GE, $1, $3);
			}	
	|	expr '<' expr
			{
				$$ = (int) fb_node2(R_LT, $1, $3);
			}	
	|	expr '>' expr
			{
				$$ = (int) fb_node2(R_GT, $1, $3);
			}	
	|	expr AND expr
			{
				$$ = (int) fb_node2(O_AND, $1, $3);
			}	
	|	expr OR expr
			{
				$$ = (int) fb_node2(O_OR, $1, $3);
			}
	;
	
combxpr	:	expr '^' expr
			{
				$$ = (int) fb_node2(O_XOR, $1, $3);
			}
	|	expr '&' expr
			{
				$$ = (int) fb_node2(O_XAND, $1, $3);
			}
	|	expr '|' expr
			{
				$$ = (int) fb_node2(O_IOR, $1, $3);
			}
	|	expr LSHFT expr
			{
				$$ = (int) fb_node2(O_LSHFT, $1, $3);
			}
	|	expr RSHFT expr
			{
				$$ = (int) fb_node2(O_RSHFT, $1, $3);
			}
	|	expr '+' expr
			{
				$$ = (int) fb_node2(O_ADD, $1, $3);
			}
	|	expr '-' expr
			{
				$$ = (int) fb_node2(O_SUB, $1, $3);
			}
	|	expr '*' expr
			{
				$$ = (int) fb_node2(O_MUL, $1, $3);
			}
	|	expr '/' expr
			{
				$$ = (int) fb_node2(O_DIV, $1, $3);
			}
	|	expr '%' expr
			{
				$$ = (int) fb_node2(O_MOD, $1, $3);
			}
	;
	
term	:	bmp_lval
			{
				$$ = $1;
			}
	|	unitrm  
			{
				$$ = $1;
			}
	|	call    
			{
				$$ = $1;
			}
	|	CON
			{
                                $$ = (int) fb_vnode(V_CON, $1);
			}
	|	OCON
			{
                                $$ = (int) fb_vnode(V_OCON, $1);
			}
	|	FCON
			{
				$$ = (int) fb_vnode(V_FCON, $1);
			}
	|	CCON
			{
				$$ = (int) fb_vnode(V_CCON, $1);
			}
	|	SCON
			{
				$$ = (int) fb_vnode(V_SCON, $1);
			}
/*
 * this does not work since it walks on the ability to use END program part.
 *
 *	|	M_END
 *			{
 *                       	if ((c = (int) fb_lookup("END")) == 0)
 *                                  c = (int) fb_sym_install("END");
 *				$$ = (int) fb_vnode(V_ID, c);
 *			}
*/
	|	nest
			{
				$$ = $1;
			}
	|	lval %prec LOW
			{
				$$ = $1;
			}
	;
			

nest	:	'(' expr ')'
			{
				$$ = $2;
			}
	;
	
call	:	vid '(' arg_lst ')'
			{
				$$ = (int) fb_node2(O_CALL, $1, $3);
			}
	;
			
bmp_lval:	lval INCR
			{
				$$ = (int) fb_node1(O_INCR_A, $1);
			}
	|	lval DECR
			{
				$$ = (int) fb_node1(O_DECR_A, $1);
			}
	
	|	INCR lval
			{
				$$ = (int) fb_node1(O_INCR_B, $2);
			}
	|	DECR lval
			{
				$$ = (int) fb_node1(O_DECR_B, $2);
			}	
	;	

unitrm	:	'-' term %prec UNARY
			{
				$$ = (int) fb_node1(O_UMINUS, $2);
			}
	|	'!' expr %prec UNARY
			{
				$$ = (int) fb_node1(O_UNOT, $2);
			}	
	|	'~' term %prec UNARY
			{
				$$ = (int) fb_node1(O_UOR, $2);
			}
	;
	
arg_lst	:	_arg_lst
			{
				$$ = $1;
			}
	|	/* empty */
			{
				$$ = (int) fb_nullnode();
			}
	;
				
_arg_lst:	expr
			{
				$$ = $1;
			}
	|	arg_lst ',' expr
			{
				$$ = (int) fb_linknode($1, $3);
			}	
	;

lval	:	vid
			{
				$$ = $1;
			}
	|	vid '[' expr ']'
			{
				$$ = (int) fb_node2(V_ARRAY, $1, $3);
			}
	|	'$' term
			{
				$$ = (int) fb_node1(O_UFIELD, $2);
			}
	;

lval_lst:	lval
			{
				$$ = $1;
			}
	|	lval_lst ',' lval
			{
				$$ = (int) fb_listnode($1, $3);
			}
	;

vid	:	ID
			{
				$$ = (int) fb_vnode(V_ID, $1);
			}
	;

vid_lst:	_vid_lst
			{
                        	$$ = $1;
			}
	|	/* empty */
			{
				$$ = (int) fb_nullnode();
			}
	;

_vid_lst:	vid
			{
				$$ = $1;
			}
	|	vid_lst ',' vid 
			{
				$$ = (int) fb_listnode($1, $3);
			}
	;
%%
