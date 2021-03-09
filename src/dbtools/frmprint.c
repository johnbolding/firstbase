/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: frmprint.c,v 9.4 2002/12/29 17:15:50 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Frmprint_sid[] = "@(#) $Id: frmprint.c,v 9.4 2002/12/29 17:15:50 john Exp $";
#endif

/* 
 *  formprint.c - provide the familiar screenprint.sh functions
 *	within a Cdb look-and-feel tool. File names of ~/ are allowed too.
 *
 *	This version does localprinting directly using escape sequences.
 *
 *	Formprint is different than screenprint in that it expects a raw
 *	*.me file that needs to be groffed first.
 *	But, if the system printer is selected, it assumes postscript.
 *      For anything else, it formats as ascii.
 */

#include <fb.h>
#include <fb_vars.h>
#include <pwd.h>

#define PAGER		"PAGER"
#define HOME		"HOME"
#define EDITOR		"EDITOR"
#define PRINTER		"PRINTER"
#define PSPRINT		"PSPRINT"

#define DIR 1
#define FIL 2

#define SEL_VIEW	1
#define SEL_VIEW132	2
#define SEL_POSTSCRIPT	3
#define SEL_LOCAL	4
#define SEL_LINE	5
#define SEL_SAVE_FORMATTED	6
#define SEL_EDIT_RAW	7
#define SEL_SAVE_RAW	8
#define SEL_VIEW_RAW	9

static char *cdb_pager = "scripts/fb_pager";
extern char *cdb_home;

char fname[FB_MAXNAME];
char tname[FB_MAXNAME];

int selection;
int landscape;
char option;

static void finit(int argc, char **argv);
static int display(void);
static void command(void);
static pwd_home(char *buf, char *pname);
static int ftype(char *f);
static void usage(void);

/*
 *  formprint - main driver
 */
 
   main(argc, argv)
      int argc;
      char **argv;
   
      {
         fb_getargs(argc, argv, FB_NODB);
	 /* fb_allow_int();		/* allows instant piping */
         
	 finit(argc, argv);
         for (;;){
            if (display() == FB_END)
               break;
            command();
            }
         unlink(tname);
         fb_ender();
      }

/*
 *  finit - check for a second argument, place it in globals.
 */
 
   static void finit(argc, argv)
      int argc;
      char *argv[];
      
      {
	 fname[0] = NULL;
         if (argc != 2)
            usage();
         strcpy(fname, argv[1]);
         strcpy(tname, "/usr/tmp/f_XXXXXX");
         close(mkstemp(tname));
         if (tname[0] == NULL)
            fb_xerror(FB_MESSAGE, "Assert: could not mkstemp unique filename.",
               NIL);
      }

/*
 * display - display the formprint menu
 */

   static int display()
      {
         char buf[FB_MAXLINE];
         int st = FB_AOK;

         fb_scrhdr((fb_database *) NULL, "Formprint Menu");
         fb_infoline();
         fb_scrstat2(fname);
         fb_move(5, 10); fb_prints("Report Output Options: ");

         fb_move(7, 10); sprintf(buf, "%d", SEL_VIEW); fb_reverse(buf);
         fb_prints("  View report on screen");

         fb_move(8, 10); sprintf(buf, "%d", SEL_VIEW132); fb_reverse(buf);
         fb_prints("  View on screen w/132 columns");

         fb_move(9, 10); sprintf(buf, "%d", SEL_POSTSCRIPT); fb_reverse(buf);
         fb_prints("  Print report on System Postscript printer ");
         fb_prints("(3L=landscape)");

         fb_move(10, 10); sprintf(buf, "%d", SEL_LOCAL); fb_reverse(buf);
         fb_prints("  Print report on Local printer");

         fb_move(11, 10); sprintf(buf, "%d", SEL_LINE); fb_reverse(buf);
         fb_prints("  Print report on System line printer ");

         fb_move(12, 10); sprintf(buf, "%d", SEL_SAVE_FORMATTED);
            fb_reverse(buf);
         fb_prints("  Save formatted report into specified file");

         fb_move(13,10); sprintf(buf, "%d", SEL_EDIT_RAW); fb_reverse(buf);
         fb_prints("  Edit raw report file");

         fb_move(14,10); sprintf(buf, "%d", SEL_SAVE_RAW); fb_reverse(buf);
         fb_prints("  Save raw report file");

         fb_move(15,10); sprintf(buf, "%d", SEL_VIEW_RAW); fb_reverse(buf);
         fb_prints("  View raw report file");

         fb_move(17,10);fb_reverse("-"); fb_prints("  Quit");

         fb_move(19,10); fb_prints("Select Report Output Option: ");

         for (; st != FB_END;){
            st = fb_input(19, 42, 2, 0, FB_ALPHA, buf, FB_ECHO, FB_OKEND,
               FB_CONFIRM);
            if (st == FB_AOK)
               if (buf[0] == 'q' || (buf[0] >= '1' && buf[0] <= '9'))
                  break;
            }
         if (buf[0] == 'q')
            st = FB_END;
         if (st == FB_AOK){
            selection = atoi(buf);
            if (buf[1] == 'l' || buf[1] == 'L')
               landscape = 1;
            else
               landscape = 0;
            }
         else
            selection = 0;
         return(st);
      }

