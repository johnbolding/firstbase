/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: xcommand.c,v 9.1 2001/02/16 19:44:23 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Xcommands_sid[] = "@(#) $Id: xcommand.c,v 9.1 2001/02/16 19:44:23 john Exp $";
#endif

#include <dbdmrg_e.h>

extern int halfsize;
extern char *ebuf;			/* buffer to edit in */
extern short int cdb_use_insert_char;
static mrg_xcommand();

/*
 * line_goto - go to line no lineno. redraw as needed.
 */

   line_goto(lineno)
      int lineno;
      
      {
         fb_aline *a;
	 int i;

         if (lineno == 0)		/* just G - to EOF */
	    mpcur->mp_acur = mpcur->mp_atail;
	 else{				/* goto lineno */
	    for (a = mpcur->mp_ahead; a != NULL; a = a->a_next)
	       if (--lineno <= 0)
	          break;
	    if (a == NULL){
	       fb_bell();
	       return(FB_ERROR);
	       }
	    mpcur->mp_acur = a;
	    }
         if (!onscreen(mpcur->mp_acur)){
            if (cdb_use_insert_char)
               clear_all_lines();
            /* redraw the screen half a fb_page back */
            for (a = mpcur->mp_acur, i = 0; i < halfsize; i++){
               if (a->a_prev == NULL)
                  break;
               a = a->a_prev;
               }
	    mpcur->mp_atop = a;
            }
	 return(FB_AOK);
      }

/*
 * line_delete - delete count lines from the current line
 */

   line_delete(count)
      int count;
      
      {
         fb_aline *a, *fb_copyalist();
         char buf[FB_MAXLINE];
         int ocount, tcount, usedel;

         if (count < 1)
            count = 1;
         for (tcount = 0, a = mpcur->mp_acur; a != NULL; a = a->a_next)
            if (++tcount >= count)
               break;
         if (tcount < count)
            return(FB_ERROR);

         usedel = cdb_use_insert_char;
	 if (count > (cdb_t_lines - 2 - whichrow(a)))
	    usedel = 0;
         ocount = count;
	 fb_freealist(killbuffer);
         killpage->mp_ahead = killpage->mp_atail = NULL;
	 killbuffer = fb_copyalist(mpcur->mp_acur, count, killpage);
         for (; count > 0; count--){
            modified = 1;
            a = mpcur->mp_acur;
	    if (usedel){
	       fb_move(whichrow(a), 1);
	       fb_deleteln();
	       fb_move(cdb_t_lines - 2, 1);
	       fb_insertln();
	       }
            if (a == mpcur->mp_atop && a == mpcur->mp_abot){
               line_up(halfsize, 0);
               }
            else if (a == mpcur->mp_atop)
               mpcur->mp_atop = a->a_next;
            if (a->a_next != NULL)
               mpcur->mp_acur = a->a_next;
            else
               mpcur->mp_acur = a->a_prev;
            fb_delete_line(a, mpcur);
            set_screen();
            /*
            if (mpcur->mp_acur == NULL)
               mpcur->mp_acur = mpcur->mp_abot;
            */
            mrg_display();
            }
	 if (ocount == 1)
	    sprintf(buf, "1 line deleted");
	 else
	    sprintf(buf, "%d lines deleted", ocount);
	 fb_move(cdb_t_lines, 1);
	 fb_prints(buf);
	 clear_lastline = 1;
	 return(FB_AOK);
      }

/*
 * line_join - join this line and the next line
 */

   line_join()
      
      {
         fb_aline *a, *na;
         int n;

         a = mpcur->mp_acur;
         if ((na = a->a_next) == NULL)
            return(FB_ERROR);
         modified = 1;
         strcpy(ebuf, a->a_text);
         fb_trim(ebuf);
         strcat(ebuf, " ");
         n = strlen(ebuf);
         strncat(ebuf, na->a_text, linewidth - n);
         ebuf[linewidth] = NULL;
         strcpy(a->a_text, ebuf);
         fb_copytlist(a, na->a_thead);
         fb_delete_line(na, mpcur);
         mpcur->mp_col = n;
         set_screen();
         mrg_display();
         return(FB_AOK);
      }

/*
 * line_open - open a line below line a
 */

   line_open(a)
      fb_aline *a;

      {
         fb_aline *o, *na, *ta, *fb_makeline();
         int n, krow;

         modified = 1;
         o = fb_makeline();
         if (a == NULL){
            na = mpcur->mp_ahead;
            mpcur->mp_atop = o;
            krow = base_top;
            }
         else{
            na = a->a_next;
            if (onscreen(a))
               krow = whichrow(a) + 1;
            else
               krow = base_top;
            /*
            if (krow > base_bottom)
               krow = base_bottom;
            */
            }

         /* check for screen conditions warranting redrawing */
         if (a != NULL && a == mpcur->mp_atop->a_prev){
            mpcur->mp_atop = o;
            }
         else if (a == mpcur->mp_abot && mpcur->mp_row == cdb_t_lines - 2){
            ta = mpcur->mp_atop;
            for (n = 1; n < 2; ta = ta->a_next, n++)
               ;
            mpcur->mp_atop = ta;
            }

         /* test for adding a line as last line ... need to deleteln then */
         if (cdb_use_insert_char){
            if (krow > base_bottom){
               fb_move(base_top, 1);
               fb_deleteln();
               fb_move(base_bottom, 1);
               fb_insertln();
               }
            else if (krow == base_bottom){
               fb_move(krow, 1);
               fb_deleteln();
               fb_move(krow, 1);
               fb_insertln();
               }
            else if (krow == base_top){
               fb_move(base_bottom, 1);
               fb_deleteln();
               fb_move(base_top, 1);
               fb_insertln();
               }
            else{
               fb_move(base_bottom, 1);
               fb_deleteln();
               fb_move(krow, 1);
               fb_insertln();
               }
            }

         fb_insert_line(o, na, mpcur);

         mpcur->mp_acur = o;
         mpcur->mp_col = 1;
         set_screen();
         mrg_display();
         put_cursor();
         fb_refresh();
         return(insert());
      }

