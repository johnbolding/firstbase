/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: macroinp.c,v 9.0 2001/01/09 02:56:40 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macroinput_sid[] = "@(#) $Id: macroinp.c,v 9.0 2001/01/09 02:56:40 john Exp $";
#endif

#include <dbve_ext.h>
#include <macro_e.h>

extern short int st_up;
extern short int cdb_create_it, cdb_write_it;
extern short int cdb_did_create_it, cdb_did_write_it;
extern short int cdb_end_keystroke;

extern char cdb_macro_b_file[];
extern char cdb_macro_e_file[];
extern char cdb_macro_w_file[];

short int cdb_m_fld;

/*
 * macroinput - macro input mechanism called from edit_field
 *	it returns a keystroke in the middle of the edit loop
 *	that helps to do the next keystroke movement.
 */

   int fb_macroinput(f, top, addf, row, col, len, fld)
      fb_field *f;
      fb_node *top;	/* a screen node */
      int addf, row, col, len, fld;

      {
         fb_stack_vars svars, *sv = &svars;
         fb_mnode *n;
         int st;

         cdb_m_fld = fld;
         m_addf_flag = addf;
         m_current_filename = 0;
         if (fb_macrotree(f->f_macro) == FB_AOK){
            e_winner = winner;
            n = locate_section(S_BODY);
            if (n == (fb_mnode *) NULL)
               return(FB_ERROR);
            m_verify_sub(n);

            /* set up distinct pre constants used to signal */
            mf_set_constant("ST_UP", st_up);
            mf_set_constant("ROW", row);
            mf_set_constant("COL", col);
            mf_set_constant("LEN", len);

            /* set up standard pre constants used to signal */
            mf_set_constant("END", FB_END);
            mf_set_constant("ERROR", FB_ERROR);
            mf_set_constant("ABORT", FB_ABORT);
            mf_set_constant("NEXT", FB_ESIGNAL);
            mf_set_constant("PREV", FB_YSIGNAL);
            mf_set_constant("DEFAULT", FB_DEFAULT);
            mf_set_constant("NO_OP", FB_ERROR);
            mf_set_constant("AOK", 1);

            /* execute the beast */
            mf_push_field(f);
            mf_inp_make_frame(sv);
            macro_statement(n, sv);
            mf_pop_field();
            if (sv->return_flag){
               /* return whatever the expression came up with, but no AOKs */
               st = sv->return_value;
               switch(st){
                  case FB_AOK:
                  case FB_DEFAULT:
                     st = FB_ESIGNAL;
                     break;
                  case FB_END:
                  case FB_ABORT:
                  case FB_ESIGNAL:
                  case FB_YSIGNAL:
                     break;
                  default:
                     st = FB_ESIGNAL;
                     break;
                  }
               sv->return_flag = 0;
               }
            else
               st = FB_ERROR;
            if (sv->break_flag){
               fb_serror(FB_MESSAGE, "Illegal `break'", NIL);
               st = FB_ERROR;
               }
            if (sv->continue_flag){
               fb_serror(FB_MESSAGE, "Illegal `continue'", NIL);
               st = FB_ERROR;
               }
            mf_inp_destroy_frame(sv);
            }
         else{
            fb_serror(FB_MESSAGE, "macro file syntax error:", f->f_macro);
            st = FB_ERROR;
            }
         fb_checkformula(fld);
         return(st);
      }

