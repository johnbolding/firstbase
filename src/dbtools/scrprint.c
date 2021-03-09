/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: scrprint.c,v 9.4 2001/02/17 22:00:49 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Scrprint_sid[] = "@(#) $Id: scrprint.c,v 9.4 2001/02/17 22:00:49 john Exp $";
#endif

/* 
 *  screenprint.c - provide the familiar screenprint.sh functions
 *	within a Cdb look-and-feel tool. File names of ~/ are allowed too.
 *
 *	This version does localprinting via the vlprint command,
 *	a shell script that just does the escape sequences, and the cat.
 */

#include <fb.h>
#include <fb_vars.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PAGER		"PAGER"
#define HOME		"HOME"
#define EDITOR		"EDITOR"
#define PRINTER		"PRINTER"
#define PSPRINT		"PSPRINT"

#define DIR 1
#define FIL 2

static char *cdb_pager = "scripts/fb_pager";
extern char *cdb_home;

char fname[FB_MAXNAME];
int selection;
char option;

static void finit(int argc, char **argv);
static int display(void);
static void command(void);
static int pwd_home(char *buf, char *pname);
static int ftype(char *f);
static void usage(void);

/*
 *  screenprint - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         fb_getargs(argc, argv, FB_NODB);
	 fb_allow_int();		/* allows instant piping */
         
	 finit(argc, argv);
         for (;;){
            if (display() == FB_END)
               break;
            command();
            }
         fb_ender();
      }

/*
 *  finit - check for a second argument, place it in globals.
 */
 
   static void finit(argc, argv)
      int argc;
      char **argv;
      
      {
	 fname[0] = NULL;
         if (argc != 2)
            usage();
         strcpy(fname, argv[1]);
      }

