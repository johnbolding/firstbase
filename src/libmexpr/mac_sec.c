/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_sec.c,v 9.1 2001/03/19 19:04:42 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macro_secure_sid[] = "@(#) $Id: mac_sec.c,v 9.1 2001/03/19 19:04:42 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>
#include <dbpwd.h>

/*
 *  macro_secure contains the record level security functions:
 *	owner, group, mode, chown, chgrp, chmod
 */

static char *RMODE = "%c%04d%03d%s";
extern short int cdb_write_it;
extern short int cdb_secure;

   mf_r_owner(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE], m[6], c[2];
         int u, g;
         fb_passwd *pwd;

         (void) Macro_secure_sid;

         if (cdb_secure){
            fb_fetch(cdb_db->kp[cdb_db->nfields], buf, cdb_db);
            sscanf(buf, RMODE, c, &u, &g, m);
            pwd = fb_getpwuid(u);
            fb_mkstr(&(r->n_nval), pwd->dbpw_name);
            }
         else
            fb_mkstr(&(r->n_nval), NIL);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_r_group(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE], m[6], c[2];
         int u, g;

         if (cdb_secure){
            fb_fetch(cdb_db->kp[cdb_db->nfields], buf, cdb_db);
            sscanf(buf, RMODE, c, &u, &g, m);
            r->n_fval = g;
            }
         else
            r->n_fval = 0;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_r_mode(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE], m[6], c[2];
         int u, g;

         if (cdb_secure){
            fb_fetch(cdb_db->kp[cdb_db->nfields], buf, cdb_db);
            sscanf(buf, RMODE, c, &u, &g, m);
            fb_mkstr(&(r->n_nval), m);
            }
         else
            fb_mkstr(&(r->n_nval), NIL);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_r_chown(n, r)
      fb_mnode *n, *r;

      {
         fb_passwd *pwd;
         char buf[FB_MAXLINE], m[6], c[2];
         int u, g, uid;

         if (!cdb_secure)
            return(FB_ERROR);
         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         fb_fetch(cdb_db->kp[cdb_db->nfields], buf, cdb_db);
         sscanf(buf, RMODE, c, &u, &g, m);
         tostring(n);
         pwd = fb_getpwnam(n->n_pval);
         if (pwd == (fb_passwd *) NULL)
            return(FB_ERROR);
         uid = pwd->dbpw_uid;
         sprintf(buf, RMODE, c, uid, g, m);
	 fb_store(cdb_db->kp[cdb_db->nfields], buf, cdb_db);
         cdb_write_it = 1;
         fb_mkstr(&(r->n_nval), NIL);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_r_chgrp(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE], m[6], c[2];
         int u, g;

         if (!cdb_secure)
            return(FB_ERROR);
         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         fb_fetch(cdb_db->kp[cdb_db->nfields], buf, cdb_db);
         sscanf(buf, RMODE, c, &u, &g, m);
         g = (int) tonumber(n);
         if (g < 0 || g > 999)
            return(FB_ERROR);
         sprintf(buf, RMODE, c, u, g, m);
	 fb_store(cdb_db->kp[cdb_db->nfields], buf, cdb_db);
         cdb_write_it = 1;
         fb_mkstr(&(r->n_nval), NIL);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }

   mf_r_chmod(n, r)
      fb_mnode *n, *r;

      {
         char buf[FB_MAXLINE], m[6], c[2];
         int u, g, mode;

         if (!cdb_secure)
            return(FB_ERROR);
         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         mode = (int) tonumber(n);
         if (mode < 0 || mode > 999)
            return(FB_ERROR);
         fb_fetch(cdb_db->kp[cdb_db->nfields], buf, cdb_db);
         sscanf(buf, RMODE, c, &u, &g, m);
         sprintf(m, "%03d", mode);
         sprintf(buf, RMODE, c, u, g, m);
	 fb_store(cdb_db->kp[cdb_db->nfields], buf, cdb_db);
         cdb_write_it = 1;
         fb_mkstr(&(r->n_nval), NIL);
         r->n_tval |= T_STR;
         return(FB_AOK);
      }