/*
 * command - do the actual command entered
 */

   static void command()

      {
	 char *p;
         char buf[FB_MAXNAME], ibuf[FB_MAXNAME], msg[FB_MAXNAME];
         char pfile[FB_MAXNAME], pname[FB_MAXNAME], dname[FB_MAXNAME];
         char obuf[FB_MAXLINE], lbuf[FB_MAXLINE], olist[FB_MAXNAME];
         int st, t, gview = 0;

         switch(selection){
            case SEL_VIEW_RAW:
               if ((p = getenv(PAGER)) != 0)
                  strcpy(ibuf, p);
               else{
                  strcpy(ibuf, cdb_home);
                  fb_assure_slash(ibuf);
                  strcat(ibuf, cdb_pager);
                  }
               sprintf(buf, "%s %s", ibuf, fname);
               st = fb_system(buf, FB_NOROOT);
               break;
            case SEL_VIEW:
               if ((p = getenv(PAGER)) != 0)
                  strcpy(ibuf, p);
               else{
                  strcpy(ibuf, cdb_home);
                  fb_assure_slash(ibuf);
                  strcat(ibuf, cdb_pager);
                  }

               fb_fmessage("formatting for fixed width printer ...");
               sprintf(buf, "pic %s | gtbl | groff -me -Tascii > %s", fname, tname);
               st = fb_system(buf, FB_NOROOT);
               fb_fmessage("Done.");
               fb_move(1, 1);
               fb_clear();
               fb_refresh();
               sprintf(buf, "%s %s", ibuf, tname);
               st = fb_system(buf, FB_NOROOT);
               break;
            case SEL_VIEW132:
               fb_fmessage("formatting for fixed width printer ...");
               sprintf(buf, "pic %s | gtbl | groff -me -Tascii > %s", fname, tname);
               st = fb_system(buf, FB_NOROOT);
               fb_fmessage("Done.");
               sprintf(buf, "less132 %s", tname);
               st = fb_system(buf, FB_NOROOT);
               break;
            case SEL_POSTSCRIPT:
            case SEL_LINE:
               fb_move(19, 10);
               fb_prints("Enter PRINTER NAME (<RETURN> for default printer):");
               st = fb_input(19, 60, 20, 0, FB_ALPHA, ibuf, FB_ECHO, FB_OKEND,
                  FB_CONFIRM);
               if (st == FB_AOK){
                  fb_trim(ibuf);
                  strcpy(msg, ibuf);
                  }
               else{
                  if ((p = getenv(PRINTER)) != 0)
                     strcpy(ibuf, p);
                  else
                     strcpy(ibuf, "lp");
                  strcpy(msg, "your default printer");
                  }

               fb_move(19, 10); fb_clrtoeol();
               fb_prints("Enter Page Options [M-N] (<RETURN> for All Pages):");
               st = fb_input(19, 61, 18, 0, FB_ALPHA, buf, FB_ECHO, FB_OKEND,
                  FB_CONFIRM);
               if (st == FB_AOK){
                  fb_trim(fb_rmlead(buf));
                  sprintf(olist, "-o%s", buf);
                  }
               else
                  olist[0] = NULL;

               /* put any options into obuf */
               obuf[0] = NULL;
               if (landscape && selection == SEL_POSTSCRIPT)
                  strcpy(lbuf, "-P-l");
               else
                  lbuf[0] = NULL;
               if (st == FB_AOK || st == FB_DEFAULT){
                  if (selection == SEL_POSTSCRIPT){
                     fb_fmessage("formatting for postscript printer ...");
                     sprintf(buf, "pic %s | gtbl | groff -me %s -Tps %s > %s",
                        fname, olist, lbuf, tname);
                     }
                  else{
                     fb_fmessage("formatting for line printer ...");
                     sprintf(buf, "pic %s | gtbl | groff -me %s -Tascii %s > %s",
                        fname, olist, lbuf, tname);
                     }
                  st = fb_system(buf, FB_NOROOT);
                  if (st == 0)
                     fb_fmessage("Done.");
                  else{
                     fb_move(cdb_t_lines - 2, 1);
                     fb_clrtoeol();
                     fb_prints(buf);
                     sprintf(buf, "status=%d", st);
                     fb_serror(FB_MESSAGE, "error status: ", buf);
                     fb_move(cdb_t_lines - 2, 1);
                     fb_clrtoeol();
                     break;
                     }
                  if ((p = getenv("USE_GVIEW")) != 0){
                     if (equal(p, "ON")){
                        gview = 1;
                        }
                     }
                  if (!gview){
#if HAVE_LPR
                     sprintf(buf, "/bin/csh -f -c \"lpr -P%s %s\"",
                        ibuf, tname);
#else
                     sprintf(buf, "/bin/csh -f -c \"lp -c -d %s %s %s\"",
                        ibuf, obuf, tname);
#endif /* ! HAVE_LPR */
                     }
                  else{
                     sprintf(buf,
                        "/bin/csh -f -c \"ghostview -magstep -1 %s\"",
                        tname);
                     }
                  fb_move(19, 10); fb_clrtoeol();
                  fb_move(20, 10); fb_clrtoeol();
                  fb_move(19, 10); fb_prints("...");
                  fb_move(cdb_t_lines - 5, 1);
                  fb_refresh();
                  st = fb_system(buf, FB_NOROOT);
                  if (st != 0){
                     fb_move(cdb_t_lines - 2, 1);
                     fb_clrtoeol();
                     fb_prints(buf);
                     sprintf(buf, "status=%d", st);
                     fb_serror(FB_MESSAGE, "error status: ", buf);
                     fb_move(cdb_t_lines - 2, 1);
                     fb_clrtoeol();
                     }
                  fb_move(19, 1); fb_clrtoeol();
                  fb_move(19, 10);
                  fb_prints(
                    "The report you requested is being printed from the file");
                  fb_move(20, 1); fb_clrtoeol();
                  fb_move(20, 10);
                  fb_printw("named %s on %s using option `%s'.",
                     fname, msg, olist);
                  fb_refresh();
                  fb_serror(FB_MESSAGE, NIL, NIL);
                  }
               break;
            case SEL_LOCAL:
               /* LOCAL local print */
               fb_fmessage("formatting for fixed width printer ...");
               sprintf(buf, "pic %s | gtbl | groff -me -Tascii > %s", fname, tname);
               st = fb_system(buf, FB_NOROOT);
               fb_fmessage("Done.");
               fb_move(19, 10); fb_clrtoeol();
               fb_move(20, 10); fb_clrtoeol();
               fb_move(19, 10); fb_prints("...");
               fb_move(cdb_t_lines - 2, 10);
               fb_refresh();
               st = localprint(tname, 0);
               if (st == FB_AOK){
                  fb_move(19, 1); fb_clrtoeol(); fb_move(19, 10);
                  fb_prints("The report you requested has been printed from");
                  fb_move(20, 1); fb_clrtoeol(); fb_move(20, 10);
                  fb_printw("the file named %s to your local printer.", fname);
                  }
               else{
                  fb_move(19, 1); fb_clrtoeol(); fb_move(19, 10);
                  fb_printw("ERROR trying to localprint file %s.", fname);
                  }
               fb_refresh();
               fb_redraw();
               fb_serror(FB_MESSAGE, NIL, NIL);
               break;
            case SEL_SAVE_FORMATTED:
            case SEL_SAVE_RAW:
               /*
                * SEL_SAVE_FORMATTED and SEL_SAVE_RAW do almost the same thing.
                * SEL_SAVE_FORMATTED formats also, where SEL_SAVE_RAW just
                * saves the raw report
                */
               fb_move(19, 10); fb_clrtoeol();
               fb_prints("Enter FILE NAME:");
               st = fb_input(19, 27, 50, 0, FB_ALPHA, ibuf, FB_ECHO, FB_OKEND,
                  FB_CONFIRM);
               if (st == FB_AOK){
                  fb_trim(ibuf);
                  if (ibuf[0] == '~'){
                     if (ibuf[1] == '/'){
                        if ((p = getenv(HOME)) == 0)
                           p = NIL;
                        strcpy(buf, ibuf + 1);
                        sprintf(ibuf, "%s%s", p, buf);
                        }
                     else{
                        /* must be ~user/xxx which means passwd lookup */
                        p = strchr(ibuf, '/');
                        if (p != 0)
                           *p = NULL;
                        strcpy(pname, ibuf + 1);
                        pfile[0] = NULL;
                        if (p != 0)
                           strcpy(pfile, p + 1);
                        if (pwd_home(buf, pname) == FB_ERROR){
                           fb_serror(FB_MESSAGE, "No user named", pname);
                           break;
                           }
                        sprintf(ibuf, "%s/%s", buf, pfile);
                        }
                     }
                  t = ftype(ibuf);
                  if (t == DIR){
                     if (ibuf[strlen(ibuf) - 1] != '/')
                        strcat(ibuf, "/");
                     strcat(ibuf, fname);
                     }
                  fb_move(19, 27);
                  fb_clrtoeol();
                  fb_prints(ibuf);
                  fb_refresh();
                  fb_dirname(dname, ibuf);
                  if (st != FB_ERROR && dname[0] != NULL &&
                        access(dname, 2) != 0){
                     fb_serror(FB_MESSAGE, "Permission Denied:", dname);
                     st = FB_ERROR;
                     }
                  if (st != FB_ERROR && access(ibuf, 0) == 0){
                     fb_move(19, 10); fb_clrtoeol();
                     fb_printw("Warning: File %s already exists", ibuf);
                     sprintf(msg, "Ok to OVERWRITE FILE %s (y/n) ?", ibuf);
                     st = fb_mustbe('y', msg, 20, 10);
                     }
                  if (st == FB_AOK){
                     if (selection == 5){
                        fb_fmessage("formatting for fixed width printer ...");
                        sprintf(buf, "pic %s | gtbl | groff -me -Tascii > %s",
                           fname, tname);
                        st = fb_system(buf, FB_NOROOT);
                        fb_fmessage("Done.");
                        sprintf(buf, "cp %s %s", tname, ibuf);
                        }
                     else{
                        sprintf(buf, "cp %s %s", fname, ibuf);
                        }
                     fb_move(19, 10); fb_clrtoeol();
                     fb_move(20, 10); fb_clrtoeol();
                     fb_move(19, 10); fb_prints("...");
                     fb_refresh();
                     st = fb_system(buf, FB_NOROOT);
                     fb_move(19, 10); fb_clrtoeol();
                     fb_prints(
                        "The report you requested has been copied to the");
                     fb_move(20, 10); fb_clrtoeol();
                     fb_printw("file named %s.", ibuf);
                     fb_serror(FB_MESSAGE, NIL, NIL);
                     }
                  }
               break;
            case SEL_EDIT_RAW:
               sprintf(msg, "Really EDIT this file (y/n) ?");
               st = fb_mustbe('y', msg, 20, 10);
               if (st != FB_AOK)
                  break;
               fb_move(1, 1);
               fb_clear();
               fb_refresh();
               if ((p = getenv(EDITOR)) != 0)
                  strcpy(buf, p);
               else
                  strcpy(buf, "vi");
               sprintf(buf, "%s %s", buf, fname);
               st = fb_system(buf, FB_NOROOT);
               break;
            default:
               fb_serror(FB_MESSAGE, "Unknown command", NIL);
            }
      }

