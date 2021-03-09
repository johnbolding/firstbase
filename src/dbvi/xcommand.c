/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: xcommand.c,v 9.1 2001/02/16 19:50:11 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Xcommands_sid[] = "@(#) $Id: xcommand.c,v 9.1 2001/02/16 19:50:11 john Exp $";
#endif

#include <dbvi_ext.h>

static crec *killbuffer = NULL;
static char *X_SET = "set";
static char *X_DATE = "date";

static xcommand();
static set_command();

extern char *cdb_T_INCR;

/*
 * colon_command - do the colon command set for dbvi.
 */

   colon_command()
      {
         char com, buffer[FB_MAXLINE];
	 int i, st = FB_AOK;

         sput_cursor(0);
	 fb_move(cdb_t_lines, 1);
	 fb_s_putw(CHAR_COLON);
	 clear_lastline = 1;
	 for (i = 0; i < 80; i++){
	    fb_refresh();
	    com = getchar();
	    if (com == FB_NEWLINE || com == FB_CRET)
	       break;
	    if (!isprint(com))
	       return(FB_ERROR);
	    fb_s_putw(com);
	    buffer[i] = com;
	    }
	 buffer[i] = NULL;
	 if (i > 0)
	    st = xcommand(buffer);
	 return(st);
      }

/*
 * xcommand - the work part of the colon command set
 */

   static xcommand(buf)
      char *buf;
      
      {
         int st = FB_ERROR;
	 char word[FB_MAXLINE];

         if (isdigit(*buf)){
	    return(line_goto(atoi(buf)));
	    }
	 fb_getword(buf, 1, word);
	 if (equal(word, X_SET))
	    return(set_command(buf));
         switch(*buf++){
	    case 'w':
	       if (*buf == 'q')
	          st = FB_END;
	       else
	          st = FB_AOK;
	       writefile();
	       genstat(0);
	       modified = 0;
	       break;
	    case 'q':
	       if (modified && *buf != '!'){
	          fb_move(cdb_t_lines, 1);
		  fb_reverse("No write since last change (:q! overrides)");
		  clear_lastline = 1;
		  st = FB_AOK;
		  }
	       else
	          st = FB_END;
	       break;
	    }
	 return(st);
      }

/*
 * set_command - implement set commands - used from colon_command.
 */
 
   static set_command(buf)
      char *buf;

      {
         char e_var[FB_MAXLINE], word[FB_MAXLINE];
	 int iv, st = FB_ERROR, j;
	 column *p;
	 fb_field *f;
	 
	 fb_trim(buf);
	 j = fb_getword(buf, 1, word);
	 if (equal(word, X_SET)){
	    j = fb_getword(buf, j, e_var);
	    word[0] = NULL;
	    j = fb_getword(buf, j, word);

	    if (equal(e_var, X_DATE)){	/* set date [0,1,3,6,12] */
	       iv = atoi(word);
	       if (iv == 0 || iv == 1 || iv == 3 || iv == 6 || iv == 12){
		  /* for each fb_field, update its default */
		  st = FB_AOK;
		  strcpy(e_var, cdb_T_INCR);
		  if (iv > 0)
		     strcat(e_var, word);
		  for (p = col_mhead; p != NULL; p = p->p_next){
		     f = p->p_field;
		     if (f->type ==FB_DATE && f->idefault != NULL &&
		           strlen(f->idefault) >= 5 && 
			   strncmp(f->idefault, cdb_T_INCR, 5) == 0)
			fb_mkstr(&(f->idefault), e_var);
		     }
		  }
	       }		/* end ofFB_DATE section */

	    }
	 return(st);
      }

/*
 * line_put - fb_put mechanism. fb_put killbuffer in place after c.
 */

   line_put(c)
      crec *c;
      
      {
         crec *khead, *ktail, *kc, *copyclist();
	 int count = 0, pcount = 0, useins = 1, krow, tcount;
	 char buf[FB_PCOL];
	 
         sput_cursor(0);
	 if (killbuffer == NULL)
	    return(FB_ERROR);
	 modified = 1;
	 khead = copyclist(killbuffer, -1);
	 for (ktail = khead, count = 1; ; ktail = kc, count++){
	    kc = ktail->c_next;
	    if (kc == NULL)
	       break;
	    }
	 /* link new list into place */
         if (c == NULL){			/* must be at head of mem */
	    ktail->c_next = crec_mhead;
	    crec_mhead->c_prev = ktail;
	    crec_mhead = crec_phead = khead;
	    }
	 else{					/* normal case */
	    ktail->c_next = c->c_next;
	    if (c->c_next != NULL)
	       c->c_next->c_prev = ktail;
	    c->c_next = khead;
	    khead->c_prev = c;
	    }
	 if (ktail->c_next == crec_phead)	/* boundry conditions */
	    crec_phead = khead;
	 else if (c == crec_ptail){
	    pcount = dbvi_onpage();
	    if (pcount >= cdb_t_lines - calc_row){
	       crec_phead = crec_phead->c_next;
	       crec_ptail = khead;
	       fb_move(calc_row + 1, 1);
	       fb_deleteln();
	       }
	    else if (pcount + count < cdb_t_lines - calc_row)
	       crec_ptail = ktail;
	    else
	       useins = 0;			/* too many per fb_page */
	    }
	 if (c == crec_mtail)			/* check for tail of mem */
	    crec_mtail = ktail;
	 crec_current = khead;
	 crec_leftcorn = crec_phead;
	 if (c == NULL)
	    krow = calc_row + 1;
	 else
	    krow = whichrow(c) + 1;
	 if (count > (cdb_t_lines - krow))
	    useins = 0;
	 if (useins){
	    for (tcount = count; tcount > 0; tcount--){
	       fb_move(krow, 1); 
	       fb_insertln();
	       fb_move(cdb_t_lines, 1); 
	       fb_deleteln();
	       }
	    }
	 set_screen();
	 draw_numbers();
	 if (useins){
	    kc = crec_current;
	    for (tcount = count; tcount > 0; tcount--){
	       if (kc == NULL)
	          break;
	       put_row(kc);
	       kc = kc->c_next;
	       }
	    }
	 else
	    draw_cells();
	 if (count == 1)
	    sprintf(buf, "1 more record");
	 else
	    sprintf(buf, "%d more records", count);
	 fb_move(cdb_t_lines, 1);
	 fb_reverse(buf);
	 clear_lastline = 1;
	 return(FB_AOK);
      }

