/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: get_vdic.c,v 9.0 2001/01/09 02:56:47 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Get_vidct_sid[] = "@(#) $Id: get_vdic.c,v 9.0 2001/01/09 02:56:47 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <vdict.h>
#include <vdict_e.h>

static char *DV = "-v";
static char *V_EXT = ".vdict";
static int pagenum = 1;
static int firstfield = 1;
static int line_no = 0;
static char *SCREEN_LBL = "View:";
static char *STR_LINE   = "Line %d: %s";
static int nerrors = 0;
static int page_items = 0;
static char *ERR_PSTR = "Empty Page";
static char *ERR_PHELP = "$Help Error";

#if FB_PROTOTYPES
static parse(char *buf);
static local_fb_serror(int e, char *s1, char *s2);
#else
static parse();
static local_fb_serror();
#endif /* FB_PROTOTYPES */

/*
 * get_vdict - open and parse a view dictionary. set up structures.
 */

   fb_get_vdict(argc, argv)
      int argc;
      char *argv[];
      
      {
         char buffer[FB_MAXLINE];
         char line[FB_MAXLINE];
	 int p, fd;
	 
         if (((p = fb_testargs(argc, argv, DV)) == 0) ||
               (equal(argv[p + 1], "-")))	/* allow default for -v - */
	    strcpy(buffer, cdb_db->dbase);
	 else if (++p >= argc)
	    return(FB_ERROR);
	 else
	    strcpy(buffer, argv[p]);
         fb_dirname(vdict, buffer);
         fb_basename(line, buffer);
	 strcat(line, V_EXT);
         strcat(vdict, line);
	 if ((fd = open(vdict, READ)) < 0)
	    return(FB_ERROR);
         phead = ptail = pcur = NULL;
         fb_r_init(fd);
	 while (fb_nextline(buffer, FB_MAXLINE) != 0)
	    if (parse(buffer) == FB_ERROR)
	       nerrors++;
	 fb_r_end();
	 if (phead == NULL || phead->p_nhead == NULL)
	    nerrors++;
	 if (nerrors == 0)
	    fb_makenedit();
	 if (nerrors == 0 && (cdb_db->ifd < 0 && !cdb_db->b_tree)){
	    if (phead->p_nedit != NULL)
	       cdb_db->ip[0] = phead->p_nedit[0]->n_fp;
	    else
	       nerrors++;
	    }
	 if (nerrors > 0)
	    return(FB_ERROR);
	 sprintf(line, SYSMSG[S_FMT_S_S], SCREEN_LBL,
            fb_basename(buffer,vdict));
	 cdb_db->sdict = NULL;
	 fb_mkstr(&(cdb_db->sdict), line);
	 return(FB_AOK);
      }

/*
 * parse_vdict - open and parse a view dictionary only. no set of structs.
 */

   fb_parse_vdict(fname)
      char *fname;
      
      {
         char buffer[FB_MAXLINE];
	 int fd;
	 
	 if ((fd = open(fname, READ)) < 0)
	    return(FB_ERROR);
         strcpy(vdict, fname);
         fb_r_init(fd);
	 while (fb_nextline(buffer, FB_MAXLINE) != 0)
	    if (parse(buffer) == FB_ERROR)
	       nerrors++;
	 fb_r_end();
	 if (phead == NULL || phead->p_nhead == NULL)
	    nerrors++;
	 if (nerrors > 0)
	    return(FB_ERROR);
	 return(FB_AOK);
      }

