/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: pgen.h,v 9.0 2001/01/09 02:56:16 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

struct totlist {			/* printout totals list */
   fb_field *tfld;
   double tval;
   struct totlist *tlink;
   };
   
struct brklist {			/* printout breaklist */
   fb_field *bfld;
   struct totlist *btlist;
   char *bval;
   int blevel;
   short simple_flag;
   long bcount;
   struct brklist *blink;
   };

#define DCNV 		"convrt.cdb"	/* default cgen  name */
#define DPRT 		"cdbprt.prt"	/* default printout name */
