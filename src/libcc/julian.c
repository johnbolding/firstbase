/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: julian.c,v 9.0 2001/01/09 02:56:20 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Julian_sid[] = "@(#) $Id: julian.c,v 9.0 2001/01/09 02:56:20 john Exp $";
#endif

#include <fb.h>

/*
 * julian - produce a julian date value since date 00/00/0000
 */
 
   fb_julian(m, d, y)
      int m, d, y;

      {
         int mp, yp, t;
         
         if (m <= 2){
            mp = 0;
            yp = y - 1;
            }
         else { /* m > 2 */
            mp = (int) (0.4 * m + 2.3);
            yp = y;
            }
         t = (int) (yp/4) - (int) (yp/100) + (int) (yp/400);
         return(365 * y + 31 * (m - 1) + d + t - mp);
      }