/*
 * display - display the screenprint menu
 */

   static int display()
      {
         char buf[FB_MAXLINE];
         int st = FB_AOK;

         fb_scrhdr((fb_database *) NULL, "Screenprint Menu");
         fb_infoline();
         fb_scrstat2(fname);
         fb_move(5, 10); fb_prints("Report Output Options: ");
         fb_move(7, 10);fb_reverse("1"); fb_prints("  View report on screen");
         fb_move(8, 10);fb_reverse("2");
            fb_prints("  View on screen w/132 columns");
         fb_move(9, 10);fb_reverse("3");
             fb_prints("  Print report on System printer");
         fb_move(10, 10);fb_reverse("4");
             fb_prints("  Print report on Local printer");
         fb_move(11, 10);fb_reverse("5");
             fb_prints("  Print report on Local Label printer");
         fb_move(12, 10);fb_reverse("6");
            fb_prints("  Save report into specified file");
         fb_move(13,10);fb_reverse("7"); fb_prints("  Edit report file");
         fb_move(15,10);fb_reverse("-"); fb_prints("  Quit");
         fb_move(17,10); fb_prints("Select Report Output Option: ");
         for (; st != FB_END;){
            st = fb_input(17, 42, 2, 0, FB_ALPHA, buf, FB_ECHO, FB_OKEND,
               FB_CONFIRM);
            if (st == FB_AOK)
               if (buf[0] == 'q' || (buf[0] >= '1' && buf[0] <= '7'))
                  break;
            }
         if (buf[0] == 'q')
            st = FB_END;
         if (st == FB_AOK){
            selection = atoi(buf);
            if (selection == 3)
               switch(buf[1]){
                  case 'b':
                  case 's':
                  case 'l':
                     option = buf[1];
                     break;
                  default:
                     option = NULL;
                     break;
                  }
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
         char obuf[FB_MAXLINE];
         int st, t;
         float margin;

         switch(selection){
            case 1:
               fb_move(1, 1);
               fb_clear();
               fb_refresh();
               if ((p = getenv(PAGER)) != 0)
                  strcpy(buf, p);
               else{
                  strcpy(buf, cdb_home);
                  fb_assure_slash(buf);
                  strcat(buf, cdb_pager);
                  }
               sprintf(buf, "%s %s", buf, fname);
               fb_system(buf, FB_NOROOT);
               break;
            case 2:
               strcpy(buf, "less132");
               sprintf(buf, "%s %s", buf, fname);
               fb_system(buf, FB_NOROOT);
               break;
            case 3:
               fb_move(17, 10);
               fb_prints("Enter PRINTER NAME (<RETURN> for default printer):");
               st = fb_input(17, 60, 20, 0, FB_ALPHA, ibuf, FB_ECHO, FB_OKEND,
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
               obuf[0] = NULL;
               /*
               switch(option){
                  case 'b': 	strcpy(obuf, "-o10s"); break;
                  case 's':	strcpy(obuf, "-o15s"); break;
                  case 'l':	strcpy(obuf, "-o10s -olq"); break;
                  default:	obuf[0] = NULL; break;
                  }
               */
               if (st == FB_AOK || st == FB_DEFAULT){
                  if ((p = getenv(PSPRINT)) != 0){
                     fb_move(19, 10);
                     margin = 0.5;
                     fb_printw("Top/Bottom margin = %.1f inches ", margin);
                     fb_move(20, 10);
                     fb_prints("<RETURN> OR Enter New Margin Value: ");
                     st = fb_input(20, 46, 5, 0,FB_FLOAT,buf,FB_ECHO,FB_OKEND,
                        FB_CONFIRM);
                     if (st != FB_AOK && st != FB_DEFAULT)
                        break;
                     if (st == FB_AOK)
                        margin = atof(buf);
#if HAVE_LPR
                     sprintf(buf, "%s -m%f %s | lpr -P%s", p, margin, fname,
                        ibuf);
#else
                     sprintf(buf, "%s -m%f %s | lp -c -d %s %s", p, margin,
                        fname, ibuf, obuf);
#endif /* ! HAVE_LPR */
                     }
                  else
#if HAVE_LPR
                     sprintf(buf, "lpr -P%s %s", ibuf, fname);
#else
                     sprintf(buf, "lp -c -d%s %s %s", ibuf, obuf, fname);
#endif /* ! HAVE_LPR_V */
                  fb_move(19, 10); fb_clrtoeol();
                  fb_move(20, 10); fb_clrtoeol();
                  fb_move(19, 10); fb_prints("...");
                  fb_move(cdb_t_lines - 2, 10);
                  fb_refresh();
                  fb_system(buf, FB_NOROOT);
                  fb_move(19, 1); fb_clrtoeol();
                  fb_move(19, 10);
                  fb_prints(
                    "The report you requested is being printed from the file");
                  fb_move(20, 1); fb_clrtoeol();
                  fb_move(20, 10);
                  fb_printw("named %s on %s using option `%c'.",
                     fname, msg, option);
                  fb_refresh();
                  fb_serror(FB_MESSAGE, NIL, NIL);
                  }
               break;
            case 4:
            case 5:
               /* LOCAL local print */
               sprintf(buf, "vlprint %s", fname);
               fb_move(19, 10); fb_clrtoeol();
               fb_move(20, 10); fb_clrtoeol();
               fb_move(19, 10); fb_prints("...");
               fb_move(cdb_t_lines - 2, 10);
               fb_refresh();
               if (selection == 4)
                  st = localprint(fname, 0);
               else
                  st = localprint(fname, 1);
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
            case 6:
               fb_move(17, 10); fb_clrtoeol();
               fb_prints("Enter FILE NAME:");
               st = fb_input(17, 27, 50, 0, FB_ALPHA, ibuf, FB_ECHO, FB_OKEND,
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
                        p = index(ibuf, '/');
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
                  fb_move(17, 27);
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
                     sprintf(buf, "cp %s %s", fname, ibuf);
                     fb_move(19, 10); fb_clrtoeol();
                     fb_move(20, 10); fb_clrtoeol();
                     fb_move(19, 10); fb_prints("...");
                     fb_refresh();
                     fb_system(buf, FB_NOROOT);
                     fb_move(19, 10); fb_clrtoeol();
                     fb_prints(
                        "The report you requested has been copied to the");
                     fb_move(20, 10); fb_clrtoeol();
                     fb_printw("file named %s.", ibuf);
                     fb_serror(FB_MESSAGE, NIL, NIL);
                     }
                  }
               break;
            case 7:
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
               fb_system(buf, FB_NOROOT);
               break;
            default:
               fb_serror(FB_MESSAGE, "Unknown command", NIL);
            }
      }

/*
*  pwd_home - look up pname in the passwd file and return its home in buf
*		return FB_ERROR if not a valid pname.
*/

   static int pwd_home(buf, pname)
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
         fb_xerror(FB_MESSAGE, "usage: scrprint file", NIL);
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
         int ffd, nchars;
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
         fb_system(buf, FB_NOROOT);
         sleep(1);
         unlink(tname);

         return(FB_AOK);
      }
