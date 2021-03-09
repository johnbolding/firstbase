/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: testtree.c,v 9.0 2001/01/09 02:56:31 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Testtree_sid[] = "@(#) $Id: testtree.c,v 9.0 2001/01/09 02:56:31 john Exp $";
#endif

#include <fb.h>

/*
 * test_tree - test for a btree file
 */

   int fb_test_tree(db)
      fb_database *db;

      {

         char iname[FB_MAXNAME];

         fb_rootname(iname, db->dindex);
         strcat(iname, ".bidx");
	 if (access(iname, 0) != 0)
            return(0);
         return(1);
      }
