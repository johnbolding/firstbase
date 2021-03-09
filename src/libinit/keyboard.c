/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: keyboard.c,v 9.1 2001/01/16 02:46:52 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Keyboard_sid[] = "@(#) $Id: keyboard.c,v 9.1 2001/01/16 02:46:52 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <keyboard.h>

/*
 * keyboard - library routines for general mapping of keystrokes to actions.
 *	used by the intraline editing feature of e_fb_input()
 */

typedef struct s_key key;
struct s_key {
   char *k_seq;
   char *k_name;
   int k_len;
   int k_action;
   };

typedef struct s_action action;
struct s_action {
   char *a_name;
   int a_value;
   };

#if FB_PROTOTYPES
static loadkey(char *line);
static action *action_lookup(char *s);
static key *key_signal_lookup(int esig);
static key_string(char *buf, char *seq, int len, int sflag);
#else /* FB_PROTOTYPES */
static void loadkey();
static key_string();
static key *key_signal_lookup();
static action *action_lookup();
#endif /* FB_PROTOTYPES */

extern char *cdb_home;
extern short int cdb_edit_input;

static key keyboard[FB_MAXKEYS];
static int topkey = 0;

char k_quit_char = '\03';

static action action_array[] = {
   "delete-char-backward",	E_DELETE_CHAR_BACKWARD,
   "delete-char-forward",	E_DELETE_CHAR_FORWARD,
   "move-char-backward",	E_MOVE_CHAR_BACKWARD,
   "move-char-forward",		E_MOVE_CHAR_FORWARD,
   "delete-word-backward",	E_DELETE_WORD_BACKWARD,
   "delete-word-forward",	E_DELETE_WORD_FORWARD,
   "move-word-backward",	E_MOVE_WORD_BACKWARD,
   "move-word-forward",		E_MOVE_WORD_FORWARD,
   "beginning-of-line",		E_BEGINNING_OF_LINE,
   "end-of-line",		E_END_OF_LINE,
   "delete-to-beginning-of-line",E_DELETE_TO_BEGINNING_OF_LINE,
   "delete-to-end-of-line",	E_DELETE_TO_END_OF_LINE,
   "capitalize-word",		E_CAPITALIZE_WORD,
   "upcase-word",		E_UPCASE_WORD,
   "downcase-word",		E_DOWNCASE_WORD,
   "dsignal",			E_DSIGNAL,
   "esignal",			E_ESIGNAL,
   "nsignal",			E_ESIGNAL,
   "ysignal",			E_YSIGNAL,
   "psignal",			E_YSIGNAL,
   "redraw",			E_REDRAW,
   "fsignal",			E_FSIGNAL,
   "bsignal",			E_BSIGNAL,
   "ssignal",			E_SSIGNAL,
   "qsignal",			E_QSIGNAL,
   "end",			E_END,
   "abort",			E_ABORT,
   "pageup",			E_PAGEUP,
   "pagedown",			E_PAGEDOWN,
   "next",			E_NEXT,
   "prev",			E_PREV,
   "print",			E_PRINT,
   "default",			E_DEFAULT,
   "clearfield",		E_CLEARFIELD,
   "writerec",			E_WRITEREC,
   "deleterec",			E_DELETEREC,
   "quit",			E_QUIT,
   "help",			E_HELP,
   "system-interrupt",		E_SYS_INTERRUPT,
   "system-stop",		E_SYS_STOP,
   0, 0
   };

/*
 * cdb_init_keyboard - aligns keyboard sequences to actions
 *	opens a HOME .cdb_kbdmap or PATH or CDBHOME ... exactly ONE.
 */

   fb_init_keyboard()
      {
         int fd;
         char buf[FB_MAXLINE], fname[FB_MAXNAME];

         fb_pathname(fname, FB_KBDMAP);
         fd = -1;
         if (fname[0] != NULL)
            fd = open(fname, READ);
         if (fd <= 0){
            strcpy(fname, cdb_home);
            fb_assure_slash(fname);
            strcat(fname, FB_KBDMAP);
            fd = open(fname, READ);
            if (fd <= 0)
               return(FB_ERROR);
            }
         fb_ln_init(fd);
         while (fb_ln_load(buf, FB_MAXLINE) != 0)
            if (strncmp(buf, "setkey", 6) == 0)
               loadkey(buf + 6);
         fb_ln_end();
         return(FB_AOK);
      }

