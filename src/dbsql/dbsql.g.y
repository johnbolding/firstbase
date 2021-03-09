%{
   
/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbsql.g.y,v 9.1 2002/12/29 17:19:01 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 *  grammer/parser stuff for dbsql
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

node *n, *nq;

%}
%token FIRSTTOKEN		/* has to be first */
%token ALL AND ANY AS ASC AVG
%token T_BEGIN BETWEEN BY
%token COUNT CREATE 
%token DEC DESC DROP DISTINCT
%token EXISTS
%token FROM
%token GROUP
%token HAVING
%token IN INTO IS INDEX
%token LIKE
%token MAX MIN NOT NULL
%token OF ON OR ORDER
%token SELECT SOME
%token SUM
%token TABLE TO
%token UNION USER
%token VIEW
%token WHENEVER WHERE WITH

%token POWER ROUND TRUNC ABS
%token LENGTH SUBSTR UPPER LOWER
%token OWNER UID GID MODE
%token SUBLINE FORMFIELD
%token HEADER FOOTER ODD EVEN PAGELENGTH INDENT LINELENGTH
%token LABEL SYSDATE
%token CON FCON CCON SCON
%token ID NEQ LEQ GEQ

%right NOT
%left OR
%left AND
%left '+' '-' '|'
%left '*' '/' '%'
%right UNARY
%start prog

%token LASTTOKEN		/* has to be last */

%%
prog	:	_prog
				{
                                	winner = (node *) $1;
                                }
	;

_prog	:	stmt_list ';'
				{
                                	$$ = $1;
                                }
	;

stmt_list:	stmt
				{
                                	$$ = $1;
                                }
	|	stmt_list ';' stmt
				{
                                	$$ = (int) linknode($1, $3);
                                }
        ;

stmt	:	q_expr
				{
                                	$$ = $1;
                                }
	|	c_view
				{
                                	$$ = $1;
                                }
	|	d_view
				{
                                	$$ = $1;
                                }
	|	c_index
				{
                                	$$ = $1;
                                }
	|	d_index
				{
                                	$$ = $1;
                                }
	;

c_view	:	CREATE VIEW tab_name col_list AS q_spec
				{
                                	$$ = (int) node3(S_CREATE_VIEW,
                                           $3,$4,$6);
                                }
	;

d_view	:	DROP VIEW tab_name
				{
                                	$$ = (int) node1(S_DROP_VIEW, $3);
                                }
	;

c_index	:	CREATE INDEX ind_name ON tab_name '(' sort_spec_list ')' where
				{
                                	$$ = (int) node4(S_CREATE_INDEX,
                                           $3,$5,$7,$9);
                                }
	;

d_index	:	DROP INDEX ind_name
				{
                                	$$ = (int) node1(S_DROP_INDEX, $3);
                                }
	;

q_expr	:	q_term
				{
                                	$$ = $1;
                                }
	|	q_expr UNION opt_all q_term
        			{
                                	$$ = (int) node3(O_UNION, $1, $3, $4);
                                }
	;

q_term	:	q_spec
				{
                                	$$ = $1;
                                }
	|	'(' q_expr ')'
				{
                                	$$ = $2;
                                }
	;

q_spec	:	SELECT sel_q sel_list tab_exp orderby heading
				{
                                	n = (node *)node4(S_QUERY,$2,$3,$4,$5);
                                        n->n_p1 = $6;
                                	$$ = (int) n;
                                }
	;

sub_q	:	'(' SELECT sel_q subq_exp tab_exp ')'
				{
                                	$$ = (int) node3(S_SUBQ, $3, $4, $5);
                                }
	;

subq_exp:	val_expr
				{
                                	$$ = $1;
                                }
	|	'*'
				{
                                	$$ = (int) node0(Q_STAR);
                                }
	;

sel_q	:	ALL
				{
                                	$$ = (int) node0(Q_ALL);
                                }
	|	DISTINCT
				{
                                	$$ = (int) node0(Q_DISTINCT);
                                }
        |	/* empty */
				{
                                	$$ = (int) node0(Q_ALL);
                                }
        ;

