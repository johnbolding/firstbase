/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: mdict.h,v 9.0 2001/01/09 02:56:15 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* global definitions for mdict (merge dictionary) items go here */

#ifndef FB_MDICT_H
#define FB_MDICT_H

/* tokens for merge files and for editable visuals of vdict files */
typedef struct fb_s_token fb_token;
struct fb_s_token {
   fb_field *t_field;
   int t_width;
   fb_token *t_next;
   fb_token *t_prev;

   /* these are for the dbdview(1) additioanl interface information */
   char *t_text;		/* pointer to text */
   short int t_sub1;		/* subscript 1 */
   short int t_sub2;		/* subscript 2 */
   short int t_reverse;		/* reverse flag */
   short int t_readonly;	/* read only flag */
   short int t_type;		/* display type of node */
   };

/* lines of text for merge files, etc */
typedef struct fb_s_aline fb_aline;
struct fb_s_aline {
   char *a_text;
   fb_token *a_thead;
   fb_token *a_ttail;
   fb_aline *a_next;
   fb_aline *a_prev;
   short int a_lineno;
   };

/* pages of lines of text for merge files, etc */
typedef struct fb_s_mpage fb_mpage;
struct fb_s_mpage {
   short mp_num;		/* merge page number */
   short mp_row;
   short mp_col;
   short mp_leftcorn;		/* left corner column of display */
   short mp_rightcorn;		/* right corner column of display */
   fb_aline *mp_ahead;		/* head of aline list for this page */
   fb_aline *mp_atail;		/* tail of aline list fot his page */
   fb_aline *mp_atop;		/* dispay top of this page */
   fb_aline *mp_abot;		/* dispay bottom of this page */
   fb_aline *mp_acur;		/* current line for this page */
   fb_mpage *mp_next;		/* next page */
   fb_mpage *mp_prev;		/* previous page */
   };

#endif /* FB_MDICT_H */
