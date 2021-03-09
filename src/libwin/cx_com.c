/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: cx_com.c,v 9.1 2001/02/16 19:36:31 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Cx_com_sid[] = "@(#) $Id: cx_com.c,v 9.1 2001/02/16 19:36:31 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

#ifndef SIGUSR1
#define SIGUSR1 30
#define SIGUSR2 31
#endif

/*
 * Cx_com - provide general communication routines for xview interface layers.
 */

static char cx_control[FB_MAXNAME];
static int cx_writeflag = 0;
static int cx_ppid;				/* PPID */
static char cx_buttons[MAXBUTTONS];		/* BUTTONS */
static char cx_menu[FB_MAXLINE];			/* MENU */
static char cx_status[FB_MAXLINE];			/* STATUS */
static char cx_toolname[FB_MAXLINE];		/* TOOLNAME */
static char cx_message[FB_MAXLINE];		/* FB_MESSAGE */
static char cx_datadict[FB_MAXNAME];		/* DATADICT set with VIEWDICT */
static char cx_viewdict[FB_MAXNAME];		/* VIEWDICT */
static int cx_viewpage;				/* VIEWPAGE */
static char cx_seekfile[FB_MAXNAME];		/* choice seek file */
static long cx_seekpoint;			/* choice seek point */
static char cx_workdir[FB_MAXNAME];		/* working directory */
static char env_buf[FB_MAXLINE];
static char env_buf1[FB_MAXLINE];

static char *cx_button_stack[MAXBUTTONS];
static int cx_button_top = 0;
static char *cx_status_stack[MAXBUTTONS];
static int cx_status_top = 0;
static char *cx_message_stack[MAXBUTTONS];
static int cx_message_top = 0;

static int cx_disable = 0;

#if FB_PROTOTYPES
static cx_null_values(void);
#else
static cx_null_values();
#endif /* FB_PROTOTYPES */

extern char *cdb_pgm;
extern char *fb_malloc(unsigned s);

/*
 * cx_init - initialize a cx file for Cdb.
 *	- create a temp file
 *	- store PID line
 *	- place temp file name in environ for other processes
 *
 *	NOTE: this routine is actually only called from cdbtool.
 */

   fb_cx_init()

      {
         extern char *cdb_menu, *cdb_tempdir;

         cx_null_values();
         strcpy(cx_control, cdb_tempdir);
         fb_assure_slash(cx_control);
         strcat(cx_control, "cdbCX_XXXXXX");
         close(mkstemp(cx_control));
         cx_ppid = getpid();
         /* assume init is from dbshell, use EHR as default buttons */
         strcpy(cx_buttons, "EHR");
         sprintf(cx_menu, "%sMAIN", cdb_menu);
         cx_writeflag = 1;
         fb_cx_write(0);
         sprintf(env_buf, "CX_CONTROL=%s", cx_control);
         putenv(env_buf);
         sleep(1);		/* sleep to enable sync between layers */
      }

/*
 * cx_boot - bootstrap a cx file for Cdb - using env var CX_CONTROL
 *	this routine is used by all layers of Cdb Tools underneath Cdbtool.
 *	I.E. this is called by getargs().
 */

   fb_cx_boot()

      {
#if CX_CODE
         char *p, buf[FB_MAXNAME];

         cx_null_values();
         if (cdb_batchmode)
            return;
         if ((p = getenv("CX_CONTROL")) == 0)
            return;
         sleep(1);		/* sleep to enable sync between layers */
         strcpy(cx_control, p);
         strcpy(env_buf1, "TERM=sun");
         putenv(env_buf1);
         fb_cx_read();
         fb_cx_set_toolname(cdb_pgm);
         fb_getwd(buf);
         fb_cx_set_workdir(buf);
#endif /* CX_CODE */
      }

