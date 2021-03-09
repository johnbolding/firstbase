/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: cgi.c,v 9.6 2002/08/04 21:51:59 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Cgi_sid[] = "@(#) $Id: cgi.c,v 9.6 2002/08/04 21:51:59 john Exp $";
#endif

#include <fb.h>

/*
 * cgi - cgi layer of library functions
 */

typedef struct fb_s_cgi_entry fb_cgi_entry;
struct fb_s_cgi_entry {
    char *c_name;
    char *c_val;
    fb_cgi_entry *c_next;
    fb_cgi_entry *c_prev;
    };

fb_cgi_entry *fb_cgi_head = NULL;
fb_cgi_entry *fb_cgi_tail = NULL;

extern char *cdb_tempdir;		/* temp directory for mkstemp files */

static int cgi_pairs = 0;
static int content_length = 0;
static char *mem = NULL, *word = NULL;
static char boundary[FB_MAXLINE];
static int blen; 			/* boundary length */

static char pfilename[FB_MAXLINE];	/* parse filename */
static char wfilename[FB_MAXLINE];	/* working filename */

static char cgi_tmpdir[FB_MAXLINE];
static int cgi_maxsize = 0;

static char *pword = NULL;
static int ppos = 0;
static int stashcount = 0;

#if !FB_PROTOTYPES

static void plustospace();
static void unescape_url();
static char x2c();
static free_cgi_list();
static int cgi_parse();
static cgi_read_post();
static cgi_read_get();
static fb_cgi_entry *make_cgi_entry();
static mem_getline();
static load_disposition_info();
static int multipart_parse();
static int compare_entry();
static int stashchar();
static void cgi_sort();
static boundary_marker_match();

#else /* FB_PROTOTYPES */

static void plustospace(char *str);
static void unescape_url(char *url);
static char x2c(char *what);
static free_cgi_list(void);
static int cgi_parse(char *pmen, char *pword);
static cgi_read_post(void);
static cgi_read_get(void);
static fb_cgi_entry *make_cgi_entry(void);
static mem_getline(void);
static load_disposition_info(char *, char *, char *);
static int multipart_parse(void);
static int compare_entry(const void *, const void *);
static int stashchar(int);
static cgi_sort(void);
static boundary_marker_match(char *);

#endif /* FB_PROTOTYPES */

#define PWORD_SIZE	10240

static dump_mem();

   void fb_cgi_init()
      {
         printf("Content-type: text/html\n\n");
         fb_cgi_head = NULL;
         fb_cgi_tail = NULL;
      }

   int fb_cgi_read()
      {
         char *p;
         int st;

         if (fb_cgi_head != NULL)
            free_cgi_list();
         p = getenv("REQUEST_METHOD");
         if (p && equal(p, "POST"))
            st = cgi_read_post();
         else
            st = cgi_read_get();
         if (st == FB_AOK)
            cgi_sort();
#ifdef DEBUG_FIRSTBASE_CGI
         fb_cgi_echo();
#endif
         return(st);
      }

   static int cgi_read_post()
      {
         int len, formdata_flag = 0, st;
         char *p, *q;

         p = getenv("CONTENT_LENGTH");
         if (!p){
            printf("<P>CONTENT_LENGTH environment variable is not set.\n");
            return(FB_ERROR);
            }
         content_length = atoi(p);
         
         p = getenv("CONTENT_TYPE");
         if (p && strncmp(p, "multipart/form-data; ", 21) == 0){
            q = strchr(p, '=');
            if (q != NULL){
               q++;
               if (*q == '\"')
                  q++;
               boundary[0] = NULL;
               /*strcpy(boundary, "--");*/
               strcat(boundary, q);
               blen = strlen(boundary);
               if (boundary[blen - 1] == '\"'){
                  boundary[blen - 1] = NULL;
                  blen--;
                  }
               formdata_flag = 1;
               }
            else{
               printf("<P>CONTENT_TYPE boundary= is not set.\n");
               return(FB_ERROR);
               }
            }
         /*
          * since the longest word could only be content_length long
          * and we dont know how big it will be, use CL as upper limit
          */
#ifdef DEBUG_FIRSTBASE_CGI
         formdata_flag = 0;
#endif
         if (formdata_flag){
            /* handle multi part forms */
            pword = (char *) fb_malloc((unsigned) (PWORD_SIZE + 1));
            strcpy(cgi_tmpdir, cdb_tempdir);
            fb_assure_slash(cgi_tmpdir);
            umask(0);
            st = multipart_parse();
            fb_free(pword);
            }
         else{
            if (mem != NULL)
               fb_free(mem);
            if (word != NULL)
               fb_free(word);
            mem = (char *) fb_malloc((unsigned) (content_length + 1));
            word = (char *) fb_malloc((unsigned) (content_length + 1));
            if (fread(mem, 1, content_length, stdin) != content_length){
               return(FB_ERROR);
               }
            mem[content_length] = NULL;
#ifdef DEBUG_FIRSTBASE_CGI
            dump_mem(mem, content_length);
#endif
            st = cgi_parse(mem, word);
            fb_free(mem);
            fb_free(word);
            } 
         return(st);
      }

   static cgi_read_get()
      {
         char *p;

         p = getenv("QUERY_STRING");
         if (!p)
            return(FB_AOK);
         content_length = strlen(p);
         /*
          * since the longest word could only be content_length long
          * and we dont know how big it will be, use CL as upper limit
          */
         if (mem != NULL)
            fb_free(mem);
         mem = p;
         if (word != NULL)
            fb_free(word);
         word = (char *) fb_malloc((unsigned) (content_length + 1));
         cgi_parse(mem, word);
         fb_free(word);
         return(FB_AOK);
      }

