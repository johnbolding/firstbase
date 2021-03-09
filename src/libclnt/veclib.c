/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: veclib.c,v 9.1 2004/12/31 04:35:23 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Veclib_sid[] = "@(#) $Id: veclib.c,v 9.1 2004/12/31 04:35:23 john Exp $";
#endif

#if RPC

#include <fb.h>
#include <fbserver.h>
#include <stdarg.h>

# This area needs work to use stdarg. -jpb 12/30/2004

#define MAXARGS 100

static char *svec[MAXARGS];
static int slen[MAXARGS];
static int s_top = 0;

extern char cdb_EOREC;

/*
 * veclib - routines to interface with a fb_varvec structure.
 *	used for fbserver communications.
 */

/*
 * fb_loadvec - load a set of strings into vector v.
 *	usage: fb_loadvec(v, s0, s1, s2 ... sn)
 */

   void fb_loadvec(va_alist)
      va_dcl

      {
	 va_list ap;
         fb_varvec *v;
         char *p, *args[MAXARGS], num[10];
         int len, maxarg = 0, tlen, i;

	 va_start(ap);
	 v = va_arg(ap, fb_varvec *);
         while ((args[maxarg] = va_arg(ap, char *)) != (char *) 0)
            maxarg++;
	 va_end(ap);
         for (len = 0, i = 0; i < maxarg; i++)
            len += strlen(args[i]);
         sprintf(num, "%d", maxarg + 1);
         tlen = strlen(num) + 1;
         len += (maxarg + 2 + tlen);
         v->v_length = len;
         v->v_data = (char *) fb_malloc((unsigned) len);
         p = v->v_data;
         strcpy(p, num);
         p += tlen;
         for (i = 0; i < maxarg; i++){
            strcpy(p, args[i]);
            p += (strlen(args[i]) + 1);
            }
         *p++ = cdb_EOREC;
         *p = NULL;
      }

/*
 * fb_clear_svec - clear the svec variables
 */

   fb_clear_svec()
      {
         int i;

         s_top = 0;
         for (i = 0; i < MAXARGS; i++){
            slen[i] = 0;
            svec[i] = NULL;
            }
      }

/*
 * fb_concat_svec - save a passed in string for later vectore building
 */

   fb_concat_svec(s)
      char *s;

      {
         int len;

         if (s == NULL)
            len = 0;
         else
            len = strlen(s);
         svec[s_top] = (char *) fb_malloc((unsigned) (len + 1));
         slen[s_top] = len + 1;
         if (len > 0)
            strcpy(svec[s_top], s);
         s_top++;
      }

/*
 * fb_build_svec - covert the svec into data stored in the varvec v
 */

   fb_build_svec(v)
      fb_varvec *v;

      {
         int i, vlen, tlen;
         char *p, num[10];

         for (vlen = 0, i = 0; i < s_top; i++)
            vlen += slen[i];
         vlen += 2;
         if (v->v_data != NULL)
            fb_free(v->v_data);
         sprintf(num, "%d", s_top + 1);
         tlen = strlen(num) + 1;
         vlen += tlen;
         v->v_data = (char *) fb_malloc((unsigned) vlen);
         v->v_length = vlen;
         p = v->v_data;
         strcpy(p, num);
         p += tlen;
         for (i = 0; i < s_top; i++){
            strcpy(p, svec[i]);
            p += slen[i];
            }
         *p++ = cdb_EOREC;
         *p = NULL;
         fb_free_svec();
      }

/*
 * fb_free_svec - fb_free the svec variables and clear at the same time.
 */

   fb_free_svec()

      {
         int i;

         for (i = 0; i < s_top; i++){
            if (svec[i] != NULL)
               fb_free(svec[i]);
            svec[i] = NULL;
            slen[i] = 0;
            }
         s_top = 0;
      }

/*
 * fb_tracevec - trace a vector - used for debugging
 */

   fb_tracevec(v, s)
      fb_varvec *v;
      char *s;

      {
         char *p;
         int maxargs, i;

         fprintf(stderr, "trace vector - %s\n", s);
         if (v == NULL){
            fprintf(stderr, "NULL vector\n");
            return;
            }
         fprintf(stderr, "  len = %d\n", v->v_length);
         p = v->v_data;
         maxargs = atoi(p);
         for (i = 0; i < maxargs; i++){
            fprintf(stderr, "arg[%d]=%s\n", i, p);
            p += strlen(p);
            p++;
            }
         if (*p == '\005')
            fprintf(stderr, "found EOR marker\n");
      }

/*
 * fb_freevec - fb_free up the data allocated inside of v
 */

   fb_freevec(v)
      fb_varvec *v;

      {
         if (v == NULL)
            return;
         if (v->v_data != NULL)
            fb_free(v->v_data);
         v->v_data = NULL;
      }

/*
 * fb_clear_vec - clear the svec variables
 */

   fb_clear_vec(v)
      fb_varvec *v;

      {
        v->v_length = 0;
        v->v_data = NULL;
      }

/*
 * fb_argvec - return a pointer to the v_data[i'th] ... the i'th argument
 *    uses real indices --- 0 ... maxarg-1
 */

   char *fb_argvec(v, k)
      fb_varvec *v;
      int k;

      {
         char *p;
         int i;

         if (k < 0)
            k = 0;
         for (i = 0, p = v->v_data; ; p++){
            if (i >= k)
               break;
            if (*p == NULL)
               i++;
            else if (*p == cdb_EOREC)
               break;
            }
         return(p);
      }

#endif /* RPC */