orderby	:	ORDER BY sort_spec_list
				{
                                	$$ = $3;
                                }
	|	/* empty */
				{
                                	$$ = (int) nullnode();
                                }
	;

heading	:	header footer
				{
                                	$$ = (int) node2(H_MASTER,$1,$2);
                                }
	;

header:		HEADER literal
				{
                                	n = nullnode();
                                	$$ = (int) node2(H_HEADER, n, $2);
                                }
	|	HEADER htype literal htype literal
				{
                                	$$ = (int) node4(H_HEADER,$2,$3,$4,$5);
                                }
	|	/* empty */
        			{
                                	$$ = (int) nullnode();
                                }
	;

footer:		FOOTER literal
				{
                                	n = nullnode();
                                	$$ = (int) node2(H_FOOTER, n, $2);
                                }
	|	FOOTER htype literal htype literal
				{
                                	$$ = (int) node4(H_FOOTER,$2,$3,$4,$5);
                                }
	|	/* empty */
        			{
                                	$$ = (int) nullnode();
                                }
	;

htype	:	ODD
				{
                                	$$ = (int) node0(H_ODD);
                                }
	|	EVEN
				{
                                	$$ = (int) node0(H_EVEN);
                                }
	;

sort_spec_list:	sort_spec
				{
                                	$$ = $1;
                                }
	|	sort_spec_list ',' sort_spec
				{
                                	$$ = (int) listnode($1, $3);
                                }
	;

sort_spec:	con sort_desc
				{
                                	$$ = (int) node2(E_SORT, $1, $2);
                                }
        |	col_spec sort_desc
				{
                                	$$ = (int) node2(E_SORT, $1, $2);
                                }
	;

sort_desc:	ASC
				{
                                	$$ = (int) node0(Q_ASC);
                                }
	|	DESC
				{
                                	$$ = (int) node0(Q_DESC);
                                }
        |	/* empty */
				{
                                	$$ = (int) node0(Q_ASC);
                                }
        ;

sel_list:	Lexpr_list
				{
                                	$$ = $1;
                                }
	|	'*'
				{
                                	$$ = (int) node0(Q_STAR);
                                }
        ;

tab_exp	:	from where groupby having
				{
                   			$$ = (int) node4(E_TABLE,$1,$2,$3,$4);
                		}
	;

from	:	FROM tab_list
				{
                                	$$ = $2;
                                }
	;

where	:	WHERE search
				{
                                	$$ = $2;
                                }
	|	/* empty */
				{
                                	$$ = (int) nullnode();
                                }
	;

groupby	:	GROUP BY col_spec_list
				{
                                	$$ = $3;
                                }
	|	/* empty */
				{
                                	$$ = (int) nullnode();
                                }
	;

having	:	HAVING search
				{
                                	$$ = $2;
                                }
	|	/* empty */
				{
                                	$$ = (int) nullnode();
                                }
	;

tab_list:	tab_ref
				{
                                	$$ = $1;
                                }
	|	tab_list ',' tab_ref
				{
                                	$$ = (int) listnode($1, $3);
                                }
	;

tab_ref	:	tab_name
				{
                                	$$ = $1;
                                }
	|	tab_name cor_name
				{
                                	$$ = (int) groupnode($1, $2);
                                }
	;

cor_name:	ID
				{
                                	$$ = (int) vnode(V_ID, $1);
                                }
	;

tab_name:	ID
				{
                                	$$ = (int) vnode(V_ID, $1);
                                }
	|	ID '.' ID
				{
                                	$$ = (int) vnode1(V_ID, $3, $1);
                                }
	;

ind_name:	ID
				{
                                	$$ = (int) vnode(V_ID, $1);
                                }
	;

col_list:	'(' a_col_list ')'
				{
                                	$$ = $2;
                                }
	|	/* empty */
				{
                                	$$ = (int) nullnode();
                                }
        ;

