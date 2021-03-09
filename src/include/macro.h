/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: macro.h,v 9.3 2001/09/29 18:10:48 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/
/*
 * header file for macro fields
 */

/* parser include file */

/* include for math.h is now in fb.h */

#define IDNTY           (yyval=yypvt[-0] )

#define MAXSYM		50
#define MIN_RLEN (MAPREC_SIZE + 2)

#define NARGS	4

/* code nodes - NOW CHANGED to mnode */
typedef struct fb_s_mnode fb_mnode;
struct fb_s_mnode {
   char n_type;
   fb_mnode *n_next;			/* next - links for statements */
   fb_mnode *n_list;			/* list - links for comma lists */
   int n_obj;				/* fb_cell pointer fb_cell, usually */
   fb_mnode *n_narg[NARGS];		/* args - subparts */
   char *n_nval;			/* constants value/variables */
   char *n_pval;			/* string value/variables */
   double n_fval;			/* value as a number */
   unsigned n_tval;			/* type information */
   short int n_width;			/* width of object */
   short int n_scale;			/* scale/precision of object */
   fb_mnode *n_virlist;			/* virtual list list for execution */
   int n_p1;				/* extra generic pointer #1 */
   int n_p2;				/* extra generic pointer #2 */
   fb_mnode *n_glink;			/* garbage link */
   short int n_lineno;			/* code line number */
   char *n_fname;			/* pointer to current function */
   };

/* symbol table structures */
typedef struct fb_s_cell fb_cell;
struct fb_s_cell {
   char *c_sval;	/* string value */
   fb_cell *c_chain;	/* hash chain */
   fb_mnode *c_vid;	/* pointer to a macro node if its a V_ID */
   fb_cell **c_symtab;	/* symtab for arrays */
   };

/* stack vars for macro frames */
typedef struct fb_s_stack_vars fb_stack_vars;
struct fb_s_stack_vars {
   int return_flag;
   int return_value;
   int break_flag;
   int continue_flag;
   int exit_flag;
   fb_mnode *s_return_list;
   fb_cell **s_symtab;
   fb_mnode *s_n_ghead;
   fb_cell *s_c_ghead;
   };

#define S_NULL		1
#define S_EXPR		2
#define S_IF		3
#define S_IFELSE	4
#define S_WHILE		5
#define S_FOR		6
#define S_BREAK		7
#define S_CONTINUE	8
#define S_LIST		9
#define S_RETURN	10
#define S_BEGIN		11
#define S_BODY		12
#define S_END		13
#define S_FUNCTION	14
#define S_EXIT		15
#define S_LOCAL		16
#define S_SWITCH	17
#define S_CASE		18
#define S_DEFAULT	19

#define O_LSHFT_A	25
#define O_RSHFT_A	26
#define O_XOR_A		27
#define O_AND_A		28
#define O_OR_A		29
#define O_ADD_A		30
#define O_MINUS_A	31
#define O_MULT_A	32
#define O_DIV_A		33
#define O_MOD_A		34
#define O_ADD		35
#define O_SUB		36
#define O_MUL		37
#define O_DIV		38
#define O_UPLUS		39
#define O_UMINUS	40
#define O_OR		41
#define O_AND		42
#define O_UNOT		43
#define O_CONCAT	46
#define O_ASSIGN	47

#define R_EQ		50
#define R_LT		51
#define R_GT		52
#define R_LE		53
#define R_GE		54
#define R_NE		55

#define O_XOR		60
#define O_XAND		61
#define O_IOR		62
#define O_LSHFT		63
#define O_RSHFT		64
#define O_MOD		65
#define O_CALL		66
#define O_UOR		67
#define O_UFIELD	68
#define O_INCR_B	69
#define O_DECR_B	70
#define O_INCR_A	71
#define O_DECR_A        72

#define R_END		73
#define R_ERROR		74
#define R_ABORT		75
#define R_NEXT		76
#define R_PREV		77

#define V_ID		90
#define V_CON		91
#define V_FCON		92
#define V_CCON		93
#define V_SCON		94
#define V_ARRAY		95
#define V_OCON		96

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
#define T_ASGN 020000		/* Assignment Type */
#define T_DOL 040000		/* Dollar Type */
#define T_GLOB 0100000		/* Global Type */

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
#define istype_dte(x) (((x->n_tval & T_DTE) == 0) ? 0 : 1)
#define istype_asgn(x) (((x->n_tval & T_ASGN) == 0) ? 0 : 1)
#define istype_dol(x) (((x->n_tval & T_DOL) == 0) ? 0 : 1)
#define istype_glob(x) (((x->n_tval & T_GLOB) == 0) ? 0 : 1)

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

/* execution stuff */

#define MAX_FSTACK 20
#define MAX_SYMSTACK 100
#define MAX_MFILES 100

/* prototypes */

#if !FB_PROTOTYPES

