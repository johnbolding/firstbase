/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_func.c,v 9.3 2001/09/29 18:07:59 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macro_function_sid[] = "@(#) $Id: mac_func.c,v 9.3 2001/09/29 18:07:59 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

extern short int cdb_m_addf_flag;	/* kludge this for m_editfield() */
extern short int cdb_e_st;
extern short int cdb_edit_input;
extern char *cdb_e_buf;
extern short int cdb_st_up;
extern char *cdb_pgm;
extern short int cdb_cgi_flag;

typedef struct sf_array f_array;

#if !FB_PROTOTYPES
static f_array *mf_find_builtin();
static copy_arguments();
struct sf_array {
   char *f_fname;
   int (*f_func)();
   };
#else /* FB_PROTOTYPES */
static f_array *mf_find_builtin(char *);
static copy_arguments(fb_mnode *, fb_mnode *, char *, fb_stack_vars *);
struct sf_array {
   char *f_fname;
   int (*f_func)(fb_mnode *, fb_mnode *);
   };
#endif /* FB_PROTOTYPES */

#if BAD_ATOF
#undef atof
#endif


/* sort these for the binary search */
f_array builtin[] = {
   "abs",	mf_abs,
   "access",	mf_access,
   "acos",	mf_acos,
   "addrec",	mf_addrec,
   "asin",	mf_asin,
   "atan",	mf_atan,
   "atan2",	mf_atan2,
   "atof",	mf_ston,
   "bell",	mf_bell,
   "cdbdate",	mf_cdbdate,
   "cdbdts",	mf_cdbdts,
   "ceil",	mf_ceil,
   "cgi_read",	mf_cgi_read,
   "chdir",	mf_chdir,
   "checkfields", mf_checkfields,
   "chmod",	mf_chmod,
   "clear",	mf_clear,
   "closedb",	mf_closedb,
   "clrtobot",	mf_clrtobot,
   "clrtoeol",	mf_clrtoeol,
   "cos",	mf_cos,
   "countkey",	mf_countkey,
   "countlines",mf_countlines,
   "creat",	mf_creat,
   "create",	mf_creat,
   "crypt",	mf_crypt,
   "date",	mf_date,
   "day",	mf_day,
   "default_dbase", mf_default_dbase,
   "delrec",	mf_delrec,
   "dts",	mf_dts,
   "dump_symtab", mf_dump_symtab,
   "editfield",	mf_editfield,
   "error",	mf_error,
   "eval",	mf_eval,
   "exp",	mf_exp,
   "fclose",	mf_fclose,
   "fflush",	mf_fflush,
   "fgetrec",	mf_fgetrec,
   "fgets",	mf_fgets,
   "field_default",mf_field_default,
   "field_name",mf_field_name,
   "field_size",mf_field_size,
   "field_type",mf_field_type,
   "findkey",	mf_findkey,
   "firstxrec",	mf_firstxrec,
   "floor",	mf_floor,
   "fmt",	mf_fmt,
   "footer",	mf_footer,
   "fopen",	mf_fopen,
   "fork",	mf_fork,
   "formfield",	mf_formfield,
   "fprintf",	mf_fprintf,
   "fputs",	mf_fputs,
   "fread",	mf_fread,
   "free_globals", mf_free_globals,
   "fseek",	mf_fseek,
   "fwrite",	mf_fwrite,
   "get_failrec", mf_get_failrec,
   "getenv",	mf_getenv,
   "getirec",	mf_getirec,
   "getrec",	mf_getrec,
   "gets",	mf_gets,
   "gettoken",	mf_gettoken,
   "getword",	mf_getword,
   "getxrec",	mf_getxrec,
   "header",	mf_header,
   "hour",	mf_hour,

   "html_blockquote_close", mf_html_blockquote_close,
   "html_blockquote_open",  mf_html_blockquote_open,
   "html_bold",		mf_html_bold,
   "html_bold_close",	mf_html_bold_close,
   "html_bold_open",	mf_html_bold_open,
   "html_br",		mf_html_br,
   "html_cell_close", 	mf_html_cell_close,
   "html_cell_open",	mf_html_cell_open,
   "html_center",	mf_html_center,
   "html_center_close",	mf_html_center_close,
   "html_center_open",	mf_html_center_open,
   "html_close",	mf_html_close,
   "html_comment",	mf_html_comment,
   "html_dd", 		mf_html_dd,
   "html_dl_close", 	mf_html_dl_close,
   "html_dl_open", 	mf_html_dl_open,
   "html_dt", 		mf_html_dt,
   "html_em_close",	mf_html_em_close,
   "html_em_open",	mf_html_em_open,
   "html_filter_lt",	mf_html_filter_lt,
   "html_font_close",	mf_html_font_close,
   "html_font_color",	mf_html_font_color,
   "html_font_open",	mf_html_font_open,
   "html_fontsize_close",mf_html_fontsize_close,
   "html_fontsize_open",mf_html_fontsize_open,
   "html_form_close",	mf_html_form_close,
   "html_form_open",	mf_html_form_open,
   "html_h1",		mf_html_h1,
   "html_h2",		mf_html_h2,
   "html_h3",		mf_html_h3,
   "html_h4",		mf_html_h4,
   "html_h5",		mf_html_h5,
   "html_h6",		mf_html_h6,
   "html_h_close",	mf_html_h_close,
   "html_h_open",	mf_html_h_open,
   "html_hr",		mf_html_hr,
   "html_href",		mf_html_href,
   "html_imgsrc",	mf_html_imgsrc,
   "html_input",	mf_html_input,
   "html_italics",	mf_html_italics,
   "html_italics_close",mf_html_italics_close,
   "html_italics_open",	mf_html_italics_open,
   "html_li", 		mf_html_li,
   "html_link",		mf_html_link,
   "html_meta",		mf_html_meta,
   "html_ol_close", 	mf_html_ol_close,
   "html_ol_open", 	mf_html_ol_open,
   "html_open", 	mf_html_open,
   "html_p_close", 	mf_html_p_close,
   "html_p_open", 	mf_html_p_open,
   "html_pre_close", 	mf_html_pre_close,
   "html_pre_open", 	mf_html_pre_open,
   "html_row",		mf_html_row,
   "html_row_close",	mf_html_row_close,
   "html_row_open", 	mf_html_row_open,
   "html_script_close",	mf_html_script_close,
   "html_script_open",	mf_html_script_open,
   "html_select",	mf_html_select,
   "html_select_close",	mf_html_select_close,
   "html_select_open",	mf_html_select_open,
   "html_select_option",mf_html_select_option,
   "html_strong_close",	mf_html_strong_close,
   "html_strong_open",	mf_html_strong_open,
   "html_table_close",	mf_html_table_close,
   "html_table_headers",mf_html_table_headers,
   "html_table_open", 	mf_html_table_open,
   "html_textarea", 	mf_html_textarea,
   "html_ul_close", 	mf_html_ul_close,
   "html_ul_open", 	mf_html_ul_open,

   "hypot",	mf_hypot,
   "in",	mf_in,
   "index",	mf_strchr,
   "initrec",	mf_initrec,
   "input",	mf_input,
   "ireccnt",	mf_ireccnt,
   "key",	mf_key,
   "lastxrec",	mf_lastxrec,
   "length",	mf_length,
   "link",	mf_link,
   "load",	mf_load,
   "lock",	mf_lock,
   "log",	mf_log,
   "lower",	mf_lower,
   "makess",	mf_makess,
   "max",	mf_max,
   "min",	mf_min,
   "minute",	mf_minute,
   "mkstemp",	mf_mkstemp,
   "mktemp",	mf_mktemp,
   "month",	mf_month,
   "move",	mf_move,
   "ndays",	mf_ndays,
   "newdate",	mf_newdate,
   "nextxrec",	mf_nextxrec,
   "nfields",	mf_nfields,
   "now",	mf_now,
   "opendb",	mf_opendb,
   "pattern",	mf_pattern,
   "pattern_comp",	mf_pattern_comp,
   "pattern_eo",	mf_pattern_eo,
   "pattern_exec",	mf_pattern_exec,
   "pattern_icase",	mf_pattern_icase,
   "pattern_so",	mf_pattern_so,
   "pattern_substr",	mf_pattern_substr,
   "pause",	mf_pause,
   "pclose",	mf_pclose,
   "popen",	mf_popen,
   "pow",	mf_pow,
   "prevxrec",	mf_prevxrec,
   "print",	mf_print,
   "printf",	mf_printf,
   "putenv",	mf_putenv,
   "putrec",	mf_putrec,
   "puts",	mf_puts,
   "r_chgrp",	mf_r_chgrp,
   "r_chmod",	mf_r_chmod,
   "r_chown",	mf_r_chown,
   "r_group",	mf_r_group,
   "r_mode",	mf_r_mode,
   "r_owner",	mf_r_owner,
   "random",	mf_random,
   "reccnt",	mf_reccnt,
   "recno",	mf_recno,
   "redraw",	mf_redraw,
   "refresh",	mf_refresh,
   "rename",	mf_rename,
   "reverse",	mf_reverse,
   "rindex",	mf_strrchr,
   "rinput",	mf_rinput,
   "rmkey",	mf_rmkey,
   "rmlead",	mf_rmlead,
   "rmlinefeed",mf_rmlinefeed,
   "rmnewline",	mf_rmnewline,
   "rmunderscore", mf_rmunderscore,
   "round",	mf_round,
   "second",	mf_second,
   "set_loadfail", mf_set_loadfail,
   "sin",	mf_sin,
   "sleep",	mf_sleep,
   "sprintf",	mf_sprintf,
   "sqrt",	mf_sqrt,
   "srandom",	mf_srandom,
   "standout",	mf_standout,
   "status",	mf_status,
   "ston",	mf_ston,
   "strchr",	mf_strchr,
   "strrchr",	mf_strrchr,
   "subline",	mf_subline,
   "substr",	mf_substr,
   "symlink",	mf_symlink,
   "system",	mf_system,
   "tan",	mf_tan,
   "terminate",	mf_terminate,
   "trim",	mf_trim,
   "trunc",	mf_trunc,
   "tts",	mf_tts,
   "unlink",	mf_unlink,
   "unlock",	mf_unlock,
   "upper",	mf_upper,
   "useidx",	mf_useidx,
   "usrlog",	mf_usrlog,
   "verify_ascii", 	mf_verify_ascii,
   "verify_date", 	mf_verify_date,
   "verify_dollar",	mf_verify_dollar,
   "verify_float",	mf_verify_float,
   "verify_numeric",	mf_verify_numeric,
   "verify_pos_numeric",mf_verify_pos_numeric,
   "year",	mf_year,
   0, 0
};

