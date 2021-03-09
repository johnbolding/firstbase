/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: macro_v.h,v 9.0 2001/01/09 02:56:15 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/
/*
 * variables header file for macro fields
 */

#include <macro.h>

int lineno = 1;				/* for tracing line number */
int i_cur;				/* current input location */
char *i_ptr;
char parsename[FB_MAXLINE];		/* name of current file being parsed */

fb_cell **symtab = NULL;		/* symbol table */
fb_cell **g_symtab = NULL;		/* globals symbol table */
fb_cell **p_symtab = NULL;		/* original parse symtab */
fb_mnode *winner = NULL;		/* yyparsed code - returned here */
fb_mnode *e_winner = NULL;		/* code execute code */

fb_mnode *n_ghead = NULL;		/* mnode garbage head */
fb_cell *c_ghead = NULL;		/* cell garbage head */

int macro_errors = 0;

#if DEBUG
int traceflag = 0;			/* debug use only - trace tree */
#endif /* DEBUG */

/* execution stuff */

fb_field *m_fstack[MAX_FSTACK];
int m_ftop = 0;

fb_cell **m_symstack[MAX_SYMSTACK];
int m_sstop = 0;

int m_addf_flag;			/* kludge this for m_editfield() */

char *m_filenames[MAX_MFILES];		/* to store file names used */
int m_current_filename = 0;		/* point to current file name used */

fb_stack_vars main_svars,
   *main_sv = &main_svars;		/* main stack for macro execution */
