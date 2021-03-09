/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_fb.c,v 9.1 2001/02/16 19:28:43 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Mac_fb_sid[] = "@(#) $Id: mac_fb.c,v 9.1 2001/02/16 19:28:43 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>

short int cdb_limited;			/* flag used by cdb, not settable! */
extern short int cdb_error;

extern long cdb_failrec;
extern short int cdb_loadfail;

/*
 * allow databases to be opened and associated with a channel.
 * this channel or integer is used in subsequent database operations.
 */

#define MAXDBASE 20
fb_database *db_array[MAXDBASE] =
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/*
 * mf_opendb - opendb(dbase, mode, iname)
 */

   mf_opendb(n, r)
      fb_mnode *n, *r;

      {
         char *dname = NULL, *iname = NIL, *s_mode;
         int mode, args, ixflag, ixoption, channel, st, i;
         fb_database *db;
         /* these take up space on the stack. leave them there. --jpb 6/96 */

         (void) Mac_fb_sid;

         args = fb_realnodes(n);
         if (args < 2)
            return(FB_ERROR);
         if (cdb_limited)
            return(FB_ERROR);
         dname = tostring(n);		n = n->n_next;
         s_mode = tostring(n);
         iname = NIL;
         if (args > 2){
            n = n->n_next;
            iname =  tostring(n);
            }
         if (equal(s_mode, "r"))
            mode = READ;
         else
            mode = READWRITE;
         /* get and alloc the next db pointer */
         for (channel = 0; channel < MAXDBASE; channel++)
            if (db_array[channel] == 0)
               break;
         if (channel >= MAXDBASE || dname == NULL){
            r->n_fval = FB_ERROR;
            return(FB_ERROR);
            }
	 db = fb_dballoc();
         fb_dbargs(dname, iname, db);
         ixflag = FB_ALLINDEX;
         if (iname == NIL)
            ixoption = FB_OPTIONAL_INDEX;
         else
            ixoption = FB_MUST_INDEX;
         st = fb_opendb(db, mode, ixflag, ixoption);
         if (st != FB_AOK){
            channel = FB_ERROR;
            mf_set_constant("cdb_error", cdb_error);
            }
         else{
            db_array[channel] = db;
            if (channel == 0 && cdb_db == (fb_database *) NULL)
               cdb_db = db;
            for (i = 0; i < db->nfields; i++)
               fb_nounders(db->kp[i]);
            }
         r->n_fval = channel;
         r->n_tval |= T_NUM;
         return(st);
      }

   mf_closedb(n, r)
      fb_mnode *n, *r;

      {
         int args, channel;
         char *s_channel;

         args = fb_realnodes(n);
         if (args < 1)
            return(FB_ERROR);
         s_channel = tostring(n);
         channel = atoi(s_channel);
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (db_array[channel] != 0){
            if (channel == 0 && cdb_db == db_array[0])
               cdb_db = (fb_database *) NULL;
            fb_closedb(db_array[channel]);
            db_array[channel] = 0;
            return(FB_AOK);
            }
         else
            return(FB_ERROR);
      }

/*
 * mf_getrec - macro getrec - getrec(rec, channel)
 */

   mf_getrec(n, r)
      fb_mnode *n, *r;

      {
         char *s_rec, *s_channel;
         fb_database *db;
         long rec;
         int channel, st, args;

         args = fb_realnodes(n);
         if (args < 2)
            return(FB_ERROR);
         if (cdb_limited)
            return(FB_ERROR);
         s_rec = tostring(n);		n = n->n_next;
         s_channel = tostring(n);	n = n->n_next;
         rec = atol(s_rec);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);
         /* read the database header here, locked */
         /*
          * this is way slow.
          *
          * assume there are the same records as at open time.
          * if outside wants to re-read headers, go ahead.
          *
          * fb_lock_head(db);
          * #if HAVE_FCNTL
          * fb_allsync(db);
          * #endif
          * fb_gethead(db);
          * fb_unlock_head(db);
          */
         /* test value rec for out of boundsness */
         if (rec <= 0 || rec > db->reccnt){
            r->n_fval = FB_ERROR;
            return(FB_AOK);
            }
         st = fb_getrec(rec, db);
         if (st > 0)
            fb_set_autoindex(db);
         r->n_fval = st;
         return(FB_AOK);
      }