/*
 * cgi_parse - parse to get the pairs
 */

   static int cgi_parse(pmem, pword)
      char *pmem, *pword;

      {
         char *p, *q;
         fb_cgi_entry *c = NULL;
         int state, i;

         /*
          * parsing has 4 states:
          *
          * 0: beginning of name
          * 1: after beginning of name
          * 2: beginning of value
          * 3: after beginning of value
          */
         cgi_pairs = 0;
         for (p = pmem, state = 0; *p; ){
            switch(state){
               case 0:
                  pword[0] = NULL;
                  q = pword;
                  for (; *p && *p != '='; p++)
                     *q++ = *p;
                  *q = NULL;
                  if (*p == NULL)
                     break;
                  plustospace(pword);
                  unescape_url(pword);
                  c = make_cgi_entry();
                  fb_mkstr(&(c->c_name), pword);
                  state = 1;
                  break;
               case 1:
                  /* at the '=' in name=value&&name=value */
                  p++;
                  state = 2;
                  break;
               case 2:
                  pword[0] = NULL;
                  q = pword;
                  for (; *p && *p != '&'; p++)
                     *q++ = *p;
                  *q = NULL;
                  plustospace(pword);
                  unescape_url(pword);
                  fb_mkstr(&(c->c_val), pword);
                  state = 3;
                  break;
               case 3:
                  /* at the '&' in name=value&&name=value */
                  p++;
                  cgi_pairs++;
                  state = 0;
                  /* link c into place */
                  if (fb_cgi_head == NULL)
                     fb_cgi_head = fb_cgi_tail = c;
                  else{
                     fb_cgi_tail->c_next = c;
                     fb_cgi_tail = c;
                     }
                  c = NULL;
                  break;
               }
            }
         /*
          * catch the last one
          */
         if (state == 3){
            cgi_pairs++;
            if (fb_cgi_head == NULL)
               fb_cgi_head = fb_cgi_tail = c;
            else{
               fb_cgi_tail->c_next = c;
               fb_cgi_tail = c;
               }
            c = NULL;
            }
         return(FB_AOK);
      }

/*
 * fb_cgi_value - locate the value field fn and store in value.
 */

   void fb_cgi_value(value, fn)
      char *value, *fn;

      {
         fb_cgi_entry *c;

         value[0] = NULL;
         for (c = fb_cgi_head; c != NULL; c = c->c_next){
            if (equal(c->c_name, fn)){
               strcpy(value, c->c_val);
               break;
               }
            }
      }

