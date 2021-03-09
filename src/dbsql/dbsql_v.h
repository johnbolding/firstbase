/* dbsql_vars.h - variables header file for dbsql @(#)dbsql_v.h	8.3 01/21/00 FB */

#include "dbsql.h"
#include <fb_vars.h>

int lineno = 1;				/* for tracing line number */
char i_mem[MAX_INTER];			/* interactive requests */
int i_cur;				/* current input location */
char *i_ptr;

cell **symtab;				/* symbol table */
node *winner;				/* top of code to execute */

node *vir_dbase = NULL;			/* list of virtual databases */
node *vir_index = NULL;			/* list of virtual indexes */

node *n_ghead = NULL;			/* node garbage head */
v_matrix *v_ghead = NULL;		/* vmatrix garbage head */
cell *c_ghead = NULL;			/* cell garbage head */

int npatterns = 0;			/* count LIKE s in a query */

relation *rel_depth[MAXVARIABLES];	/* globals for recursive solution */
relation *rel_single[MAXVARIABLES];
relation *r_ghead = NULL;
long rel_val[MAXVARIABLES];		/* 0 ... vloc+vsize */

FILE *infile;				/* input file */
int interactive = 1;			/* flag for interactive */
char user_home[FB_MAXNAME];		/* end user home directory */
node *int_open = NULL;			/* list of trees open for interupts */

int emitflag = 0;			/* emit flag, and its flags */
int quoteflag = 0;			/* quote flag for emit flags */
int verbose = 0;			/* verbose flag for emit flags */
int newline_flag = 1;			/* print newlines for long fields */
char separator = ',';			/* standard separator */
int html = 0;				/* HTML flag */
int html_border = 0;			/* HTML BORDER value */
int html_cellpadding = 0;		/* HTML CELLPADDING value */

char header1[FB_MAXLINE];		/* page formatting variables */
char header2[FB_MAXLINE];
char footer1[FB_MAXLINE];
char footer2[FB_MAXLINE];
int pagelength = 66;
int linelength = 75;
int pagenumber = 1;
int pageindent = 5;
int formatpage = 0;
int margin[5];				/* top/bottom margin arrays */
int linenumber;
int last_printline;

#if DEBUG
int traceflag = 0;			/* debug use only - trace tree */
#endif /* DEBUG */

char sql_tempdir[FB_MAXNAME];		/* -t */
char sql_pager[FB_MAXNAME];
char *group_value;

node *g_restrict, *g_slist, *g_from, *g_group_by, *g_order_by;
node *vn;
node *sub_list, *sp;

fb_database *vp;
fb_field *vir_by[FB_MAXBY];

short int create_ddict = 0;
short int create_virtual = 0;
short int save_virtual = 0;		/* used by create view command */
short int eval_functions = 1;
short int lastchar_was_newline = 0;
short int group_expr = 0;
short int pagerflag = 0;		/* -P */
short int relation_function = 0;

long rpos, rlen, wcount;
long virtual_count = 0;

int print_horizontal = 1;

relation *r_head;

mat_head h_output, h_single, h_double, h_reduced;

FILE *sql_ofs;				/* for popen use */

node *title_node;			/* head of title list */