/*
 * mf_fgetrec - macro fgetrec - given an index fixed record location #
 *	read the index at that location and return the database record number
 */

   mf_fgetrec(n, r)
      fb_mnode *n, *r;

      {
         char *s_rec, *s_channel;
         fb_database *db;
         long irec, rec;
         int channel, st, args;
         fb_bseq *bs;

         args = fb_realnodes(n);
         if (args < 2)
            return(FB_ERROR);
         if (cdb_limited)
            return(FB_ERROR);
         s_rec = tostring(n);		n = n->n_next;
         s_channel = tostring(n);	n = n->n_next;
         irec = atol(s_rec);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);

         if (db->b_tree){
            r->n_fval = FB_ERROR;
            return(FB_AOK);
            }
         else{
            /* test value irec for out of boundsness */
            if (irec <= 0 || irec > db->bsmax){
               r->n_fval = FB_ERROR;
               return(FB_AOK);
               }
            if (fb_fgetrec(irec, db->ifd, db->irecsiz, db->irec, 0) ==
                  FB_ERROR)
               return(FB_ERROR);
            rec = atol((char *) (db->irec + db->irecsiz - 11));
            }
         r->n_fval = rec;
         return(FB_AOK);
      }

/*
 * mf_getirec - macro getrec - getrec(rec, channel)
 */

   mf_getirec(n, r)
      fb_mnode *n, *r;

      {
         char *s_rec, *s_channel;
         fb_database *db;
         long rec;
         int channel, st, args;

         args = fb_realnodes(n);
         if (args < 2)
            return(FB_ERROR);
         s_rec = tostring(n);		n = n->n_next;
         s_channel = tostring(n);	n = n->n_next;
         rec = atol(s_rec);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);
         /* read the database header here, locked */
         /*
          * fb_lock_head(db);
          * #if HAVE_FCNTL
          * fb_allsync(db);
          * #endif
          * fb_gethead(db);
          * fb_unlock_head(db);
          */
         /* test value rec for out of boundsness */
         if (rec <= 0 || rec > db->reccnt){
            r->n_fval = FB_ERROR;
            return(FB_AOK);
            }
         st = fb_getirec(rec, db);
         if (st > 0)
            fb_set_autoindex(db);
         r->n_fval = st;
         return(FB_AOK);
      }

/*
 * mf_getxrec - macro getxrec - getxrec(key, channel, array)
 */

   mf_getxrec(n, r)
      fb_mnode *n, *r;

      {
         char *s_key, *s_channel;
         fb_database *db;
         int channel, st, args;

         args = fb_realnodes(n);
         if (args < 2)
            return(FB_ERROR);
         if (cdb_limited)
            return(FB_ERROR);
         s_key = tostring(n);		n = n->n_next;
         s_channel = tostring(n);	n = n->n_next;
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);
         /*
          * fb_lock_head(db);
          * #if HAVE_FCNTL
          * fb_allsync(db);
          * #endif
          * fb_gethead(db);
          * fb_unlock_head(db);
          */
         st = fb_getxrec(s_key, db);
         if (st == 1)
            fb_set_autoindex(db);
         r->n_fval = st;
         return(FB_AOK);
      }

/*
 * mf_putrec - macro putrec - putrec(rec, channel)
 */

   mf_putrec(n, r)
      fb_mnode *n, *r;

      {
         char *s_rec, *s_channel;
         fb_database *db;
         long rec;
         int channel, st, args;

         args = fb_realnodes(n);
         if (args < 2)
            return(FB_ERROR);
         if (cdb_limited)
            return(FB_ERROR);
         s_rec = tostring(n);		n = n->n_next;
         s_channel = tostring(n);	n = n->n_next;
         rec = atol(s_rec);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);
         /* test value rec for out of boundsness */
         if (rec <= 0 || rec > db->reccnt){
            r->n_fval = FB_ERROR;
            return(FB_AOK);
            }
         st = fb_putrec(rec, db);
         if (st == FB_AOK)
            st = fb_put_autoindex(db);
	 fb_allsync(db);
         r->n_fval = st;
         return(FB_AOK);
      }

/*
 * mf_reccnt - returns number of records in a database
 */

   mf_reccnt(n, r)
      fb_mnode *n, *r;

      {
         fb_database *db;
         int channel;
         char *s_channel;

         r->n_fval = 0;
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         s_channel = tostring(n);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         else
            return(FB_ERROR);
         fb_lock_head(db);
         /*
          * fb_allsync(db);
          */
         fb_gethead(db);
         fb_unlock_head(db);
         r->n_fval = db->reccnt;
         return(FB_AOK);
      }

