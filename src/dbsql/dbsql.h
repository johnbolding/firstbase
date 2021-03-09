/* dbsql.h - main header file for dbsql @(#)dbsql.h	8.2 04 Aug 1997 FB */

/* parser include file */

#include <fb.h>

#define IDNTY           (yyval=yypvt[-0] )
#define MAX_INTER	2500

#define MAXSYM		50
#define MIN_RLEN 	(FB_MAPREC_SIZE + 2)

/* symbol table structures */
typedef struct s_cell cell;
struct s_cell {
   char *c_sval;	/* string value */
   cell *c_chain;	/* hash chain */
   };

cell **makesymtab(), *lookup(), *install(), *makecell();

#define NARGS	4

/* code nodes */
typedef struct s_node node;
struct s_node {
   char n_type;
   node *n_next;			/* next - links for statements */
   node *n_list;			/* list - links for comma lists */
   node *n_group;			/* group- links for groups */
   node *n_label;			/* group- links for groups */
   int n_obj;				/* pointer to generic object (cell) */
   node *n_narg[NARGS];			/* args - subparts, and DOT parts */
   short int n_lvarc;			/* variable counters */
   short int n_tvarc;

   char *n_nval;			/* name or string value/variables */
   char *n_pval;			/* printable expr string value */
   float n_fval;			/* value as a number */
   unsigned n_tval;			/* type information */
   int n_width;				/* width of object */
   int n_vwidth;			/* object width when projected vert. */
   int n_height;			/* height of object */
   int n_scale;				/* scale/precision of object */
   int n_id;				/* id - used for instance matrices */
   node *n_virlist;			/* virtual list list for execution */
   int n_p1;				/* extra generic pointer #1 */
   int n_p2;				/* extra generic pointer #2 */
   node *n_glink;			/* garbage link */
   };

node *makenode(), *nullnode();

#define S_SELECT	1
#define S_QUERY		2
#define S_NULL		3
#define S_SUBQ		4
#define S_CREATE_VIEW	5
#define S_DROP_VIEW	6
#define S_CREATE_INDEX	7
#define S_DROP_INDEX	8

#define F_OWNER		10
#define F_GROUP		11
#define F_UID		12
#define F_GID		13
#define F_MODE		14

#define Q_STAR		27
#define Q_ALL		28
#define Q_DISTINCT	29
#define Q_USER		30
#define Q_ANY		31
#define Q_SOME		32
#define Q_ASC		33
#define Q_DESC		34

#define O_ADD		35
#define O_SUB		36
#define O_MUL		37
#define O_DIV		38
#define O_UPLUS		39
#define O_UMINUS	40
#define O_OR		41
#define O_AND		42
#define O_UNOT		43
#define O_UNION		44
#define O_UEXISTS	45
#define O_CONCAT	46

#define R_EQ		50
#define R_LT		51
#define R_GT		52
#define R_LE		53
#define R_GE		54
#define R_NE		55

#define P_BETWEEN	60
#define P_NOT_BETWEEN	61
#define P_IN		62
#define P_NOT_IN	63
#define P_LIKE		64
#define P_NOT_LIKE	65
#define P_IS		66
#define P_IS_NOT	67

#define E_TABLE		80
#define E_SORT		81

#define V_ID		90
#define V_CON		91
#define V_FCON		92
#define V_CCON		93
#define V_SCON		94

#define C_ROOT		97
#define C_AND		98

#define F_COUNTALL	100
#define F_AVG		101
#define F_MAX		102
#define F_MIN		103
#define F_SUM		104
#define F_COUNT		105

#define F_POWER		106
#define F_ROUND		107
#define F_TRUNC		108
#define F_ABS		109
#define F_LENGTH	110
#define F_SUBSTR	111
#define F_UPPER		112
#define F_LOWER		113
#define F_SUBLINE	114
#define F_FORMFIELD	115
#define F_SYSDATE	116

#define H_HEADER	117
#define H_FOOTER	118
#define H_ODD		119
#define H_EVEN		120
#define H_MASTER	121

/* cant go past 126 here ^^^^ ! */

/*
 * expression types and macros  -
 *	use x |= T_VAL to turn on, x &= ~T_VAL to turn off.
 */

#define T_STR 01		/* string value */
#define T_NUM 02		/* numeric value */
#define T_FLD 04		/* database field */
#define T_CON 010		/* constant */
#define T_ARR 020		/* array */
#define T_FCN 040		/* Function */
#define T_BOP 0100		/* Binary Op */
#define T_UOP 0200		/* Unary Op */
#define T_VIR 0400		/* Virtual Object */
#define T_TOP 01000		/* Trinary Op */
#define T_LOP 02000		/* List Op ... with list of args */
#define T_SFCN 04000		/* Simple Function */
#define T_DTE 010000		/* Date Type */