a_col_list:	a_col_spec
				{
                                	$$ = $1;
                                }
	|	a_col_list ',' a_col_spec
				{
                                	$$ = (int) listnode($1, $3);
                                }
        ;

a_col_spec:	ID
				{
                                	$$ = (int) vnode(V_ID, $1);
                                }
	;

col_spec_list:	col_spec
				{
                                	$$ = $1;
                                }
	|	col_spec_list ',' col_spec
				{
                                	$$ = (int) listnode($1, $3);
                                }
	;

col_spec:	ID
				{
                                	$$ = (int) vnode(V_ID, $1);
                                }
        |	ID '.' ID
				{
                                	$$ = (int) vnode1(V_ID, $3, $1);
                                }
        |	ID '.' '*'
				{
                                	$$ = (int) vnode(Q_STAR, $1);
                                }
        |	ID '.' ID '.' ID
				{
                                	$$ = (int) vnode2(V_ID, $5, $3, $1);
                                }
	;

search	:	bool_term
				{
                                	$$ = $1;
                                }
	|	search OR bool_term
				{
                                	$$ = node2(O_OR, $1, $3);
                                }
	;

bool_term:	bool_fact
				{
                                	$$ = $1;
                                }
	|	bool_term AND bool_fact
				{
                                	$$ = node2(O_AND, $1, $3);
                                }
        ;

bool_fact:	bool_prim
				{
                                	$$ = $1;
                                }
	|	NOT bool_prim
				{
                                	$$ = (int) node1(O_UNOT, $2);
                                }
	;

bool_prim:	predicate
				{
                                	$$ = $1;
                                }
	|	'(' search ')'
				{
                                	$$ = $2;
                                }
	;

predicate:	val_expr comp_op val_expr
				{
                                	$$ = (int) node2($2, $1, $3);
                                }
	|	val_expr comp_op sub_q
				{
					nq = (node *) node0(Q_ALL);
					n = (node *) $3;
         				n->n_narg[3] = nq;
                                	$$ = (int) node2($2, $1, $3);
                                }
	|	val_expr between val_expr AND val_expr
				{
                                	$$ = (int) node3($2, $1, $3, $5);
                                }
	|	val_expr in '(' val_spec_list ')'
				{
                                	$$ = (int) node2($2, $1, $4);
                                }
	|	val_expr in sub_q
				{
                                	$$ = (int) node2($2, $1, $3);
                                }
	|	col_spec like val_spec
				{
                                	$$ = (int) node2($2, $1, $3);
                                }
	|	col_spec is NULL
				{
                                	$$ = (int) node2($2, $1, nullnode());
                                }
	|	val_expr comp_op p_quant sub_q
				{
					nq = (node *) $3;
					n = (node *) $4;
         				n->n_narg[3] = nq;
                                	$$ = (int) node2($2, $1, $4);
                                }
	|	EXISTS sub_q
				{
                                	$$ = (int) node1(O_UEXISTS, $2);
                                }
	;

p_quant	:	ALL
				{
					$$ = (int) node0(Q_ALL);
                                }
	|	ANY
				{
					$$ = (int) node0(Q_ANY);
                                }
        |	SOME
				{
					$$ = (int) node0(Q_ANY);
                                }
	;

between	:	NOT BETWEEN
				{
                                	$$ = P_NOT_BETWEEN;
                                }
	|	BETWEEN
				{
                                	$$ = P_BETWEEN;
                                }
        ;

in	:	NOT IN
				{
                                	$$ = P_NOT_IN;
                                }
	|	IN
				{
                                	$$ = P_IN;
                                }
        ;

like	:	NOT LIKE
				{
                                	$$ = P_NOT_LIKE;
                                }
	|	LIKE
				{
                                	$$ = P_LIKE;
                                }
	;

is	:	IS NOT
				{
                                	$$ = P_IS_NOT;
                                }
	|	IS
				{
                                	$$ = P_IS;
                                }
	;

