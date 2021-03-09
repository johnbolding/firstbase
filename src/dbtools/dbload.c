/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbload.c,v 9.1 2001/02/16 19:47:51 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbload_sid[] = "@(#) $Id: dbload.c,v 9.1 2001/02/16 19:47:51 john Exp $";
#endif

/* 
 *  dbload.c - load a file of fb_field values in comma seperated format 
 *     into a cdb fb_database. needs the dictionary to load describe
 *     how to load the values.
 */

#include <fb.h>
#include <fb_vars.h>
#include <emit.h>

#if !FB_PROTOTYPES
static int load();
static int aload();
static int simplefield();
static finit();
static void usage();
#else /* FB_PROTOTYPES */
extern main(int, char **);
static int load(void);
static int aload(void);
static int simplefield(void);
static finit(int argc, char **argv);
static void usage(void);
#endif /* FB_PROTOTYPES */

extern char *cdb_DBCLEAN;
extern short int cdb_secure;
extern char *cdb_S_EOREC;
extern char *cdb_S_FILLC;
extern char *cdb_S_FILLE;
extern short int cdb_dbase_byteorder;
extern short int cdb_cpu_byteorder;

static char *FMT1 = "%04d%03d%s";	/* secure bits */

static char lastc = NULL;		/* last char seen - pushback kinda */
static char separator = ',';		/* standard separator */
static append = 0;			/* -a flag for fast append */
static long rlen = 0L, size = 0L;
static lengthcheck = 0;

extern short int cdb_interrupt;