/*
 * mf_ireccnt - returns number of records in an open index
 */

   mf_ireccnt(n, r)
      fb_mnode *n, *r;

      {
         fb_database *db;
         int channel;
         char *s_channel;
         long v1, v2;

         r->n_fval = 0;
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         s_channel = tostring(n);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         else
            return(FB_ERROR);
         if (db->ihfd <= 0)
            return(FB_ERROR);
         fb_lock_head(db);
         if (fb_getxhead(db->ihfd, &v1, &v2) != FB_AOK)
            v1 = FB_ERROR;
         fb_unlock_head(db);
         r->n_fval = v1;
         return(FB_AOK);
      }

/*
 * mf_nfields - returns number of records in a database
 */

   mf_nfields(n, r)
      fb_mnode *n, *r;

      {
         fb_database *db;
         int channel;
         char *s_channel;

         r->n_fval = 0;
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         s_channel = tostring(n);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         else
            return(FB_ERROR);
         r->n_fval = db->nfields;
         return(FB_AOK);
      }

/*
 * mf_recno - returns record number of current record
 */

   mf_recno(n, r)
      fb_mnode *n, *r;

      {
         fb_database *db;
         int channel;
         char *s_channel;

         r->n_fval = 0;
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         s_channel = tostring(n);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         else
            return(FB_ERROR);
         r->n_fval = db->rec;
         return(FB_AOK);
      }

/*
 * mnode_to_array_dbase - the n_obj value is the dbase slot
 */

   fb_database *mnode_to_array_dbase(n)
      fb_mnode *n;

      {
         if (n->n_obj >= 0 && n->n_obj < MAXDBASE)
            return((fb_database *) db_array[n->n_obj]);
         else
            return((fb_database *) NULL);
      }

/*
 * mf_initrec - init rec by nulling fields. tie array to dbase.
 *	initrec(array, channel)
 */

   mf_initrec(n, r)
      fb_mnode *n, *r;

      {
         char *s_channel;
         fb_database *db;
         int channel, st = FB_AOK, args, j;
         fb_mnode *an;

         args = fb_realnodes(n);
         if (args < 2)
            return(FB_ERROR);
         an = mnode_to_var(n);		n = n->n_next;
         s_channel = tostring(n);
         if (an == NULL){
            fb_serror(FB_MESSAGE, "Illegal parameter", NIL);
            return(FB_ERROR);
            }
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);
         /* null the database fields here */
 	 for (j = 0; j <= db->nfields; j++)
	    FB_FLD(j, db) = NIL;		/* all fields = true null */
         fb_clear_autoindex(db);

         /* set the array to type T_FLD also */
         an->n_tval |= T_FLD;
         an->n_tval |= T_ARR;
         /* use n_obj to store the channel number */
         an->n_obj = channel;
         r->n_fval = st;
         return(FB_AOK);
      }

/*
 * mf_addrec - macro addrec - addrec(channel)
 */

   mf_addrec(n, r)
      fb_mnode *n, *r;

      {
         char *s_channel;
         fb_database *db;
         int channel, st, args;

         args = fb_realnodes(n);
         if (args < 1)
            return(FB_ERROR);
         s_channel = tostring(n);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);
         st = fb_addrec(db);
         r->n_fval = st;
         return(FB_AOK);
      }

/*
 * mf_useidx - tie in to fb_useidx
 *	useidx(i, channel)
 */

   mf_useidx(n, r)
      fb_mnode *n, *r;

      {
         char *s_channel, *s_inum;
         fb_database *db;
         int channel, st, args, inum;

         args = fb_realnodes(n);
         if (args < 2)
            return(FB_ERROR);
         s_inum = tostring(n);	n = n->n_next;
         inum = atoi(s_inum);
         s_channel = tostring(n);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);
         st = fb_useidx(inum, db);
         r->n_fval = st;
         return(FB_AOK);
      }

/*
 * mf_nextxrec - macro nextxrec - nextxrec(channel)
 */

   mf_nextxrec(n, r)
      fb_mnode *n, *r;

      {
         char *s_channel;
         fb_database *db;
         int channel, st, args;

         args = fb_realnodes(n);
         if (args < 1)
            return(FB_ERROR);
         s_channel = tostring(n);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);
         /*
          * fb_lock_head(db);
          * fb_allsync(db);
          * fb_gethead(db);
          * fb_unlock_head(db);
          */
         st = fb_nextxrec(db);
         if (st == 1)
            fb_set_autoindex(db);
         r->n_fval = st;
         return(FB_AOK);
      }