/*
 * fb_cgi_foreach - foreach pair, call some function with <name,value> pair
 */

   void fb_cgi_foreach(f)
      int (*f)(char *name, char *val);

      {
         fb_cgi_entry *c;

         for (c = fb_cgi_head; c != NULL; c = c->c_next)
	    (*f)(c->c_name, c->c_val);
      }

/*
 * fb_cgi_checkbox - foreach  pair that has either the next or prev as the
 *    same name (thus, is a checkbox), call some function
 *    with <name,value> pair.
 */

   void fb_cgi_checkbox(f)
      int (*f)(char *name, char *val);

      {
         fb_cgi_entry *c, *p, *n;
         int cb;

         for (c = fb_cgi_head; c != NULL; c = c->c_next){
            p = c->c_prev;
            n = c->c_next;
            cb = 0;
            if (p != NULL && equal(p->c_name, c->c_name))
               cb = 1;
            if (n != NULL && equal(n->c_name, c->c_name))
               cb = 1;
            if (cb)
	       (*f)(c->c_name, c->c_val);
            }
      }

/*
 * fb_cgi_echo - spit out all the lines pairs read from the form
 */

   void fb_cgi_echo()
      {
         fb_cgi_entry *c;

         printf("Content-type: text/html\n\n");
         printf("<ul>\n");

         for (c = fb_cgi_head; c != NULL; c = c->c_next)
            printf("<li> <code>%s = %s</code>\n", c->c_name, c->c_val);
         printf("</ul>\n");
      }

   static void plustospace(str)
      char *str;

      {
         register int x;

         for(x=0; str[x]; x++)
            if(str[x] == '+')
               str[x] = ' ';
      }

   static void unescape_url(url)
      char *url;

      {
         register int x, y;

         for(x=0,y=0; url[y]; ++x, ++y) {
            if((url[x] = url[y]) == '%') {
               url[x] = x2c(&url[y+1]);
               y+=2;
               }
            }
         url[x] = '\0';
      }

   static char x2c(what)
      char *what;

      {
         char digit;

         digit = (what[0] >= 'A' ? ((what[0] & 0xdf)-'A')+10 : (what[0]-'0'));
         digit *= 16;
         digit += (what[1] >= 'A' ? ((what[1] & 0xdf)-'A')+10 : (what[1]-'0'));
         return(digit);
      }

   static fb_cgi_entry *make_cgi_entry()
      {
         fb_cgi_entry *c;

         c = (fb_cgi_entry *) fb_malloc(sizeof(fb_cgi_entry));
         c->c_name = NULL;
         c->c_val = NULL;
         c->c_next = NULL;
         c->c_prev = NULL;
         return(c);
      }

   static free_cgi_list()
      {
         fb_cgi_entry *c, *n;

         for (c = fb_cgi_head; c; c = n){
            n = c->c_next;
            fb_free(c->c_name);
            fb_free(c->c_val);
            fb_free((char *) c);
            }
      }

   static int compare_entry(x, y)

#if !FB_PROTOTYPES
         void *x, *y;
#else
         const void *x, *y;
