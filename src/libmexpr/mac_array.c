/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_array.c,v 9.0 2001/01/09 02:56:50 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Mac_array_sid[] = "@(#) $Id: mac_array.c,v 9.0 2001/01/09 02:56:50 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#include <macro_e.h>

static int checkbox_subscript = 0;
static char checkbox_name[FB_MAXLINE];

/*
 * assign_array - assign n2 to V_ARRAY n1 - using assignment operator op.
 *	leave results in n1. 
 */

   void assign_array(n1, n2, op)
      fb_mnode *n1, *n2;
      int op;

      {
         fb_mnode *vn, *basen, *arg, *an, *subscript;
         fb_cell *bcell, **stab, *cn;

         (void) Mac_array_sid;

         /* need to find place to store - new sym tab */
         arg = n1->n_narg[0];
         bcell = (fb_cell *) arg->n_obj;
         bcell = string_to_cell(bcell->c_sval);
         basen = bcell->c_vid;  	/* same as mnode_to_var(arg) */
         basen->n_tval |= T_ARR;
         if (basen == NULL){
            fb_serror(FB_MESSAGE, "Illegal base of array", NIL);
            return;
            }
         subscript = n1->n_narg[1];
         /*
          * subscript was being executed again! here.
          * macro_expr(subscript);
          */

         /*
          * set basen with the val here also, since it needs it for propigat.
          * once done, assign values into indexed value of subscript
          */
         vn = basen;
         switch(op){
            case O_ASSIGN:	vn->n_fval  = n2->n_fval; break;
            case O_ADD_A:	vn->n_fval += n2->n_fval; break;
            case O_MINUS_A:	vn->n_fval -= n2->n_fval; break;
            case O_MULT_A:	vn->n_fval *= n2->n_fval; break;
            case O_DIV_A:	if (n2->n_fval != 0)
				   vn->n_fval /= n2->n_fval;
                                break;
            case O_LSHFT_A:
               vn->n_fval = (double) ((int) vn->n_fval << (int) n2->n_fval);
               break;
            case O_RSHFT_A:
               vn->n_fval = (double) ((int) vn->n_fval >> (int) n2->n_fval);
               break;
            case O_XOR_A:
               vn->n_fval = (double) ((int) vn->n_fval ^ (int) n2->n_fval);
               break;
            case O_AND_A:
               vn->n_fval = (double) ((int) vn->n_fval & (int) n2->n_fval);
               break;
            case O_OR_A:
               vn->n_fval = (double) ((int) vn->n_fval | (int) n2->n_fval);
               break;
            case O_MOD_A:
               if (n2->n_fval != 0)
                  vn->n_fval = (double) ((int) vn->n_fval % (int) n2->n_fval);
               break;

            /* the _A series need special linkage to after effect stuff */
            case O_INCR_A:	break;
            case O_DECR_A:	break;

            /* the _B series is FB_AOK */
            case O_INCR_B:	++vn->n_fval; break;
            case O_DECR_B:	--vn->n_fval; break;
            }

         /* if its a string, store this too, and covert all to strings */
         if (n2 != NULL){
            if (istype_str(n2)){
               tostring(n2);
               fb_mkstr(&(vn->n_nval), n2->n_pval);
               fb_mkstr(&(n1->n_nval), n2->n_pval);
               vn->n_tval |= T_STR;
               /* turn off NUM if set */
               if (istype_num(vn))
                  vn->n_tval &= ~T_NUM;
               }
            else if (istype_num(n2)){
               vn->n_tval |= T_NUM;
               /* turn off STR if set */
               if (istype_str(vn))
                  vn->n_tval &= ~T_STR;
               }
            vn->n_scale = n2->n_scale;
            if (istype_dol(n2))
               vn->n_tval |= T_DOL;
            if (istype_dte(n2))
               vn->n_tval |= T_DTE;
            }
         else { /* n2 is NULL - must be an INCR of some kind */
            /* turn off STR if set */
            if (istype_str(vn))
               vn->n_tval &= ~T_STR;
            }

         /* store result in n1 --- the actual lval node */
         n1->n_fval = vn->n_fval;
         n1->n_tval = vn->n_tval;
         n1->n_scale = vn->n_scale;

         /* now patch up any _A series that were done */
         if (op == O_INCR_A)
            vn->n_fval++;
         else if (op == O_DECR_A)
            vn->n_fval--;

         /* bcell is the object with the symtab in it */
         if (bcell->c_symtab == NULL)
            bcell->c_symtab = fb_makesymtab();
         stab = bcell->c_symtab;
         cn = fb_u_sinstall(tostring(subscript), stab);
         if (cn->c_vid == NULL){
            if (istype_glob(basen))
               cn->c_vid = fb_s_node0(V_ID);
            else
               cn->c_vid = fb_node0(V_ID);
            }
         /* array mnode - the place to store the value */
         an = cn->c_vid;
         an->n_fval = vn->n_fval;
         an->n_tval = vn->n_tval;
         an->n_scale = vn->n_scale;
         if (istype_str(an))
            fb_mkstr(&(an->n_nval), vn->n_nval);
      }