/* libmpars/libsym.c */
extern fb_cell **fb_makesymtab();
extern void fb_expunge_symtab();
extern void fb_gcollect_s_pool();
extern fb_cell *fb_lookup();
extern fb_cell *fb_glookup();
extern fb_cell *fb_sym_install();
extern fb_cell *fb_sinstall();
extern fb_cell *fb_ginstall();
extern fb_cell *fb_u_sinstall();
extern fb_cell *fb_u_lookup();
extern fb_cell *fb_u_install();
extern fb_cell *fb_s_makecell();
extern fb_cell *fb_makecell();
extern void fb_gcollect_cell();
extern void fb_gcollect_c_pool();
extern void fb_freecell();
extern void fb_dump_symtab();

/* libmpars/mac_trac.c */
extern void fb_macrotrace();
extern void fb_tracesource();
extern void r_macrotrace();

/* libmpars/mac_tree.c */
extern fb_macrotree();
extern fb_macroscript();
extern fb_macrostdin();

/* libmpars/node.c */
extern fb_mnode *fb_makenode();
extern fb_mnode *fb_s_makenode();
extern fb_clearnode();
extern void fb_freenode();
extern void fb_free_globalnode();
extern fb_copynode();
extern fb_mnode *fb_nullnode();
extern fb_mnode *fb_vnode();
extern fb_mnode *fb_vnode1();
extern fb_mnode *fb_vnode2();
extern fb_mnode *fb_s_node0();
extern fb_mnode *fb_node0();
extern fb_mnode *fb_node1();
extern fb_mnode *fb_node2();
extern fb_mnode *fb_node3();
extern fb_mnode *fb_node4();
extern fb_mnode *fb_linknode();
extern fb_mnode *fb_listnode();
extern fb_testnull();
extern fb_countnodes();
extern fb_realnodes();
extern void fb_gcollect_mnode();
extern void fb_gcollect_m_pool();
extern void fb_gcollect();
extern void fb_2nd_gcollect();
extern void yyerror();

/* libmpars/token.c */
extern int ptoken();
extern char *tokname();

/* libmpars/token.c */
extern ptoken();
extern char *tokname();

/* libmexpr/mac_array.c */
extern void assign_array();
extern assign_copy_array();
extern mf_key();
extern mf_countkey();
extern mf_findkey();
extern mf_rmkey();
extern int mf_set_checkbox();
extern fb_mnode *array_to_list();
fb_mnode *copy_array_tree();
fb_mnode *copy_array_tree();

/* libexpr/mac_stmt.c */
double macro_statement();
void mf_set_constant();
int mf_set_value();
void mf_push_field();
void mf_pop_field();
int mf_search_stack();
void mf_push_symtab();
void mf_pop_symtab();
fb_cell **mf_current_symtab();
fb_cell **mf_current_symtab();
void mf_init_stack();
void mf_make_frame();
void mf_destroy_frame();
void mf_local();
void mf_perror();


/* libmexpr/mac_lib.c */
extern m_verify_sub();
extern fb_mnode *locate_section();
extern fb_mnode *locate_function();
extern void m_destroy_call_lists();

extern fb_cell **fb_makesymtab();
extern fb_cell *fb_lookup();
extern fb_cell *fb_sinstall();
extern fb_cell *fb_makecell();

extern double macro_function();


extern int fb_macroend();
extern int fb_macrobegin();
extern void mf_inp_make_frame();
extern void mf_inp_destroy_frame();
extern int fb_macro_level();

/* libmexpr/mac_date.c */
extern mf_now();
extern mf_cdbdate();
extern mf_year();
extern mf_month();
extern mf_day();
extern mf_hour();
extern mf_minute();
extern mf_second();
extern mf_date();
extern mf_dts();
extern mf_cdbdts();
extern mf_tts();
extern mf_newdate();
extern mf_ndays();

/* libmexpr/mac_expr.c */
extern char *tostring();
extern double tonumber();
extern is_null();
extern double macro_expr();
extern fb_field *mnode_to_field();
extern fb_field *mnode_to_field_via();
extern fb_mnode *mnode_to_var();
extern fb_mnode *mnode_to_local_var();

extern fb_mnode *string_to_var();
extern fb_mnode *string_to_local_var();
extern fb_cell *string_to_cell();
extern fb_cell *string_to_cell_via();
extern fb_cell *string_to_local_cell();
extern void eval_field();

/* libmexpr/mac_fb.c */
extern fb_database *mnode_to_array_dbase();

/* libmexpr/mac_func.c */

extern int mf_print();
extern int mf_reverse();
extern int mf_standout();
extern int mf_pause();
extern int mf_error();
extern int mf_redraw();
extern int mf_header();
extern int mf_footer();
extern int mf_status();
extern int mf_sleep();
extern int mf_system();
extern int mf_move();
extern int mf_clear();
extern int mf_clrtoeol();
extern int mf_clrtobot();
extern int mf_refresh();
extern int mf_editfield();
extern int mf_input();
extern int mf_rinput();
extern int mf_checkfields();
extern int mf_bell();
extern void destroy_list();