/*
 * macroend - macroend - called at back end of record level, on FB_END.
 */

   fb_macroend(t)
      int t;

      {
         fb_stack_vars svars, *sv = &svars;
         fb_mnode *n;
         int st = FB_END;
         char *efile;

         cdb_m_fld = -1;
         if (t == 0)
            efile = cdb_macro_e_file;
         else
            efile = cdb_macro_w_file;
         if (access(efile, 0) != 0)
            return(FB_END);
         m_current_filename = 0;
         if (fb_macrotree(efile) == FB_AOK){
            e_winner = winner;
            n = locate_section(S_BODY);
            if (n == (fb_mnode *) NULL)
               return(FB_ERROR);
            m_verify_sub(n);

            /* set up distinct pre constants used to signal */
            if (t == 0){
               mf_set_constant("CREATE_RECORD", cdb_create_it);
               mf_set_constant("WRITE_RECORD", cdb_write_it);
               }
            else{
               mf_set_constant("CREATE_RECORD", cdb_did_create_it);
               mf_set_constant("WRITE_RECORD", cdb_did_write_it);
               }
            mf_set_constant("END_KEYSTROKE", cdb_end_keystroke);
            mf_set_constant("ROW", 24);
            mf_set_constant("COL", 1);
            mf_set_constant("LEN", 15);

            /* set up standard pre constants used to signal */
            mf_set_constant("END", FB_END);
            mf_set_constant("ERROR", FB_ERROR);
            mf_set_constant("ABORT", FB_ABORT);
            mf_set_constant("NEXT", FB_ESIGNAL);
            mf_set_constant("PREV", FB_YSIGNAL);
            mf_set_constant("DEFAULT", FB_DEFAULT);
            mf_set_constant("NO_OP", FB_ERROR);
            mf_set_constant("AOK", 1);

            /* execute the beast */
            mf_inp_make_frame(sv);
            macro_statement(n, sv);
            if (sv->return_flag){
               /* return FB_ERROR or FB_END, thats it */
               st = sv->return_value;
               if (st != FB_ERROR && st != FB_END)
                  st = FB_ERROR;
               sv->return_flag = 0;
               }
            else
               st = FB_END;
            mf_inp_destroy_frame(sv);
            }
         else{
            fb_serror(FB_MESSAGE, "Macro End-File Error", efile);
            st = FB_ERROR;
            }
         return(st);
      }

/*
 * macrobegin - macrobegin - called at front end of record level.
 *	no return status.
 */

   int fb_macrobegin(erec)
      long erec;

      {
         fb_stack_vars svars, *sv = &svars;
         fb_mnode *n;
         int cval, st = FB_ERROR;

         cdb_m_fld = -1;
         if (access(cdb_macro_b_file, 0) != 0)
            return(FB_AOK);
         m_current_filename = 0;
         if (fb_macrotree(cdb_macro_b_file) == FB_AOK){
            e_winner = winner;
            n = locate_section(S_BODY);
            if (n == (fb_mnode *) NULL)
               return(FB_ERROR);
            m_verify_sub(n);

            /* set up all the pre constants used to signal */
            if (erec == -1L)
               cval = 1;
            else
               cval = 0;
            
            /* set up distinct pre constants used to signal */
            mf_set_constant("CREATE_RECORD", cval);
            mf_set_constant("ROW", 24);
            mf_set_constant("COL", 1);
            mf_set_constant("LEN", 15);

            /* set up standard pre constants used to signal */
            mf_set_constant("END", FB_END);
            mf_set_constant("ERROR", FB_ERROR);
            mf_set_constant("ABORT", FB_ABORT);
            mf_set_constant("NEXT", FB_ESIGNAL);
            mf_set_constant("PREV", FB_YSIGNAL);
            mf_set_constant("DEFAULT", FB_DEFAULT);
            mf_set_constant("NO_OP", FB_ERROR);
            mf_set_constant("AOK", 1);

            /* execute the beast */
            mf_inp_make_frame(sv);
            macro_statement(n, sv);

            /* now do garbage collection */
            mf_inp_destroy_frame(sv);
            }
         else
            fb_serror(FB_MESSAGE, "Macro Begin-File Error", cdb_macro_b_file);
         return(st);
      }

#if MEM_CHECK__DEBUG
/*
 * mem_check - verify the heap of storage
 */

   mem_check(s)
      char *s;

      {
         if (malloc_verify() == 1)
            fb_serror(FB_MESSAGE, s, "PASS");
         else
            fb_serror(FB_MESSAGE, s, "FAIL");
      }
#endif /* MEM_CHECK__DEBUG */

/*
 * mf_inp_make_frame - common code used to make a macro "frame"
 * 	from inside of dbvedit
 */

   void mf_inp_make_frame(sv)
      fb_stack_vars *sv;

      {
         mf_init_stack(sv);
         mf_make_frame(sv);
      }

/*
 * mf_inp_destroy_frame - common code used to destroy a macro "frame"
 * 	from inside of dbvedit
 */

   void mf_inp_destroy_frame(sv)
      fb_stack_vars *sv;

      {
         fb_mnode *nn, *sn, *nx;
         /* now do garbage collection */
         mf_destroy_frame(sv);
         /* destroy the sv->s_return_list, created by fb_s_makenode() */
         for (sn = sv->s_return_list; sn != NULL; sn = nn){
            /* check for next node, an encoded array */
            for (nx = sn->n_next; nx != NULL; nx = nn){
               if (nx->n_list != NULL)
                  fb_freenode(nx->n_list);
               nn = nx->n_next;
               fb_freenode(nx);
               }
            nn = sn->n_list;
            fb_freenode(sn);
            }
      }