/*
 *  dbload - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];
   
      {
         char str[FB_MAXLINE], com[FB_MAXLINE];
	 int j, st, ddflag = 0;

         (void) Dbload_sid;

         fb_getargs(argc, argv, FB_ALLOCDB);
	 if (cdb_interrupt)
            fb_allow_int();
	 if (fb_testargs(argc, argv, "-dd") > 0)
	    ddflag = 1;
         if (fb_getd_dict(cdb_db) == FB_ERROR)
            fb_xerror(FB_BAD_DICT, cdb_db->ddict, NIL);
	 if ((j = fb_testargs(argc, argv, "-a")) > 0){
	    append = 1;
	    fb_opendb(cdb_db, READWRITE, FB_NOINDEX, 0);
	    }
         lengthcheck = 1;
         fb_getauto(cdb_db, READWRITE);
         fb_scrhdr(cdb_db, "Open Files"); fb_infoline();
	 if ((j = fb_testargs(argc, argv, "-c")) > 0){
	    if (++j >= argc)
	       usage();
	    if (argv[j][1] != NULL)
	       usage();
	    separator = argv[j][0];
	    }
	 finit(argc, argv);
	 if (!cdb_batchmode && !cdb_yesmode){
	    if (!append)
	       sprintf(str, "Ok to Load into FirstBase Database %s (y/n) ?", 
		  cdb_db->dbase);
	    else
	       sprintf(str, "Ok to Append to FirstBase Database %s (y/n) ?", 
		  cdb_db->dbase);
	    if (fb_mustbe('y', str, cdb_t_lines, 1) == FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
	 if (append)
	    st = aload();
	 else
	    st = load();
         if (st == FB_ERROR)
	    fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	 if (append){
	    fb_closedb(cdb_db);
	    }
	 else {
	    fb_rootname(str, cdb_db->dbase);
	    if (!cdb_batchmode){
	       fb_move(4,1), fb_clrtobot();
	       fb_scrstat("Cleaning");
	       sprintf(com, "Cleaning %s", str);
	       fb_fmessage(com);
	       }
	    sprintf(com, "%s -b %s", cdb_DBCLEAN, str);
            if (ddflag){
               fb_rootname(str, cdb_db->ddict);
               sprintf(com, "%s -dd %s", com, str);
               }
	    fb_system(com, FB_WITHROOT);
	    }
	 fb_r_end();
	 fb_ender();
      }

/* 
 *  load - load all lines from a load file (fd) into a FirstBase database 
 */
 
   static int load()
      
      {
         long n;
	 int i, eof, st, nc, line_no = 1;
         char perm[FB_MAXNAME], tbuf[FB_MAXLINE];

	 n = 0L;
         if (!cdb_batchmode){
	    FB_XWAIT();
	    fb_gcounter(n);
	    }
	 close(creat(cdb_db->dbase, 0666));		/* create the dbase */
         cdb_db->fd = fb_mustopen(cdb_db->dbase, 1);
	 fb_putseq(cdb_db->fd);
	 fb_puthead(cdb_db->fd, 0L, 0L);
         if (cdb_secure){
            if (fb_putmode(cdb_db->fd, fb_getuid(), fb_getgid(),
                  "666") == FB_ERROR)
               fb_xerror(FB_IO_ERROR, cdb_db->dbase, NIL);
	    sprintf(perm, FMT1, fb_getuid(), fb_getgid(), "666");
            }
	 fb_w_init(1, cdb_db->fd, -1);
	 for (i = 0, eof = 0 ; ; ){
	    for (i = 0; i < cdb_db->nfields; i++)
	       if (cdb_kp[i]->type != FB_FORMULA &&
                     cdb_kp[i]->dflink == NULL && cdb_kp[i]->type != FB_LINK){
	          if (lastc != FB_NEWLINE){
		     if (simplefield() == EOF){	/* returns in cdb_bfld */
			eof = 1;
			break;
			}
                     if (cdb_kp[i]->type != FB_BINARY){
                        if (lengthcheck && strlen(cdb_bfld) > cdb_kp[i]->size){
	                   sprintf(tbuf, "Truncating Field %d of line %d",
		              i + 1, line_no);
                           cdb_bfld[cdb_kp[i]->size] = NULL;
	                   fb_serror(FB_MESSAGE, tbuf, NIL);
                           }
		        fb_nextwrite(0, cdb_bfld);
                        }
		     }
                  if (cdb_kp[i]->type != FB_BINARY)
		     fb_w_write(0, "\000");		/* EOS = e-o-field */
                  else{					/* pad BINARY field */
                     for (nc = cdb_kp[i]->size; nc > 0; nc--)
                        fb_w_write(0, "\000");
                     }
		  }
	    if (lastc != FB_NEWLINE){
	       for (;;){
	          if ((st = fb_nextread(&lastc)) == 0)
		     break;
		  if (lastc == FB_NEWLINE)
		     break;
		  }
	       if (st == 0){
		  eof = 1;
		  break;
		  }
	       }
	    if (eof == 1)
	       break;
	    fb_w_write(0, " ");
            if (cdb_secure)
 	       fb_nextwrite(0, perm);		      	/* permissions */
	    fb_w_write(0, "\000");			/* EOS = e-o-field */
	    fb_w_write(0, cdb_S_EOREC);			/* end of record */
	    lastc = NULL;
	    n++;
            line_no++;
	    if (!cdb_batchmode)
	       fb_gcounter(n);
	    }
	 fb_wflush(1);
	 if (fb_puthead(cdb_db->fd, n, 0L) == FB_ERROR)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_HEADER], cdb_db->dbase);
	 fb_w_end(1);
         return(FB_AOK);
      }

