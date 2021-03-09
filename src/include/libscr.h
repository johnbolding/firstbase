/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: libscr.h,v 9.1 2001/02/05 18:12:04 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

/*
 * define the structures needed for the screens
 */

#define FB_PROW	80
#define FB_PCOL	150

/* screen macros */
#define FB_XWAIT()	fb_move(12,35), fb_printw(SYSMSG[S_WORKING])
#define FB_PAUSE()	fb_serror(FB_MESSAGE, NIL, NIL)

/* macros for physical display */
#define TrueClear()	(tputs(cdb_CL, 1, fb_outc))	/* Clear screen (cl) */
#define TrueClrtobot()  (tputs(cdb_CD, 1, fb_outc))	/* Clr to bottom(cd) */
#define TrueClrtoeol()	(tputs(cdb_CE, 1, fb_outc))	/* Clear to EOL (ce) */
#define TrueMove(x,y)	(tputs(tgoto(cdb_CM, y-1, x-1), 1, fb_outc))
#define TrueStandout()	(tputs(cdb_SO, 1, fb_outc))	/* standout sequence */
#define TrueStandend()	(tputs(cdb_SE, 1, fb_outc))	/* standend sequence */
#define TrueReverseout() (tputs(cdb_RV, 1, fb_outc))	/* standout sequence */
#define TrueReverseend() (tputs(cdb_RE, 1, fb_outc))	/* standout sequence */
#define Truebell()	(tputs(cdb_VB, 1, fb_outc))	/* bell (vb or ^G) */ 

typedef struct fb_s_pscreen fb_pscreen;
struct fb_s_pscreen {
   char line[FB_PROW+1][FB_PCOL+1];
   char attr[FB_PROW+1][FB_PCOL+1];
   int  s_ncount[FB_PROW + 1];
   char s_bell, s_attron;
   int s_x, s_y;
   };

#if !FB_PROTOTYPES

extern fb_allow_input_interrupt();
extern fb_bell();
extern fb_clear();
extern fb_clrtobot();
extern fb_clrtoeol();
extern fb_deletech();
extern fb_deleteln();
extern fb_e_beginning_of_line();
extern fb_e_capitalize_word();
extern fb_e_cur_past_dot();
extern void fb_e_delete_char_backward();
extern void fb_e_delete_char_forward();
extern void fb_e_delete_to_beginning_of_line();
extern void fb_e_delete_to_end_of_line();
extern void fb_e_delete_word_backward();
extern void fb_e_delete_word_forward();
extern fb_e_display();
extern fb_e_dotcount();
extern fb_e_downcase_word();
extern fb_e_end_of_line();
extern fb_e_input();
extern fb_e_insert_char();
extern fb_e_move_char_backward();
extern fb_e_move_char_forward();
extern void fb_e_move_end_word_forward();
extern void fb_e_move_word_backward();
extern void fb_e_move_word_forward();
extern fb_e_num_past_dot();
extern fb_e_upcase_word();
extern fb_fmessage();
extern fb_force();
extern fb_gcounter();
extern void fb_getco();
extern fb_getfilename();
extern fb_initopt();
extern fb_input();
extern fb_inputdate();
extern fb_insertch();
extern fb_insertln();
extern fb_move();
extern fb_mustbe();
extern fb_nullscr();
extern fb_o_input();
extern fb_redraw();
extern fb_reverse();
extern fb_reverseend();
extern fb_reverseout();
extern fb_scrhlp();
extern fb_smessage();
extern fb_stand();
extern fb_standend();
extern fb_standout();
extern void fb_infoline();
extern void fb_infotoggle();
extern void fb_initscreen();
extern void fb_optomize();
extern void fb_outc();
extern void fb_prints();
extern void fb_refresh();
extern void fb_refresh_lastline();
extern void fb_s_putw();
extern void fb_scrhdr();
extern void fb_scrlbl();
extern void fb_scrstat();
extern void fb_scrstat2();
extern void fb_scrtime();
extern void fb_spause();
extern void fb_store_lastline();

#else  /* FB_PROTOTYPES */

extern fb_allow_input_interrupt(void);
extern fb_bell(void);
extern fb_clear(void);
extern fb_clrtobot(void);
extern fb_clrtoeol(void);
extern fb_deletech(void);
extern fb_deleteln(void);
extern fb_e_beginning_of_line(void);
extern fb_e_capitalize_word(void);
extern fb_e_cur_past_dot(void);
extern void fb_e_delete_char_backward(void);
extern void fb_e_delete_char_forward(void);
extern void fb_e_delete_to_beginning_of_line(void);
extern void fb_e_delete_to_end_of_line(void);
extern void fb_e_delete_word_backward(void);
extern void fb_e_delete_word_forward(void);
extern fb_e_display(int row, int col);
extern fb_e_dotcount(void);
extern fb_e_downcase_word(void);
extern fb_e_end_of_line(void);
extern fb_e_input(int, int, int, int, int, char *, int, int, int);
extern fb_e_insert_char(int c);
extern fb_e_move_char_backward(void);
extern fb_e_move_char_forward(void);
extern void fb_e_move_end_word_forward(void);
extern void fb_e_move_word_backward(void);
extern void fb_e_move_word_forward(void);
extern fb_e_num_past_dot(void);
extern fb_e_upcase_word(void);
extern fb_fmessage(char *s);
extern fb_force(char *s);
extern fb_gcounter(long j);
extern void fb_getco(fb_database *hp);
extern fb_getfilename(char *p, char *msg, char *def);
extern fb_initopt(void);
extern fb_input(int, int, int, int, int, char *, int, int, int);
extern fb_inputdate(int, int, int, int, int, char *, int, int, int);
extern fb_insertch(void);
extern fb_insertln(void);
extern fb_move(int x, int y);
extern fb_mustbe(int typ, char *msg, int row, int col);
extern fb_nullscr(fb_pscreen *s);
extern fb_o_input(int, int, int, int, int, char *, int, int, int);
extern fb_redraw(void);
extern fb_reverse(char *s);
extern fb_reverseend(void);
extern fb_reverseout(void);
extern fb_scrhlp(char *s);
extern fb_smessage(char *s);
extern fb_stand(char *s);
extern fb_standend(void);
extern fb_standout(void);
extern void fb_infoline(void);
extern void fb_infotoggle(void);
extern void fb_initscreen(void);
extern void fb_optomize(void);
extern void fb_outc(int c);
extern void fb_prints(char *buf);
extern void fb_refresh(void);
extern void fb_refresh_lastline(void);
extern void fb_s_putw(int c);
extern void fb_scrhdr(fb_database *hp, char *s);
extern void fb_scrlbl(char *s);
extern void fb_scrstat(char *p);
extern void fb_scrstat2(char *s);
extern void fb_scrtime(fb_database *hp);
extern void fb_spause(void);
extern void fb_store_lastline(void);

#endif /* FB_PROTOTYPES */
