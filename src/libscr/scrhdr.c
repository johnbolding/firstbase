/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: scrhdr.c,v 9.1 2001/02/05 18:21:29 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Scrhdr_sid[] = "@(#) $Id: scrhdr.c,v 9.1 2001/02/05 18:21:29 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

extern fb_database *cdb_LastDbase;
extern short int cdb_scr_header;
extern short int cdb_scr_stat2;
extern short int cdb_scr_label;
extern short int cdb_scr_toolname;
extern short int cdb_scr_version_mask;
extern short int cdb_secure;
extern char *cdb_pgm;
extern char *cdb_coname;

#if FB_PROTOTYPES
static make_header(char *line);
#else /* FB_PROTOTYPES */
static make_header();
#endif /* FB_PROTOTYPES */

static char header_line[FB_PCOL+1];
static char label_line[FB_PCOL+1];
static char stat_line[FB_PCOL+1];
static char stat2_line[FB_PCOL+1];

/* 
 *  scrhdr - print screen header on line 1 if not fb_batchmode.
 */
 
   void fb_scrhdr(hp, s)
      fb_database *hp;
      char *s;

      {
	 cdb_LastDbase = hp;
         if (cdb_coname == NULL)
	    fb_getco(hp);
	 if (!cdb_batchmode){
            header_line[0] = label_line[0] = stat2_line[0] = NULL;
	    fb_clear();
            fb_move(1,1);
            fb_refresh();
	    if (cdb_scr_header){
               make_header(header_line);
	       fb_reverse(header_line);
	       }
            strcpy(stat_line, s);
	    fb_scrstat(stat_line);
	    fb_move(1, cdb_t_cols);
	    }
      }

/*
 * make_header - center the coname, left justify the toolname.
 *	place this mess in a field of cdb_t_cols ...
 *	pay attention to scr_name_mask, cdb_secure
 */
 
   static make_header(line)
      char *line;

      {

         char buf[FB_MAXLINE], sec_char[2], version[50];
         int pos, len;

         fb_pad(line, " ", cdb_t_cols);
         if (cdb_secure)
            strcpy(sec_char, "s");
         else
            sec_char[0] = NULL;
         if (!cdb_scr_version_mask)
            sprintf(version, "FirstBase %s%s: ", VERSION, sec_char);
         else
            version[0] = NULL;
         strcpy(buf, version);
         if (cdb_pgm != NULL)
            strcat(buf, cdb_pgm);
         if (cdb_scr_toolname)
            strncpy(line, buf, (int) strlen(buf));

         /* now do coname */
         len = strlen(cdb_coname);
         pos = (cdb_t_cols / 2) - (len / 2);
         if (pos < 0 || pos > cdb_t_cols)
            pos = 0;
         strncpy(line + pos, cdb_coname, len);
      }

   void fb_scrlbl(s)
      char *s;

      {
         if (cdb_scr_label){
            fb_move(2, 7), fb_clrtoeol(), fb_reverse(s);
            }
      }

   void fb_scrstat2(s)
      char *s;

      {
         if (cdb_scr_stat2){
            fb_move(2, cdb_t_cols - 16), fb_clrtoeol(), fb_reverse(s);
            }
      }

   void fb_restore_header()
      {
         fb_move(1,1);
	 if (cdb_scr_header){
	    fb_reverse(header_line);
            fb_scrstat(stat_line);
            if (stat2_line[0] != NULL){
               fb_move(2, cdb_t_cols - 16);
               fb_clrtoeol();
               fb_reverse(stat2_line);
               }
            if (label_line[0] != NULL){
               fb_move(2, 7);
               fb_clrtoeol();
               fb_reverse(label_line);
               }
            }
      }