/* 
 *  aload - append load all lines from a load file (fd) into database 
 */
 
   static int aload()
      
      {
         long n, avail, rpos, freep, w_avail, w_rlen;
	 int i, eof, st, nc, line_no = 1, length;
         char perm[FB_MAXNAME], tbuf[FB_MAXLINE];

         if (cdb_secure)
	    sprintf(perm, FMT1, fb_getuid(), fb_getgid(), "666");

	 if (!cdb_batchmode)
	    FB_XWAIT();

	 /* get header information from fb_database map */
	 if (fb_getmap(cdb_db->mfd, 0L, &avail, &freep, &rpos, &size) ==
               FB_ERROR){
	    fb_serror(FB_IO_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }

         /* seek fb_database fd to avail, the end of the fb_database */
	 if (lseek(cdb_db->fd, avail, 0) < 0L){
	    fb_serror(FB_SEEK_ERROR, SYSMSG[S_BAD_DATA], NIL);
	    return(FB_ERROR);
	    }

         /* set the fd for the map to the end of the map */
	 n = cdb_db->reccnt;
	 if (lseek(cdb_db->mfd, (n + 1L) * (long) FB_MAPREC_SIZE, 0) < 0L){
	    fb_serror(FB_SEEK_ERROR, SYSMSG[S_BAD_MAP], NIL);
	    return(FB_ERROR);
	    }

         fb_w_init(2, cdb_db->fd, cdb_db->mfd);
	 for (eof = 0 ; ; ){
	    for (i = 0, size = 0L; i < cdb_db->nfields; i++)
	       if (cdb_kp[i]->type != FB_FORMULA &&
                     cdb_kp[i]->dflink == NULL && cdb_kp[i]->type != FB_LINK){
	          if (lastc != FB_NEWLINE){
		     if (simplefield() == EOF){	/* returns in cdb_bfld */
			eof = 1;
			break;
			}
                     if (cdb_kp[i]->type != FB_BINARY){
                        if (lengthcheck){
                           length =  strlen(cdb_bfld);
                           if (length > cdb_kp[i]->size){
	                      sprintf(tbuf, "Truncating Field %d of line %d",
		                 i + 1, line_no);
                              cdb_bfld[cdb_kp[i]->size] = NULL;
                              size -= (length - cdb_kp[i]->size);
	                      fb_serror(FB_MESSAGE, tbuf, NIL);
                              }
                           }
		        fb_nextwrite(0, cdb_bfld);
                        }
		     }
                  if (cdb_kp[i]->type != FB_BINARY){
		     fb_w_write(0, "\000");	/* EOS = e-o-fb_field */
		     size++;
                     }
                  else{				/* pad FB_BINARY fb_field */
                     for (nc = cdb_kp[i]->size; nc > 0; nc--)
                        fb_w_write(0, "\000");
                     size += cdb_kp[i]->size;
                     }
		  }
	    if (lastc != FB_NEWLINE){
	       for (;;){
	          if ((st = fb_nextread(&lastc)) == 0)
		     break;
		  if (lastc == FB_NEWLINE)
		     break;
		  }
	       if (st == 0){
		  eof = 1;
		  break;
		  }
	       }
	    if (eof == 1)
	       break;
	    n++;				/* set proper record number */
            line_no++;
	    if (!cdb_batchmode)
	       fb_gcounter(n);
	    fb_w_write(0, " ");
	    size++;
            if (cdb_secure){
               fb_nextwrite(0, perm);		      	/* permissions */
               size += 10;
               }
	    fb_w_write(0, "\000");			/* EOS = e-o-field */
	    fb_w_write(0, cdb_S_EOREC);			/* end of record */
	    size += 2;
	    lastc = NULL;

	    rlen = MAX(size, (long) (FB_MAPREC_SIZE + 2L));
	    if (size < rlen){			/* fill hole if needed */
	       while (++size < rlen)
	          fb_w_write(0, cdb_S_FILLC);	/* end of record */
	       fb_w_write(0, cdb_S_FILLE);	/* rec[rlen] = fille */
	       }

	    w_avail = avail;
	    w_rlen = rlen;
            if (cdb_dbase_byteorder != cdb_cpu_byteorder){
               M_32_SWAP(w_avail);
               M_32_SWAP(w_rlen);
               }
	    /* fb_put the map entry */
	    if (fb_w_writen(1, (char *) &w_avail, FB_SLONG) != 1){
	       fb_serror(FB_WRITE_ERROR, SYSMSG[S_BAD_MAP], NIL);
	       return(FB_ERROR);
	       }
	    if (fb_w_writen(1, (char *) &w_rlen, FB_SLONG) != 1){
	       fb_serror(FB_WRITE_ERROR, SYSMSG[S_BAD_MAP], NIL);
	       return(FB_ERROR);
	       }
	    avail += rlen;
	    }

         fb_wflush(2);
	 /* patch up fb_database header */
	 if (fb_puthead(cdb_db->fd, n, cdb_db->delcnt) == FB_ERROR)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_HEADER], cdb_db->dbase);

	 /* patch up fb_database header */
	 if (fb_putmap(cdb_db->mfd, 0L, avail, 0L, 0L)==FB_ERROR){
	    fb_serror(FB_IO_ERROR, SYSMSG[S_BAD_DATA], SYSMSG[S_BAD_MAP]);
	    return(FB_ERROR);
	    }
         return(FB_AOK);
      }

