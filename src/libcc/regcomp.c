/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: regcomp.c,v 9.0 2001/01/09 02:56:21 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Regcomp_sid[] = "@(#) $Id: regcomp.c,v 9.0 2001/01/09 02:56:21 john Exp $";
#endif

#include <fb.h>
#include <regex.h>

#define N_PMATCH	20
static regex_t *GPAT = NULL;
static regmatch_t pmatch[N_PMATCH];
static int regcomp_cflags = (REG_EXTENDED);

/*
 * RE_COMP and RE_EXEC are used if the function regcomp() is available
 */

   char *RE_COMP(pat)
      const char *pat;

      {
         int status, i;

         if (GPAT != NULL)
            regfree(GPAT);
         else
            GPAT = (regex_t *) fb_malloc(sizeof(regex_t));
         status = regcomp(GPAT, pat, regcomp_cflags);
         for (i = 0; i < N_PMATCH; i++){
            pmatch[i].rm_so = -1;
            pmatch[i].rm_eo = -1;
            }
         if (status == 0)
            return((char *) 0);
         else
            return("bad pattern");
      }

   int RE_EXEC(s)
      const char *s;

      {
         int status;

         status = regexec(GPAT, s, N_PMATCH, pmatch, 0);
         if (status == 0)
            return(1);
         else
            return(0);
      }

   fb_set_regcomp_icase(flag)
      int flag;

      {
         if (flag == 0)
            regcomp_cflags = (REG_EXTENDED);
         else
            regcomp_cflags = (REG_EXTENDED | REG_ICASE);
      }

   fb_get_regexec_so(i)
      int i;

      {
         if (i >= 0 && i < N_PMATCH)
            return(pmatch[i].rm_so);
         else
            return(-1);
      }

   fb_get_regexec_eo(i)
      int i;

      {
         if (i >= 0 && i < N_PMATCH)
            return(pmatch[i].rm_eo);
         else
            return(-1);
      }
