/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: nobuf.c,v 9.0 2001/01/09 02:57:00 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Nobuf_sid[] = "@(#) $Id: nobuf.c,v 9.0 2001/01/09 02:57:00 john Exp $";
#endif

#include <fb.h>

#ifndef NOBUF_DEBUG
#define fb_serror(a,b,c)		/* makes fb_serrors noop here */
					/* some fd's are readonly, thus ... */
#endif	/* NOBUF_DEBUG */

#if HAVE_FCNTL
#include <fcntl.h>
static struct flock fk;
#if FB_PROTOTYPES
static t_nobuf(int fd, char *f);
#else /* FB_PROTOTYPES */
static t_nobuf();
#endif /* FB_PROTOTYPES */
#endif /* FCNTL */

/*
 *  nobuf.c - turn off all NFS buffering
 */

   fb_nobuf(db)
      fb_database *db;
      
      {

#if HAVE_FCNTL
	 int i;
         fb_autoindex *ix;

	 fk.l_whence = 0;
         fk.l_start = 0L;
         fk.l_len = 1;

         t_nobuf(db->mfd, db->dmap);
         t_nobuf(db->fd,  db->dbase);
         t_nobuf(db->ifd, db->dindex);
         t_nobuf(db->ihfd,db->idict);
	 
         for (i = 0; i < db->nfields; i++){
            if (db->kp[i]->aid != NULL && db->kp[i]->aid->autoname != NULL &&
                   db->kp[i]->aid->autoname[0] != NULL){
	       t_nobuf(db->kp[i]->aid->afd, db->kp[i]->aid->autoname);
	       t_nobuf(db->kp[i]->aid->hfd, db->kp[i]->aid->autoname);
	       }
	    }
         for (i = 0; i < db->b_maxauto; i++){
            ix = db->b_autoindex[i];
	    t_nobuf(ix->afd, ix->autoname);
	    t_nobuf(ix->hfd, ix->autoname);
            if (ix->ix_tree){
	       t_nobuf(ix->ix_seq->bs_fd, ix->autoname);
	       t_nobuf(ix->ix_idx->bi_fd, ix->autoname);
               }
            }
#endif /* HAVE_FCNTL */
      }

#if HAVE_FCNTL
   static t_nobuf(fd, f)		/* actual meat of nobuf */
      int fd;
      char *f;

      {
         int st;

         (void) f;
         if (fd >= 0){
	    fk.l_type = F_WRLCK;
	    st = fcntl(fd, F_SETLK, &fk);
#if NOBUF_DEBUG
	    if (st == -1)
	       fb_serror(FB_MESSAGE, ERRMSG, f);
#endif
	    fk.l_type = F_UNLCK;
	    st = fcntl(fd, F_SETLK, &fk);
#if NOBUF_DEBUG
	    if (st == -1)
	       fb_serror(FB_MESSAGE, ERRMSG, f);
#endif
	    }
      }

#endif /* HAVE_FCNTL */