/*
 * fb_cx_write - write out all of the non-null cx variables to the control file
 */

   void fb_cx_write(sigflag)
      int sigflag;

      {
         int fd;
         char buf[FB_MAXLINE];

         if (cx_control[0] == NULL)
            return;
         if (cx_writeflag == 0)
            return;
         cx_writeflag = 0;
         close(creat(cx_control, 0600));
         fd = open(cx_control, WRITE);
         if (fd < 0)
            return;
         fb_w_init(1, fd, 0);
         sprintf(buf, "PPID=%d\n", cx_ppid);
         fb_nextwrite(0, buf);
         sprintf(buf, "BUTTONS=%s\n", cx_buttons);
         fb_nextwrite(0, buf);
         if (cx_menu[0] != NULL){
            sprintf(buf, "MENU=%s\n", cx_menu);
            fb_nextwrite(0, buf);
            }
         sprintf(buf, "STATUS=%s\n", cx_status);
         fb_nextwrite(0, buf);
         sprintf(buf, "TOOLNAME=%s\n", cx_toolname);
         fb_nextwrite(0, buf);
         if (cx_message[0] != NULL){
            sprintf(buf, "FB_MESSAGE=%s\n", cx_message);
            fb_nextwrite(0, buf);
            }
         if (cx_viewdict[0] != NULL){
            sprintf(buf, "VIEWDICT=%s\n", cx_viewdict);
            fb_nextwrite(0, buf);
            }
         if (cx_datadict[0] != NULL){
            sprintf(buf, "DATADICT=%s\n", cx_datadict);
            fb_nextwrite(0, buf);
            }
         if (cx_viewpage != 0){
            sprintf(buf, "VIEWPAGE=%d\n", cx_viewpage);
            fb_nextwrite(0, buf);
            }
         if (cx_seekfile[0] != NULL){
            sprintf(buf, "SEEKFILE=%s\n", cx_seekfile);
            fb_nextwrite(0, buf);
            }
         if (cx_seekpoint >= 0){
            sprintf(buf, "SEEKPOINT=%ld\n", cx_seekpoint);
            fb_nextwrite(0, buf);
            }
         if (cx_workdir[0] != NULL){
            sprintf(buf, "WORKDIR=%s\n", cx_workdir);
            fb_nextwrite(0, buf);
            }
         fb_wflush(1);
         fb_sync_fd(fd);
         close(fd);
         if (sigflag)
            fb_cx_signal_2();
      }

/*
 * cx_read - read the cx variables from control file
 */

   void fb_cx_read()
      {
         int fd;
         char buf[FB_MAXLINE];

         if (cx_control[0] == NULL)
            return;
         if ((fd = open(cx_control, READ)) <= 0)
            return;
         fb_r_init(fd);
         for (; fb_nextline(buf, FB_MAXLINE) != 0; ){
            if (sscanf(buf, "PPID=%d", &cx_ppid) == 1)
               ;
            else if (sscanf(buf, "BUTTONS=%s", cx_buttons) == 1)
               ;
            else if (sscanf(buf, "MENU=%s", cx_menu) == 1)
               ;
            else if (sscanf(buf, "STATUS=%s", cx_status) == 1)
               ;
            else if (sscanf(buf, "TOOLNAME=%s", cx_toolname) == 1)
               ;
            else if (sscanf(buf, "FB_MESSAGE=%s", cx_message) == 1)
               ;
            else if (sscanf(buf, "VIEWDICT=%s", cx_viewdict) == 1)
               ;
            else if (sscanf(buf, "DATADICT=%s", cx_datadict) == 1)
               ;
            else if (sscanf(buf, "VIEWPAGE=%d", &cx_viewpage) == 1)
               ;
            else if (sscanf(buf, "SEEKFILE=%s", cx_seekfile) == 1)
               ;
            else if (sscanf(buf, "SEEKPOINT=%ld", &cx_seekpoint) == 1)
               ;
            else if (sscanf(buf, "WORKDIR=%s", cx_workdir) == 1)
               ;
            }
         close(fd);
      }