/*
 * loadkey - load up a single <key sequence, action> pair
 */

   static void loadkey(line)
      char *line;

      {
         char nbuf[FB_MAXLINE], *np, *q, *p, kact_buf[FB_MAXNAME];
         int len = 0, i, kact;
         key *k;
         action *a;

         /* skip to first double quote */
         for (p = line; *p && *p != CHAR_QUOTE; p++)
            ;
         if (*p != CHAR_QUOTE)
            return;
         for (p++, np = nbuf; *p; p++, np++){
            if (*p == '\\'){
               /* some meta character */
               p++;
               switch(*p){
                  case 'e':			/* escape */
                  case 'E':
                     *p = '\033';
                     break;
                  case '\\':			/* escaped backslash */
                     *p = '\\';
                     break;
                  case '^':			/* control character */
                     p++;
                     if (islower(*p))
                        *p = toupper(*p);
                     if (*p != '?')
                        *p -= 0100;		/* convert to control */
                     else
                        *p = FB_RUBOUT;
                     break;
                  }
               }
            else if (*p == CHAR_QUOTE)
               break;
            *np = *p;
            len++;
            }
         if (*p != CHAR_QUOTE)
            return;
         strcpy(kact_buf, ++p);
         if ((p = strchr(kact_buf, '#')) != 0)
            *p = NULL;
         fb_rmlead(kact_buf);
         fb_trim(kact_buf);
         i = strlen(kact_buf);
         if (kact_buf[i] == FB_NEWLINE)
            kact_buf[i] = NULL;
         a = action_lookup(kact_buf);
         if (a == NULL)
            return;
         kact = a->a_value;
         k = &keyboard[topkey++];
         k->k_len = len;
         k->k_seq = fb_malloc((unsigned) len);
         k->k_action = kact;
         k->k_name = a->a_name;
         for (np = nbuf, i = 0, q = k->k_seq; i < len; i++)
            *q++ = *np++;
         if (kact == E_QUIT)
            k_quit_char = k->k_seq[0];
      }

/*
 * fb_test_keyboard - return action, 0 if still could be command, -1 if not.
 */

   fb_test_keyboard(s, len)
      char *s;
      int len;

      {
         int i;
         key *k;

         for (i = 0; i < topkey; i++){
            k = &keyboard[i];
            if (k->k_len == len && strncmp(s, k->k_seq, len) == 0)
               return(k->k_action);
            }
         for (i = 0; i < topkey; i++){
            k = &keyboard[i];
            if (k->k_len > len && strncmp(s, k->k_seq, len) == 0)
               return(FB_AOK);
            }
         /* test for internal built ins only if the command is failing */
         if (len == 1){
            switch (*s){
               case FB_DSIGNAL: 	return(E_DSIGNAL);
               case FB_ESIGNAL:
               case FB_ESIGNAL2:
               case FB_ESIGNAL3: 	return(E_ESIGNAL);
               case FB_YSIGNAL:
               case FB_YSIGNAL2: 	return(E_YSIGNAL);
               case FB_KILL:	return(E_ABORT);
               case FB_REDRAW1: 	return(E_REDRAW);
               case FB_FSIGNAL: 	return(E_FSIGNAL);
               case FB_BSIGNAL: 	return(E_BSIGNAL);
               case FB_SSIGNAL: 	return(E_SSIGNAL);
               case FB_QSIGNAL: 	return(E_QSIGNAL);
               case FB_HELPKEY:	return(E_HELP);
               case FB_RUBOUT:	return(E_DELETE_CHAR_BACKWARD);
               }
            }
         return(FB_ERROR);
      }

/*
 * fb_test_help - return 1 if ^H stands for a backward motion, else 0
 */

   fb_test_help()
      {
         char buf[2];
         int f;

         buf[0] = FB_HELPKEY; buf[1] = NULL;
         f = fb_test_keyboard(buf, 1);
         if (f == E_DELETE_CHAR_BACKWARD || f == E_DELETE_WORD_BACKWARD ||
               f == E_MOVE_CHAR_BACKWARD   || f == E_MOVE_WORD_BACKWARD ||
               f == E_HELP)
            return(1);
         return(0);
      }

/*
 * fb_interpret_esig - convert an internal E_SIGNAL into a standard Cdb signal
 */

   fb_interpret_esig(k)
      int k;

      {
         switch(k){
            /* also built in */
            case E_DSIGNAL: 	return(FB_DSIGNAL);
            case E_ESIGNAL: 	return(FB_ESIGNAL);
            case E_YSIGNAL: 	return(FB_YSIGNAL);
            case E_REDRAW: 	return(FB_REDRAW1);
            case E_FSIGNAL: 	return(FB_FSIGNAL);
            case E_BSIGNAL: 	return(FB_BSIGNAL);
            case E_SSIGNAL: 	return(FB_SSIGNAL);
            case E_QSIGNAL: 	return(FB_QSIGNAL);

            /* other signals */
            case E_END:     	return(FB_END);
            case E_ABORT:   	return(FB_ABORT);
            case E_HELP:    	return(FB_QHELP);
            case E_PAGEUP:  	return(FB_PAGEUP);
            case E_PAGEDOWN:	return(FB_PAGEDOWN);
            case E_NEXT:    	return(FB_ESIGNAL);
            case E_PREV:    	return(FB_YSIGNAL);
            case E_PRINT:   	return(FB_PSIGNAL);
            case E_DEFAULT: 	return(FB_DSIGNAL);
            case E_CLEARFIELD:	return(FB_CSIGNAL);
            case E_WRITEREC:	return(FB_WSIGNAL);
            case E_DELETEREC:	return(FB_DELSIGNAL);
            }
         return(0);
      }

/*
 * action_lookup - look up a string given in the action structures.
 */

   static action *action_lookup(s)
      char *s;

      {
         action *a;

         for (a = action_array; a->a_value != 0; a++)
            if (equal(s, a->a_name))
               return(a);
         return(NULL);
      }

