/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: vdict.h,v 9.0 2001/01/09 02:56:17 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/* global definitions for vdict items go here */

#ifndef FB_VDICT_H
#define FB_VDICT_H

/* node display codes */
#define T_SUBSTRING_R	3
#define T_SUBSTRING_N	4
#define T_SUBELEMENT_R	7
#define T_SUBELEMENT_N	8

typedef struct fb_s_node fb_node;

struct fb_s_node {
   char *n_text;		/* pointer to text */
   fb_field *n_fp;		/* pointer to field */
   int  n_sub1;			/* subscript 1 */
   int  n_sub2;			/* subscript 2 */
   int  n_row;			/* display row */
   int  n_col;			/* display column */
   int n_len;			/* length of display */
   fb_node *n_next;		/* next node */
   fb_node *n_prev;		/* previous node */
   short n_reverse;		/* reverse flag */
   short n_readonly;		/* read only flag */
   short n_type;		/* display type of node */
   };

typedef struct fb_s_page fb_page;

struct fb_s_page {
   int p_num;			/* page number */
   fb_node *p_nhead;		/* top node of page */
   fb_node *p_ntail;		/* bottom node of page */
   fb_node **p_nedit;		/* dynamic array of editable nodes */
   int p_maxedit;		/* max number of editable field on page */
   fb_page *p_next;		/* next page */
   fb_page *p_prev;		/* previous page */
   char *p_help;		/* help file on this page */
   };

typedef struct fb_s_cnode fb_cnode;/* choice node definitions */
struct fb_s_cnode {
   char *c_label;
   char *c_meaning;
   int c_row;
   int c_col;
   fb_cnode *c_next;
   };

#define NFILTERS	30
#define NDISPLAYS	30

typedef struct fb_s_exchoice fb_exchoice; /* extended choice node */
struct fb_s_exchoice {
   fb_database *ex_db;
   fb_field *ex_filters[NFILTERS];
   fb_field *ex_displays[NDISPLAYS];
   fb_field *ex_return;
   char *ex_help;
   char *ex_prompt;
   long ex_first;
   long ex_last;
   long ex_ptop;
   long ex_pbot;
   int ex_firstrow;
   int ex_lastrow;

   /*
    * when a btree is used, the above numbers are used to save the
    * particular node number that is used to get a <key,key,key> triple.
    * the values below are used to store the curkey value for each node.
    */

   int ex_tree;
   int ex_first_curkey;
   int ex_last_curkey;
   int ex_ptop_curkey;
   int ex_pbot_curkey;
   };

#endif /* FB_VDICT_H */