/*
 * cx_close - close down the cx system
 */

   void fb_cx_close()
      {
         if (cx_control[0] == NULL)
            return;
         unlink(cx_control);
      }

/*
 * cx_get
 * cx_set - various get and set routines - to transfer variables
 *	in case something else needs doing.
 */

   fb_cx_get_menu(fname)				/* cx_menu */
      char *fname;

      {
         strcpy(fname, cx_menu);
      }

   void fb_cx_set_menu(fname)
      char *fname;

      {
         
         if (cx_control[0] == NULL)
            return;
         strcpy(cx_menu, fname);
         cx_writeflag = 1;
      }

   fb_cx_get_status(s)				/* cx_status */
      char *s;

      {
         strcpy(s, cx_status);
      }

   void fb_cx_set_status(s)
      char *s;

      {
         if (cx_control[0] == NULL)
            return;
         strcpy(cx_status, s);
         cx_writeflag = 1;
      }

   fb_cx_get_toolname(s)				/* cx_toolname */
      char *s;

      {
         strcpy(s, cx_toolname);
      }

   void fb_cx_set_toolname(s)
      char *s;

      {
         if (cx_control[0] == NULL)
            return;
         strcpy(cx_toolname, s);
         cx_writeflag = 1;
      }

   fb_cx_get_message(s)				/* cx_message */
      char *s;

      {
         strcpy(s, cx_message);
      }

   void fb_cx_set_message(s)
      char *s;

      {
         if (cx_control[0] == NULL)
            return;
         strcpy(cx_message, s);
         cx_writeflag = 1;
      }

   fb_cx_get_dict(ddict, vdict)			/* cx_viewdict, cx_datadict */
      char *ddict, *vdict;

      {
         strcpy(ddict, cx_datadict);
         strcpy(vdict, cx_viewdict);
      }

   void fb_cx_set_dict(ddict, vdict)
      char *ddict, *vdict;

      {
         
         if (cx_control[0] == NULL)
            return;
         strcpy(cx_datadict, ddict);
         strcpy(cx_viewdict, vdict);
         cx_writeflag = 1;
      }

   fb_cx_get_viewpage()				/* cx_viewpage */

      {
         return(cx_viewpage);
      }

   void fb_cx_set_viewpage(p)
      int p;

      {
         if (cx_control[0] == NULL)
            return;
         cx_viewpage = p;
         cx_writeflag = 1;
      }

   long fb_cx_get_seekpoint()			/* cx_seekpoint */

      {
         return(cx_seekpoint);
      }

   void fb_cx_set_seekpoint(p)
      long p;

      {
         if (cx_control[0] == NULL)
            return;
         cx_seekpoint = p;
         cx_writeflag = 1;
      }

   fb_cx_get_seekfile(s)				/* cx_seekfile */
      char *s;

      {
         strcpy(s, cx_seekfile);
      }

   void fb_cx_set_seekfile(s)
      char *s;

      {
         if (cx_control[0] == NULL)
            return;
         strcpy(cx_seekfile, s);
         cx_writeflag = 1;
      }

   fb_cx_get_workdir(s)				/* cx_workdir */
      char *s;

      {
         strcpy(s, cx_workdir);
      }

   void fb_cx_set_workdir(s)
      char *s;

      {
         if (cx_control[0] == NULL)
            return;
         strcpy(cx_workdir, s);
         cx_writeflag = 1;
      }

/*
 * cx_set_buttons - store the buttons
 */

   void fb_cx_set_buttons(s)
      char *s;

      {
         if (cx_control[0] == NULL)
            return;
         strcpy(cx_buttons, s);
         cx_writeflag = 1;
      }

/*
 * cx_get_buttons - copy the current buttons into s
 */

   fb_cx_get_buttons(s)
      char *s;

      {
         strcpy(s, cx_buttons);
      }