/*
 * parse - parse a vdict line
 */

   static parse(buf)
      char *buf;
      {
	 char subject[512];
	 int row, col, sub1, sub2, type = -1, reverse, st = FB_AOK;
	 char tname[FB_MAXNAME], hdir[FB_MAXNAME];

         line_no++;
         sub1 = sub2 = -1;
	 reverse = 1;
	 /* buf[strlen(buf) - 1] = NULL; */		/* remove FB_NEWLINE */
	 if (buf[0] == '#')
	    return(FB_AOK);
	 else if (equal(buf, "$PAGE")){
	    if (!firstfield){
	       if (page_items == 0){
		  local_fb_serror(FB_BAD_DICT, vdict, ERR_PSTR);
		  return(FB_ERROR);
	          }
	       pagenum++;
	       page_items = 0;
	       }
	    firstfield = 0;
	    return(FB_AOK);
	    }
	 else if (strncmp(buf, "$HELP ", 6) == 0){
	    if (ptail == NULL){
	       local_fb_serror(FB_BAD_DICT, vdict, ERR_PHELP);
	       return(FB_ERROR);
	       }
	    fb_dirname(hdir, cdb_db->dbase);
	    if (buf[6] == CHAR_SLASH || 
	       buf[6] == CHAR_DOT || hdir[0] == NULL)
	       strcpy(tname, buf + 6);
	    else{
	       strcpy(tname, hdir);
	       strcat(tname, buf + 6);
	       }
	    fb_mkstr(&ptail->p_help, tname);
	    return(FB_AOK);
	    }
	 firstfield = 0;
	 page_items++;
	 /* "abc def"@x,y - reverse display text string */
	 if (sscanf(buf, "\"%[^\"]\"@%d,%d", subject, &row, &col) == 3){
	    type = 0;
	    }
	 /* "abc def":x,y - normal display text string */
	 else if (sscanf(buf, "\"%[^\"]\":%d,%d",subject,&row,&col) == 3){
	    type = 0;
	    reverse = 0;
	    }
	 /* Field@x,y - reverse display */
	 else if (sscanf(buf, "%[^[@]@%d,%d", subject, &row, &col) == 3){
	    type = 1;
	    }
	 /* Field:x,y - normal display */
	 else if (sscanf(buf, "%[^[:]:%d,%d", subject, &row, &col) == 3){
	    type = 2;
	    reverse = 0;
	    }
	 /* Field[s1-s2]@x,y - reverse substring display */
	 else if (sscanf(buf, "%[^[][%d-%d]@%d,%d", 
	       subject, &sub1, &sub2, &row, &col) == 5){
	    type = 3;
            if (sub1 < 0 || sub2 < 0)
               st = FB_ERROR;
	    }
	 /* Field[s1-s2]:x,y - normal substring display */
	 else if (sscanf(buf, "%[^[][%d-%d]:%d,%d", 
	       subject, &sub1, &sub2, &row, &col) == 5){
	    type = 4;
	    reverse = 0;
            if (sub1 < 0 || sub2 < 0)
               st = FB_ERROR;
	    }
	 /* Field[s1]@x,y - reverse long fb_field display */
	 else if (sscanf(buf, "%[^[][%d]@%d,%d", 
	       subject, &sub1, &row, &col) == 4){
	    type = 5;
            if (sub1 < 0)
               st = FB_ERROR;
	    }
	 /* Field[s1]:x,y - normal long fb_field display */
	 else if (sscanf(buf, "%[^[][%d]:%d,%d", 
	       subject, &sub1,  &row, &col) == 4){
	    type = 6;
	    reverse = 0;
            if (sub1 < 0)
               st = FB_ERROR;
	    }
	 /* Field[s1:s2]@x,y - reverse sub choice/sub newline display */
	 else if (sscanf(buf, "%[^[][%d:%d]@%d,%d", 
	       subject, &sub1, &sub2, &row, &col) == 5){
	    type = 7;
            if (sub1 == 0 || sub2 < 0)
               st = FB_ERROR;
	    }
	 /* Field[s1:s2]:x,y - normal sub choice/sub newline display */
	 else if (sscanf(buf, "%[^[][%d:%d]:%d,%d", 
	       subject, &sub1, &sub2, &row, &col) == 5){
	    type = 8;
	    reverse = 0;
            if (sub1 == 0 || sub2 < 0)
               st = FB_ERROR;
	    }
	 else
            st = FB_ERROR;			/* no matches - FB_ERROR */
         if (st != FB_ERROR)
	    st = fb_install(type,pagenum,subject,row,col,sub1,sub2,reverse);
	 if (st == FB_ERROR){
	    sprintf(subject, STR_LINE, line_no, buf);	/* abuse subject */
	    local_fb_serror(FB_BAD_DICT, vdict, subject);
            return(FB_ERROR);
	    }
	 return(FB_AOK);
      }

   static local_fb_serror(e, s1, s2)
      int e;
      char *s1, *s2;

      {
         if (cdb_batchmode || cdb_t_lines <= 0){
	    fprintf(stderr, "bad dictionary: %s: %s\n", s1, s2);
	    fflush(stderr);
	    }
         else{
            /* fb_move(cdb_t_lines, 1); */
            /* fb_prints(s2); */
            fb_serror(e, s1, s2);
            fb_move(cdb_t_lines, 1); fb_clrtoeol();
            }
      }