#define istype_str(x) (((x->n_tval & T_STR) == 0) ? 0 : 1)
#define istype_num(x) (((x->n_tval & T_NUM) == 0) ? 0 : 1)
#define istype_fld(x) (((x->n_tval & T_FLD) == 0) ? 0 : 1)
#define istype_con(x) (((x->n_tval & T_CON) == 0) ? 0 : 1)
#define istype_arr(x) (((x->n_tval & T_ARR) == 0) ? 0 : 1)
#define istype_fcn(x) (((x->n_tval & T_FCN) == 0) ? 0 : 1)
#define istype_bop(x) (((x->n_tval & T_BOP) == 0) ? 0 : 1)
#define istype_uop(x) (((x->n_tval & T_UOP) == 0) ? 0 : 1)
#define istype_top(x) (((x->n_tval & T_TOP) == 0) ? 0 : 1)
#define istype_lop(x) (((x->n_tval & T_LOP) == 0) ? 0 : 1)
#define istype_vir(x) (((x->n_tval & T_VIR) == 0) ? 0 : 1)
#define istype_sfcn(x) (((x->n_tval & T_SFCN) == 0) ? 0 : 1)
#define istype_date(x) (((x->n_tval & T_DTE) == 0) ? 0 : 1)

#define vtype_str(x) (((x & T_STR) == 0) ? 0 : 1)
#define vtype_num(x) (((x & T_NUM) == 0) ? 0 : 1)
#define vtype_fld(x) (((x & T_FLD) == 0) ? 0 : 1)
#define vtype_con(x) (((x & T_CON) == 0) ? 0 : 1)
#define vtype_arr(x) (((x & T_ARR) == 0) ? 0 : 1)
#define vtype_fcn(x) (((x & T_FCN) == 0) ? 0 : 1)
#define vtype_bop(x) (((x & T_BOP) == 0) ? 0 : 1)
#define vtype_uop(x) (((x & T_UOP) == 0) ? 0 : 1)
#define vtype_top(x) (((x & T_TOP) == 0) ? 0 : 1)
#define vtype_lop(x) (((x & T_LOP) == 0) ? 0 : 1)
#define vtype_vir(x) (((x & T_VIR) == 0) ? 0 : 1)

/* instance matrix structures */

#define MAXVARIABLES	15
#define MAXCLAUSES	25

#define V_SINGLE	1
#define V_DOUBLE	2
#define V_NOREDUCE	3
#define V_TARGET	4

typedef struct sv_matrix v_matrix;
struct sv_matrix {
   short int v_array[MAXVARIABLES];
   short int v_list[MAXCLAUSES];
   v_matrix *v_next;
   v_matrix *v_prev;
   short int v_type;				/* type 1...4, Wong et al */
   short int v_overlap;				/* type 2 overlap marker */
   v_matrix *v_glink;				/* garbage collection */
   };

typedef struct s_mat_head mat_head;
struct s_mat_head {
   v_matrix *m_head;
   v_matrix *m_tail;
   node *m_vars;
   };

/*
 * relation header information
 */

#define REL_PER_PAGE 512
#define DBSQL_PAGESIZE (REL_PER_PAGE * FB_SLONG)

typedef struct S_relation relation;
struct S_relation {
   /* variable length fields */
   char *r_tmpfile;
   fb_database **r_dbase;
   long *r_rec;
   char *r_join_value;

   /* fixed length */
   int r_fd;
   long r_reccnt;
   long r_curp;
   long r_recsiz;
   int r_nrecs;				/* number of mini-recs/logical rec */
   long r_mem[REL_PER_PAGE];		/* storage of the current page */
   int r_offset[MAXVARIABLES];		/* offsets into r_dbase array */
   int r_isize[MAXVARIABLES];		/* sizes of individual parts */

   long r_page_id;
   short int r_writepage;
   long r_npages;

   v_matrix *r_vm;			/* current v_matrix */
   node *r_vars;			/* node list of vars */

   int r_nvars;				/* nvars in the v_matrix r_vm */
   short int r_used;			/* to flag when used during relate() */

   relation *r_next;			/* next in list */

   int r_irecsiz;			/* index record size */
   long r_ireccnt;			/* index record size */
   int r_ifields;			/* number of index fields */
   int r_ifd;				/* index file descriptor */
   int r_ihfd;				/* index file descriptor */
   char *r_irec;			/* buffer for an index record */
   char *r_ibase;			/* base name for rel index */
   char *r_idict;			/* dict filename for rel index */
   char *r_index;			/* index filename for rel index */

   fb_autoindex *r_aid;			/* auto index for optimization */
   fb_field *r_join_fld;		/* join field - to get back to data */
   int r_join_op;			/* join operator - R_EQ, R_GT, etc */
   relation *r_glink;			/* relation garbage list */
   };

/* some dbsql functions defined */
extern float expr();
extern float function();
extern float sfunction();