/* 
 *  simplefield - get a single simple fb_field from file fd (comma sep format)
 *     return value in cdb_bfld.
 */
 
   static simplefield()
      
         {
	    char *p, c;
	    int qlev, eof;  	/* eof is end of fb_field */
	       
	    qlev = 0; 
	    eof = 0;
	    p = cdb_bfld;
	    for (;;){			/* use cdb_bfld to gather */
	       if (fb_nextread(&c) == 0)
	          return(EOF);
	       if (c != '\n' && c != '\t' && c != ' ' && !isprint(c))
		  c = CHAR_STAR;
	       if (c == CHAR_BACKSLASH){	/* covers \\ and \" .. */
		  fb_nextread(&c);
		  if (c == 'n')
		     c = CHAR_NEWLINE;
		  }
	       else if (c == separator){
		  if (qlev == 0 || qlev == 2)  
		     eof = 1;		/* must be end of fb_field */
		  }
	       else if (c == CHAR_NEWLINE){
		  eof = 1;
		  }
	       else if (c == CHAR_QUOTE){
		  qlev++;
		  continue;			/* jump out */
		  }
	       if (eof)
	          break;
	       *p++ = c;			/* store character */
	       size++;
	       }
	    *p = NULL;				/* return field in cdb_bfld */
	    lastc = c;
	    return(FB_AOK);
	 }

/* 
 *  finit - intiialize --- merely check for existance. ask about overwrite 
 */
 
   static finit(argc, argv)
      int argc;
      char *argv[];
      
      {
         char str[FB_MAXLINE], fname[FB_MAXNAME];
	 int i;
	 
	 if (!append)
            if (access(cdb_db->dbase, 0) != FB_ERROR)
               fb_xerror(FB_MESSAGE, "Database already exists: ",
                  cdb_db->dbase);
	 fname[0] = NULL;
	 for (i = 1; i < argc; i++)
	    if (argv[i][0] == '-'){
	       if (argv[i][1] == 'd' || argv[i][1] == 'i' ||
		   argv[i][1] == 's' || argv[i][1] == 'c')
	          i++;
	       }
	    else{
	       strcpy(fname, argv[i]);
	       break;
	       }
	 if (fname[0] == NULL){		/* requires full path name */
	    fb_basename(fname, cdb_db->dbase);	/* calc default file name */
	    sprintf(str, "%s%s", fname, EMITEXTENSION);
	    if (fb_getfilename(fname, "Load File Name: ", str) == FB_ERROR)
	       fb_xerror(FB_ABORT_ERROR, SYSMSG[S_NOT_DONE], NIL);
	    }
	 sprintf(str, "from %s", fname);
	 fb_scrstat(str);
	 fb_r_init(fb_mustopen(fname, READ));
      }

/* 
 *  usage message 
 */
 
   static void usage()
      {
         fb_xerror(FB_MESSAGE, 
	    "usage: dbload [-b] [-y] [-a] [-l] [-c sep] [-d dbase] [LOADFILE]",
             NIL);
      }