/*
 * cx_add_buttons - add a set of buttons to the current set.
 */
 
   void fb_cx_add_buttons(s)
      char *s;

      {
         if (cx_control[0] == NULL)
            return;
         strcat(cx_buttons, s);
         cx_writeflag = 1;
      }

/*
 * cx_sub_buttons - subtract a set of buttons from the current set.
 */
 
   void fb_cx_sub_buttons(s)
      char *s;

      {
         char *p, *q, tb[FB_MAXLINE];

         if (cx_control[0] == NULL)
            return;
         for (q = s; *q; q++){
            for (p = cx_buttons; *p; p++){
               if (*q == *p){
                  *p = '.';
                  break;
                  }
               }
            }
         strcpy(tb, cx_buttons);
         for (p = tb, q = cx_buttons; *p; p++)
            if (*p != '.')
               *q++ = *p;
         *q = NULL;
         cx_writeflag = 1;
      }

/*
 * cx_push_buttons - push the current cx_buttons onto stack
 *	then place s into the cx_buttons slot
 */

   void fb_cx_push_buttons(s)
      char *s;

      {
         char *p;

         if (cx_control[0] == NULL)
            return;
         if (cx_button_top >= MAXBUTTONS)
            fb_cx_fb_xerror(FB_MESSAGE, "Too many button states saved.", "");
         p = fb_malloc((unsigned) (MAXBUTTONS + 1));
         strcpy(p, cx_buttons);
         cx_button_stack[cx_button_top] = p;
         cx_button_top++;
         strcpy(cx_buttons, s);
         cx_writeflag = 1;
      }

/*
 * cx_pop_buttons - pop the top of the cx_buttons stack into cx_buttons
 *	NOTE: popping buttons does an implicit write/signal
 */

   void fb_cx_pop_buttons()
      {
         char *p;

         if (cx_control[0] == NULL)
            return;
         if (cx_button_top <= 0)
            fb_xerror(FB_MESSAGE, "Trying to pop unknown button state.", "");
         cx_button_top--;
         p = cx_button_stack[cx_button_top];
         strcpy(cx_buttons, p);
         fb_free(p);
         cx_button_stack[cx_button_top] = NULL;
         cx_writeflag = 1;
      }

/*
 * cx_push_status - push the current cx_status onto stack
 *	then place s into the cx_status slot
 */

   void fb_cx_push_status(s)
      char *s;

      {
         char *p;
         int len;

         if (cx_control[0] == NULL)
            return;
         if (cx_status_top >= MAXBUTTONS)
            fb_cx_fb_xerror(FB_MESSAGE, "Too many status states saved.", "");
         len = strlen(cx_status);
         p = fb_malloc((unsigned) (len + 1));
         strcpy(p, cx_status);
         cx_status_stack[cx_status_top] = p;
         cx_status_top++;
         strcpy(cx_status, s);
         cx_writeflag = 1;
      }

/*
 * cx_pop_status - pop the top of the cx_status stack into cx_status
 */

   void fb_cx_pop_status()
      {
         char *p;

         if (cx_control[0] == NULL)
            return;
         if (cx_status_top <= 0)
            fb_xerror(FB_MESSAGE, "Trying to pop unknown status state.", "");
         cx_status_top--;
         p = cx_status_stack[cx_status_top];
         strcpy(cx_status, p);
         fb_free(p);
         cx_status_stack[cx_status_top] = NULL;
         cx_writeflag = 1;
      }

/*
 * cx_push_message - push the current cx_message onto stack
 *	then place s into the cx_message slot
 */

   void fb_cx_push_message(s)
      char *s;

      {
         char *p;
         int len;

         if (cx_control[0] == NULL)
            return;
         if (cx_message_top >= MAXBUTTONS)
            fb_cx_fb_xerror(FB_MESSAGE, "Too many message states saved.", "");
         len = strlen(cx_message);
         p = fb_malloc((unsigned) (len + 1));
         strcpy(p, cx_message);
         cx_message_stack[cx_message_top] = p;
         cx_message_top++;
         strcpy(cx_message, s);
         cx_writeflag = 1;
      }