/*
*  pwd_home - look up pname in the passwd file and return its home in buf
*		return FB_ERROR if not a valid pname.
*/

   static pwd_home(buf, pname)
      char *buf, *pname;

      {
         struct passwd *gp;

         gp = getpwnam(pname);
         if (gp == NULL)
            return(FB_ERROR);
         strcpy(buf, gp->pw_dir);
         return(FB_AOK);
      }

/*
 * ftype - return DIR or FIL depending on type of file f
 */

   static int ftype(f)
      char *f;

      {
         struct stat statb;

         if (stat(f,&statb)<0)
            return(0);
         if ((statb.st_mode&S_IFMT)==S_IFDIR)
            return(DIR);
         return(FIL);
      }

/* 
 *  usage message 
 */
 
   static void usage()
      {
         fb_xerror(FB_MESSAGE, "usage: frmprint file", NIL);
      }

/*
 * localprint - use the wyse local print characters to dump a file
 *    to a local printer.
 */

   int localprint(fname, epson)
      char *fname;
      int epson;

      {
         char buf[2000], tname[FB_MAXLINE];
         int ffd, nchars, st;
         FILE *fs;

         ffd = open(fname, READ);
         if (ffd < 0)
            return(FB_ERROR);

         strcpy(tname, "/usr/tmp/local_XXXXXX");
         close(mkstemp(tname));
         fs = fopen(tname, "w");

         fprintf(fs, "\033[5i");
         if (epson){
            /* these escapge sequences for EPSON style label printer */
            fprintf(fs, "\033*\035t\056\035V1\033M");
            /* esc * gs t 46 gs V1 esc M */
            /*
             * this is the code that was working. could be esc t 46 would do.
             * fprintf(fs, "*tV1A");
             */
            }
         for (;;){
            nchars = read(ffd, buf, 128);
            if (nchars <= 0)
               break;
            buf[nchars] = 0;
            fprintf(fs, "%s", buf);
            }
         close(ffd);

         fprintf(fs, "\033[4i");
         fclose(fs);
         sprintf(buf, "cat %s", tname);
         st = system(buf);
         sleep(1);
         unlink(tname);

         return(FB_AOK);
      }
