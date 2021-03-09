/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: creatidx.c,v 9.0 2001/01/09 02:56:45 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Createidx_sid[] = "@(#) $Id: creatidx.c,v 9.0 2001/01/09 02:56:45 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char *BSEQ = ".bseq";
static char *BIDX = ".bidx";
extern short int cdb_use_rpc;

#if FB_PROTOTYPES
static insert_sortby(int hfd, char *iname, int *p_btree);
#else
static insert_sortby();
#endif /* FB_PROTOTYPES */

/*
 *  createidx - create a fb_database index structure. Return FB_AOK or FB_ERROR
 */

   fb_createidx(iname, dp)
      char *iname;
      fb_database *dp;

      {
         char idict[FB_MAXNAME], seq[FB_SEQSIZE + 1], idx[FB_MAXNAME];
         char bseqname[FB_MAXNAME], bidxname[FB_MAXNAME];
         int hfd, ifd, st, btree = 0, bfd;

#if RPC
         if (cdb_use_rpc)
            return(fb_createidx_clnt(iname, dp));
#endif /* RPC */
         /* always bootstrap the idict file */
	 strcpy(idict, iname);
	 strcat(idict, SYSMSG[S_EXT_IDICT]);
         close(creat(idict, 0666));
         hfd = open(idict, READWRITE);
         if (hfd < 0)
            return(FB_ERROR);

         sprintf(seq, SYSMSG[S_FMT_04D], fb_getseq(dp->fd));
         lseek(hfd, FB_SEQSTART, 0);
         write(hfd, seq, FB_SEQSIZE);
         fb_putxhead(hfd, 0L, 0L);
         /* now write all the fields */
         st = insert_sortby(hfd, iname, &btree);
         close(hfd);

         /* if last line of idicti file is a % then btree==1, else flat idx */
         if (btree){
            strcpy(bseqname, iname);
            strcat(bseqname, BSEQ);
            close(creat(bseqname, 0666));
            bfd = fb_mustopen(bseqname, READWRITE);
            write(bfd, seq, 4);
            fb_putbxhead(bfd, 0L, 0L, 0L, 0L);
            close(bfd);

            strcpy(bidxname, iname);
            strcat(bidxname, BIDX);
            close(creat(bidxname, 0666));
            bfd = fb_mustopen(bidxname, READWRITE);
            write(bfd, seq, 4);
            fb_putbxhead(bfd, 0L, 0L, 0L, 0L);
            close(bfd);
            }
         else{
            /* create an empty idx file */
            strcpy(idx, iname);
            strcat(idx, SYSMSG[S_EXT_IDX]);
            close(creat(idx, 0666));
            ifd = open(idx, READWRITE);
            if (ifd < 0)
               return(FB_ERROR);
            close(ifd);
            }
	 return(st);
      }

   static insert_sortby(hfd, iname, p_btree)
      int hfd;
      char *iname;
      int *p_btree;

      {
         char idicti[FB_MAXNAME], by_line[FB_MAXLINE], line[FB_MAXLINE];
         char field_name[FB_MAXLINE];
         FILE *fs;
         int j, n;
         
	 strcpy(idicti, iname);
	 strcat(idicti, SYSMSG[S_EXT_IDICTI]);
         fs = fopen(idicti, FB_F_READ);
         if (fs == NULL)
            return(FB_ERROR);
         by_line[0] = NULL;
         while (fgets(line, FB_MAXLINE, fs) != NULL)
            if (line[0] == '%'){
               fgets(by_line, FB_MAXLINE, fs);
               break;
               }
         if (by_line[0] == NULL)
            return(FB_ERROR);
         for (j = 1; (j = fb_getword(by_line, j, field_name)) != 0;){
            strcat(field_name, SYSMSG[S_STRING_NEWLINE]);
	    n = strlen(field_name);
	    if (write(hfd, field_name, (unsigned) n) != n)
               return(FB_ERROR);
            }
         if (fgets(by_line, FB_MAXLINE, fs) != NULL)
            if (by_line[0] == '%'){
               *p_btree = 1;
	       n = strlen(by_line);
	       if (write(hfd, by_line, (unsigned) n) != n)
                  return(FB_ERROR);
               }
         fclose(fs);
         return(FB_AOK);
      }