comp_op	:	'='
				{
                                	$$ = R_EQ;
                                }
	|	'<'
				{
                                	$$ = R_LT;
                                }
        |	'>'
				{
                                	$$ = R_GT;
                                }
        |	LEQ
				{
                                	$$ = R_LE;
                                }
        |	GEQ
				{
                                	$$ = R_GE;
                                }
        |	NEQ
				{
                                	$$ = R_NE;
                                }
        ;

primary	:	val_spec
				{
                                	$$ = $1;
                                }
	|	col_spec %prec UNARY
				{
                                	$$ = $1;
                                }
	|	set_spec
				{
                                	$$ = $1;
                                }
	|	fcn_spec
				{
                                	$$ = $1;
                                }
        |	'(' val_expr ')'
				{
                                	$$ = $2;
                                }
	;

fcn_spec:	POWER '(' val_expr ',' val_expr ')'
				{
                                	$$ = (int) node2(F_POWER, $3, $5);
                                }
	|	ROUND '(' val_expr ',' val_expr ')'
				{
                                	$$ = (int) node2(F_ROUND, $3, $5);
                                }
	|	TRUNC '(' val_expr ',' val_expr ')'
				{
                                	$$ = (int) node2(F_TRUNC, $3, $5);
                                }
	|	ABS '(' val_expr ')'
				{
                                	$$ = (int) node1(F_ABS, $3);
                                }
	|	LENGTH '(' val_expr ')'
				{
                                	$$ = (int) node1(F_LENGTH, $3);
                                }
	|	SUBSTR '(' val_expr ',' val_expr ',' val_expr ')'
				{
                                	$$ = (int) node3(F_SUBSTR, $3, $5, $7);
                                }
	|	UPPER '(' val_expr ')'
				{
                                	$$ = (int) node1(F_UPPER, $3);
                                }
	|	LOWER '(' val_expr ')'
				{
                                	$$ = (int) node1(F_LOWER, $3);
                                }
	|	SUBLINE '(' val_expr ',' val_expr ',' val_expr ')'
				{
                                	$$ = (int) node3(F_SUBLINE, $3, $5,$7);
                                }
	|	FORMFIELD '(' val_expr ',' val_expr ',' val_expr ')'
				{
                                	$$ = (int) node3(F_FORMFIELD,$3,$5,$7);
                                }
	|	SYSDATE
        			{
                                	$$ = (int) node0(F_SYSDATE);
                                }
	|	OWNER
				{
                                	$$ = (int) node0(F_OWNER);
                                }
	|	GROUP
				{
                                	$$ = (int) node0(F_GROUP);
                                }
	|	UID
				{
                                	$$ = (int) node0(F_UID);
                                }
	|	GID
				{
                                	$$ = (int) node0(F_GID);
                                }
	|	MODE
				{
                                	$$ = (int) node0(F_MODE);
                                }
	;

val_spec_list:	val_spec
				{
                                	$$ = $1;
                                }
	|	val_spec_list ',' val_spec
				{
                                	$$ = (int) listnode($1, $3);
                                }
        ;

val_spec:	literal
				{
                                	$$ = $1;
                                }
        |	USER
				{
                                	$$ = (int) node0(Q_USER);
                                }
	;

val_expr:	term
				{
                                	$$ = $1;
                                }
	|	val_expr '+' term
				{
                                	$$ = (int) node2(O_ADD, $1, $3);
                                }
        |	val_expr '-' term
				{
                                	$$ = (int) node2(O_SUB, $1, $3);
                                }
	|	val_expr '|' term
				{
                                	$$ = (int) node2(O_CONCAT, $1, $3);
                                }
        ;

set_spec:	COUNT '(' '*' ')'
				{
                                	$$ = (int) node0(F_COUNTALL);
                                }
	|	dist_set
				{
                                	$$ = $1;
                                }
        |	all_set
				{
                                	$$ = $1;
                                }
	;