#define F_ARRAY_MAX 237		/* f_array_builtin[F_ARRAY_MAX] == 0 */

/*
 * macro_function - provide the interface to the builtin functions
 */

char macrotrace_filename[FB_MAXNAME];

   double macro_function(n)
      fb_mnode *n;

      {
         fb_cell *c;
         fb_mnode *na, *f_exec, *f_call, *f_top, *nn;
         char *func, buffer[FB_MAXLINE];
         f_array *f;
         struct fb_s_stack_vars *sv;

         (void) Macro_function_sid;

         na = n->n_narg[0];
         c = (fb_cell *) na->n_obj;
         func = c->c_sval;

         /* need to eval all arguments here via macro_expr */
         for (na = n->n_narg[1]; na != NULL; na = na->n_next)
            macro_expr(na);

         /* search the builtins - execute matches */
         f = mf_find_builtin(func);
         n->n_fval = 0;
         if (f != NULL){
            if (equal(func, "opendb")){
               if ((*(f->f_func))(n->n_narg[1], n) == FB_ERROR){
                  sprintf(buffer, "%s (near line %d of file %s)", func,
                     n->n_lineno, n->n_fname);
                  fb_serror(FB_MESSAGE, "Function call failed:", buffer);
                  n->n_fval = FB_ERROR;
                  }
               }
            else if ((*(f->f_func))(n->n_narg[1], n) == FB_ERROR){
                  sprintf(buffer, "%s (near line %d of file %s)", func,
                     n->n_lineno, n->n_fname);
                  fb_serror(FB_MESSAGE, "Function call failed:", buffer);
                  n->n_fval = FB_ERROR;
                  }
            }
         else if ((f_top = locate_function(func)) != (fb_mnode *) NULL){
            /*
             * set up an execution frame, new symtab, etc.
             * copy in arguments, execute function f_exec
             */
            sv = (fb_stack_vars *) fb_malloc(sizeof(struct fb_s_stack_vars));
            mf_init_stack(sv);
            mf_make_frame(sv);
            f_call = f_top->n_narg[0];
            f_exec = f_top->n_narg[1];
            m_verify_sub(f_call);
            copy_arguments(f_call, n->n_narg[1], func, sv);
            destroy_list(f_exec);
            m_verify_sub(f_exec);
            macro_statement(f_exec, sv);
            if (sv->break_flag)
               fb_xerror(FB_MESSAGE, "Illegal `break'", NIL);
            if (sv->continue_flag)
               fb_xerror(FB_MESSAGE, "Illegal `continue'", NIL);
            /* this destroys most things, but not the return_list */
            mf_destroy_frame(sv);
            if (sv->return_flag){
               destroy_list(n);
               n->n_list = sv->s_return_list;
               sv->s_return_list = NULL;
               /* set the type of the return node */
               nn = n->n_list;
               if (nn != NULL){
                  n->n_fval = nn->n_fval;
                  if (istype_str(nn)){
                     fb_mkstr(&(n->n_nval), nn->n_nval);
                     n->n_tval |= T_STR;
                     }
                  if (istype_arr(nn))
                     n->n_tval |= T_ARR;
                  }
               }
            m_destroy_call_lists(f_exec);
            fb_free((char *) sv);
            }
         else{
            mf_perror("Unknown Macro Function", func, n);
            n->n_fval = FB_ERROR;
            fb_mkstr(&(n->n_nval), "-1");
            }
         return(n->n_fval);
      }