/*
 * mf_prevxrec - macro prevxrec - prevxrec(channel)
 */

   mf_prevxrec(n, r)
      fb_mnode *n, *r;

      {
         char *s_channel;
         fb_database *db;
         int channel, st, args;

         args = fb_realnodes(n);
         if (args < 1)
            return(FB_ERROR);
         s_channel = tostring(n);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);
         /*
          * fb_lock_head(db);
          * fb_allsync(db);
          * fb_gethead(db);
          * fb_unlock_head(db);
          */
         st = fb_prevxrec(db);
         if (st == 1)
            fb_set_autoindex(db);
         r->n_fval = st;
         return(FB_AOK);
      }

/*
 * mf_lastxrec - macro lastxrec - lastxrec(channel)
 */

   mf_lastxrec(n, r)
      fb_mnode *n, *r;

      {
         char *s_channel;
         fb_database *db;
         int channel, st, args;

         args = fb_realnodes(n);
         if (args < 1)
            return(FB_ERROR);
         s_channel = tostring(n);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);
         fb_lock_head(db);
         fb_allsync(db);
         fb_gethead(db);
         fb_unlock_head(db);
         st = fb_lastxrec(db);
         if (st == 1)
            fb_set_autoindex(db);
         r->n_fval = st;
         return(FB_AOK);
      }

/*
 * mf_firstxrec - macro firstxrec - firstxrec(channel)
 */

   mf_firstxrec(n, r)
      fb_mnode *n, *r;

      {
         char *s_channel;
         fb_database *db;
         int channel, st, args;

         args = fb_realnodes(n);
         if (args < 1)
            return(FB_ERROR);
         s_channel = tostring(n);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);
         fb_lock_head(db);
         fb_allsync(db);
         fb_gethead(db);
         fb_unlock_head(db);
         st = fb_firstxrec(db);
         if (st == 1)
            fb_set_autoindex(db);
         r->n_fval = st;
         return(FB_AOK);
      }

/*
 * mf_lock - macro lock - lock(rec, channel, fwait)
 */

   mf_lock(n, r)
      fb_mnode *n, *r;

      {
         char *s_rec, *s_channel, *s_fwait;
         fb_database *db;
         long rec;
         int channel, st, args, fwait = 0;

         args = fb_realnodes(n);
         if (args < 2)
            return(FB_ERROR);
         s_rec = tostring(n);		n = n->n_next;
         s_channel = tostring(n);
         rec = atol(s_rec);
         channel = atoi(s_channel);
         if (args > 2){
            n = n->n_next;
            s_fwait =  tostring(n);
            fwait = atoi(s_fwait);
            }
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);
         st = fb_lock(rec, db, fwait);
         r->n_fval = st;
         return(FB_AOK);
      }

/*
 * mf_unlock - macro unlock - unlock(rec, channel)
 */

   mf_unlock(n, r)
      fb_mnode *n, *r;

      {
         char *s_rec, *s_channel;
         fb_database *db;
         long rec;
         int channel, st, args;

         args = fb_realnodes(n);
         if (args < 2)
            return(FB_ERROR);
         s_rec = tostring(n);		n = n->n_next;
         s_channel = tostring(n);
         rec = atol(s_rec);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);
         st = fb_unlock(rec, db);
         r->n_fval = st;
         return(FB_AOK);
      }

/*
 * mf_delrec - macro delrec - delrec(channel)
 */

   mf_delrec(n, r)
      fb_mnode *n, *r;

      {
         char *s_channel;
         fb_database *db;
         int channel, st, args;

         args = fb_realnodes(n);
         if (args < 1)
            return(FB_ERROR);
         s_channel = tostring(n);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         if (db == NULL)
            return(FB_ERROR);
         st = fb_delrec(db);
         r->n_fval = st;
         return(FB_AOK);
      }

/*
 * mf_field_name - returns the name of a field
 */

   mf_field_name(n, r)
      fb_mnode *n, *r;

      {
         fb_database *db;
         int channel, fld;
         char *s_channel, *s_fld, buf[FB_MAXLINE];
         fb_field *f;

         r->n_fval = 0;
         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         s_fld = tostring(n);		n = n->n_next;
         s_channel = tostring(n);
         fld = atoi(s_fld);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         else
            return(FB_ERROR);
         buf[0] = NULL;
         if (fld >= 1 && fld <= db->nfields){
            f = db->kp[fld - 1];
            r->n_tval |= T_STR;
            strcpy(buf, f->id);
            }
         fb_mkstr(&(r->n_nval), buf);
         return(FB_AOK);
      }