/*
 * line_yank - yank count lines from a forward.
 */

   line_yank(a, count)
      fb_aline *a;
      int count;

      {
         fb_aline *fb_copyalist(), *na;
	 char buf[FB_PCOL];
	 int tcount;
	 
	 if (count < 1)
	    count = 1;
	 for (na = a, tcount = count; tcount > 0 && na != NULL; tcount--)
	    na = na->a_next;
	 if (na == NULL && tcount > 0)
	    return(FB_ERROR);
	 fb_freealist(killbuffer);
         killpage->mp_ahead = killpage->mp_atail = NULL;
	 killbuffer = fb_copyalist(a, count, killpage);
	 if (count == 1)
	    sprintf(buf, "1 line yanked");
	 else
	    sprintf(buf, "%d lines yanked", count);
	 fb_move(cdb_t_lines, 1);
	 fb_prints(buf);
	 clear_lastline = 1;
	 return(FB_AOK);
      }

/*
 * line_put - fb_put mechanism. fb_put killbuffer in place after a.
 */

   line_put(a)
      fb_aline *a;
      
      {
         fb_aline *khead, *ktail, *ka, *fb_copyalist();
	 int count = 0, pcount = 0, useins, krow = 0, tcount;
	 char buf[FB_PCOL];
	 
	 if (killbuffer == NULL)
	    return(FB_ERROR);
	 modified = 1;
         useins = cdb_use_insert_char;
         copypage->mp_ahead = copypage->mp_atail = NULL;
	 copybuffer = khead = fb_copyalist(killbuffer, -1, copypage);
         for (ktail = khead, count = 1; ; ktail = ka, count++){
            ka = ktail->a_next;
            if (ka == NULL)
               break;
            }
         ktail = copypage->mp_atail;
	 /* link new list into place */
         if (a == NULL){			/* must be at head of mem */
	    ktail->a_next = mpcur->mp_ahead;
	    mpcur->mp_ahead->a_prev = ktail;
	    mpcur->mp_ahead = mpcur->mp_atop = khead;
	    }
	 else{					/* normal case */
	    ktail->a_next = a->a_next;
	    if (a->a_next != NULL)
	       a->a_next->a_prev = ktail;
	    a->a_next = khead;
	    khead->a_prev = a;
	    }
	 if (ktail->a_next == mpcur->mp_atop)	/* boundry conditions */
	    mpcur->mp_atop = khead;
	 else if (a == mpcur->mp_abot){
	    pcount = onpage();
	    if (pcount >= base_bottom - base_top + 1){
               mpcur->mp_atop = mpcur->mp_atop->a_next;
               if (useins){
                  fb_move(base_top, 1);
                  fb_deleteln();
                  fb_move(base_bottom, 1); 
                  fb_insertln();
                  }
	       }
	    else if (pcount + count < base_bottom - base_top + 1)
	       mpcur->mp_abot = ktail;
            else
               useins = 0;
	    }
	 if (a == mpcur->mp_atail)		/* check for tail of mem */
	    mpcur->mp_atail = ktail;
	 mpcur->mp_acur = khead;
	 mpcur->mp_col = 1;
         mpcur->mp_leftcorn = 1;
         if (useins){
            if (onscreen(a))
               krow = whichrow(a) + 1;
            else
               krow = base_top;
            if (krow > base_bottom)
               krow = base_bottom;
            if (count > (base_bottom - krow + 1))
               useins = 0;
            }
	 if (useins){
	    for (tcount = count; tcount > 0; tcount--){
	       fb_move(base_bottom, 1); 
	       fb_deleteln();
	       fb_move(krow, 1); 
	       fb_insertln();
	       }
	    }
            
	 set_screen();
         mrg_display();
	 if (count == 1)
	    sprintf(buf, "1 more line");
	 else
	    sprintf(buf, "%d more lines", count);
	 fb_move(cdb_t_lines, 1);
	 fb_prints(buf);
	 clear_lastline = 1;
	 return(FB_AOK);
      }

/*
 * colon_command - do the colon command set for dbdmrg.
 */

   colon_command()
      {
         char ccom, buffer[FB_MAXLINE];
	 int i, st = FB_AOK;

	 fb_move(cdb_t_lines, 1);
	 fb_s_putw(CHAR_COLON);
	 clear_lastline = 1;
	 for (i = 0; i < 80; i++){
	    fb_refresh();
	    ccom = getchar();
	    if (ccom == FB_NEWLINE || ccom == FB_CRET)
	       break;
	    if (!isprint(ccom))
	       return(FB_ERROR);
	    fb_s_putw(ccom);
	    buffer[i] = ccom;
	    }
	 buffer[i] = NULL;
	 if (i > 0)
	    st = mrg_xcommand(buffer);
	 return(st);
      }

/*
 * xcommand - the work part of the colon command set
 */

   static mrg_xcommand(buf)
      char *buf;
      
      {
         int st = FB_ERROR;
	 char word[FB_MAXLINE];

         if (isdigit(*buf)){
	    return(line_goto(atoi(buf)));
	    }
	 fb_getword(buf, 1, word);
         switch(*buf++){
	    case 'r':				/* run , merge */
	    case 'm':
               clear_lastline = 1;
               mface();
               st = FB_AOK;
               break;
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
