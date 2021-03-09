/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbls.c,v 9.1 2001/01/16 02:46:47 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char *Dbls_sid = "%W% %G% FB";
#endif

#include <fb.h>
#include <fb_vars.h>
                                                                  
#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif


typedef struct dirent dstruct;

dstruct *pp;

static void onels();
static display();
static dload();
static dshow();
static compar();
static void afile();
static void install();

static char *DDOT = ".";

#define NTYPES		19		/* MUST BE MULTIPLE OF 3 + 1 */
#define MAXFILES	500

static char *UTYPES[NTYPES] = {		/* MUST BE MULTIPLE OF 3 + 1 */
   0,
   "========",				/* 1 dbase */
   "===================",		/* 2 ddict */
   "============",			/* 3 map */
   "=====",				/* 4 idx */
   "================",			/* 5 idict */
   "==========================",	/* 6 idicti */
   "===============",			/* 7 view */
   "=================",			/* 8 screen */
   "=====================",		/* 9 conversion */
   "========",				/* 10 printout */
   "===================",		/* 11 print dict */
   "=========",				/* 12 emit */
   "======",				/* 13 labels */
   "================",			/* 14 label dict */
   "=================",			/* 15 update */
   "==============",			/* 16 */
   "===========",			/* 17 */
   0
   };

static char *FTYPES[NTYPES] = {		/* MUST BE MULTIPLE OF 3 + 1 */
   0,
   "Database",				/* 1 */
   "Database Dictionary",		/* 2 */
   "Database Map",			/* 3 */
   "Index",				/* 4 */
   "Index Dictionary",			/* 5 */
   "Index Generator Dictionary",	/* 6 */
   "View Dictionary",			/* 7 */
   "Screen Dictionary",			/* 8 */
   "Conversion Dictionary",		/* 9 */
   "Printout",				/* 10 */
   "Printout Dictionary",		/* 11 */
   "Emit File",				/* 12 */
   "Labels",				/* 13 */
   "Label Dictionary",			/* 14 */
   "Update Dictionary",			/* 15 */
   "No-Delete Lock",			/* 16 */
   "No-Add Lock",			/* 17 */
   0
   };

struct elem {
   char *fname;
   struct elem *next;
   struct elem *prev;
   };

struct elem *e_files[NTYPES] = {NULL};

/*
 * dbls - 
 *	this version expects 4.2 directory routines opendir, readdir, etc.
 *	these routines are available in a library usually called libndir.a
 */

   main(argc, argv)
      int argc;
      char **argv;
   
      {
	 int i;

         fb_setup();
	 for (i = 0; i < NTYPES; i++)
	    e_files[i] = NULL;
         if (argc <= 1)
            onels(DDOT);
         else
            for (--argc, ++argv; argc > 0; argc--){
               onels(*argv);
               argv++;
               }
      }

/*
 * onels - do one dbls command, on DDOT or given argument (directory)
 */

   static void onels(s)
      char *s;

      {
	 DIR *dirpp;
	 dstruct *pp;

	 dirpp = opendir(s);
         if (dirpp == NULL){
            fb_serror(FB_CANT_OPEN, s, NIL);
            return;
            }
         pp = readdir(dirpp);
	 for (; pp != NULL;  pp = readdir(dirpp))
	    if (pp->d_name[0] != '.')
	       afile(pp->d_name);
	 closedir(dirpp);
	 display();
      }

/* display - driver to display the argvecs three at a time */
   static display()
   
      {
         int n, typ;
	 char **p1, **p2, **p3, **argvec1, **argvec2, **argvec3;
	 
	 argvec1 = (char **) fb_malloc(sizeof(char *) * MAXFILES);
	 argvec2 = (char **) fb_malloc(sizeof(char *) * MAXFILES);
	 argvec3 = (char **) fb_malloc(sizeof(char *) * MAXFILES);
	 p1 = argvec1; p2 = argvec2; p3 = argvec3;
	 for (n = 0; n < MAXFILES; n++){
	    *p1++ = NULL;
	    *p2++ = NULL;
	    *p3++ = NULL;
	    }
	 for (typ = 1; typ < NTYPES; ){
	    dload(argvec1, typ++);
	    dload(argvec2, typ++);
	    dload(argvec3, typ++);
	    dshow(argvec1, argvec2, argvec3);
	    }
      }

