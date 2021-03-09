/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: custom.c,v 9.0 2001/01/09 02:55:57 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Custom_sid[] = "@(#) $Id: custom.c,v 9.0 2001/01/09 02:55:57 john Exp $";
#endif

#include <dbve_ext.h>

#define MAX_CCOMMAND	10

extern char *cdb_ccommand[];	/* area for custom commands */
extern char *cdb_ccommand_prompt[]; /* area for custom commands prompts */
extern char *cdb_wdir_ccommand;	/* custom commands working directory */
extern char *cdb_home_ccommand;	/* custom commands home directory (scripts) */
extern char *cdb_ccommand_shell;/* custom commands shell */
extern char *cdb_pgm;

int fd;				/* global so as to match stolen dbemit code */
extern  char separator;		/* standard separator */
extern short int fflag;		/* 'force-formula-flag' */
static quoteflag = 0;		/* quote flag for fields */
extern short int verbose;	/* for verbose dates */
static char *S_QUOTE = "\"";
static char *S_BACKSLASH = "\\";

static docustom();
static gendata();
static semit();

/*
 * testcustom - reply with FB_AOK if custom command exists, else FB_ERROR.
 */

   testcustom(com)
      char *com;
      
      {
         int n;
	 
	 for (n = 0; n < MAX_CCOMMAND; n++)
	    if (cdb_ccommand[n] != NULL && equal(cdb_ccommand[n], com))
	       return(FB_AOK);
	 return(FB_ERROR);
      }

/*
 * custom - provide the custom command interface
 *	if found, do action and return FB_AOK, else error.
 */

   custom(com)
      char *com;
      
      {
         int n;
	 
	 for (n = 0; n < MAX_CCOMMAND; n++)
	    if (cdb_ccommand[n] != NULL && equal(cdb_ccommand[n], com)){
	       if (docustom(com, cdb_ccommand_prompt[n]) == FB_ERROR)
                  fb_serror(FB_MESSAGE, SYSMSG[S_NOT_DONE], NIL);
	       return(FB_AOK);
	       }
	 return(FB_ERROR);
      }

/*
 * docustom - do one custom command.
 *	emit vals of current rec into com.data file using com.sdict if needed.
 *	execute com com.data. com can then call whatever it likes.
 */

   static docustom(com, prompt)
      char *com, *prompt;

      {
         char script[FB_MAXNAME], data[FB_MAXNAME], buf[FB_MAXLINE], iname[FB_MAXNAME];
	 char p_iname[FB_MAXNAME], p_idict[FB_MAXNAME];
         char tname[FB_MAXNAME], hdir[FB_MAXNAME];

         if (prompt == NULL)
            sprintf(buf, "Executing custom command `%s'...", com);
	 else
            strcpy(buf, prompt);
         fb_fmessage(buf);
         /* locate the shell script - return if not found */
	 fb_dirname(hdir, hp->ddict);
         if (cdb_home_ccommand[0] == CHAR_SLASH || 
             cdb_home_ccommand[0] == CHAR_DOT || hdir[0] == NULL)
            strcpy(tname, cdb_home_ccommand);
         else{
            strcpy(tname, hdir);
            strcat(tname, cdb_home_ccommand);
            }
	 sprintf(script, "%s/%s", tname, com);
	 sprintf(data, "%s/D.XXXXXX", cdb_wdir_ccommand);
	 close(mkstemp(data));
	 if (access(script, 0) != 0){
            fb_serror(FB_MESSAGE, "No custom command:", script);
	    return(FB_ERROR);
	    }
         /* generate the data file */
	 gendata(data);

         /* generate the index file */
	 sprintf(iname, "%s/I_XXXXXX", cdb_wdir_ccommand);
	 if (cdb_db->ifd > 0 || cdb_db->b_tree){
	    close(mkstemp(iname));
	    sprintf(p_iname, SYSMSG[S_FMT_2S], iname, SYSMSG[S_EXT_IDX]);
	    sprintf(p_idict, SYSMSG[S_FMT_2S], iname, SYSMSG[S_EXT_IDICT]);
	    fb_put(iname);
	    }
         /* run the custom command */
         fb_move(1,1);
         fb_clear();
	 fb_refresh();
	 sprintf(buf, "%s %s %s %s", cdb_ccommand_shell, script, data, iname);
         fb_cx_set_toolname("NOTOOL");
         fb_cx_push_env("T", CX_NO_SELECT, NIL);
         fb_cx_write(1);		/* since no fb_input() is used */
	 fb_system(buf, FB_NOROOT);
         fb_cx_set_toolname(cdb_pgm);
         fb_cx_set_dict(cdb_db->ddict, vdict);
         fb_cx_set_status(CX_EDIT_BOOT);
         fb_cx_write(0);		/* no signal to redraw */
         fb_cx_signal_1();		/* signal_1 for special db work */
	 unlink(data);
	 unlink(p_iname);
	 unlink(p_idict);
	 unlink(iname);
	 return(FB_AOK);
      }

   static gendata(fname)
      char *fname;
      
      {
	 fd = fb_mustopen(fname, WRITE);
         fb_w_init(1, fd, -1);
	 semit(cdb_db);
	 fb_wflush(1);
	 close(fd);
      }

   static semit(hp)
      fb_database *hp;
      
      {
         int i;
	 char *p, *df;
	 
         for (i = 0; i < cdb_sfields; i++)
	    if (fflag == 1 || 
	          (cdb_sp[i]->type != FB_FORMULA && cdb_sp[i]->dflink == NULL)){
	       if (i != 0)
		  fb_w_write(0, &separator);	/* output separator (,:) */
	       if (cdb_sp[i]->type == FB_FORMULA || cdb_sp[i]->dflink != NULL){
	          fb_fetch(cdb_sp[i], cdb_afld, hp);
		  df = cdb_afld;
		  }
	       else
	          df = cdb_sp[i]->fld;
               if (cdb_sp[i]->type == FB_BINARY)
                  ; /* do nothing */
	       else if (!(FB_OFNUMERIC(cdb_sp[i]->type))){
	          if (quoteflag)
	             fb_w_write(0, S_QUOTE);
		  if (cdb_sp[i]->type ==FB_DATE && verbose){
		     strcpy(cdb_afld, df);
		     p = fb_formfield(cdb_bfld, cdb_afld, cdb_sp[i]->type, 8);
		     }
		  else
		     p = df;
		  for ( ; *p; p++){ /* escape emb quotes/backg */
		     if (*p == FB_NEWLINE)
		        fb_nextwrite(0, "\\n");
		     else {
		        if (*p == CHAR_QUOTE || *p == CHAR_BACKSLASH)
		           fb_w_write(0, S_BACKSLASH);
		        fb_w_write(0, p);
			}
		     }
	          if (quoteflag)
	             fb_w_write(0, S_QUOTE);
		  }
	       else {
	          if (cdb_sp[i]->type == FB_DOLLARS && verbose){
		     strcpy(cdb_afld, df);
		     df = fb_formdollar(cdb_bfld, cdb_afld, cdb_sp[i]->size);
		     }
		  fb_nextwrite(0, df);
		  }
	       }
	 fb_w_write(0, SYSMSG[S_STRING_NEWLINE]);
	 return(FB_AOK);
      }