#endif  /* FB_PROTOTYPES */
         
      {
         fb_cgi_entry **a, **b;

         a = (fb_cgi_entry **) x;
         b = (fb_cgi_entry **) y;
         if (a == NULL || *a == NULL)
            return(1);
         else if (b == NULL || *b == NULL)
            return(-1);
         else
            return(strcmp((*a)->c_name, (*b)->c_name));
      }

   static dump_mem(p, len)
      char *p;
      int len;
      
      {
         FILE *fs;
         char *q;

         fs = fopen("/usr/tmp/MEMORY", "w");
         q = getenv("CONTENT_TYPE");
         /*fprintf(fs, "%s\n", q);*/
         
         fwrite(p, sizeof(char), (unsigned) len, fs);
         fclose(fs);
      }

   static int multipart_parse()
      {
         int state = 0, len = 0, i, j, tlen, k, fd;
         char buf[FB_MAXLINE], name[FB_MAXLINE];
         char obuf[FB_MAXLINE], bbuf[FB_MAXLINE], c, *p;
         fb_cgi_entry *e = NULL;

         fb_r_init(0);
         cgi_pairs = 0;
         ppos = 0;
         /* assume the first line is the boundary */
         if ((tlen = fb_nextline(buf, FB_MAXLINE)) <= 0)
            return(FB_ERROR);
         len += tlen;
         p = buf+tlen-2;
         if (*p == '\015' || *p == '\013')
            *p = NULL;
         if (!boundary_marker_match(buf))
            return(FB_ERROR);
         strcpy(boundary, buf);
         blen = strlen(boundary);

         while (len < content_length){
            /*
             * the second line must be the Content-Disposition
             */
            if ((tlen = fb_nextline(buf, FB_MAXLINE)) <= 0)
               break;
            len += tlen;
            if (len >= content_length)
               break;
            name[0] = NULL;
            pfilename[0] = NULL;
            load_disposition_info(name, pfilename, buf);

            /* printf("\nname: %s    filename: %s\n", name, pfilename); */

            /* third line is either a ^M or a Content-Type */
            if ((tlen = fb_nextline(buf, FB_MAXLINE)) <= 0)
               break;
            len += tlen;

            if (pfilename[0] != NULL){
               /* this must be the ^M line AFTER a Content-Type */
               if ((tlen = fb_nextline(buf, FB_MAXLINE)) <= 0)
                  break;
               len += tlen;

               /*
                * pfilename is the parse filename and it means that
                * a downloaded file is coming at us.
                *
                * create a temp filename in wfilename, create the file,
                * and open an fd for this
                */

               strcpy(wfilename, cgi_tmpdir);
               strcat(wfilename, "cdbXXXXXX");
               close(mkstemp(wfilename));
               fd = open(wfilename, 1);
               if (fd < 0)
                  return(FB_ERROR);
               fb_w_init(1, fd, -1);
               stashcount = 0;
               }

            /* now loop until a ^M ^J - is located */
            for (i = 0;;){
               if (fb_nextread(&c) == 0)
                  return(FB_ERROR);
               len++;
               if (len >= content_length)
                  break;
               if ((i == 0 && c == '\015') || (i == 1 && c == '\012'))
                  obuf[i++] = c;
               else if (i == 2 && c == '-'){
                  bbuf[0] = NULL;
                  for (j = 0; j < blen;){
                     bbuf[j++] = c;
                     if (fb_nextread(&c) == 0)
                        return(FB_ERROR);
                     len++;
                     if (j == 1 && c != '-'){
                        bbuf[j++] = c;
                        break;
                        }
                     }
                  bbuf[j] = NULL;
                  if ((j == blen || j == blen+2) &&
                        boundary_marker_match(bbuf)){
                     /* found a boundary marker -- seal up file or vars */
                     i = j = 0;
                     obuf[0] = obuf[1] = NULL;
                     bbuf[0] = NULL;
                     pword[ppos] = NULL;
                     /*
                      * if a magic marker, set the corresponding var
                      */
                     if (equal(name, "TEMPDIR")){
                        strcpy(cgi_tmpdir, pword);
                        fb_assure_slash(cgi_tmpdir);
                        }
                     else if (equal(name, "MAXSIZE")){
                        cgi_maxsize = atoi(pword);
                        if (cgi_maxsize < 0)
                           cgi_maxsize = 0;
                        }
                     e = make_cgi_entry();
                     fb_mkstr(&(e->c_name), name);
                     if (pfilename[0] != NULL){
                        fb_wflush(1);
                        fb_w_end(1);	/* this closes the fd */
                        fb_mkstr(&(e->c_val), wfilename);
                        }
                     else
                        fb_mkstr(&(e->c_val), pword);
                     /* link e into place */
                     if (fb_cgi_head == NULL)
                        fb_cgi_head = fb_cgi_tail = e;
                     else{
                        fb_cgi_tail->c_next = e;
                        fb_cgi_tail = e;
                        }
                     cgi_pairs++;
                     ppos = 0;
                     break;
                     }
                  else{
                     /* woops. must not be a boundary marker */
                     /* stash obuf[0] obuf[1] and bbuf[0...blen-1] */
                     stashchar(obuf[0]);
                     stashchar(obuf[1]);
                     for (k = 0; k < blen && bbuf[k]; k++){
                        stashchar(bbuf[k]);
                        }
                     i = 0;
                     obuf[0] = NULL;
                     obuf[1] = NULL;
                     }
                  }
               else{
                  /* if obuf was started above, flush it now */
                  if (i > 0){
                     for (k = 0; k < i; k++)
                        stashchar(obuf[k]);
                     }
                  i = 0;
                  if (i == 0 && c == '\015')
                     obuf[i++] = c;
                  else{
                     /* store or write the character somewhere */
                     stashchar(c);
                     }
                  }
               }

            /*
             * a marker was located. now read the "next line"
             * this is either a ^M or a --^M (end)
             */
            if (len < content_length){
               if ((tlen = fb_nextline(buf, FB_MAXLINE)) <= 0)
                  return(FB_ERROR);
               len += tlen;
               if (strncmp(buf, "--\015", 3) == 0)
                  break;
               }
            }
         return(FB_AOK);
      }

   static load_disposition_info(name, filename, line)
      char *name, *filename, *line;

      {
         int pos = 1;
         char token[FB_MAXLINE];

         *name = NULL;
         *filename = NULL;
         
         while (pos = fb_gettoken(line, pos, token, '_')){
            if (equal(token, "name")){
               pos += 1;	/* +2 for the = and the \", -1 for 1 based */
               for (; line[pos] != '\"'; pos++)
                  *name++ = line[pos];
               *name = NULL;
               }
            else if (equal(token, "filename")){
               pos += 1;	/* +2 for the = and the \", -1 for 1 based */
               for (; line[pos] != '\"'; pos++)
                  *filename++ = line[pos];
               *filename = NULL;
               }
            }
      }

   static stashchar(c)
      int c;

      {
         if (pfilename[0] == NULL){
            if (ppos < PWORD_SIZE)
               pword[ppos++] = c;
            }
         else if (cgi_maxsize == 0 || (++stashcount < cgi_maxsize))
            fb_w_write(0, &c);
      }

   static void cgi_sort()
      {
         fb_cgi_entry *c = NULL, **t, *pt, **t_entry;
         int i;

         /* if there are no pairs at all, set up empty list, return */
         if (cgi_pairs <= 0){
            fb_cgi_head = fb_cgi_tail = NULL;
            return;
            }

         /*
          * at this point, a link list of structures exists
          * from fb_cgi_head ... until c_next == NULL
          *
          * these elements need to be sorted so that checkbox
          * values can be grouped together for easy location
          */
         t_entry = (fb_cgi_entry **)
            fb_malloc(cgi_pairs * sizeof(fb_cgi_entry *));
         t = t_entry;
         c = fb_cgi_head;
         while (c != NULL){
            *t = c;
            c = c->c_next;
            t++;
            }
         /* sort the array t_entry */
         qsort((char *) t_entry,
            (unsigned) cgi_pairs, sizeof(fb_cgi_entry *), compare_entry);
         t = t_entry;
         /*
          * recreate the fb_cgi_head to fb_cgi_tail list from t_entry
          */
         pt = *t;
         fb_cgi_head = pt;
         t++;
         pt->c_prev = NULL;
         for (i = 1; i < cgi_pairs; i++, t++){
            pt->c_next = *t;
            (*t)->c_prev = pt;
            pt = *t;
            }
         pt->c_next = NULL;
         fb_cgi_tail = pt;
         fb_free((char *) t_entry);
      }

   static boundary_marker_match(s)
      char *s;

      {
         if (strncmp(boundary, s, blen) == 0)
            return(1);
         if (strncmp(boundary, s+1, blen) == 0)
            return(1);
         if (strncmp(boundary, s+2, blen) == 0)
            return(1);
         return(0);
      }
