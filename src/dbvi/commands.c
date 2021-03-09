/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: commands.c,v 9.0 2001/01/09 02:56:04 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Commands_sid[] = "@(#) $Id: commands.c,v 9.0 2001/01/09 02:56:04 john Exp $";
#endif

#include <dbvi_ext.h>

void step_down();
void step_up();

   cell_right()
      {
         int st = FB_ERROR;

         sput_cursor(0);
         if (col_current->p_next != NULL){
	    if (col_current == col_ptail)
	       col_phead = col_phead->p_next;
	    col_current = col_current->p_next;
	    st = FB_AOK;
	    }
	 return(st);
      }

   cell_left()
      {
         
         int st = FB_ERROR;

         sput_cursor(0);
         if (col_current->p_prev != NULL){
	    if (col_current == col_phead)
	       col_phead = col_phead->p_prev;
	    col_current = col_current->p_prev;
	    st = FB_AOK;
	    }
	 return(st);
      }

   cell_up()
      {
         int st = FB_ERROR;

         sput_cursor(0);
         if (crec_current->c_prev != NULL){
	    if (crec_current == crec_phead){
	       step_up();
	       }
	    crec_current = crec_current->c_prev;
	    st = FB_AOK;
	    }
	 return(st);
      }

   cell_down()
      {
         int st = FB_ERROR;

         sput_cursor(0);
         if (crec_current->c_next != NULL){
	    if (crec_current == crec_ptail){
	       step_down();
	       }
	    crec_current = crec_current->c_next;
	    st = FB_AOK;
	    }
	 return(st);
      }

   void step_down()

      {
         crec *nc;

	 if (crec_ptail == crec_mtail)
	    return;
	 if ((nc = crec_phead->c_next) == NULL)
	    return;
	 crec_phead = nc;
	 if ((nc = crec_ptail->c_next) == NULL)
	    return;
	 crec_ptail = nc;
	 crec_leftcorn = crec_phead;
	 set_screen();
	 fb_move(calc_row + 1, 1);
	 fb_deleteln();
	 fb_move(cdb_t_lines - 1, 1);
	 fb_insertln();
	 put_row(crec_ptail);
	 draw_numbers();
      }

   void step_up()

      {
         crec *nc;

	 if (crec_phead == crec_mhead)
	    return;
	 if ((nc = crec_phead->c_prev) == NULL)
	    return;
	 crec_phead = nc;
	 if ((nc = crec_ptail->c_prev) == NULL)
	    return;
	 crec_ptail = nc;
	 crec_leftcorn = crec_phead;
	 set_screen();
	 fb_move(cdb_t_lines - 1, 1);
	 fb_deleteln();
	 fb_move(calc_row + 1, 1);
	 fb_insertln();
	 put_row(crec_phead);
	 draw_numbers();
      }

/*
 * scroll_down - do line motion in a scrolling manner
 */

   scroll_down(n, cflag)
      int n, cflag;

      {
         int j, reset = 0;
	 crec *errorc;

         if (cflag)			/* if gonna move cursor */
	    errorc = crec_current;
	 else
	    errorc = crec_ptail;
	 if (errorc == crec_mtail)
	    return(FB_ERROR);
	 if (n == 0)
	    n = 1;
         sput_cursor(0);
         for (j = n; j > 0; j--){
	    if (crec_ptail == crec_mtail)
	       break;
	    /* if the window slides past cursor, flag for reset below */
	    if (!cflag && crec_current == crec_phead)
	       reset = 1;
	    step_down();
	    }
	 if (cflag){
	    for (j = n; j > 0; j--){
	       if (crec_current->c_next != NULL)
		  crec_current = crec_current->c_next;
	       else
		  break;
	       }
	    }
	 else if (reset)		/* reset cursor to top of p window */
	    crec_current = crec_phead;
	 return(FB_AOK);
      }

