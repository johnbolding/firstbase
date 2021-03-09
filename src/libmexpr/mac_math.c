/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_math.c,v 9.2 2001/01/16 02:46:54 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macro_math_sid[] = "@(#) $Id: mac_math.c,v 9.2 2001/01/16 02:46:54 john Exp $";
#endif

#include <fb.h>
#include <macro_e.h>
#include <float.h>

   mf_max(n, r)
      fb_mnode *n, *r;

      {
         double max = 0;

         (void) Macro_math_sid;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         for (; n != NULL; n = n->n_next)
            if (n->n_fval > max)
               max = n->n_fval;
         r->n_fval = max;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_min(n, r)
      fb_mnode *n, *r;

      {
         double min = DBL_MAX;

         if (fb_realnodes(n) < 1 )
            return(FB_ERROR);
         for (; n != NULL; n = n->n_next)
            if (n->n_fval < min)
               min = n->n_fval;
         r->n_fval = min;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_ston(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         tostring(n);
         r->n_fval = atof(n->n_pval);
         r->n_tval |= T_NUM;
         r->n_scale = 6;
         return(FB_AOK);
      }

   mf_pow(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         n2 = n;
         r->n_fval = (double) pow(n1->n_fval, n2->n_fval);
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_round(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2;
         char buf[120], *p;
         double d;
         int pos;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         n2 = n;
         r->n_fval = n1->n_fval;
         r->n_scale = (int) n2->n_fval;
         r->n_tval |= T_NUM;
         d = r->n_fval;
         pos = n2->n_fval;
	 if (d >= 0)
	    d += (0.5 / pow(10.0, (double) pos));
	 else
	    d -= (0.5 / pow(10.0, (double) pos));
	 sprintf(buf, "%f", d);
	 if (pos > 6)
	    pos = 6;
	 p = strrchr(buf, '.') + pos + 1;
	 /* no check for a 0 p value since sprintf guarantees a decimal */
	 *p = NULL;
	 r->n_fval = atof(buf);

         return(FB_AOK);
      }

   mf_trunc(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2;
         char buf[120], *p;
         double v1, v2;

         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         n2 = n;
         v1 = n1->n_fval;
         v2 = n2->n_fval;
         if (v2 < 0)
            v2 = 0;
         sprintf(buf, "%.11f", v1);
         p = strchr(buf, '.');
         p += ((int) v2 + 1);
         *p = NULL;
         r->n_fval = (double) atof(buf);
         r->n_scale = v2;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_abs(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         r->n_fval = fabs(n->n_fval);
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_sin(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         r->n_fval = sin(n->n_fval);
         r->n_scale = 6;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_cos(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         r->n_fval = cos(n->n_fval);
         r->n_scale = 6;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_tan(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         r->n_fval = tan(n->n_fval);
         r->n_scale = 6;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_asin(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         r->n_fval = asin(n->n_fval);
         r->n_scale = 6;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_acos(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         r->n_fval = acos(n->n_fval);
         r->n_scale = 6;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_atan(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         r->n_fval = atan(n->n_fval);
         r->n_scale = 6;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_atan2(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         n2 = n;
         r->n_fval = atan2(n1->n_fval, n2->n_fval);
         r->n_scale = 6;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_hypot(n, r)
      fb_mnode *n, *r;

      {
         fb_mnode *n1, *n2;

         if (fb_realnodes(n) < 2)
            return(FB_ERROR);
         n1 = n; n = n->n_next;
         n2 = n;
         r->n_fval = hypot(n1->n_fval, n2->n_fval);
         r->n_scale = 6;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_exp(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         r->n_fval = exp(n->n_fval);
         r->n_scale = 6;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_sqrt(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         r->n_fval = sqrt(n->n_fval);
         r->n_scale = 6;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_log(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         r->n_fval = log(n->n_fval);
         r->n_scale = 6;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_ceil(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         r->n_fval = ceil(n->n_fval);
         r->n_scale = 0;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_floor(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
         r->n_fval = floor(n->n_fval);
         r->n_scale = 0;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }

   mf_srandom(n, r)
      fb_mnode *n, *r;

      {
         if (fb_realnodes(n) < 1)
            return(FB_ERROR);
#if HAVE_SRANDOM
         srandom(n->n_fval);
#endif /* HAVE_SRANDOM */
         return(FB_AOK);
      }

   mf_random(n, r)
      fb_mnode *n, *r;

      {
#if HAVE_RANDOM
         r->n_fval = random();
#else
         r->n_fval = 0;
#endif /* HAVE_RANDOM */
         r->n_scale = 0;
         r->n_tval |= T_NUM;
         return(FB_AOK);
      }