/*
 * signal_lookup - look up a signal in the keys, return that *key
 */

   static key *key_signal_lookup(esig)
      int esig;

      {
         int i;
         key *k;

         for (i = 0; i < topkey; i++){
            k = &keyboard[i];
            if (k->k_action == esig)
               return(k);
            }
         return(NULL);
      }

/*
 * fb_key_str - get a printable key string for E_SIGNAL
 */

   fb_key_str(buf, esig)
      char *buf;
      int esig;

      {
         int sflag = 0;
         key *k;

         k = key_signal_lookup(esig);
         if (k != NULL){
            if (k->k_len > 1)
               sflag = 1;
            key_string(buf, k->k_seq, k->k_len, sflag);
            }
      }

/*
 * key_string - convert seq into printable characters for help string
 */

   static key_string(buf, seq, len, sflag)
      char *buf, *seq;
      int len, sflag;

      {
         char c, *p;
         int ctl_flag = 0;

         p = buf;
         *p = NULL;
         for (; len > 0; len--, seq++){
            c = *seq;
            if (c >= 00 && c < 040){
               if (c == 033){
                  strcat(p, "<ESC> ");
                  p += 6;
                  continue;
                  }
               else if (!sflag){
                  strcat(p, "<CTL>-");
                  p += 6;
                  }
               else
                  *p++ = '^';
               c += 0100;
               ctl_flag = 1;
               }
            else if (c == FB_RUBOUT){
               strcat(p, "DEL ");
               p += 4;
               continue;
               }
            *p++ = c;
            if (ctl_flag){
               ctl_flag = 0;
               *p++ = FB_BLANK;
               }
            *p = NULL;
            }
         fb_trim(buf);
      }

/*
 * cdb_seq_convert - convert an old cdb seqence to a new one - for cdbtool.
 */

   fb_seq_convert(buf, seq, len)
      char *buf, *seq;
      int len;

      {
         int esig;
         key *k;

         strncpy(buf, seq, len);
         if (!cdb_edit_input)
            return(len);
         /* first decide what signal we need to gen a seq for */
         if (strncmp(seq, "\016", 1) == 0)		/* ^N */
            esig = E_NEXT;
         else if (strncmp(seq, "\020", 1) == 0)		/* ^P */
            esig = E_PREV;
         else if (strncmp(seq, "\033\020", 2) == 0)	/* ESC ^P */
            esig = E_PAGEUP;
         else if (strncmp(seq, "\033\016", 2) == 0)	/* ESC ^N */
            esig = E_PAGEDOWN;
         else if (strncmp(seq, "\033\027", 2) == 0)	/* ESC ^W */
            esig = E_WRITEREC;
         else if (strncmp(seq, "\033\004", 2) == 0)	/* ESC ^D */
            esig = E_DSIGNAL;
         else if (strncmp(seq, "\006", 1) == 0)		/* ^F */
            esig = E_FSIGNAL;
         else if (strncmp(seq, "\002", 1) == 0)		/* ^B */
            esig = E_BSIGNAL;
         else if (strncmp(seq, "\014", 1) == 0)		/* ^L / ^R */
            esig = E_REDRAW;
         else if (strncmp(seq, "\030", 1) == 0)		/* ^X */
            esig = E_ABORT;
         else
            esig = 0;
         if (esig != 0 && (k = key_signal_lookup(esig)) != NULL){
            strncpy(buf, k->k_seq, k->k_len);
            len = k->k_len;
            }
         return(len);
      }

/*
 * fb_describe_bindings - dump bindings into provided for fs
 */

   fb_describe_bindings(fs)
      FILE *fs;

      {
         int i;
         key *k;
         char rbuf[FB_MAXLINE];

         fprintf(fs, "Key          binding\n");
         fprintf(fs, "---          -------\n\n");
         for (i = 0; i < topkey; i++){
            k = &keyboard[i];
            key_string(rbuf, k->k_seq, k->k_len, 1);
            fprintf(fs, "%-10s   %s\n", rbuf, k->k_name);
            }
      }

/*
 * fb_free_keyboard - free keyboard memory allocations
 */

   fb_free_keyboard()

      {
         int i;
         key *k;

         for (i = 0; i < topkey; i++){
            k = &keyboard[i];
            if (k != NULL && k->k_seq != NULL)
               fb_free(k->k_seq);
            }
      }

#if FB_KEYBOARD_DEBUG
   fb_trace_keyboard()
      {
         int i, j;
         key *k;
         char *q;

         fprintf(stderr, "Begin Trace Keys:\n");
         for (i = 0; i < topkey; i++){
            k = &keyboard[i];
            fprintf(stderr, "Keyboard %d, k_len=%d, k_act=%d: ",
               i, k->k_len, k->k_action);
            for (j = 0, q = k->k_seq; j < k->k_len; j++, q++)
               fprintf(stderr, "\\0%o ", *q);
            fprintf(stderr, "\n");
            }
      }
#endif /* FB_KEYBOARD_DEBUG */
