/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: dbwstat.c,v 9.4 2002/12/29 17:14:30 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Dbwstat_sid[] = "@(#) $Id: dbwstat.c,v 9.4 2002/12/29 17:14:30 john Exp $";
#endif

/*  
 *  dbwstat.c - read a WN log file and generate a database for stats gathering
 */

#include <fb.h>
#include <fb_vars.h>

#define BUFSIZE 2000
#define HOSTNAME 	0
#define DATE 	1
#define HOUR 	2
#define MINUTE 	3
#define SECOND 	4
#define URL 	5
#define BYTES 	6
#define AGENT 	7
#define REFERRER 8
#define IP 	9
#define AUTH 	10
#define EXT 	11
#define CODE 	12

#define HOSTNAME_SIZE 60
#define URL_SIZE 200
#define AGENT_SIZE 70
#define REFERRER_SIZE 70
#define EXT_SIZE 10
#define IP_SIZE 40

char host[BUFSIZE];
char date[BUFSIZE];
char hour[BUFSIZE];
char minute[BUFSIZE];
char second[BUFSIZE];
char url[BUFSIZE];
char t_url[BUFSIZE];
char ext[BUFSIZE];
char bytes[BUFSIZE];
char agent[BUFSIZE];
char referrer[BUFSIZE];
char ip[BUFSIZE];
char auth[BUFSIZE];
char code[BUFSIZE];
long rec = 0;
long line_no = 0;

fb_database *db;
extern short int cdb_lockdaemon;
extern short int cdb_locklevel;

int main(int, char **);
static int begin(int, char **);
static void end(void);
static int process(char *);
static void convert(char *);
static int deposit(void);
static int filter(char *);
static int forceascii(char *);

