/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: scrtime.c,v 9.1 2001/02/05 18:21:29 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Scrtime_sid[] = "@(#) $Id: scrtime.c,v 9.1 2001/02/05 18:21:29 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

static char LastInfoLine[FB_PCOL+1] = {NULL};
extern short int cdb_InfoLineToggle;
extern short int cdb_error_row_offset;
extern fb_database *cdb_LastDbase;		/* used to prime fb_infoline */

static char *FMT2= "%s%s(%ld/%ld)";
static char *FMT2T= "%s%s(%ld)";
static char *FMT3= "%s%s";
static char *MSG = "Files: ";
static char *MSG1= "**  ";

extern short int cdb_showreccnt;
extern short int cdb_scr_fb_infoline;
extern short int cdb_scr_fb_infoline_solid;
extern short int cdb_secure;
extern char *cdb_dbuser;
extern char *cdb_user;
extern char *cdb_work_dir;

/* 
 *  scrtime - print time on screen (to constant location).
 */
 
   void fb_scrtime(hp)
      fb_database *hp;
      
      {
         char line[250], t1[250], t2[250], t[250], FMT[50];
	 int erow;
	 
         if (cdb_secure)
            strcpy(FMT, " %s  %s  [%s-%s: %s %s]");
         else
            strcpy(FMT, " %s  %s  [%s: %s %s]");
	 erow = cdb_t_lines - cdb_error_row_offset;
	 cdb_LastDbase = hp;
	 if (!cdb_batchmode){
	    if (cdb_InfoLineToggle > 0 && cdb_scr_fb_infoline){
	       t1[0] = NULL;
	       if (hp != NULL && hp->dbase != NULL && strlen(hp->dbase) > 0){
	          fb_basename(t, hp->dbase);
		  if (cdb_showreccnt)
	             sprintf(t1, FMT2, MSG, t, hp->reccnt, hp->delcnt);
		  else
	             sprintf(t1, FMT3, MSG, t);
		  }
	       
	       strcpy(t2, MSG1);
	       if (hp != NULL && hp->dindex != NULL && strlen(hp->dindex)>0){
	          if (!(equal(hp->dindex, SYSMSG[S_NO_INDEX]))){
	             fb_basename(t, hp->dindex);
		     if (cdb_showreccnt){
                        if (hp->b_tree)
	                   sprintf(t2, FMT2T, NIL, t, hp->bsmax);
                        else
	                   sprintf(t2, FMT2, NIL, t, hp->bsend, hp->bsmax);
                        }
		     else
	                sprintf(t2, FMT3, NIL, t);
		     }
		  }
               if (cdb_secure)
	          sprintf(t, FMT, t1, t2, cdb_dbuser, cdb_user,
                     cdb_work_dir, fb_tdate(line));
               else
	          sprintf(t, FMT, t1, t2, cdb_user, cdb_work_dir,
                     fb_tdate(line));
	       if (strlen(t) > (FB_PCOL-1))
	          t[FB_PCOL-1] = CHAR_DOLLAR;
               if (cdb_scr_fb_infoline_solid)
                  t[0] = NULL;
	       fb_pad(LastInfoLine, t, cdb_t_cols);
	       fb_move(erow,1), fb_clrtoeol(), fb_reverse(LastInfoLine);
	       fb_move(erow, cdb_t_cols);
	       }
	    else{
	       cdb_InfoLineToggle = -1;
	       fb_move(erow,1), fb_clrtoeol();
	       }
	    }
      }

   void fb_infoline()
      {
         fb_scrtime(cdb_LastDbase);
      }

   void fb_infotoggle()
      {
         cdb_InfoLineToggle = -cdb_InfoLineToggle;
         fb_scrtime(cdb_LastDbase);
      }