/* string functions */
extern int mf_substr();
extern int mf_length();
extern int mf_upper();
extern int mf_lower();
extern int mf_subline();
extern int mf_formfield();
extern int mf_fmt();
extern int mf_in();
extern int mf_pattern();
extern int mf_pattern_comp();
extern int mf_pattern_exec();
extern int mf_pattern_icase();
extern int mf_pattern_so();
extern int mf_pattern_eo();
extern int mf_pattern_substr();
extern int mf_gets();
extern int mf_puts();
extern int mf_countlines();
extern int mf_printf();
extern int mf_sprintf();
extern int mf_strchr();
extern int mf_strrchr();
extern int mf_rmlead();
extern int mf_rmnewline();
extern int mf_rmlinefeed();
extern int mf_rmunderscore();
extern int mf_trim();
extern int mf_makess();
extern int mf_getword();
extern int mf_gettoken();
extern int mf_crypt();

/* math functions */
extern int mf_max();
extern int mf_min();
extern int mf_ston();
extern int mf_pow();
extern int mf_round();
extern int mf_trunc();
extern int mf_abs();
extern int mf_sin();
extern int mf_cos();
extern int mf_tan();
extern int mf_asin();
extern int mf_acos();
extern int mf_atan();
extern int mf_atan2();
extern int mf_hypot();
extern int mf_exp();
extern int mf_sqrt();
extern int mf_log();
extern int mf_ceil();
extern int mf_floor();
extern int mf_srandom();
extern int mf_random();

/* date functions */
extern int mf_now();
extern int mf_cdbdate();
extern int mf_year();
extern int mf_month();
extern int mf_day();
extern int mf_hour();
extern int mf_minute();
extern int mf_second();
extern int mf_date();
extern int mf_dts();
extern int mf_tts();
extern int mf_newdate();
extern int mf_ndays();
extern int mf_cdbdts();

/* stream I/O functions */
extern int mf_access();
extern int mf_creat();
extern int mf_chmod();
extern int mf_fclose();
extern int mf_fflush();
extern int mf_fgets();
extern int mf_fopen();
extern int mf_fprintf();
extern int mf_fputs();
extern int mf_fread();
extern int mf_fseek();
extern int mf_fwrite();
extern int mf_mktemp();
extern int mf_mkstemp();
extern int mf_unlink();
extern int mf_link();
extern int mf_rename();
extern int mf_symlink();
extern int mf_pclose();
extern int mf_popen();

/* general UNIX functions */
extern int mf_getenv();
extern int mf_putenv();
extern int mf_fork();
extern int mf_terminate();

/* FirstBase Secure functions */
extern int mf_r_owner();
extern int mf_r_group();
extern int mf_r_mode();
extern int mf_r_chown();
extern int mf_r_chgrp();
extern int mf_r_chmod();

/* FirstBase library interface */
extern int mf_opendb();
extern int mf_closedb();
extern int mf_getrec();
extern int mf_getirec();
extern int mf_nfields();
extern int mf_reccnt();
extern int mf_getxrec();
extern int mf_recno();
extern int mf_putrec();
extern int mf_initrec();
extern int mf_addrec();
extern int mf_useidx();
extern int mf_usrlog();
extern int mf_nextxrec();
extern int mf_prevxrec();
extern int mf_lastxrec();
extern int mf_firstxrec();
extern int mf_lock();
extern int mf_unlock();
extern int mf_delrec();
extern int mf_field_name();
extern int mf_field_type();
extern int mf_field_default();
extern int mf_field_size();
extern int mf_default_dbase();
extern int mf_set_loadfail();
extern int mf_get_failrec();

extern int mf_key();
extern int mf_countkey();
extern int mf_findkey();
extern int mf_rmkey();

extern int mf_html_blockquote_open();
extern int mf_html_blockquote_close();
extern int mf_html_open();
extern int mf_html_close();
extern int mf_html_table_open();
extern int mf_html_table_close();
extern int mf_html_row_open();
extern int mf_html_row_close();
extern int mf_html_cell_open();
extern int mf_html_cell_close();
extern int mf_html_table_headers();

extern int mf_html_h1();
extern int mf_html_h2();
extern int mf_html_h3();
extern int mf_html_h4();
extern int mf_html_h5();
extern int mf_html_h6();

extern int mf_html_bold();
extern int mf_html_bold_open();
extern int mf_html_bold_close();
extern int mf_html_em_open();
extern int mf_html_em_close();
extern int mf_html_italics();
extern int mf_html_italics_open();
extern int mf_html_italics_close();
extern int mf_html_strong_open();
extern int mf_html_strong_close();
extern int mf_html_p_open();
extern int mf_html_p_close();
extern int mf_html_pre_open();
extern int mf_html_pre_close();
extern int mf_html_center_open();
extern int mf_html_center_close();
extern int mf_html_hr();
extern int mf_html_br();
extern int mf_html_comment();

