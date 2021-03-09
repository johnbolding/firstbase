/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: fndfield.c,v 9.0 2001/01/09 02:56:26 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Findfield_sid[] = "@(#) $Id: fndfield.c,v 9.0 2001/01/09 02:56:26 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>

/* 
 * findfield - find a field in kp given name. return ptr to it or NULL 
 */
 
   fb_field *fb_findfield(s, hdb)
      register char *s;
      register fb_database *hdb;
      
         {
	    register int i;
	    
            for (i = 0; i <= hdb->nfields; i++)
	       if (equal(hdb->kp[i]->id, s))
	          return(hdb->kp[i]);
	    return(NULL);
	 }

/* 
 * findnfield - find a field in kp given name. return array position n.
 */
 
   fb_findnfield(s, hdb)
      register char *s;
      register fb_database *hdb;
      
         {
	    register int i;
	    
            for (i = 0; i <= hdb->nfields; i++)
	       if (equal(hdb->kp[i]->id, s))
	          return(i);
	    return(-1);
	 }
