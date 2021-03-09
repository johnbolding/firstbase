/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: fmessage.c,v 9.1 2001/02/16 18:51:46 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Fmessage_sid[] = "@(#) $Id: fmessage.c,v 9.1 2001/02/16 18:51:46 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <keyboard.h>

extern short int cdb_scr_help;
extern short int cdb_edit_input;

#if FB_PROTOTYPES
static void convert_keymsg(char *buf, char *s);
#else /* FB_PROTOTYPES */
static void convert_keymsg();
#endif /* FB_PROTOTYPES */

/*
 * fb_fmessage, fb_scrhlp, fb_smessage - simple routines that print help
 *	messages to constant locations on Cdb screens.
 *
 *	modified to handle a few soft keystrokes (if cdb_edit_input):
 *		<CTL>-X -- setkey FB_ABORT  	(E_ABORT)
 *		<CTL>-F -- setkey FB_PAGEDOWN 	(E_FB_FSIGNAL)
 *		<CTL>-B -- setkey FB_PAGEUP 	(E_FB_BSIGNAL)
 *
 *	returns length of new string.
 */

   fb_fmessage(s)
      char *s;

      {
         char buf[FB_MAXLINE];

         convert_keymsg(buf, s);
         fb_move(cdb_t_lines,1);
         fb_clrtoeol();
         fb_force(buf);
         return(strlen(buf));
      }

   fb_scrhlp(s)
      char *s;

      {
         char buf[FB_MAXLINE];

         if (cdb_scr_help){
            convert_keymsg(buf, s);
            fb_move(cdb_t_lines, (int) (cdb_t_cols - strlen(buf)));
            fb_clrtoeol();
            fb_prints(buf);
            }
      }

   fb_smessage(s)
      char *s;

      {
         char buf[FB_MAXLINE];

         convert_keymsg(buf, s);
         fb_move(cdb_t_lines, 1);
         fb_clrtoeol();
         fb_prints(buf);
      }

   static void convert_keymsg(buf, s)
      char *buf, *s;

      {
         char *p, rbuf[FB_MAXLINE];
         int found;

         if (!cdb_edit_input){
            strcpy(buf, s);
            return;
            }
         for (p = buf; *s; ){
            if (*s == '<'){
               /* test for replacement  */
               found = 1;
               if (strncmp(s, "<CTL>-X", 7) == 0)
                  fb_key_str(rbuf, E_ABORT);
               else if (strncmp(s, "<CTL>-F", 7) == 0)
                  fb_key_str(rbuf, E_PAGEDOWN);
               else if (strncmp(s, "<CTL>-B", 7) == 0)
                  fb_key_str(rbuf, E_PAGEUP);
               else
                  found = 0;
               if (found){
                  strcpy(p, rbuf);
                  p += strlen(rbuf);
                  s += 7;
                  continue;
                  }
               /* non replacement drops thru */
               }
            *p++ = *s++;
            }
         *p = NULL;
      }