extern int mf_html_center();
extern int mf_html_form_open();
extern int mf_html_form_close();
extern int mf_html_input();
extern int mf_html_row();

extern int mf_html_select();
extern int mf_html_select_open();
extern int mf_html_select_close();
extern int mf_html_select_option();
extern int mf_html_textarea();

extern int mf_html_href();
extern int mf_html_imgsrc();
extern int mf_html_font_open();
extern int mf_html_font_close();
extern int mf_html_font_color();
extern int mf_html_fontsize_open();
extern int mf_html_fontsize_close();
extern int mf_verify_ascii();
extern int mf_verify_date();
extern int mf_verify_dollar();
extern int mf_verify_float();
extern int mf_verify_numeric();
extern int mf_verify_pos_numeric();

extern int mf_load();

extern int mf_html_dl_open();
extern int mf_html_dl_close();
extern int mf_html_dt();
extern int mf_html_dd();
extern int mf_html_ul_open();
extern int mf_html_ul_close();
extern int mf_html_li();
extern int mf_html_h_open();
extern int mf_html_h_close();
extern int mf_html_ol_open();
extern int mf_html_ol_close();
extern int mf_html_filter_lt();

extern int mf_html_meta();
extern int mf_html_link();
extern int mf_html_script_open();
extern int mf_html_script_close();

extern int mf_chdir();
extern int mf_dump_symtab();
extern int mf_free_globals();
extern int mf_eval();
extern int mf_cgi_read();

extern int mf_ireccnt();
extern int mf_fgetrec();

/* libmepxr/mac_load.c */
extern int mf_load();
extern void fb_gcollect_loadnode();

#else /* ! FB_PROTOTYPES */

/* libmpars/libsym.c */
extern fb_cell **fb_makesymtab(void);
extern void fb_expunge_symtab(fb_cell **);
extern void fb_gcollect_s_pool(void);
extern fb_cell *fb_lookup(char *);
extern fb_cell *fb_glookup(char *);
extern fb_cell *fb_sym_install(char *);
extern fb_cell *fb_sinstall(char *);
extern fb_cell *fb_ginstall(char *);
extern fb_cell *fb_u_sinstall(char *, fb_cell **);
extern fb_cell *fb_u_lookup(char *, fb_cell **);
extern fb_cell *fb_u_install(char *, fb_cell **);
extern fb_cell *fb_s_makecell(void);
extern fb_cell *fb_makecell(void);
extern void fb_gcollect_cell(fb_cell *);
extern void fb_gcollect_c_pool(void);
extern void fb_freecell(fb_cell *c);
extern void fb_dump_symtab(void);

/* libmpars/mac_trac.c */
extern void fb_macrotrace(fb_mnode *, char *);
extern void fb_tracesource(FILE *fs, char *fname);
extern void r_macrotrace(fb_mnode *);

/* libmpars/mac_tree.c */
extern fb_macrotree(char *);
extern fb_macroscript(char *);
extern fb_macrostdin(void);

/* libmpars/node.c */
extern fb_mnode *fb_makenode(void);
extern fb_mnode *fb_s_makenode(void);
extern fb_clearnode(fb_mnode *);
extern void fb_freenode(fb_mnode *);
extern void fb_free_globalnode(fb_mnode *);
extern fb_copynode(fb_mnode *, fb_mnode *);
extern fb_mnode *fb_nullnode(void);
extern fb_mnode *fb_vnode(int, int);
extern fb_mnode *fb_vnode1(int, int, int);
extern fb_mnode *fb_vnode2(int, int, int, int);
extern fb_mnode *fb_s_node0(int);
extern fb_mnode *fb_node0(int);
extern fb_mnode *fb_node1(int, fb_mnode *);
extern fb_mnode *fb_node2(int, fb_mnode *, fb_mnode *);
extern fb_mnode *fb_node3(int, fb_mnode *, fb_mnode *, fb_mnode *);
extern fb_mnode *fb_node4(int, fb_mnode *, fb_mnode *, fb_mnode *, fb_mnode *);
extern fb_mnode *fb_linknode(fb_mnode *, fb_mnode *);
extern fb_mnode *fb_listnode(fb_mnode *, fb_mnode *);
extern fb_testnull(fb_mnode *);
extern fb_countnodes(fb_mnode *);
extern fb_realnodes(fb_mnode *);
extern void fb_gcollect_mnode(fb_mnode *);
extern void fb_gcollect_m_pool(void);
extern void fb_gcollect(fb_mnode *, fb_cell *);
extern void fb_2nd_gcollect(void);
extern void yyerror(char *);

/* libmpars/token.c */
extern int ptoken(int);
extern char *tokname(int);

/* libmpars/token.c */
extern ptoken(int);
extern char *tokname(int);

