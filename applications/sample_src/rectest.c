/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: rectest.c,v 9.0 2001/01/22 18:28:07 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 * rectest.c:
 *      main program for rectest, a simple record i/o test
 *      controls reading and writing of records to a
 *      firstbase database named 'dbase'.
 */

#include <fb.h>
#include <fb_vars.h>

   main()
      
      {
	 int quit = 0;
	 char buf[FB_MAXLINE];
         fb_database *d;
	 
	 /* allocate database d */
	 d = fb_dballoc();

	 /* assign names to fb_database - just do database here */
	 fb_dbargs("dbase", NIL, d);

	 /* open database d with readwrite. this creates d files if needed. */
	 fb_opendb(d, READWRITE, 0, 0);

	 for (;;){
	    printf("(r)ead record, (w)rite record, (q)uit: ");
	    if (fgets(buf, FB_MAXLINE, stdin) == NULL)
	       break;
	    switch(buf[0]){
	       case 'r':
	          dbread(d);
		  break;
	       case 'w':
	          dbwrite(d);
		  break;
	       case 'q':
	          quit = 1;
	          break;
	       default:
	          printf("?\n");
		  break;
	       }
	    if (quit)
	       break;
	    }
	 /* close all open file descriptors in fb_database d, etc */
	 fb_closedb(d);
      }