/*
 * assign_copy_array - into n1 from n2, the array to be copied.
 *	create an array in n1, and put all elements from n2 there.
 *	if n1 happens to be an array already, destroy it.
 *	if n1 is a field style database array already, barf it up.
 */

   assign_copy_array(tn, fn, p_stab)
      fb_mnode *tn, *fn;
      fb_cell **p_stab;

      {
         fb_mnode *basen, *basen_2, *n1, *n2, *n;
         fb_cell *bcell, *bcell_2, **stab, **stab_2, *c1, *c2;
         char v1[FB_MAXNAME+1], v2[FB_MAXNAME+1];
         int i;

         bcell = (fb_cell *) tn->n_obj;
         strncpy(v1, bcell->c_sval, FB_MAXNAME);
         v1[FB_MAXNAME] = NULL;
         bcell = string_to_cell(bcell->c_sval);
         basen = bcell->c_vid;  	/* same as mnode_to_var(arg) */
         /* if its a dbase array, return error */
         if (basen == NULL || istype_fld(basen)){
            mf_perror("Illegal array assign variable:", v1, tn);
            return(FB_ERROR);
            }
         /* if a previous array is there, get rid of it */
         if (bcell->c_symtab != NULL){
            fb_expunge_symtab(bcell->c_symtab);
            bcell->c_symtab = NULL;
            }
         stab = bcell->c_symtab = fb_makesymtab();

         if (fn->n_obj == NULL || fn->n_type == O_CALL){
            /*
             * when an array comes in right off the stack, it is
             * only marked here, but the array actually starts in the n_list
             *
             * when the array comes in as a variable, its just in n_next
             */
            if (fn->n_type == O_CALL)
               fn = fn->n_list;
            /* must be an encoded array passed up from a function - recode */
            for (n = fn->n_next; n != NULL; n = n->n_next){
               /* n1/n2 are the value nodes */
               if (istype_glob(basen))
                  n1 = fb_s_node0(V_ID);
               else
                  n1 = fb_node0(V_ID);
               n2 = n->n_list;
               fb_copynode(n1, n2);

               /* n holds the subscript used into the known stab */
               c1 = fb_u_install(n->n_nval, stab);
               c1->c_vid = n1;
               }
            }
         else{
            /* this is the normal fn path of simple a = b stmt */
            /* double check type of fn - _2 is the from stuff */
            bcell_2 = (fb_cell *) fn->n_obj;
            strncpy(v2, bcell_2->c_sval, FB_MAXNAME);
            v2[FB_MAXNAME] = NULL;
            bcell_2 = string_to_cell_via(bcell_2->c_sval, p_stab);
            stab_2 = bcell_2->c_symtab;
            basen_2 = bcell_2->c_vid;  /* same as mnode_to_var(arg) */
            /* if its a dbase array, return error */
            if (basen_2 == NULL || istype_fld(basen_2)){
               mf_perror("Illegal array assign variable:", v2, fn);
               return(FB_ERROR);
               }

            /* for each element in n2 copy to new array n1 */
            for (i = 0; i < MAXSYM; i++){
               for (c2 = stab_2[i]; c2 != NULL; c2 = c2->c_chain){
                     if (c2->c_sval != NULL && c2->c_vid != NULL){
                        n2 = c2->c_vid;
                        if (istype_glob(basen))
                           n1 = fb_s_node0(V_ID);
                        else
                           n1 = fb_node0(V_ID);
                        fb_copynode(n1, n2);
                        c1 = fb_u_install(c2->c_sval, stab);
                        c1->c_vid = n1;
                        }
                  }
               }
            }

         /* set the type in the new array */
         basen->n_tval |= T_ARR;
         tn->n_tval |= T_ARR;
         return(FB_AOK);
      }