/* libmexpr/mac_array.c */
extern void assign_array(fb_mnode *, fb_mnode *, int);
extern assign_copy_array(fb_mnode *, fb_mnode *, fb_cell **);
extern mf_key(fb_mnode *, fb_mnode *);
extern mf_countkey(fb_mnode *, fb_mnode *);
extern mf_findkey(fb_mnode *, fb_mnode *);
extern mf_rmkey(fb_mnode *, fb_mnode *);
extern int mf_set_checkbox(char *, char *);
extern fb_mnode *array_to_list(fb_mnode *);
fb_mnode *copy_array_tree(fb_mnode *);
fb_mnode *copy_array_tree(fb_mnode *);

/* libexpr/mac_stmt.c */
double macro_statement(fb_mnode *, fb_stack_vars *);
void mf_set_constant(char *, int);
int mf_set_value(char *, char *);
void mf_push_field(fb_field *f);
void mf_pop_field(void);
int mf_search_stack(fb_field *);
void mf_push_symtab(fb_cell **);
void mf_pop_symtab(void);
fb_cell **mf_current_symtab(void);
fb_cell **mf_current_symtab(void);
void mf_init_stack(fb_stack_vars *);
void mf_make_frame(fb_stack_vars *);
void mf_destroy_frame(fb_stack_vars *);
void mf_local(fb_mnode *);
void mf_perror(char *, char *, fb_mnode *n);

/* libmexpr/mac_lib.c */
extern m_verify_sub(fb_mnode *);
extern fb_mnode *locate_section(int);
extern fb_mnode *locate_function(char *);
extern void m_destroy_call_lists(fb_mnode *);

extern fb_cell **fb_makesymtab(void);
extern fb_cell *fb_lookup(char *s);
extern fb_cell *fb_sinstall(char *s);
extern fb_cell *fb_makecell(void);

extern double macro_function(fb_mnode *n);


extern int fb_macroend(int t);
extern int fb_macrobegin(long erec);
extern void mf_inp_make_frame(fb_stack_vars *sv);
extern void mf_inp_destroy_frame(fb_stack_vars *sv);
extern int fb_macro_level(int t);

/* libmexpr/mac_date.c */
extern mf_now(fb_mnode *, fb_mnode *);
extern mf_cdbdate(fb_mnode *, fb_mnode *);
extern mf_year(fb_mnode *, fb_mnode *);
extern mf_month(fb_mnode *, fb_mnode *);
extern mf_day(fb_mnode *, fb_mnode *);
extern mf_hour(fb_mnode *, fb_mnode *);
extern mf_minute(fb_mnode *, fb_mnode *);
extern mf_second(fb_mnode *, fb_mnode *);
extern mf_date(fb_mnode *, fb_mnode *);
extern mf_dts(fb_mnode *, fb_mnode *);
extern mf_cdbdts(fb_mnode *, fb_mnode *);
extern mf_tts(fb_mnode *, fb_mnode *);
extern mf_newdate(fb_mnode *, fb_mnode *);
extern mf_ndays(fb_mnode *, fb_mnode *);

/* libmexpr/mac_expr.c */
extern char *tostring(fb_mnode *);
extern double tonumber(fb_mnode *);
extern is_null(fb_mnode *);
extern double macro_expr(fb_mnode *);
extern fb_field *mnode_to_field(fb_mnode *);
extern fb_field *mnode_to_field_via(fb_mnode *, fb_database *);
extern fb_mnode *mnode_to_var(fb_mnode *);
extern fb_mnode *mnode_to_local_var(fb_mnode *);

extern fb_mnode *string_to_var(char *);
extern fb_mnode *string_to_local_var(char *);
extern fb_cell *string_to_cell(char *);
extern fb_cell *string_to_cell_via(char *, fb_cell **);
extern fb_cell *string_to_local_cell(char *);
extern void eval_field(fb_field *, fb_database *, fb_mnode *);

/* libmexpr/mac_fb.c */
extern fb_database *mnode_to_array_dbase(fb_mnode *);
extern int mf_ireccnt(fb_mnode *, fb_mnode *);
extern int mf_fgetrec(fb_mnode *, fb_mnode *);

/* libmexpr/mac_func.c */

extern int mf_print(fb_mnode *, fb_mnode *);
extern int mf_reverse(fb_mnode *, fb_mnode *);
extern int mf_standout(fb_mnode *, fb_mnode *);
extern int mf_pause(fb_mnode *, fb_mnode *);
extern int mf_error(fb_mnode *, fb_mnode *);
extern int mf_redraw(fb_mnode *, fb_mnode *);
extern int mf_header(fb_mnode *, fb_mnode *);
extern int mf_footer(fb_mnode *, fb_mnode *);
extern int mf_status(fb_mnode *, fb_mnode *);
extern int mf_sleep(fb_mnode *, fb_mnode *);
extern int mf_system(fb_mnode *, fb_mnode *);
extern int mf_move(fb_mnode *, fb_mnode *);
extern int mf_clear(fb_mnode *, fb_mnode *);
extern int mf_clrtoeol(fb_mnode *, fb_mnode *);
extern int mf_clrtobot(fb_mnode *, fb_mnode *);
extern int mf_refresh(fb_mnode *, fb_mnode *);
extern int mf_editfield(fb_mnode *, fb_mnode *);
extern int mf_input(fb_mnode *, fb_mnode *);
extern int mf_rinput(fb_mnode *, fb_mnode *);
extern int mf_checkfields(fb_mnode *, fb_mnode *);
extern int mf_bell(fb_mnode *, fb_mnode *);
extern void destroy_list(fb_mnode *n);

