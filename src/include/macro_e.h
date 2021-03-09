/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: macro_e.h,v 9.0 2001/01/09 02:56:15 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/
/*
 * extern variables header file for macro fields
 */

#include <macro.h>

extern int lineno;			/* for tracing line number */
extern int i_cur;			/* current input location */
extern char *i_ptr;
extern char parsename[];		/* name of current file being parsed */

extern fb_cell **symtab;		/* symbol table */
extern fb_cell **g_symtab;		/* globals symbol table */
extern fb_cell **p_symtab;		/* original parse sym table */
extern fb_mnode *winner;		/* yyparsed code - returned here */
extern fb_mnode *e_winner;		/* code execute code */

extern fb_mnode *n_ghead;		/* mnode garbage head */
extern fb_cell *c_ghead;		/* cell garbage head */

extern int macro_errors;

#if DEBUG
extern int traceflag;			/* debug use only - trace tree */
#endif /* DEBUG */

extern fb_field *m_fstack[];
extern int m_ftop;

extern fb_cell **m_symstack[];
extern int m_sstop;

extern int m_addf_flag;			/* kludge this for m_editfield() */

extern char *m_filenames[];		/* to store file names used */
extern int m_current_filename;		/* point to current file name used */

extern fb_stack_vars main_svars,
   *main_sv;				/* main stack for macro execution */
