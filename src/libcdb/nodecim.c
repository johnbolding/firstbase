/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: nodecim.c,v 9.0 2001/01/09 02:56:28 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Nodecimal_sid[] = "@(#) $Id: nodecim.c,v 9.0 2001/01/09 02:56:28 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/*
 * nodecimal - remove the decimal the user used. add 0s if needed.
 *	return the number of zeros added.
 */

   fb_nodecimal(buf)
      char *buf;

      {
         char newbuf[FB_MAXINPUT], *p, *q;
	 int pastdot = 0, countpast = 0, addz = 0;
	 
	 for (p = buf, q = newbuf; *p; ){
	    if (*p != CHAR_DOT){
	       if (pastdot)
	          countpast++;
	       *q++ = *p;
	       }
	    else{
	       pastdot = 1;
	       addz--;
	       }
	    p++;
	    }
	 for (; countpast < 2; countpast++){
	    *q++ = CHAR_0;
	    addz++;
	    }
	 *q = NULL;
	 strcpy(buf, newbuf);
	 return(addz);
      }

/*
 * putdecimal - this noise removes the need for a divide -- Cdb stores
 * 		all dollar amounts without a decimal. this puts it back.
 */
   void fb_putdecimal(buf)
      char *buf;
      
      {
         char *p, tbuf[FB_MAXLINE];
	 int i, len, neg = 0;

         if (*buf == NULL)
	    return;
         if (buf[0] == CHAR_MINUS){
            neg = 1;
            strcpy(tbuf, buf+1);
            strcpy(buf, tbuf);
            }
         len = strlen(buf);
         if (len == 1){
            sprintf(tbuf, "00%s", buf);
            strcpy(buf, tbuf);
            }
         else if (len == 2){
            sprintf(tbuf, "0%s", buf);
            strcpy(buf, tbuf);
            }
	 for (p = buf; *p; p++)			/* set p to end */
	    ;
	 for (p++, i = 3; i > 0; i--, p--)	/* slide 3 bytes right */
	    *p = *(p-1);
	 *p = CHAR_DOT;				/* insert decimal */
         if (neg){
            strcpy(tbuf, buf);
            sprintf(buf, "-%s", tbuf);
            }
      }
