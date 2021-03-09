/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: initdbd.c,v 9.0 2001/01/09 02:56:38 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Initdbd_sid[] = "@(#) $Id: initdbd.c,v 9.0 2001/01/09 02:56:38 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/*
 * initdbd - possibly get record totals from dbase and index. no errors.
 */

   fb_initdbd(hp)
      fb_database *hp;
      
      {
         int fd, st;
	 char buf[FB_MAXNAME];
	 
	 hp->bsmax = hp->bsend = 0L;
	 hp->reccnt = hp->delcnt = 0L;
	 fb_basename(buf, hp->idict);
         if (access(hp->idict, 4) == 0){		/* R_OK read == 4 */
	    fd = open(hp->idict,  0);
	    if (fd > 0){
	       fb_getxhead(fd, &(hp->bsmax), &(hp->bsend));
	       close(fd);
	       }
            hp->b_tree = fb_test_tree(hp);
	    /*
	     * strcat(buf, SYSMSG[S_EXT_IDX]);
	     * fb_mkstr(&(hp->dindex), buf);
	     */
	    }
	 else
	    fb_mkstr(&(hp->dindex), buf);
         if (access(hp->dbase, 4) == 0){		/* R_OK read == 4 */
	    hp->fd = open(hp->dbase,  0);
	    if (hp->fd > 0){
	       st = fb_gethead(hp);	/* not important to lock for dbds */
	       if (st == FB_ERROR)
		  fb_xerror(FB_READ_ERROR, SYSMSG[S_BAD_HEADER], hp->dbase);
	       close(hp->fd);
	       }
	    }
         fb_cx_set_dict(hp->ddict, NIL);
      }