/* dload - load in a single argvec */
   static dload(a, t)
      char **a;
      int t;
      
      {
         int n, compar();
	 struct elem *e;
	 char **astart;
         
	 if (e_files[t] != NULL){
	    *a++ = FTYPES[t];
	    *a++ = UTYPES[t];
	    astart = a;
	    for (n = 0, e = e_files[t]; e != NULL; e = e->next){
	       *a++ = e->fname;
	       if (++n >= MAXFILES || e == e_files[t]->prev)
		  break;
	       }
	    *a = NULL;
	    qsort(astart, n, sizeof(char *), compar);
	    }
	 else
	    *a = NULL;
      }

/* dshow - show three argvecs */
   static dshow(a, b, c)
      char **a, **b, **c;
      
      {
         char **p, *buf, line[FB_MAXLINE], temp[FB_MAXLINE];
	 int output = 0;
	 
	 for (;;){
	    line[0] = temp[0] = NULL;
	    if (!(*a) && !(*b) && !(*c))
	       break;
	       
	    buf = NIL;
	    if (*a)
	       buf = *a++;
	    strcat(line, fb_pad(temp, buf, 26));
	       
	    buf = NIL;
	    if (*b)
	       buf = *b++;
	    strcat(line, fb_pad(temp, buf, 26));
	       
	    buf = NIL;
	    if (*c)
	       buf = *c++;
	    strcat(line, fb_pad(temp, buf, 26));
	    if (strlen(line) > 0){
	       puts(line);
	       output = 1;
	       }
	    }
	 if (output)
	    puts(NIL);
      }

/* compar - compar two **char objects */
   static compar(a, b)
      char **a, **b;
      
      {
         return(strcmp(*a, *b));
      }

/* afile - determine if f is a cdb file */
   static void afile(f)
      char *f;
      
      {
         char *ext;
	 int typ;
	 
	 ext = strrchr(f, '.');
	 if (ext == 0)
	    return;
	 *ext++ = NULL;
	 if (equal(ext, "cdb"))
	    typ = 1;
	 else if (equal(ext, "ddict"))
	    typ = 2;
	 else if (equal(ext, "map"))
	    typ = 3;
	 else if (equal(ext, "idx"))
	    typ = 4;
	 else if (equal(ext, "idict"))
	    typ = 5;
	 else if (equal(ext, "idicti"))
	    typ = 6;
	 else if (equal(ext, "vdict"))
	    typ = 7;
	 else if (equal(ext, "sdict"))
	    typ = 8;
	 else if (equal(ext, "idictc"))
	    typ = 9;
	 else if (equal(ext, "prt"))
	    typ = 10;
	 else if (equal(ext, "idictp"))
	    typ = 11;
	 else if (equal(ext, "emit"))
	    typ = 12;
	 else if (equal(ext, "lbl"))
	    typ = 13;
	 else if (equal(ext, "idictl"))
	    typ = 14;
	 else if (equal(ext, "idictu"))
	    typ = 15;
	 else if (equal(ext, "nodel"))
	    typ = 16;
	 else if (equal(ext, "noadd"))
	    typ = 17;
	 else
	    return;
	 install(f, typ);
      }

/* install f as a cdb file of typ */
   static void install(f, typ)
      char *f;
      int typ;
      
      {
         struct elem *e, *h;
	 
	 e = (struct elem *) fb_malloc(sizeof(struct elem));
	 e->fname = NULL;
	 fb_mkstr(&(e->fname), f);
	 if (e_files[typ] == NULL){
	    e_files[typ] = e;
	    e->next = e->prev = e;
	    return;
	    }
	 e->next = e_files[typ];
	 e->prev = e_files[typ]->prev;
	 e_files[typ]->prev->next = e;
	 e_files[typ]->prev = e;
      }
