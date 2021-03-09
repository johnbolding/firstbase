/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: makedef.c,v 9.0 2001/01/09 02:56:06 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Makedef_sid[] = "@(#) $Id: makedef.c,v 9.0 2001/01/09 02:56:06 john Exp $";
#endif

#include <dbvi_ext.h>

static char *FB_LONGDIGIT = "%ld";

extern char *cdb_T_DATE;
extern char *cdb_T_TIME;
extern char *cdb_T_MTIME;
extern char *cdb_T_INCR;
extern char *cdb_T_PREV;
extern char *cdb_T_NPREV;
extern char *cdb_T_USER;
extern char *cdb_user;

static bumpmonth();
static get_lastday();
static incrdate();

/*
 * makedef - make a FirstBase default string.
 */

   makedef(inp, f)
      char *inp;
      fb_field *f;

      {
         char *line, tbuffer[FB_MAXNAME];
	 crec *c;
	 
	 line = cdb_bfld;
	 if (equal(f->idefault, cdb_T_DATE))
	    strcpy(inp, fb_simpledate(tbuffer, 1));
	 else if (equal(f->idefault, cdb_T_TIME))
	    strcpy(inp, fb_simpledate(tbuffer, 0));
	 else if (equal(f->idefault, cdb_T_MTIME))
	    strcpy(inp, fb_simpledate(tbuffer, 2));
	 else if (strncmp(f->idefault, cdb_T_INCR, 5) == 0){/* 5=len(t_incr) */
	    if (f->type ==FB_DATE){
	       incrdate(inp, f);
	       }
	    else{
	       line[0] = NULL;
	       if ((c = crec_current->c_prev) != NULL)	/* pull prev fb_cell */
		  strcpy(line, c->c_cell[col_current->p_array]);
/*
* does this do anything?
*
*	       else if (f->incr != 0)
*	          strcpy(line, f->incr);
*/
	       if (line[0] != NULL){
		  sprintf(line, FB_FDIGITS, ++(f->incr));
		  fb_rjustify(inp, line, f->size, f->type); /* FB_NUMERIC */
		  }
	       }
	    }
	 else if (equal(f->idefault, cdb_T_PREV)){
	    if ((c = crec_current->c_prev) != NULL)	/* pull prev fb_cell */
	       strcpy(inp, c->c_cell[col_current->p_array]);
	    else if (f->prev != NULL)
	       strcpy(inp, f->prev);
	    }
	 else if (equal(f->idefault, cdb_T_NPREV)){
	    line[0] = NULL;
	    if ((c = crec_current->c_prev) != NULL)	/* pull prev fb_cell */
	       strcpy(line, c->c_cell[col_current->p_array]);
	    else if (f->prev != NULL)
	       strcpy(line, f->prev);
	    if (line[0] != NULL)
	       sprintf(inp, FB_LONGDIGIT, -atol(line));
	    }
	 else if (equal(f->idefault, cdb_T_USER))
	    strcpy(inp, cdb_user);
	 else
	    strcpy(inp, f->idefault);
      }

/*
 * incrdate - increment a Cdb date
 */

   static incrdate(inp, f)
      char *inp;
      fb_field *f;

      {
         char pv[20];
	 crec *c;
	 int type;
	 
	 pv[0] = NULL;
	 if ((c = crec_current->c_prev) != NULL)	/* pull prev fb_cell */
	    strcpy(pv, c->c_cell[col_current->p_array]);
/*
* - what is this?
*	 else if (f->incr != NULL)
*	    strcpy(pv, f->incr);
*/
	 type = atoi(f->idefault + 5);	/* type is 0, 1, 3, 6, 12 */
	 bumpmonth(inp, pv, type);
      }

   static bumpmonth(inp, pv, count)
      char *inp, *pv;
      int count;
      
      {
         int m = 1, d = 1, y = 1, extra_y = 0, lastday;
	 char dline[10];
	 
	 if (strlen(pv) == 6){
	    dline[2] = 0;
	    dline[0] = pv[0]; dline[1] = pv[1]; m = atoi(dline);
	    dline[0] = pv[2]; dline[1] = pv[3]; d = atoi(dline);
	    dline[0] = pv[4]; dline[1] = pv[5]; y = atoi(dline);
	    }
	 if (d >= 28 && get_lastday(m,y) == d)
	    lastday = 1;
	 else
	    lastday = 0;

	 m += count;
	 for (; m > 12; ){
	    extra_y++;
	    m -= 12;
	    }
	 y += extra_y;
	 if (y > 99)
	    y = 0;
	 /* now if date is bad, fix the day */
	 if (d == 31 && (m == 4 || m == 6 || m ==9 || m == 11))
	    d = 30;
	 else if (m == 2 && d > 28){
	    if (y%4 == 0 && y%100 != 0 || y%400 == 0){
	       if (d > 29)
		  d = 29;
	       }
	    else if (d > 28)
	       d = 28;
	    }
	 if (lastday)
	    d = get_lastday(m, y);
	 sprintf(inp, "%02d%02d%02d", m, d, y);
      }

/*
 * get_lastday - get the last day of the month/year.
 */

   static get_lastday(m, y)
      int m, y;
      
      {
         int d;

	 if (m==1 || m==3 || m==5 || m==7 || m==8 || m==10 || m==12)
	    d = 31;
	 else if (m == 4 || m == 6 || m == 9 || m == 11)
	    d = 30;
	 else if (m == 2){
	    if (y%4 == 0 && y%100 != 0 || y%400 == 0)
	       d = 29;
	    else
	       d = 28;
	    }
	 return(d);
      }

/*
 * setdef - setdef is called with the fb_field fb_input value to set up defaults
 *	for any subsequent default requests.
 */

   setdef(inp, f)
      char *inp;
      fb_field *f;

      {
         char *line;
	 
	 line = cdb_bfld;
	 if (equal(f->idefault, cdb_T_INCR))
	    f->incr = atoi(inp);
	 else if (equal(f->idefault, cdb_T_PREV) ||
		  equal(f->idefault, cdb_T_NPREV)){
	    if (f->prev != NULL)
	       fb_free(f->prev);
	    strcpy(line, inp);
	    fb_trim(line);
	    f->prev = (char *) fb_malloc(strlen(line) + 1);
	    strcpy(f->prev, line);
	    }
      }