/*
 * mf_find_builtin - find a builtin function via binary search on builtin
 */

   static f_array *mf_find_builtin(func)
      char *func;

      {
         int top, bot, mid, omid, st, diff;
         f_array *f;

         top = 0;
         bot = F_ARRAY_MAX;
         omid = 0;
         for (;;){
            diff = bot - top;
            mid = top + (diff / 2);
            if (mid == omid || top > bot)
               break;
            f = &(builtin[mid]);
            st = strcmp(func, f->f_fname);
            if (st == 0)
               return(f);
            else if (st > 0)
               top = mid;
            else
               bot = mid;
            omid = mid;
            }
         return((f_array *) NULL);
      }

/*
 * copy_arguments - copy arguments from n to c. by value.
 *	i.e. this should emulate code of c1 = n1, c2 = n2;
 */

   static copy_arguments(c, n, func, sv)
      fb_mnode *c, *n;
      char *func;
      fb_stack_vars *sv;

      {
         fb_mnode *cn, *cp, *np;

         /* clear all the parameters -- these are local to this function */
         if (c->n_type != O_CALL){
            fb_serror(FB_MESSAGE, "Illegal function declaration:", func);
            return(FB_ERROR);
            }
         /* clear the type of the callee parameters */
         for (cp = c->n_narg[1]; cp != NULL; cp = cp->n_next){
            if (cp->n_type == S_NULL)
               break;
            if (cp->n_type != V_ID){
               fb_serror(FB_MESSAGE, "Illegal parameter in funcion:", func);
               return(FB_ERROR);
               }
            cn = mnode_to_local_var(cp);
            cn->n_fval = 0;
            cn->n_scale = 0;
            cn->n_tval = 0;
            }

         np = n;
         for (cp = c->n_narg[1]; cp != NULL && np != NULL; cp = cp->n_next){
            if (cp->n_type == S_NULL)
               break;
            if (istype_arr(np) && !istype_fld(np) && np->n_narg[0] == NULL){
               /* must be the special encoded array via a variable */
               assign_copy_array(cp, np, sv->s_symtab);
               }
            else if (istype_arr(np) && !istype_fld(np) && np->n_type ==O_CALL){
               /* must be the special encoded array via a function call */
               assign_copy_array(cp, np, sv->s_symtab);
               }
            else{
               /* else a normal argument */
               cn = mnode_to_local_var(cp);
               cn->n_fval = np->n_fval;
               cn->n_scale = np->n_scale;
               if (istype_num(np))
                  cn->n_tval |= T_NUM;
               if (istype_str(np)){
                  fb_mkstr(&(cn->n_nval), np->n_nval);
                  cn->n_tval |= T_STR;
                  }
               }
            np = np->n_next;
            }
         return(FB_AOK);
      }

   void destroy_list(n)
      fb_mnode *n;

      {
         fb_mnode *sn, *nx, *nn;

         if (n == NULL || n->n_list == NULL)
            return;
         /* destroy the old list that was here */
         for (sn = n->n_list; sn != NULL; sn = nn){
            /* check for next node, an encoded array */
            for (nx = sn->n_next; nx != NULL; nx = nn){
               if (nx->n_list != NULL)
                  fb_freenode(nx->n_list);
               nn = nx->n_next;
               fb_freenode(nx);
               }
            nn = sn->n_list;
            fb_freenode(sn);
            }
         n->n_list = NULL;
      }