/* string functions */
extern int mf_substr(fb_mnode *, fb_mnode *);
extern int mf_length(fb_mnode *, fb_mnode *);
extern int mf_upper(fb_mnode *, fb_mnode *);
extern int mf_lower(fb_mnode *, fb_mnode *);
extern int mf_subline(fb_mnode *, fb_mnode *);
extern int mf_formfield(fb_mnode *, fb_mnode *);
extern int mf_fmt(fb_mnode *, fb_mnode *);
extern int mf_in(fb_mnode *, fb_mnode *);
extern int mf_pattern(fb_mnode *, fb_mnode *);
extern int mf_pattern_icase(fb_mnode *, fb_mnode *);
extern int mf_pattern_so(fb_mnode *, fb_mnode *);
extern int mf_pattern_eo(fb_mnode *, fb_mnode *);
extern int mf_gets(fb_mnode *, fb_mnode *);
extern int mf_puts(fb_mnode *, fb_mnode *);
extern int mf_countlines(fb_mnode *, fb_mnode *);
extern int mf_printf(fb_mnode *, fb_mnode *);
extern int mf_sprintf(fb_mnode *, fb_mnode *);
extern int mf_index(fb_mnode *, fb_mnode *);
extern int mf_rindex(fb_mnode *, fb_mnode *);
extern int mf_rmlead(fb_mnode *, fb_mnode *);
extern int mf_rmnewline(fb_mnode *, fb_mnode *);
extern int mf_rmlinefeed(fb_mnode *, fb_mnode *);
extern int mf_rmunderscore(fb_mnode *, fb_mnode *);
extern int mf_trim(fb_mnode *, fb_mnode *);
extern int mf_makess(fb_mnode *, fb_mnode *);
extern int mf_getword(fb_mnode *, fb_mnode *);
extern int mf_gettoken(fb_mnode *, fb_mnode *);
extern int mf_crypt(fb_mnode *, fb_mnode *);

/* math functions */
extern int mf_max(fb_mnode *, fb_mnode *);
extern int mf_min(fb_mnode *, fb_mnode *);
extern int mf_ston(fb_mnode *, fb_mnode *);
extern int mf_pow(fb_mnode *, fb_mnode *);
extern int mf_round(fb_mnode *, fb_mnode *);
extern int mf_trunc(fb_mnode *, fb_mnode *);
extern int mf_abs(fb_mnode *, fb_mnode *);
extern int mf_sin(fb_mnode *, fb_mnode *);
extern int mf_cos(fb_mnode *, fb_mnode *);
extern int mf_tan(fb_mnode *, fb_mnode *);
extern int mf_asin(fb_mnode *, fb_mnode *);
extern int mf_acos(fb_mnode *, fb_mnode *);
extern int mf_atan(fb_mnode *, fb_mnode *);
extern int mf_atan2(fb_mnode *, fb_mnode *);
extern int mf_hypot(fb_mnode *, fb_mnode *);
extern int mf_exp(fb_mnode *, fb_mnode *);
extern int mf_sqrt(fb_mnode *, fb_mnode *);
extern int mf_log(fb_mnode *, fb_mnode *);
extern int mf_ceil(fb_mnode *, fb_mnode *);
extern int mf_floor(fb_mnode *, fb_mnode *);

/* date functions */
extern int mf_now(fb_mnode *, fb_mnode *);
extern int mf_cdbdate(fb_mnode *, fb_mnode *);
extern int mf_year(fb_mnode *, fb_mnode *);
extern int mf_month(fb_mnode *, fb_mnode *);
extern int mf_day(fb_mnode *, fb_mnode *);
extern int mf_hour(fb_mnode *, fb_mnode *);
extern int mf_minute(fb_mnode *, fb_mnode *);
extern int mf_second(fb_mnode *, fb_mnode *);
extern int mf_date(fb_mnode *, fb_mnode *);
extern int mf_dts(fb_mnode *, fb_mnode *);
extern int mf_tts(fb_mnode *, fb_mnode *);
extern int mf_newdate(fb_mnode *, fb_mnode *);
extern int mf_ndays(fb_mnode *, fb_mnode *);
extern int mf_cdbdts(fb_mnode *, fb_mnode *);