/*
 * mf_field_type - returns type of field as a string
 */

   mf_field_type(n, r)
      fb_mnode *n, *r;

      {
         fb_database *db;
         int channel, fld;
         char *s_channel, *s_fld, buf[10];
         fb_field *f;

         r->n_fval = 0;
         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         s_fld = tostring(n);		n = n->n_next;
         s_channel = tostring(n);
         fld = atoi(s_fld);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         else
            return(FB_ERROR);
         f = NULL;
         if (fld >= 1 && fld <= db->nfields)
            f = db->kp[fld - 1];
         else{
            fb_underscore(s_fld, 0);
            f = fb_findfield(s_fld, db);
            }
         buf[0] = NULL;
         if (f != NULL){
            r->n_tval |= T_STR;
            buf[0] = f->type;
            buf[1] = NULL;
            }
         fb_mkstr(&(r->n_nval), buf);
         return(FB_AOK);
      }

/*
 * mf_field_type - returns default of a field as a string
 */

   mf_field_default(n, r)
      fb_mnode *n, *r;

      {
         fb_database *db;
         int channel, fld;
         char *s_channel, *s_fld, buf[FB_MAXLINE];
         fb_field *f;

         r->n_fval = 0;
         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         s_fld = tostring(n);		n = n->n_next;
         s_channel = tostring(n);
         fld = atoi(s_fld);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         else
            return(FB_ERROR);
         f = NULL;
         if (fld >= 1 && fld <= db->nfields)
            f = db->kp[fld - 1];
         else{
            fb_underscore(s_fld, 0);
            f = fb_findfield(s_fld, db);
            }
         buf[0] = NULL;
         if (f != NULL){
            r->n_tval |= T_STR;
            if (f->idefault != NULL)
               strcpy(buf, f->idefault);
            }
         fb_mkstr(&(r->n_nval), buf);
         return(FB_AOK);
      }

/*
 * mf_field_size - returns field size
 */

   mf_field_size(n, r)
      fb_mnode *n, *r;

      {
         fb_database *db;
         int channel, fld;
         char *s_channel, *s_fld;
         fb_field *f;

         r->n_fval = 0;
         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         s_fld = tostring(n);		n = n->n_next;
         s_channel = tostring(n);
         fld = atoi(s_fld);
         fb_underscore(s_fld, 0);
         channel = atoi(s_channel);
         db = NULL;
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (channel >= 0 && channel < MAXDBASE)
            db = db_array[channel];
         else
            return(FB_ERROR);
         if (fld >= 1 && fld <= db->nfields){
            f = db->kp[fld - 1];
            r->n_fval = f->size;
            }
         else if ((f = fb_findfield(s_fld, db)) != NULL)
            r->n_fval = f->size;
         return(FB_AOK);
      }

   mf_checkfields(n, r)
      fb_mnode *n, *r;

      {
         fb_database *db = NULL;
         int args, channel;
         char *s_channel;

         args = fb_realnodes(n);
         if (args < 1)
            db = cdb_db;
         else{
            s_channel = tostring(n);
            channel = atoi(s_channel);
            if (channel < 0 || channel >= MAXDBASE)
               return(FB_ERROR);
            if (channel >= 0 && channel < MAXDBASE)
               db = db_array[channel];
            }
         if (db == NULL)
            return(FB_ERROR);
         r->n_fval = fb_checkfields(db, 0);
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_default_dbase(n, r)
      fb_mnode *n, *r;

      {
         int args, channel;
         char *s_channel;

         args = fb_realnodes(n);
         if (args < 1)
            return(FB_ERROR);
         s_channel = tostring(n);
         channel = atoi(s_channel);
         if (channel < 0 || channel >= MAXDBASE)
            return(FB_ERROR);
         if (db_array[channel] != (fb_database *) NULL){
            cdb_db = db_array[channel];
            r->n_fval = FB_AOK;
            }
         else
            r->n_fval = FB_ERROR;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_set_loadfail(n, r)
      fb_mnode *n, *r;

      {
         int args, st;
         char *s_st;

         args = fb_realnodes(n);
         if (args < 1)
            return(FB_ERROR);
         s_st = tostring(n);
         st = atoi(s_st);
         if (st != 0 && st != 1)
            return(FB_ERROR);
         cdb_loadfail = (short) st;
         r->n_fval = FB_AOK;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_get_failrec(n, r)
      fb_mnode *n, *r;

      {
         r->n_fval = (double) cdb_failrec;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }
