/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: recmode.c,v 9.0 2001/01/09 02:57:01 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Recmode_sid[] = "@(#) $Id: recmode.c,v 9.0 2001/01/09 02:57:01 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <secure.h>

static char *FMT1 = "%c%04d%03d%s";

static int p_gid = -1;
static int p_uid = -1;

extern int cdb_record_umask;
#if FB_PROTOTYPES
static testmode(int type, fb_database *db, int uid, int gid);
#else /* FB_PROTOTYPES */
static testmode();
#endif /* FB_PROTOTYPES */

   fb_recmode(db, delc, uid, gid, mode)
      fb_database *db;
      int delc;
      int uid, gid;
      char *mode;
      
      {
         char buf[5], mbuf[15];
         int perm;

         /* used to set these to FB_RECUMOD[6] ... */
         if (uid == -1)
	    uid = 0;
         if (gid == -1)
	    gid = 0;
         if (mode == NIL){
	    strcpy(buf, "666");
	    mode = buf;
	    }

         /* if needed, mask the mode here */
         if (cdb_record_umask > 0){
            sscanf(mode, "%o", &perm);
            perm ^= cdb_record_umask;
            sprintf(mode, "%03o", perm);
            }

	 sprintf(mbuf, FMT1, delc, uid, gid, mode);
	 fb_store(db->kp[db->nfields], mbuf, db);
      }

/*
 * record_permission - test record permission against READ or WRITE
 *	WRITE assumes READ/WRITE
 */

   fb_record_permission(db, type)
      fb_database *db;
      int type;

      {
         if (p_uid == -1)
            p_uid = fb_getuid();
         if (p_gid == -1)
            p_gid = fb_getgid();
         return(testmode(type, db, p_uid, p_gid));
      }

/*
 * testmode - return FB_AOK or FB_ERROR on test
 */

   static testmode(type, db, uid, gid)
      int type, uid, gid;
      fb_database *db;
      
      {
         char buf[20], c[2], m[6];
         int u, g;
         unsigned perm;

         fb_fetch(db->kp[db->nfields], buf, db);
         sscanf(buf, "%c%04d%03d%s", c, &u, &g, m);
         sscanf(m, "%o", &perm);
         if (u == uid){
            if (type == READ && isperm_u_read(perm))
               return(FB_AOK);
            else if (type == WRITE && isperm_u_write(perm))
               return(FB_AOK);
            }
         else if (g == gid){
            if (type == READ && isperm_g_read(perm))
               return(FB_AOK);
            else if (type == WRITE && isperm_g_write(perm))
               return(FB_AOK);
            }
         else{
            /* now test to see if a fit into OTHER is allowed via the modes */
            if (type == READ && isperm_o_read(perm))
               return(FB_AOK);
            else if (type == WRITE && isperm_o_write(perm))
               return(FB_AOK);
            }

         /* allow super user override */
         if (uid == 0 || gid == 0)
            return(FB_AOK);

         return(FB_ERROR);
      }
