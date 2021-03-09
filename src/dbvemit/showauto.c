/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: showauto.c,v 9.0 2001/01/09 02:56:03 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Showauto_sid[] = "@(#) $Id: showauto.c,v 9.0 2001/01/09 02:56:03 john Exp $";
#endif

#include <dbve_ext.h>

/* showauto - show any forced auto increments.
*/

   showauto(hp)
      fb_database *hp;

      {
	 return(FB_AOK);
      }
