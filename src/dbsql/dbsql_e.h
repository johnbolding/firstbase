/* dbsql_ext.h - external header file for dbsql @(#)dbsql_e.h	8.3 01/21/00 FB */

#include "dbsql.h"
#include <fb_ext.h>

extern int lineno;			/* for tracing line number */
extern char i_mem[];			/* interactive requests */
extern int i_cur;			/* current input location */
extern char *i_ptr;

extern cell **symtab;
extern node *winner;

extern node *vir_dbase;			/* list of virtual databases */
extern node *vir_index;			/* list of virtual indexes */

extern node *n_ghead;			/* node garbage head */
extern v_matrix *v_ghead;		/* vmatrix garbage head */
extern cell *c_ghead;			/* cell garbage head */

extern int npatterns;			/* count LIKE s in a query */

extern relation *rel_depth[];		/* globals for recursive solution */
extern relation *rel_single[];
extern relation *r_ghead;
extern long rel_val[];

extern FILE *infile;			/* input file */
extern int interactive;			/* flag for interactive */
extern char user_home[];		/* end user home directory */
extern node *int_open;			/* list of trees open for interupts */

extern int emitflag;			/* emit flag, and its flags */
extern int quoteflag;			/* quote flag for emit flags */
extern int verbose;			/* verbose flag for emit flags */
extern newline_flag;			/* print newlines for long fields */
extern char separator;			/* standard separator */
extern int html;			/* HTML flag */
extern int html_border;			/* HTML BORDER value */
extern int html_cellpadding;		/* HTML CELLPADDING value */

extern char header1[];			/* page formatting variables */
extern char header2[];
extern char footer1[];
extern char footer2[];
extern int pagelength;
extern int linelength;
extern int pagenumber;
extern int pageindent;
extern int formatpage;
extern int margin[];			/* top/bottom margin arrays */
extern int linenumber;
extern int last_printline;

#if DEBUG
extern int traceflag;			/* debug use only - trace tree */
#endif /* DEBUG */

extern short cdb_datedisplay;

extern char sql_tempdir[];		/* -t */
extern char sql_pager[];
extern char *group_value;

extern node *g_restrict, *g_slist, *g_from, *g_group_by, *g_order_by;
extern node *vn;
extern node *sub_list, *sp;

extern fb_database *vp;
extern fb_field *vir_by[];

extern short int create_ddict;
extern short int create_virtual;
extern short int save_virtual;		/* used by create view command */
extern short int eval_functions;
extern short int lastchar_was_newline;
extern short int group_expr;
extern short int pagerflag;		/* -P */
extern short int relation_function;

extern long rpos, rlen, wcount;
extern long virtual_count;

extern int print_horizontal;

extern relation *r_head;

extern mat_head h_output, h_single, h_double, h_reduced;

extern FILE *sql_ofs;			/* for popen use */

extern node *title_node;		/* head of title list */
