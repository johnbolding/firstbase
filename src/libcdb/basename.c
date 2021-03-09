/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: basename.c,v 9.1 2001/01/16 02:46:50 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Basename_sid[] = "@(#) $Id: basename.c,v 9.1 2001/01/16 02:46:50 john Exp $";
#endif

#include <fb.h>

static char *MAP = ".map", *CDB = ".cdb", *DDICT = ".ddict", 
   *IDICT = ".idict", *IDX = ".idx", *SDICT = ".sdict";
static char *PRT = ".prt", *LBL = ".lbl", *EMIT = ".emit";   
static char *RDX = ".rdx", *RDICT = ".rdict", *VDICT = ".vdict";
static char *BIDX = ".bidx", *BSEQ = ".bseq";

/* 
 *  basename - get base name from t and place in s. return s.
 */
 
   char *fb_basename(s, t)
      char *s, *t;
      
      {
         char *p;
         
	 *s = 0;
	 if (t == 0 || !(*t))
	    return(s);
	 if ((p = strrchr(t, '/')) != 0)
	    p++;
	 else
	    p = t;
	 strcpy(s, p);
	 if (((p = strrchr(s, '.')) != 0) && (!strncmp(p, IDICT, 6) ||
	       !strcmp(p, MAP) || !strcmp(p, CDB) || !strcmp(p, IDX) ||
	       !strcmp(p, PRT) || !strcmp(p, LBL) || !strcmp(p, EMIT) ||
	       !strcmp(p, RDX) || !strcmp(p, RDICT) || !strcmp(p, SDICT) ||
	       !strcmp(p, BIDX) || !strcmp(p, BSEQ) ||
	       !strcmp(p,DDICT)|| !strcmp(p, VDICT)))
	    *p = 0;
	 return(s);
      }