/*
 * key - take the array as a name, no subscript, generate its keys
 *	key(word, array)
 */

   mf_key(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *arg, *basen, *ret, *t_ret;
         fb_cell **stab = NULL, *c, *bcell;
         char v1[FB_MAXNAME+1];
         
         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         ret = n; n = n->n_next;
         arg = n;
         t_ret = mnode_to_var(ret);
         /* locate the symbol table inside of array element */
         bcell = (fb_cell *) arg->n_obj;
         strncpy(v1, bcell->c_sval, FB_MAXNAME);
         v1[FB_MAXNAME] = NULL;
         bcell = string_to_cell(bcell->c_sval);
         basen = bcell->c_vid;  /* same as mnode_to_var(arg) */
         /* if its a dbase array, return error */
         if (basen != NULL && istype_fld(basen)){
            mf_perror("Illegal array variable in key():", v1, arg);
            r->n_fval = FB_ERROR;
            basen->n_obj = NULL;
            basen->n_p1 = 0;
            return(FB_AOK);
            }
         stab = bcell->c_symtab;
         /*
          * basen is storing the base name of this table
          * and the "marker" for mf_key() to iterate through
          *
          * if basen.n_p1 == 0 && basen.n_obj == 0, then
          * mf_key has either never been called, or has looped
          * and the keys are starting over again.
          *
          * if basen.n_obj != NULL, get its next cell, return it
          *    or bump basen.n_p1 and get next chain
          */
         c = NULL;
         if (stab != NULL){
            if (basen->n_obj != 0){
               c = (fb_cell *) basen->n_obj;
               if (c != NULL){
                  c = c->c_chain;
                  if (c == NULL)
                     basen->n_p1++;
                  }
               }
            if (c == NULL){
               for (; basen->n_p1 < MAXSYM; basen->n_p1++)
                  if (stab[basen->n_p1] != NULL){
                     c = stab[basen->n_p1];
                     break;
                     }
               }
            }
         /* if c exists, it is set here */
         if (c != NULL){
            fb_mkstr(&(t_ret->n_nval), c->c_sval);
            t_ret->n_tval |= T_STR;
            basen->n_obj = (int) c;
            r->n_fval = 1;
            }
         else{
            r->n_fval = 0;
            basen->n_obj = NULL;
            basen->n_p1 = 0;
            }
         return(FB_AOK);
      }