/*
 * cx_pop_message - pop the top of the cx_message stack into cx_message
 */

   void fb_cx_pop_message()
      {
         char *p;

         if (cx_control[0] == NULL)
            return;
         if (cx_message_top <= 0)
            fb_xerror(FB_MESSAGE, "Trying to pop unknown message state.", "");
         cx_message_top--;
         p = cx_message_stack[cx_message_top];
         strcpy(cx_message, p);
         fb_free(p);
         cx_message_stack[cx_message_top] = NULL;
         cx_writeflag = 1;
      }

/*
 * fb_cx_push_env - environment push and pop mechanisms
 */

   void fb_cx_push_env(buttons, status, msg)
      char *buttons, *status, *msg;

      {
         if (cx_disable)
            return;
         if (cx_control[0] == NULL)
            return;
         fb_cx_push_status(status);
         fb_cx_push_message(msg);
         fb_cx_push_buttons(buttons);
      }

   void fb_cx_pop_env()
      {
         if (cx_disable)
            return;
         if (cx_control[0] == NULL)
            return;
         fb_cx_pop_status();
         fb_cx_pop_message();
         fb_cx_pop_buttons();
      }

/*
 * cx_null_values - null out the cx variables
 */

   static cx_null_values()
      {
         int i;

         cx_control[0]	= NULL;
         cx_ppid 	= -1;
         cx_buttons[0] 	= NULL;
         cx_menu[0] 	= NULL;
         cx_status[0] 	= NULL;
         cx_toolname[0]	= NULL;
         cx_message[0]	= NULL;
         cx_viewdict[0]	= NULL;
         cx_datadict[0]	= NULL;
         cx_viewpage	= 0;
         cx_seekpoint	= -1;
         cx_seekfile[0]	= NULL;
         cx_workdir[0]	= NULL;
         for (i = 0; i < MAXBUTTONS; i++)
            cx_button_stack[i] = NULL;
         for (i = 0; i < MAXBUTTONS; i++)
            cx_status_stack[i] = NULL;
         cx_writeflag	= 0;
      }

/*
 * cx_signal_1 - send a signal to the registered cx_ppid
 */

   fb_cx_signal_1()
      {
         if (cx_control[0] != NULL && cx_ppid > 0)
            kill(cx_ppid, SIGUSR1);
      }

/*
 * cx_signal_2 - send a signal to the registered cx_ppid
 */

   fb_cx_signal_2()
      {
         if (cx_control[0] != NULL && cx_ppid > 0)
            kill(cx_ppid, SIGUSR2);
      }

/*
 * cx_testcontrol - test whether a control and ppid process exist. 1=yes, 0=no
 *
 */

   fb_cx_testcontrol()
      {
         if (cx_control[0] != NULL && cx_ppid > 0)
            return(1);
         return(0);
      }

   fb_cx_fb_xerror(e, p, q)
      int e;
      char *p, *q;

      {
         cx_disable = 1;
         fb_xerror(e, p, q);
      }

   fb_cx_werror(m, a, b)
      int m;
      char *a, *b;

      {
         FILE *fs;

         fs = fopen("/dev/console", "w");
         if (fs == NULL)
            fs = stderr;
         fprintf(fs, "%s: %s\n", a, b);
         fclose(fs);
      }

#if DEBUG
   cx_trace(s)
      char *s;

      {
         fprintf("cx_trace *** %s:\n", s);
         fprintf("   cx_control: %s\n", cx_control);
         fprintf("   cx_ppid:    %d\n", cx_ppid);
         fprintf("   cx_buttons: %s\n", cx_buttons);
         fprintf("   cx_menu: %s\n", cx_menu);
         fprintf("   cx_status: %s\n", cx_status);
      }
#endif /* DEBUG */