/*
 * line_yank - yank mechanism. yank in from c forward, count records.
 */

   line_yank(c, count)
      crec *c;
      int count;
      
      {
         crec *copyclist(), *nc;
	 char buf[FB_PCOL];
	 int tcount;
	 
	 if (count < 1)
	    count = 1;
	 for (nc = c, tcount = count; tcount > 0 && nc != NULL; tcount--){
	    nc = nc->c_next;
	    }
	 if (nc == NULL && tcount > 0)
	    return(FB_ERROR);
	 freeclist(killbuffer);
	 killbuffer = copyclist(c, count);
	 if (count == 1)
	    sprintf(buf, "1 record yanked");
	 else
	    sprintf(buf, "%d records yanked", count);
	 fb_move(cdb_t_lines, 1);
	 fb_reverse(buf);
	 clear_lastline = 1;
	 return(FB_AOK);
      }

/*
 * line_delete - fb_delete from crec c for count records.
 *    actually, place these onto the unnamed buffer. fb_delete those there.
 */

   line_delete(c, count)
      crec *c;
      int count;
      
      {
         crec *nc, *pc, *ktail = NULL;
	 char buf[FB_PCOL];
	 int tcount, usedel = 1;

         sput_cursor(0);
	 if (count == 0)
	    count = 1;
	 for (nc = c, tcount = count; tcount > 0 && nc != NULL; tcount--){
	    nc = nc->c_next;
	    }
	 if (nc == NULL && tcount > 0)
	    return(FB_ERROR);
	 modified = 1;
	 if (count > (cdb_t_lines - whichrow(c)))
	    usedel = 0;
	 /* now patch up the killbuffer */
         freeclist(killbuffer);
	 killbuffer = c;
	 crec_current = c;
	 for (tcount = count; tcount > 0; tcount--){
	    c = crec_current;
            pc = c->c_prev;
	    nc = c->c_next;
	    if (usedel){
	       fb_move(whichrow(c), 1);
	       fb_deleteln();
	       }
	    c->c_prev = NULL;
	    if (ktail == NULL){
	       ktail = c;
	       ktail->c_prev = NULL;
	       ktail->c_next = NULL;
	       }
	    else{
	       c->c_prev = ktail;
	       ktail->c_next = c;
	       c->c_next = NULL;
	       ktail = c;
	       }
	    if (pc == NULL && nc != NULL){ /* first mem line - nc non NULL */
	       crec_mhead = nc;
	       crec_mhead->c_prev = NULL;
	       }
	    else if (pc != NULL && nc == NULL){	/* last mem line - pc exist */
	       crec_mtail = pc;
	       crec_mtail->c_next = NULL;
	       }
	    /* and finally patch crec physical memory */
	    /* special case, one line on the screen */
	    if (c == crec_phead && c == crec_ptail && pc != NULL){
	       crec_phead = crec_current = pc;
	       line_up(halfsize, 0);
	       usedel = 0;
	       }
	    else if (c == crec_phead)	/* physical fb_page head deleted */
	       crec_phead = nc;
	    if (pc != NULL)		/* reset the links */
	       pc->c_next = nc;
	    if (nc != NULL){
	       nc->c_prev = pc;
	       crec_current = nc;
	       }
	    else
	       crec_current = pc;
	    if (crec_current == NULL)
	       emptyclist();
	    else if (usedel){
	       fb_move(cdb_t_lines, 1);
	       fb_insertln();
	       }
	    }
	 crec_leftcorn = crec_phead;
	 set_screen();
	 draw_numbers();
	 if (usedel){
	    pc = crec_ptail;
	    for (tcount = count; tcount > 0; tcount--){
	       if (pc == NULL)
	          break;
	       put_row(pc);
	       pc = pc->c_prev;
	       }
	    }
	 else
	    draw_cells();
	 if (count == 1)
	    sprintf(buf, "1 record deleted");
	 else
	    sprintf(buf, "%d records deleted", count);
	 fb_move(cdb_t_lines, 1);
	 fb_reverse(buf);
	 clear_lastline = 1;
	 return(FB_AOK);
      }

/*
 * line_goto - go to line no lineno. redraw as needed.
 */

   line_goto(lineno)
      int lineno;
      
      {
         crec *c;
	 int i;

	 sput_cursor(0);
         if (lineno == 0)		/* just G - to EOF */
	    crec_current = crec_mtail;
	 else{				/* goto lineno */
	    for (c = crec_mhead; c != NULL; c = c->c_next)
	       if (--lineno <= 0)
	          break;
	    if (c == NULL){
	       fb_bell();
	       return(FB_AOK);
	       }
	    crec_current = c;
	    }
	 /* redraw the screen half a fb_page back */
	 for (c = crec_current, i = 0; i < halfsize; i++){
	    if (c->c_prev == NULL)
	       break;
	    c = c->c_prev;
	    }
	 crec_phead = c;
	 return(FB_AOK);
      }
