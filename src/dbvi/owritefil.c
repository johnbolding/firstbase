/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: owritefil.c,v 9.0 2001/01/09 02:56:06 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Writefile_sid[] = "@(#) $Id: owritefil.c,v 9.0 2001/01/09 02:56:06 john Exp $";
#endif

#include <dbvi_ext.h>

extern short int cdb_secure;
extern char *cdb_S_EOREC;

#define TEMPLATE "DBVI_XXXXXX"

static char *FMT1 = "%04d%03d%s";       /* secure bits */

   writefile()
      {
         int n;

         fb_move(cdb_t_lines, 1);
	 fb_printw("\"%s\" ", cdb_db->dbase);
	 fb_refresh();
	 n = savefile(0);
	 fb_printw("%ld Records", n);
	 fb_refresh();
	 clear_lastline = 1;
      }

/*
 * savefile - save the file. if type == 0, into db, else into save area.
 */

   savefile(type)
      int type;
      
      {
         int fd;
	 int n = 0, j;
	 char filen[FB_MAXNAME];
	 crec *c;
	 column *p;
	 fb_field *f;
         char perm[FB_MAXNAME];

         if (type == 0){		/* normal write */
	    strcpy(filen, TEMPLATE);
	    close(mkstemp(filen));
	    }
	 else				/* panic write */
	    strcpy(filen, "dbvi_save");
	 strcat(filen, ".cdb");
	 fd = fb_mustopen(filen, 2);
	 fb_putseq(fd);
	 fb_puthead(fd, 0L, 0L);
         if (cdb_secure){
            if (fb_putmode(fd, fb_getuid(), fb_getgid(), "666") == FB_ERROR)
               fb_xerror(FB_IO_ERROR, filen, NIL);
	    sprintf(perm, FMT1, fb_getuid(), fb_getgid(), "666");
            }
         fb_w_init(1, fd, -1);
	 for (c = crec_mhead; c != NULL; c = c->c_next){
	    for (j = 0; j < cdb_db->nfields; j++){
	       f = cdb_db->kp[j];
	       if (f->type == FB_FORMULA || f->dflink != NULL)
	          continue;		/* do not write formula/link fields */
	       for (p = col_mhead; p != NULL; p = p->p_next)
	          if (f == p->p_field)	/* fb_field match of fb_cell located */
		     break;
	       if (p != NULL)
		  fb_nextwrite(0, c->c_cell[p->p_array]);
	       fb_w_write(0, "\000");	/*  NULL gets written too */
	       }
	    fb_w_write(0, " ");      	/* deletion place holder */
            if (cdb_secure)
	       fb_nextwrite(0, perm);      	/* permissions */
	    fb_w_write(0, "\000");
	    fb_w_write(0, cdb_S_EOREC);      	/* end of record marker */
	    ++n;
	    }
	 fb_wflush(1);
         if (fb_puthead(fd, n, 0L) == FB_ERROR)
	    fb_xerror(FB_IO_ERROR, SYSMSG[S_BAD_HEADER], filen);
	 fb_w_end(1);	/* closes fd */
	 /* for normal writes - overwrite fb_database, and 
	  * remove the map to a force clean at the end
	  */
	 if (type == 0){
	    if (unlink(cdb_db->dbase) < 0)
	       fb_xerror(FB_EXEC_FAILURE, "Could not unlink: ", cdb_db->dbase);
	    if (link(filen, cdb_db->dbase) < 0)
	       if (fb_copyfile(filen, cdb_db->dbase) < 0)
	          fb_xerror(FB_EXEC_FAILURE, "Could not link/copy: ", filen);
	    if (unlink(filen) < 0)
	       fb_xerror(FB_EXEC_FAILURE, "Could not unlink: ", filen);
	    if (access(cdb_db->dmap, 0) == 0)
	       if (unlink(cdb_db->dmap) < 0)
	          fb_xerror(FB_EXEC_FAILURE, "Could not unlink: ", cdb_db->dmap);
	    }
         return(n);
      }