/*
 * countkey - take the array as a name, no subscript, count keys
 *	countkey(array)
 */

   mf_countkey(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *arg, *basen;
         fb_cell **stab, *c, *bcell;
         int count = 0, i;
         
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         arg = n;
         /* locate the symbol table inside of array element */
         bcell = (fb_cell *) arg->n_obj;
         bcell = string_to_cell(bcell->c_sval);
         basen = bcell->c_vid;  /* same as mnode_to_var(arg) */
         /* if its a dbase array, return error */
         if (basen != NULL && istype_fld(basen))
            return(FB_ERROR);
         stab = bcell->c_symtab;

         if (stab != NULL)
            for (count = 0, i = 0; i < MAXSYM; i++)
               for (c = stab[i]; c != NULL; c = c->c_chain)
                  count++;
         r->n_fval = count;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

/*
 * findkey - find key for string s from array: findkey(key, s, array)
 *	store result into key and return either AOK or ERROR
 */

   mf_findkey(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *arg, *basen, *ret, *t_ret, *an;
         fb_cell **stab, *rc, *c, *bcell;
         int i, st = FB_ERROR;
         char *sval;
         
         if (fb_realnodes(n) < 3)
            return(FB_ERROR);
         ret = n; 		n = n->n_next;
         sval = tostring(n);	n = n->n_next;
         arg = n;
         t_ret = mnode_to_var(ret);
         /* locate the symbol table inside of array element */
         bcell = (fb_cell *) arg->n_obj;
         bcell = string_to_cell(bcell->c_sval);
         basen = bcell->c_vid;  /* same as mnode_to_var(arg) */
         /* if its a dbase array, return error */
         if (basen != NULL && istype_fld(basen))
            return(FB_ERROR);
         stab = bcell->c_symtab;

         /* locate the key used to store s */
         if (stab != NULL){
            for (i = 0, rc = NULL; i < MAXSYM; i++){
               for (c = stab[i]; c != NULL; c = c->c_chain){
                  /* array mnode - the place to where the value is */
                  an = c->c_vid;
                  if (istype_str(an)){
                     if (equal(an->n_nval, sval)){
                        rc = c;
                        break;
                        }
                     }
                  else if (atof(sval) == an->n_fval){
                     rc = c;
                     break;
                     }
                  }
               if (rc != NULL)
                  break;
               }
            if (rc != NULL){
               fb_mkstr(&(t_ret->n_nval), rc->c_sval);
               t_ret->n_tval |= T_STR;
               st = FB_AOK;
               }
            }
         r->n_fval = st;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

/*
 * rmkey - remove the key from the array: rmkey(key, array)
 *	does not whine if key is already not there
 */

   mf_rmkey(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *arg, *basen, *an;
         fb_cell **stab, *c, *rc, *pc, *bcell;
         int i;
         char *sval;
         
         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         sval = tostring(n);	n = n->n_next;
         arg = n;
         /* locate the symbol table inside of array element */
         bcell = (fb_cell *) arg->n_obj;
         bcell = string_to_cell(bcell->c_sval);
         basen = bcell->c_vid;  /* same as mnode_to_var(arg) */
         /* if its a dbase array, return error */
         if (basen != NULL && istype_fld(basen))
            return(FB_ERROR);
         stab = bcell->c_symtab;
         /* locate the key used to store s */
         if (stab != NULL){
            for (i = 0, rc = NULL; i < MAXSYM; i++){
               for (c = stab[i], pc = NULL; c != NULL; pc = c, c = c->c_chain){
                  /* array mnode - the place where the value is */
                  if (equal(c->c_sval, sval)){
                     rc = c;
                     break;
                     }
                  }
               if (rc != NULL)
                  break;
               }
            if (rc != NULL){
               /*
                * assert: pc should be the previous chain element
                *    rc is the actual element to remove
                *    could be stab[i] needs patching
                */
               if (pc == NULL){
                  /* must be stab[i] == rc */
                  stab[i] = rc->c_chain;
                  }
               else{
                  pc->c_chain = rc->c_chain;
                 }
               rc->c_chain = NULL;

               /* now free up element in rc - use type of basen as a guide */
               an = rc->c_vid;
               rc->c_vid = NULL;
               if (istype_glob(basen))
                  fb_free_globalnode(an);
               /* else if local, let garbage pick get var an ... */
               /* always remove the cell */
               fb_free(rc->c_sval);
               fb_freecell(rc);
               }
            }
         return(FB_AOK);
      }

/*
 * mf_set_checkbox - values are guarateed to be a CGI checkbox and sorted!.
 *	find the place a sym tab should be, and maybe put one there
 *	store the current one based array value
 *	make the assignment name[i] = value
 */

   int mf_set_checkbox(name, value)
      char *name, *value;

      {
         fb_cell *bcell, **stab, *cn;
         fb_mnode *basen, *an;
         char subscript[FB_MAXLINE];

         if (checkbox_subscript == 0)
            checkbox_subscript++;
         else if (!equal(checkbox_name, name))
            checkbox_subscript = 1;
         else
            checkbox_subscript++;
         strcpy(checkbox_name, name);
         sprintf(subscript, "%d", checkbox_subscript);

         bcell = string_to_cell(name);	/* this is the c_vid */
         basen = bcell->c_vid;
         if (basen == NULL){
            fb_serror(FB_MESSAGE, "Illegal base of array", NIL);
            return(FB_ERROR);
            }
         /* bcell is the object with the symtab in it */
         if (bcell->c_symtab == NULL)
            bcell->c_symtab = fb_makesymtab();
         stab = bcell->c_symtab;
         cn = fb_u_sinstall(subscript, stab);
         if (cn->c_vid == NULL)
            cn->c_vid = fb_s_node0(V_ID);
         an = cn->c_vid;
         an->n_tval |= T_STR;
         fb_mkstr(&(an->n_nval), value);
         return(FB_AOK);
      }

/*
 * array_to_list - generate a non-glink-threaded list of nodes
 *	that represent the array -
 *		use n_next's to get to each element -- each a key/subscript
 *		use n_list to link the value of the key/subscript
 */

   fb_mnode *array_to_list(array)
      fb_mnode *array;

      {
         fb_mnode *n, *sn, *vn, *root_sn = NULL, *psn = NULL;
         fb_mnode *basen;
         fb_cell *bcell, **stab, *c;
         int i;

         bcell = (fb_cell *) array->n_obj;
         bcell = string_to_cell(bcell->c_sval);
         basen = bcell->c_vid;  	/* same as mnode_to_var(arg) */
         stab = bcell->c_symtab;
         if (stab == NULL)
            return(root_sn);

         /* for each element in stab, copy to new nodelist pair */
         for (i = 0; i < MAXSYM; i++){
            for (c = stab[i]; c != NULL; c = c->c_chain){
                  if (c->c_sval != NULL && c->c_vid != NULL){
                     n = c->c_vid;
                     vn = fb_s_makenode();
                     sn = fb_s_makenode();
                     fb_copynode(vn, n);
                     fb_mkstr(&(sn->n_nval), c->c_sval);
                     sn->n_list = vn;
                     if (psn != NULL)
                        psn->n_next = sn;
                     else
                        root_sn = sn;
                     psn = sn;
                     }
               }
            }
         return(root_sn);
      }

/*
 * copy_array_tree - copy the next-list tree used to encode returned arrays
 */

   fb_mnode *copy_array_tree(n)
      fb_mnode *n;

      {
         fb_mnode *psn = NULL, *ln, *vn, *sn, *root_sn = NULL;

         for (; n != NULL; n = n->n_next){
            ln = n->n_list;
            if (ln == NULL)
               continue;
            vn = fb_makenode();
            sn = fb_makenode();
            fb_copynode(vn, ln);
            fb_copynode(sn, n);
            sn->n_list = vn;
            if (psn != NULL)
               psn->n_next = sn;
            else
               root_sn = sn;
            psn = sn;
            }
         return(root_sn);
      }