dist_set:	AVG distinct
				{
                                	$$ = (int) node1(F_AVG, $2);
                                }
	|	MAX distinct
				{
                                	$$ = (int) node1(F_MAX, $2);
                                }
	|	MIN distinct
				{
                                	$$ = (int) node1(F_MIN, $2);
                                }
	|	SUM distinct
				{
                                	$$ = (int) node1(F_SUM, $2);
                                }
	|	COUNT distinct
				{
                                	$$ = (int) node1(F_COUNT, $2);
                                }
	;
        
distinct:	'(' DISTINCT col_spec ')'
				{
                                	$$ = (int) node1(Q_DISTINCT, $3);
                                }
	;

all_set:	AVG all
				{
                                	$$ = (int) node1(F_AVG, $2);
                                }
	|	MAX all
				{
                                	$$ = (int) node1(F_MAX, $2);
                                }
	|	MIN all
				{
                                	$$ = (int) node1(F_MIN, $2);
                                }
	|	SUM all
				{
                                	$$ = (int) node1(F_SUM, $2);
                                }
	;
        
all	:	'(' opt_all val_expr ')'
				{
                                	$$ = (int) node1(Q_ALL, $3);
                                }
	;

opt_all	:	/* empty */
				{
                                	$$ = (int) nullnode();
                                }
	|	ALL
				{
                                	$$ = (int) node0(Q_ALL);
                                }
	;

Lexpr_list:	Lexpr
				{
                                	$$ = $1;
                                }
	|	Lexpr_list ',' Lexpr
				{
                                	$$ = (int) listnode($1, $3);
                                }
        ;

Lexpr	:	expr opt_lab
				{
					n = (node *) $1;
                                        if ($2 != 0)
         				   n->n_label = (node *) $2;
                                	$$ = $1;
                                }
	;

opt_lab	:	LABEL literal
				{
                                	$$ = $2;
                                }
	|	LABEL AS literal
				{
                                	$$ = $3;
                                }
	|	LABEL ID
				{
                                	$$ = (int) vnode(V_ID, $2);
                                }
	|	LABEL AS ID
				{
                                	$$ = (int) vnode(V_ID, $3);
                                }
	|	/* empty */
				{
                                	$$ = 0;
                                }
        ;

/*
* expr_list:	expr
* 				{
*                                 	$$ = $1;
*                                 }
* 	|	expr_list ',' expr
* 				{
*                                 	$$ = (int) listnode($1, $3);
*                                 }
*         ;
*/

expr	:	term
				{
                                	$$ = $1;
                                }
	|	expr '+' term
				{
                                	$$ = (int) node2(O_ADD, $1, $3);
                                }
        |	expr '-' term
				{
                                	$$ = (int) node2(O_SUB, $1, $3);
                                }
	|	expr '|' term
				{
                                	$$ = (int) node2(O_CONCAT, $1, $3);
                                }
        ;

term	:	factor
				{
                                	$$ = $1;
                                }
	|	term '*' factor
				{
                                	$$ = (int) node2(O_MUL, $1, $3);
                                }
	|	term '/' factor
				{
                                	$$ = (int) node2(O_DIV, $1, $3);
                                }
        ;

factor	:	primary
				{
                                	$$ = $1;
                                }
	|	uni_prim
				{
                                	$$ = $1;
                                }
	;

uni_prim:	'+' primary %prec UNARY
				{
                                	$$ = (int) node1(O_UPLUS, $2);
                                }
	| 	'-' primary %prec UNARY
				{
                                	$$ = (int) node1(O_UMINUS, $2);
                                }
        ;

literal	:	con
				{
                                	$$ = (int) $1;
                                }
	|	FCON
				{
                                	$$ = (int) vnode(V_FCON, $1);
                                }
	|	CCON
				{
                                	$$ = (int) vnode(V_CCON, $1);
                                }
	|	SCON
				{
                                	$$ = (int) vnode(V_SCON, $1);
                                }
	;

con:		CON
				{
                                	$$ = (int) vnode(V_CON, $1);
                                }
	;
%%