/*
 * more mf style functions are below this point
 */

   mf_sleep(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         sleep((unsigned) n->n_fval);
         return(FB_AOK);
      }

   mf_system(n, r)
      fb_mnode *n, *r;

      {
         int eval;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         fb_cx_set_toolname("NOTOOL");
         fb_cx_push_env("T", CX_NO_SELECT, NIL);
         fb_cx_write(1);		/* since no fb_input() is used */
         eval = fb_system(tostring(n), FB_NOROOT);
         fb_cx_set_toolname(cdb_pgm);
         /*
          * cx_ddict stuff should be set here, like in custom.c
          * but it does not need to be since what is in this mem
          * will be rewritten next time. wild.
          */
         mf_header(n, r);
         mf_redraw(n, r);
         mf_footer(n, r);
         r->n_tval |= T_NUM;
         r->n_fval = eval;
         return(FB_AOK);
      }

   mf_getenv(n, r)
      fb_mnode *n, *r;

      {
         char *p;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         tostring(n);
         p = getenv(n->n_pval);
         if (p != NULL){
            r->n_tval |= T_STR;
            fb_mkstr(&(r->n_nval), p);
            }
         else{
            r->n_tval |= T_NUM;
            r->n_fval = 0;
            }
         return(FB_AOK);
      }

   mf_putenv(n, r)
      fb_mnode *n, *r;

      {
         char *buf, *ebuf;
         int len;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         buf = tostring(n);		n = n->n_next;
         len = strlen(buf);
         ebuf = fb_malloc((unsigned) len + 1);
         strcpy(ebuf, buf);
         r->n_fval = putenv(ebuf);
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_fork(n, r)
      fb_mnode *n, *r;

      {
         int val;

         val = fork();
         if (val != 0)
            signal(SIGCHLD, SIG_IGN);
         else
            cdb_batchmode = 1;
         r->n_fval = val;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_terminate(n, r)
      fb_mnode *n, *r;

      {
         int val;

         val = n->n_fval;
         exit(val);
      }

   mf_chdir(n, r)
      fb_mnode *n, *r;

      {
         int st;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         tostring(n);
         st = chdir(n->n_pval);
         r->n_fval = st;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_dump_symtab(n, r)
      fb_mnode *n, *r;

      {
         fb_dump_symtab();
         r->n_fval = FB_AOK;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_free_globals(n, r)
      fb_mnode *n, *r;

      {
         fb_free_globals();
         r->n_fval = FB_AOK;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_eval(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *vn;
         fb_mnode t;
         char buf[FB_MAXLINE], *p;
         int ufield = 0;
         fb_field *f;

         tostring(n);
         p = n->n_pval;
         if (*p == CHAR_DOLLAR){
            p++;
            ufield = 1;
            fb_mkstr(&(n->n_nval),  p);
            n->n_tval |= T_FLD;
            f = mnode_to_field(n);
            eval_field(f, cdb_db, n);
            vn = n;
            }
         else{
            strcpy(buf, p);
            vn = string_to_var(buf);
            }
         /*
          * arrive at a node that is for this variable.
          * tostring it and copy in all its pieces
          */
         tostring(vn);
         r->n_fval = vn->n_fval;
         r->n_scale = vn->n_scale;
         fb_mkstr(&(r->n_nval), vn->n_nval);
         if (istype_str(vn)){
            r->n_tval |= T_STR;
            }
         return(FB_AOK);
      }

/*
 * write an error message to the USRLOG file for firstbase
 */

   mf_usrlog(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         fb_usrlog_msg(tostring(n));
         return(FB_AOK);
      }

/*
 * mf_cgi_read - call fb_cgi_read() to handle all macro-to-cgi vars
 */

   mf_cgi_read(n, r)
      fb_mnode *n, *r;

      {
         char path_dir[FB_MAXLINE], path_trans[FB_MAXLINE], *p;
         
         if (!cdb_cgi_flag){
            /* PATH_TRANSLATED is the cgi fname */
            path_dir[0] = NULL;
            p = getenv("PATH_TRANSLATED");
            if (p){
               strcpy(path_trans, p);
               fb_dirname(path_dir, path_trans);
               mf_set_value("PATH_TRANSLATED", path_trans);
               mf_set_value("path_translated", path_trans);
               mf_set_value("PATH_DIR", path_dir);
               mf_set_value("path_dir", path_dir);
               }
            fb_cgi_read();
            fb_cgi_foreach(mf_set_value);
            fb_cgi_checkbox(mf_set_checkbox);
            }
         r->n_fval = FB_AOK;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }
