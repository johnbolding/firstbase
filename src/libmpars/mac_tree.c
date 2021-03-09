/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: mac_tree.c,v 9.0 2001/01/09 02:56:54 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Macrotree_sid[] = "@(#) $Id: mac_tree.c,v 9.0 2001/01/09 02:56:54 john Exp $";
#endif

#include <fb.h>
#include <macro_e.h>
#include <sys/types.h>
#include <sys/stat.h>

extern int lineno;			/* for tracing line number */

typedef struct sblock block;
struct sblock {
   char s_stream[1024];
   block *s_next;
   int s_size;
   };
static block *headblock = NULL;
#define BLOCKSIZE	1024

#if !FB_PROTOTYPES
static void readstdin();
#else
static void readstdin(void);
#endif

/*
 * macrotree - given a file name, return AOK or ERROR. sets "winner".
 */

   fb_macrotree(fname)
      char *fname;
   
      {
         char line[FB_MAXLINE], *p, *i_mem, *q;
         int fd, st = 0;
         long lsize;
         struct stat statb;

         (void) Macrotree_sid;

         strcpy(parsename, fname);
         fb_mkstr(&(m_filenames[m_current_filename]), fname);
         if (symtab == NULL)
            symtab = fb_makesymtab();
         fd = fb_mustopen(fname, READ);
         if (fd <= 0){
            fb_serror(FB_CANT_OPEN, fname, "");
            return(FB_ERROR);
            }

         /*
          * determine file size so as to allocate memory
          */
         if (stat(fname,&statb) < 0)
            return(FB_ERROR);
         lsize = statb.st_size;
         i_mem = (char *) fb_malloc((unsigned) lsize + 1);

         /* copy file into memory -- assumes file will fit */
         fb_r_init(fd);
         for (p = i_mem;;){
            if (fb_nextline(line, FB_MAXLINE) == 0)
               break;
            for (q = line; *q; )
               *p++ = *q++;
            *p++ = '\n';
            *p = NULL;
            }

         i_cur = 0;
         i_ptr = i_mem;
         winner = NULL;
         lineno = 1;
         macro_errors = 0;
         if (lsize > 0)
            st = yyparse();
         fb_r_end();	/* closes fd */
         /* fb_free up the temporary storage used */
         if (++m_current_filename >= MAX_MFILES)
            m_current_filename = 0;
         fb_free(i_mem);
         if (st == 0)
            return(FB_AOK);
         else
            return(FB_ERROR);
      }

/*
 * macroscript - given a string (script) return AOK or ERROR. sets "winner".
 */

   fb_macroscript(script)
      char *script;
   
      {
         char *i_mem;
         int st = 0;
         long lsize;

         *parsename = NULL;
         fb_mkstr(&(m_filenames[m_current_filename]), "");
         if (symtab == NULL)
            symtab = fb_makesymtab();

         lsize = strlen(script);
         i_mem = (char *) fb_malloc((unsigned) lsize + 1);

         /* copy file into memory -- assumes file will fit */
         strcpy(i_mem, script);

         i_cur = 0;
         i_ptr = i_mem;
         winner = NULL;
         lineno = 1;
         macro_errors = 0;
         if (lsize > 0)
            st = yyparse();
         /* free up the temporary storage used */
         fb_free(i_mem);
         if (st == 0)
            return(FB_AOK);
         else
            return(FB_ERROR);
      }

/*
 * macrostdin - read stdin, move to i_mem, parse, return AOK or ERROR.
 *	sets "winner".
 */

   fb_macrostdin()
   
      {
         char *i_mem, *p;
         int st = 0;
         long lsize = 0;
         block *cb, *nb;

         *parsename = NULL;
         fb_mkstr(&(m_filenames[m_current_filename]), "");
         if (symtab == NULL)
            symtab = fb_makesymtab();

         readstdin();

         for (cb = headblock; cb != NULL; cb = cb->s_next)
            lsize += cb->s_size;
         i_mem = (char *) fb_malloc((unsigned) lsize + 1);

         /* copy blocks into memory -- assumes file will fit */
         for (p = i_mem, cb = headblock; cb != NULL; cb = nb){
            strncpy(p, cb->s_stream, cb->s_size);
            p += cb->s_size;
            nb = cb->s_next;
            fb_free((char *) cb);
            }
         *p = NULL;

         i_cur = 0;
         i_ptr = i_mem;
         winner = NULL;
         lineno = 1;
         macro_errors = 0;
         if (lsize > 0)
            st = yyparse();
         /* free up the temporary storage used */
         fb_free(i_mem);
         if (st == 0)
            return(FB_AOK);
         else
            return(FB_ERROR);
      }

   static void readstdin()
      {
         char *p;
         block *cb, *nb;

         headblock = (block *) fb_malloc(sizeof(block));
         headblock->s_next = NULL;
         headblock->s_size = 0;
         cb = headblock;
         for (;;){
            p = cb->s_stream;
            if ((cb->s_size = read(0, p, BLOCKSIZE)) == 0)
               break;
            nb = (block *) fb_malloc(sizeof(block));
            nb->s_next = NULL;
            nb->s_size = 0;
            cb->s_next = nb;
            cb = nb;
            }
      }