/*
 *  dbwstat - main driver
 */
 
   main(argc, argv)
      int argc;
      char *argv[];

      {
         char buf[BUFSIZE+1];
         int st;
         int lines = 0;

         st = begin(argc, argv);
         if (st == FB_AOK){
            while(fb_nextline(buf, BUFSIZE)){
               lines++;
               line_no++;
               /*
               if (lines % 10 == 0)
                  fprintf(stderr, "%d\n", lines);
               */
               if (process(buf))
                  if (deposit() != FB_AOK)
                     fprintf(stderr, "error adding a record\n");
               }
            end();
            }
         fb_ender();
      }

   static int begin(argc, argv)
      int argc;
      char *argv[];

      {
         cdb_batchmode = 1;
         fb_getargs(argc, argv, FB_NODB);
         cdb_lockdaemon = 0;
         cdb_locklevel = 0;
	 fb_allow_int();		/* allows instant piping */	
         fb_r_init(0);
         db = fb_dballoc();
         fb_dbargs("wstat", NIL, db);
         if (fb_opendb(db, READWRITE, 0, 0) != FB_AOK){
            fprintf(stderr, "can't open wstat database\n");
            return(FB_ERROR);
            }
         fb_bulkrec_begin(db, FB_WAIT);
         return(FB_AOK);
      }

   static void end()
      {
         fb_r_end();
         fb_bulkrec_end(db);
         fb_closedb(db);
      }

   static int process(buf)
      char *buf;

      {
         char *q, *p, mon[5], day[5], year[5], w3[100], w4[100];
         char w5[100], t_buf[BUFSIZE+1];
         int depth = 0;

         /*
          * parse the buffer line for
          *   host
          *   day
          *   hour
          *   minute
          *   url
          *   bytes
          *   agent
          *   referrer
          *   ip
          */

#if DEBUG
printf("---------------------\n");
puts(buf);
printf("---------------------\n");
#endif

         host[0] = NULL;
         date[0] = NULL;
         hour[0] = NULL;
         minute[0] = NULL;
         second[0] = NULL;
         url[0] = NULL;
         t_url[0] = NULL;
         ext[0] = NULL;
         bytes[0] = NULL;
         agent[0] = NULL;
         referrer[0] = NULL;
         ip[0] = NULL;
         auth[0] = NULL;
         code[0] = NULL;

         /* get host */
         fb_subline(host, buf, 1, FB_BLANK);
         host[HOSTNAME_SIZE - 1] = NULL;

         if (filter(host))
            return(0);

         fb_subline(auth, buf, 3, FB_BLANK);
         if (equal(auth, "-"))
            auth[0] = NULL;

         /* get date hour minute [14/Nov/1995:07:08:06 -0700] */
         p = buf;
         buf = strchr(p+1, '\[');
         if (buf == 0)
            return(0);
         buf++;
         day[0] = *buf++;
         day[1] = *buf++;
         day[2] = NULL;
         buf++;	/* skip slash */
         mon[0] = *buf++;
         mon[1] = *buf++;
         mon[2] = *buf++;
         mon[3] = NULL;
         buf += 3;	/* skip slash and century */
         year[0] = *buf++;
         year[1] = *buf++;
         year[2] = NULL;
         convert(mon);
         sprintf(date, "%s%s%s", mon, day, year);
         buf += 1;	/* skip colon */
         hour[0] = *buf++;
         hour[1] = *buf++;
         buf += 1;	/* skip colon */
         minute[0] = *buf++;
         minute[1] = *buf++;
         buf += 1;	/* skip colon */
         second[0] = *buf++;
         second[1] = *buf++;
         hour[2] = minute[2] = second[2] = NULL;

         /* skip over quoted section of GET or POST command */
         
         p = strchr(buf, '\"');
         if (p == 0)
            return(0);
         buf = p + 1;		/* "_GET /blah.htm" */

         t_url[0] = NULL;	/* try and get t_url for 304s */
         p = strchr(buf, ' ');
         if (p > 0){
            strcpy(t_buf, p+1);
            p = strchr(t_buf, ' ');
            if (p > 0){
               *p = NULL;
               strcpy(t_url, t_buf);
               }
            }

         p = strchr(buf, '\"');
         if (p == 0)
            return(0);

         buf = p + 2;	/* skip blank after quote, also */
         fb_subline(code, buf, 1, FB_BLANK);
         fb_subline(bytes, buf, 2, FB_BLANK);
         fb_subline(w3, buf, 3, FB_BLANK);
         fb_subline(w4, buf, 4, FB_BLANK);
         fb_subline(w5, buf, 5, FB_BLANK);
         /* filter out some fluff right now */
         if (!equal(code, "200") && !equal(code, "304")
               && !equal(code, "301"))
            return(0);
         if (!equal(w4, "Sent"))
	    return(0);
         if (!equal(w5, "file:") && !equal(w5, "CGI") &&
               !equal(w5, "304") &&
               !equal(w5, "redirection:"))
            return(0);

         p = strchr(buf, ':');
         if (p == 0)
            return(0);
         buf = p + 2; /* get past colon and blank */
         p = strchr(buf, '>');
         if (p == 0)
            return(0);
         *p = NULL;
         strcpy(url, buf);

         /*
          * with the advent of ads, we need to count everything.
          * so, do not filter non htm|html|cgi files here.
          * q = strrchr(url, '.');
          * if (q == 0)
          *   return(0);
          * q++;
          */

         /*
          * if (!equal(q, "htm") && !equal(q, "html") && !equal(q, "cgi"))
          *  return(0);
          */

         if (equal(code, "304")){
            strcpy(url, t_url);
            if (t_url[strlen(t_url) - 1] == '/'){
               strcat(url, "index.htm");
               }
            }

         url[URL_SIZE - 1] = NULL;

         for (p++; *p != '<'; p++)
            ;
         /* get user agent */
         buf = p + 1;
         for (depth = 1, p = buf; *p; p++){
            if (*p == '<')
               depth++;
            if (*p == '>')
               depth--;
            if (*p == '>' && depth == 0)
               break;
            }
         if (p == 0)
            return(0);
         *p = NULL;
         strcpy(agent, buf);
         agent[AGENT_SIZE - 1] = NULL;

         /* get referrer */
         buf = p + 3;
         p = strchr(buf, '>');
         if (p == 0)
            return(0);
         *p = NULL;
         strcpy(referrer, buf);
         if (referrer[strlen(referrer) - 1] == '/'){
            strcat(referrer, "index.htm");
            }
         referrer[REFERRER_SIZE - 1] = NULL;

         /* get ip  --- alg changes here ... look for next < to skip cookie */
         buf = p + 3;
         p = strchr(buf, '<');
         if (p == 0)
            return(0);

         q = p;
         buf = p + 1;
         p = strchr(buf, '<');
         if (p == 0)
            p = buf = q;

         buf = p + 1;
         p = strchr(buf, '>');
         if (p == 0)
            return(0);
         *p = NULL;
         strcpy(ip, buf);
         ip[IP_SIZE - 1] = NULL;

         ext[0] = NULL;
         /* if this is not a redirection, get the extension being served */
         if (!equal(code, "301")){
            p = strrchr(url, '.');
            if (p > 0){
               strcpy(ext, p+1);
               p = strrchr(ext, ';');
               if (p > 0)
                  *p = NULL;
               p = strrchr(ext, '?');
               if (p > 0)
                  *p = NULL;
               if (equal(ext, "html"))
                  strcpy(ext, "htm");
               for (p = ext; *p; p++)
                  *p = toupper(*p);
               ext[EXT_SIZE] = NULL;
               }
            }
         else
            strcpy(ext, code);

         if (!equal(ext, "M") && !equal(ext, "CGI")){
            strcpy(url, t_url);
            if (t_url[strlen(t_url) - 1] == '/'){
               strcat(url, "index.htm");
               }
            }
         return(1);
      }

   static void convert(mon)
      char *mon;

      {
         if (equal(mon, "Jan"))
            strcpy(mon, "01");
         else if (equal(mon, "Feb"))
            strcpy(mon, "02");
         else if (equal(mon, "Mar"))
            strcpy(mon, "03");
         else if (equal(mon, "Apr"))
            strcpy(mon, "04");
         else if (equal(mon, "May"))
            strcpy(mon, "05");
         else if (equal(mon, "Jun"))
            strcpy(mon, "06");
         else if (equal(mon, "Jul"))
            strcpy(mon, "07");
         else if (equal(mon, "Aug"))
            strcpy(mon, "08");
         else if (equal(mon, "Sep"))
            strcpy(mon, "09");
         else if (equal(mon, "Oct"))
            strcpy(mon, "10");
         else if (equal(mon, "Nov"))
            strcpy(mon, "11");
         else if (equal(mon, "Dec"))
            strcpy(mon, "12");
      }

   static int deposit()
      {
         char nbuf[FB_MAXLINE];

         /* bump record --- show only */
         rec++;
         forceascii(agent);
         forceascii(referrer);
         forceascii(host);

#if DEBUG
printf("<%ld> <%s> <%s> <%s> <%s> <%s> <%s> <%s> <%s> <%s> <%s>\n",
   line_no,
   host, url, agent, referrer, ip, date, hour, minute, ext, code);
return(FB_AOK);
#endif

#if !DEBUG
         fb_store(db->kp[HOSTNAME], 	host, db);
         fb_store(db->kp[DATE], 	date, db);
         fb_store(db->kp[HOUR], 	hour, db);
         fb_store(db->kp[MINUTE],	minute, db);
         fb_store(db->kp[SECOND],	second, db);
         sprintf(nbuf, "%s:%s", ip, url);
         fb_store(db->kp[URL], 		nbuf, db);
         fb_store(db->kp[BYTES], 	bytes, db);
         fb_store(db->kp[AGENT], 	agent, db);
         fb_store(db->kp[REFERRER],	referrer, db);
         fb_store(db->kp[IP], 		ip, db);
         fb_store(db->kp[AUTH], 	auth, db);
         fb_store(db->kp[EXT], 		ext, db);
         fb_store(db->kp[CODE], 	code, db);
         return(fb_b_addrec(db));
#endif
      }

   static int filter(host)
      char *host;

      {
         char *p;

         p = strchr(host, '.');
         if (p == 0)
            return(1);
         else
            p++;
         if (equal(p, "firstbase.com"))
            return(1);
         return(0);
      }

/*
 * forceascii - force a charcter string to be 10, 13, or 32-126
 *    (as seen on a decimal ascii chart)
 */

   static int forceascii(p)
      char *p;

      {
         for (; *p; p++){
            if (*p == 10 || *p == 13 || (*p >= 32 && *p <= 126))
               ;
            else
               *p = '0';
            }
      }
