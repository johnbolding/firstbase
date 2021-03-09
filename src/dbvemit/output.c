/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: output.c,v 9.0 2001/01/09 02:56:02 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Output_sid[] = "@(#) $Id: output.c,v 9.0 2001/01/09 02:56:02 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern fb_field **cdb_sp;		/* screen filter -- for CCOMANDS */
extern short int cdb_sfields;		/* max screen fields */

int quoteflag = 0;			/* quote flag for fields */
char separator = ',';			/* standard separator */
char *begtok, *endtok;			/* beginning and ending tokens */

static int fd;
static char fname[FB_MAXNAME];

#define BLKSIZE	2048

static int w_curp = 0;			/* current pointer into wbuffer */
static char wbuffer[BLKSIZE + 1];	/* for buffered write */
static char *S_QUOTE = "\"";
static char *S_BACKSLASH = "\\";

static nextwrite();
static stringwrite();
static wflush();

/* 
 * output - module to output data from the dbvemit record
 */

   output(hp)
      fb_database *hp;
      
      {
         int i;
	 char *p, *df;
	 
	 cdb_sp = hp->kp;
	 fb_s_lock(fd, 1, fname);
	 lseek(fd, 0L, 2);
	 if (begtok != NULL){
	    stringwrite(begtok);
	    nextwrite(SYSMSG[S_STRING_NEWLINE]);
	    }
         for (i = 0; i < hp->nfields; i++){
	    if (cdb_sp[i]->type != FB_FORMULA && cdb_sp[i]->dflink == NULL &&
	        cdb_sp[i]->type != FB_LINK){
	       if (i != 0)
		  nextwrite(&separator);	/* output separator (,:) */
	       df = cdb_sp[i]->fld;
	       if (!(FB_OFNUMERIC(cdb_sp[i]->type))){
	          if (quoteflag)
	             nextwrite(S_QUOTE);
		  p = df;
		  for ( ; *p; p++){ /* escape emb quotes/backg */
		     if (*p == FB_NEWLINE)
		        stringwrite("\\n");
		     else {
		        if (*p == CHAR_QUOTE || *p == CHAR_BACKSLASH)
		           nextwrite(S_BACKSLASH);
		        nextwrite(p);
			}
		     }
	          if (quoteflag)
	             nextwrite(S_QUOTE);
		  }
	       else {
		  stringwrite(df);
		  }
	       }
	    }
	 nextwrite(SYSMSG[S_STRING_NEWLINE]);
	 if (endtok != NULL){
	    stringwrite(endtok);
	    nextwrite(SYSMSG[S_STRING_NEWLINE]);
	    }
	 wflush();
	 fb_s_unlock(fd, fname);
	 return(FB_AOK);
      }

   outinit(name)
      char *name;

      {
         strcpy(fname, name);
	 if (access(fname, 0) != -1){
	    fd = fb_mustopen(fname, WRITE);
	    lseek(fd, 0L, 2);
	    }
	 else{
	    close(creat(fname, 0666));	/* create fname */
	    fd = fb_mustopen(fname, WRITE);
	    }
      }

   outend()

      {
	 close(fd);
      }

/*
 * nextwrite - semi buffered write - write a block if needed.
 */
   static nextwrite(buf)
      char *buf;

      {
         if (w_curp < 0 || w_curp >= BLKSIZE){
	    if (write(fd, wbuffer, BLKSIZE) != BLKSIZE)
	       return(0);
	    w_curp = 0;
	    }
	 wbuffer[w_curp++] = *buf;
         return(1);
      }

/*
 * stringwrite - semi buffered write - write a block if needed.
 */
   static stringwrite(buf)
      char *buf;

      {
         for (; *buf; buf++){
	    if (w_curp < 0 || w_curp >= BLKSIZE){
	       if (write(fd, wbuffer, BLKSIZE) != BLKSIZE)
		  return(0);
	       w_curp = 0;
	       }
	    wbuffer[w_curp++] = *buf;
	    }
         return(1);
      }

/*
 * wflush - flush anything left in the wbuffer
 */
   static wflush()

      {
         if (w_curp > 0){
	    if (write(fd, wbuffer, w_curp) != w_curp)
	       return(0);
	    w_curp = 0;
	    }
         return(1);
      }
