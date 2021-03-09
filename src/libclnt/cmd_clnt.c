/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: cmd_clnt.c,v 9.0 2001/01/09 02:56:33 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Cmd_clnt_sid[] = "@(#) $Id: cmd_clnt.c,v 9.0 2001/01/09 02:56:33 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fb_ext.h>
#include <fbserver.h>

/*
 * fb_cmd_clnt - used for a single command that returns a single status,
 *	where no side affects are neeeded.
 *	however, if there is a second argument, its assumed to be the record
 *	number of the record just dealt with.
 */

   fb_cmd_clnt(command, hp)
      char *command;
      register fb_database *hp;

      {
         static fb_varvec v, *r;
         char sid[10];
         int st = FB_AOK, nargs;

         /*
          * arguments for this generic command:
          *   r_command SID
          */
         sprintf(sid, "%d", hp->b_sid);
         fb_loadvec(&v, command, sid, 0);
         r = fb_toserver(&v);
         if (r == NULL)
            st = FB_ERROR;
#if 0
         fprintf(stderr, "cmd_clnt command=%s\n", command);
         fb_tracevec(r, "cmd_clnt - results from toserver:");
#endif
         if (st == FB_AOK){
            /*
             * results back from these generic commands are:
             *    - nargs
             *    - status
             *    - record number if needed
             */
            nargs = atoi(fb_argvec(r, 0));
            st = atoi(fb_argvec(r, 1));
            if (nargs > 2 && st == FB_AOK)
               hp->rec = atol(fb_argvec(r, 2));
            }
         /* fb_free the results */
         fb_freevec(&v);
         fb_free_xdr_vec(r);
         return(st);
      }

   fb_addrec_clnt(hp)
      register fb_database *hp;

      {
         return(fb_cmd_clnt(R_ADDREC, hp));
      }

   fb_b_addrec_clnt(hp)
      register fb_database *hp;

      {
         return(fb_cmd_clnt(R_B_ADDREC, hp));
      }

   /* fb_bulkrec_begin_clnt - now appears in bulk_c.c */

   fb_bulkrec_end_clnt(hp)
      register fb_database *hp;

      {
         return(fb_cmd_clnt(R_BULKREC_END, hp));
      }

   fb_delrec_clnt(hp)
      register fb_database *hp;

      {
         return(fb_cmd_clnt(R_DELREC, hp));
      }

   fb_b_delrec_clnt(hp)
      register fb_database *hp;

      {
         return(fb_cmd_clnt(R_B_DELREC, hp));
      }

   fb_nextxrec_clnt(hp)
      register fb_database *hp;

      {
         return(fb_cmd_clnt(R_NEXTXREC, hp));
      }

   fb_prevxrec_clnt(hp)
      register fb_database *hp;

      {
         return(fb_cmd_clnt(R_PREVXREC, hp));
      }

   fb_lastxrec_clnt(hp)
      register fb_database *hp;

      {
         return(fb_cmd_clnt(R_LASTXREC, hp));
      }

   fb_firstxrec_clnt(hp)
      register fb_database *hp;

      {
         return(fb_cmd_clnt(R_FIRSTXREC, hp));
      }

   fb_put_autoindex_clnt(hp)
      register fb_database *hp;

      {
         return(fb_cmd_clnt(R_PUT_AUTOINDEX, hp));
      }

   fb_set_autoindex_clnt(hp)
      register fb_database *hp;

      {
         return(fb_cmd_clnt(R_SET_AUTOINDEX, hp));
      }

   fb_sync_clnt(hp)
      register fb_database *hp;

      {
         return(fb_cmd_clnt(R_SYNC, hp));
      }

   fb_closeidx_clnt(hp)
      register fb_database *hp;

      {
         int st;

         st = fb_cmd_clnt(R_CLOSEIDX, hp);
         if (st == FB_AOK && hp->b_tree)
            hp->b_tree = 0;
         return(st);
      }

#endif /* RPC */