/* stream I/O functions */
extern int mf_access(fb_mnode *, fb_mnode *);
extern int mf_creat(fb_mnode *, fb_mnode *);
extern int mf_fclose(fb_mnode *, fb_mnode *);
extern int mf_fflush(fb_mnode *, fb_mnode *);
extern int mf_fgets(fb_mnode *, fb_mnode *);
extern int mf_fopen(fb_mnode *, fb_mnode *);
extern int mf_fprintf(fb_mnode *, fb_mnode *);
extern int mf_fputs(fb_mnode *, fb_mnode *);
extern int mf_fread(fb_mnode *, fb_mnode *);
extern int mf_fseek(fb_mnode *, fb_mnode *);
extern int mf_fwrite(fb_mnode *, fb_mnode *);
extern int mf_mktemp(fb_mnode *, fb_mnode *);
extern int mf_unlink(fb_mnode *, fb_mnode *);
extern int mf_pclose(fb_mnode *, fb_mnode *);
extern int mf_popen(fb_mnode *, fb_mnode *);
extern int mf_link(fb_mnode *, fb_mnode *);
extern int mf_rename(fb_mnode *, fb_mnode *);
extern int mf_symlink(fb_mnode *, fb_mnode *);

/* general UNIX functions */
extern int mf_getenv(fb_mnode *, fb_mnode *);
extern int mf_putenv(fb_mnode *, fb_mnode *);
extern int mf_fork(fb_mnode *, fb_mnode *);
extern int mf_terminate(fb_mnode *, fb_mnode *);

/* FirstBase Secure functions */
extern int mf_owner(fb_mnode *, fb_mnode *);
extern int mf_group(fb_mnode *, fb_mnode *);
extern int mf_mode(fb_mnode *, fb_mnode *);
extern int mf_chown(fb_mnode *, fb_mnode *);
extern int mf_chgrp(fb_mnode *, fb_mnode *);
extern int mf_chmod(fb_mnode *, fb_mnode *);

/* FirstBase library interface */
extern int mf_opendb(fb_mnode *, fb_mnode *);
extern int mf_closedb(fb_mnode *, fb_mnode *);
extern int mf_getrec(fb_mnode *, fb_mnode *);
extern int mf_getirec(fb_mnode *, fb_mnode *);
extern int mf_nfields(fb_mnode *, fb_mnode *);
extern int mf_reccnt(fb_mnode *, fb_mnode *);
extern int mf_getxrec(fb_mnode *, fb_mnode *);
extern int mf_recno(fb_mnode *, fb_mnode *);
extern int mf_putrec(fb_mnode *, fb_mnode *);
extern int mf_initrec(fb_mnode *, fb_mnode *);
extern int mf_addrec(fb_mnode *, fb_mnode *);
extern int mf_useidx(fb_mnode *, fb_mnode *);
extern int mf_usrlog(fb_mnode *, fb_mnode *);
extern int mf_nextxrec(fb_mnode *, fb_mnode *);
extern int mf_prevxrec(fb_mnode *, fb_mnode *);
extern int mf_lastxrec(fb_mnode *, fb_mnode *);
extern int mf_firstxrec(fb_mnode *, fb_mnode *);
extern int mf_lock(fb_mnode *, fb_mnode *);
extern int mf_unlock(fb_mnode *, fb_mnode *);
extern int mf_delrec(fb_mnode *, fb_mnode *);
extern int mf_field_name(fb_mnode *, fb_mnode *);
extern int mf_field_type(fb_mnode *, fb_mnode *);
extern int mf_field_default(fb_mnode *, fb_mnode *);
extern int mf_field_size(fb_mnode *, fb_mnode *);
extern int mf_default_dbase(fb_mnode *, fb_mnode *);
extern int mf_set_loadfail(fb_mnode *, fb_mnode *);
extern int mf_get_failrec(fb_mnode *, fb_mnode *);

extern int mf_key(fb_mnode *, fb_mnode *);
extern int mf_countkey(fb_mnode *, fb_mnode *);
extern int mf_findkey(fb_mnode *, fb_mnode *);
extern int mf_rmkey(fb_mnode *, fb_mnode *);

extern int mf_html_blockquote_open(fb_mnode *, fb_mnode *);
extern int mf_html_blockquote_close(fb_mnode *, fb_mnode *);
extern int mf_html_open(fb_mnode *, fb_mnode *);
extern int mf_html_close(fb_mnode *, fb_mnode *);
extern int mf_html_table_open(fb_mnode *, fb_mnode *);
extern int mf_html_table_close(fb_mnode *, fb_mnode *);
extern int mf_html_row_open(fb_mnode *, fb_mnode *);
extern int mf_html_row_close(fb_mnode *, fb_mnode *);
extern int mf_html_cell_open(fb_mnode *, fb_mnode *);
extern int mf_html_cell_close(fb_mnode *, fb_mnode *);
extern int mf_html_table_headers(fb_mnode *, fb_mnode *);

extern int mf_html_h1(fb_mnode *, fb_mnode *);
extern int mf_html_h2(fb_mnode *, fb_mnode *);
extern int mf_html_h3(fb_mnode *, fb_mnode *);
extern int mf_html_h4(fb_mnode *, fb_mnode *);
extern int mf_html_h5(fb_mnode *, fb_mnode *);
extern int mf_html_h6(fb_mnode *, fb_mnode *);

extern int mf_html_bold(fb_mnode *, fb_mnode *);
extern int mf_html_bold_open(fb_mnode *, fb_mnode *);
extern int mf_html_bold_close(fb_mnode *, fb_mnode *);
extern int mf_html_em_open(fb_mnode *, fb_mnode *);
extern int mf_html_em_close(fb_mnode *, fb_mnode *);
extern int mf_html_italics(fb_mnode *, fb_mnode *);
extern int mf_html_italics_open(fb_mnode *, fb_mnode *);
extern int mf_html_italics_close(fb_mnode *, fb_mnode *);
extern int mf_html_strong_open(fb_mnode *, fb_mnode *);
extern int mf_html_strong_close(fb_mnode *, fb_mnode *);
extern int mf_html_p_open(fb_mnode *, fb_mnode *);
extern int mf_html_p_close(fb_mnode *, fb_mnode *);
extern int mf_html_pre_open(fb_mnode *, fb_mnode *);
extern int mf_html_pre_close(fb_mnode *, fb_mnode *);
extern int mf_html_center_open(fb_mnode *, fb_mnode *);
extern int mf_html_center_close(fb_mnode *, fb_mnode *);
extern int mf_html_hr(fb_mnode *, fb_mnode *);
extern int mf_html_br(fb_mnode *, fb_mnode *);
extern int mf_html_comment(fb_mnode *, fb_mnode *);

extern int mf_html_center(fb_mnode *, fb_mnode *);
extern int mf_html_form_open(fb_mnode *, fb_mnode *);
extern int mf_html_form_close(fb_mnode *, fb_mnode *);
extern int mf_html_input(fb_mnode *, fb_mnode *);
extern int mf_html_row(fb_mnode *, fb_mnode *);

extern int mf_html_select(fb_mnode *, fb_mnode *);
extern int mf_html_select_open(fb_mnode *, fb_mnode *);
extern int mf_html_select_close(fb_mnode *, fb_mnode *);
extern int mf_html_select_option(fb_mnode *, fb_mnode *);
extern int mf_html_textarea(fb_mnode *, fb_mnode *);

extern int mf_html_href(fb_mnode *, fb_mnode *);
extern int mf_html_imgsrc(fb_mnode *, fb_mnode *);
extern int mf_html_font_open(fb_mnode *, fb_mnode *);
extern int mf_html_font_close(fb_mnode *, fb_mnode *);
extern int mf_html_fontsize_open(fb_mnode *, fb_mnode *);
extern int mf_html_fontsize_close(fb_mnode *, fb_mnode *);
extern int mf_verify_ascii(fb_mnode *, fb_mnode *);
extern int mf_verify_date(fb_mnode *, fb_mnode *);
extern int mf_verify_dollar(fb_mnode *, fb_mnode *);
extern int mf_verify_float(fb_mnode *, fb_mnode *);
extern int mf_verify_numeric(fb_mnode *, fb_mnode *);
extern int mf_verify_pos_numeric(fb_mnode *, fb_mnode *);


extern int mf_html_dl_open(fb_mnode *, fb_mnode *);
extern int mf_html_dl_close(fb_mnode *, fb_mnode *);
extern int mf_html_dt(fb_mnode *, fb_mnode *);
extern int mf_html_dd(fb_mnode *, fb_mnode *);
extern int mf_html_ul_open(fb_mnode *, fb_mnode *);
extern int mf_html_ul_close(fb_mnode *, fb_mnode *);
extern int mf_html_li(fb_mnode *, fb_mnode *);
extern int mf_html_h_open(fb_mnode *, fb_mnode *);
extern int mf_html_h_close(fb_mnode *, fb_mnode *);
extern int mf_html_ol_open(fb_mnode *, fb_mnode *);
extern int mf_html_ol_close(fb_mnode *, fb_mnode *);
extern int mf_html_filter_lt(fb_mnode *, fb_mnode *);

extern int mf_html_meta(fb_mnode *, fb_mnode *);
extern int mf_html_link(fb_mnode *, fb_mnode *);
extern int mf_html_script_open(fb_mnode *, fb_mnode *);
extern int mf_html_script_close(fb_mnode *, fb_mnode *);

extern int mf_chdir(fb_mnode *, fb_mnode *);
extern int mf_dump_symtab(fb_mnode *, fb_mnode *);
extern int mf_free_globals(fb_mnode *, fb_mnode *);
extern int mf_eval(fb_mnode *, fb_mnode *);
extern int mf_cgi_read(fb_mnode *, fb_mnode *);

/* libmepxr/mac_load.c */
extern int mf_load(fb_mnode *, fb_mnode *);
extern void fb_gcollect_loadnode(void);

#endif