/*
 * line_down - do line motion as a new page
 */

   line_down(n, cflag)
      int n, cflag;

      {
         int j, reset = 0;
	 crec *nc, *errorc;

         if (cflag)			/* if gonna move cursor */
	    errorc = crec_current;
	 else
	    errorc = crec_ptail;
	 if (errorc == crec_mtail)
	    return(FB_ERROR);
	 if (n == 0)
	    n = 1;
         sput_cursor(0);
         for (j = n; j > 0; j--){
	    if (crec_ptail == crec_mtail)
	       break;
	    /* if the window slides past cursor, flag for reset below */
	    if (!cflag && crec_current == crec_phead)
	       reset = 1;
	    nc = crec_phead->c_next;
	    if (nc != NULL)
	       crec_phead = nc;
	    nc = crec_ptail->c_next;
	    if (nc != NULL)
	       crec_ptail = nc;
	    }
	 if (cflag){
	    for (j = n; j > 0; j--){
	       if (crec_current->c_next != NULL)
		  crec_current = crec_current->c_next;
	       else
		  break;
	       }
	    }
	 else if (reset)		/* reset cursor to top of p window */
	    crec_current = crec_phead;
	 return(FB_AOK);
      }

   line_up(n, cflag)
      int n, cflag;

      {
         int j, reset = 0;
	 crec *nc, *errorc;

         if (cflag)			/* if gonna move cursor */
	    errorc = crec_current;
	 else
	    errorc = crec_phead;
	 if (errorc == crec_mhead)
	    return(FB_ERROR);
	 if (n == 0)
	    n = 1;
         sput_cursor(0);
         for (j = n; j > 0; j--){
	    if (crec_phead == crec_mhead)
	       break;
	    /* if the window slides past cursor, flag for reset below */
	    if (!cflag && crec_current == crec_ptail)
	       reset = 1;
	    nc = crec_ptail->c_prev;
	    if (nc != NULL)
	       crec_ptail = nc;
	    nc = crec_phead->c_prev;
	    if (nc != NULL)
	       crec_phead = nc;
	    }
	 if (cflag){
	    for (j = n; j > 0; j--){
	       if (crec_current->c_prev != NULL)
		  crec_current = crec_current->c_prev;
	       else
		  break;
	       }
	    }
	 else if (reset)		/* reset cursor to top of p window */
	    crec_current = crec_ptail;
	 return(FB_AOK);
      }

   scroll_up(n, cflag)
      int n, cflag;

      {
         int j, reset = 0;
	 crec *errorc;

         if (cflag)			/* if gonna move cursor */
	    errorc = crec_current;
	 else
	    errorc = crec_phead;
	 if (errorc == crec_mhead)
	    return(FB_ERROR);
	 if (n == 0)
	    n = 1;
         sput_cursor(0);
         for (j = n; j > 0; j--){
	    if (crec_phead == crec_mhead)
	       break;
	    /* if the window slides past cursor, flag for reset below */
	    if (!cflag && crec_current == crec_ptail)
	       reset = 1;
	    step_up();
	    }
	 if (cflag){
	    for (j = n; j > 0; j--){
	       if (crec_current->c_prev != NULL)
		  crec_current = crec_current->c_prev;
	       else
		  break;
	       }
	    }
	 else if (reset)		/* reset cursor to top of p window */
	    crec_current = crec_ptail;
	 return(FB_AOK);
      }

   genstat(type)
      int type;

      {
         int i, count;
	 crec *c;
	 char buffer[FB_MAXLINE];

         i = count = 0;
	 for (c = crec_mhead; c != NULL; c = c->c_next){
	    count++;
	    if (c == crec_current)
	       i = count;
	    }
	 if (type == 0 || (type == -1 && cdb_db->reccnt > 0))
	    sprintf(buffer,"\"%s\" %d records",cdb_db->dbase,count);
	 else if (type == 1)
	    sprintf(buffer,"\"%s\" record %d of %d",cdb_db->dbase,i,count);
	 else if (type == -1 && cdb_db->reccnt == 0)
	    sprintf(buffer,"\"%s\" [New file]",cdb_db->dbase);
	 fb_move(cdb_t_lines, 1);
	 fb_reverse(buffer);
	 clear_lastline = 1;
      }
