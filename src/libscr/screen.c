/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: screen.c,v 9.3 2001/02/23 23:56:46 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Screen_sid[] = "@(#) $Id: screen.c,v 9.3 2001/02/23 23:56:46 john Exp $";
#endif

#include <fb.h>
#include <fb_ext.h>
#if HAVE_LIBNCURSES
#include <ncurses.h>
#include <term.h>
#endif /* HAVE_LIBNCURSES */

/*
 *  screen.c - system library for screen routines.
 *     handles it all...from termcap, to standard, to wimpy database
 *     of numbers representing terminal dependent screen sequences.
 */

/* 
 *  standard level three terminal display: ce=\Et:cl=\E*:cd=\Ey:cm=\E=%+ %+
 */

#define BELL 	"\007"
#define ESC 	"\033"              /* standard level 3 esc sequences */
#define CURMOV 	"\033="
#define CLREOL 	"\033t"
#define CLREOS 	"\033y"
#define CLRSCR 	"\033*"
#define OFFSET 	31

static char Bp[2048];
static char *S_TERM = "TERM";
#ifndef HAVE_LIBNCURSES
static char *S_CL = "cl";
static char *S_CD = "cd";
static char *S_CE = "ce";
static char *S_CM = "cm";
static char *S_VB = "vb";
static char *S_SO = "so";
static char *S_SE = "se";
static char *S_RV = "mr";
static char *S_RE = "me";
static char *S_LI = "li";
static char *S_CO = "co";
static char *S_DL = "dl";
static char *S_AL = "al";
static char *S_DC = "dc";
static char *S_IC = "ic";
static char *S_KU = "ku";
static char *S_KD = "kd";
static char *S_KL = "kl";
static char *S_KR = "kr";
static char *S_K0 = "k0";
static char *S_K1 = "k1";
static char *S_K2 = "k2";
static char *S_K3 = "k3";
static char *S_K4 = "k4";
static char *S_K5 = "k5";
static char *S_K6 = "k6";
static char *S_K7 = "k7";
static char *S_K8 = "k8";
static char *S_K9 = "k9";

#if FB_PROTOTYPES
static init_termcap(char *term);
#else /* FB_PROTOTYPES */
static init_termcap();
#endif /* FB_PROTOTYPES */

#endif /* not HAVE_LIBNCURSES */
#ifdef HAVE_LIBNCURSES
static char *S_CL = "clear";
static char *S_CD = "ed";
static char *S_CE = "el";
static char *S_CM = "cup";
static char *S_VB = "flash";
static char *S_SO = "smso";
static char *S_SE = "rmso";
static char *S_RV = "rev";
static char *S_RE = "sgr0";
static char *S_LI = "lines";
static char *S_CO = "cols";
static char *S_DL = "dl1";
static char *S_AL = "il1";
static char *S_DC = "dch1";
static char *S_IC = "ich1";
static char *S_KU = "kcuu1";
static char *S_KD = "kcud1";
static char *S_KL = "kcub1";
static char *S_KR = "kcuf1";
static char *S_K0 = "kf0";
static char *S_K1 = "kf1";
static char *S_K2 = "kf2";
static char *S_K3 = "kf3";
static char *S_K4 = "kf4";
static char *S_K5 = "kf5";
static char *S_K6 = "kf6";
static char *S_K7 = "kf7";
static char *S_K8 = "kf8";
static char *S_K9 = "kf9";

#if FB_PROTOTYPES
static int init_terminfo(char *);
static char *TSTR(char *);
static int TNUM(char *);
#else /* FB_PROTOTYPES */
static int init_terminfo();
static char *TSTR();
static int TNUM();
#endif /* FB_PROTOTYPES */

#endif /* HAVE_LIBNCURSES */

char cdb_CL[20]  = {NULL}, *cdb_cl_area = cdb_CL;	/* clear screen */
char cdb_CM[100] = {NULL}, *cdb_cm_area = cdb_CM;	/* cursor motion */
char cdb_CD[20]  = {NULL}, *cdb_cd_area = cdb_CD;	/* clear display */
char cdb_CE[20]  = {NULL}, *cdb_ce_area = cdb_CE;	/* clear to eol */
char cdb_VB[100] = {NULL}, *cdb_vb_area = cdb_VB;	/* visible bell */
char cdb_SO[20]  = {NULL}, *cdb_so_area = cdb_SO;	/* standout begin */
char cdb_SE[20]  = {NULL}, *cdb_se_area = cdb_SE;	/* standout end */
char cdb_RV[20]  = {NULL}, *cdb_rv_area = cdb_RV;	/* reverse begin */
char cdb_RE[20]  = {NULL}, *cdb_re_area = cdb_RE;	/* reverse end */
char cdb_DL[20]  = {NULL}, *cdb_dl_area = cdb_DL;	/* delete line */
char cdb_AL[20]  = {NULL}, *cdb_al_area = cdb_AL;	/* add (insert) line */
char cdb_DC[20]  = {NULL}, *cdb_dc_area = cdb_DC;	/* delete character */
char cdb_IC[20]  = {NULL}, *cdb_ic_area = cdb_IC;	/* insert character */
char cdb_KU[20]  = {NULL}, *cdb_ku_area = cdb_KU;	/* key up */
char cdb_KD[20]  = {NULL}, *cdb_kd_area = cdb_KD;	/* key down */
char cdb_KL[20]  = {NULL}, *cdb_kl_area = cdb_KL;	/* key left */
char cdb_KR[20]  = {NULL}, *cdb_kr_area = cdb_KR;	/* key right */
char cdb_K0[20]  = {NULL}, *cdb_k0_area = cdb_K0;	/* function k0-k9 */
char cdb_K1[20]  = {NULL}, *cdb_k1_area = cdb_K1;
char cdb_K2[20]  = {NULL}, *cdb_k2_area = cdb_K2;
char cdb_K3[20]  = {NULL}, *cdb_k3_area = cdb_K3;
char cdb_K4[20]  = {NULL}, *cdb_k4_area = cdb_K4;
char cdb_K5[20]  = {NULL}, *cdb_k5_area = cdb_K5;
char cdb_K6[20]  = {NULL}, *cdb_k6_area = cdb_K6;
char cdb_K7[20]  = {NULL}, *cdb_k7_area = cdb_K7;
char cdb_K8[20]  = {NULL}, *cdb_k8_area = cdb_K8;
char cdb_K9[20]  = {NULL}, *cdb_k9_area = cdb_K9;

fb_pscreen cdb_curscr, cdb_newscr;	/* the screens */

static int motion = 0;			/* set if cm is termcapable */
extern short int cdb_initscreen_done;
extern short int cdb_standout;
extern short int cdb_reverse;

/* 
 *  initscreen - initialize screen.
 */
 
   void fb_initscreen()
      {

	 char *term;

         cdb_initscreen_done = 1;
         if ((term = getenv(S_TERM)) == 0)
	    fb_xerror(FB_MESSAGE, "Environment variable TERM not set", NIL);
         cdb_CL[0] = 0;
         cdb_CM[0] = 0;
         cdb_CD[0] = 0;
         cdb_CE[0] = 0;
         cdb_VB[0] = 0;
         cdb_SO[0] = 0;
         cdb_SE[0] = 0;
         cdb_RV[0] = 0;
         cdb_RE[0] = 0;
         cdb_DL[0] = 0;
         cdb_AL[0] = 0;
         cdb_DC[0] = 0;
         cdb_IC[0] = 0;
         cdb_KU[0] = 0;
         cdb_KD[0] = 0;
         cdb_KL[0] = 0;
         cdb_KR[0] = 0;
         cdb_K0[0] = 0;
         cdb_K1[0] = 0;
         cdb_K2[0] = 0;
         cdb_K3[0] = 0;
         cdb_K4[0] = 0;
         cdb_K5[0] = 0;
         cdb_K6[0] = 0;
         cdb_K7[0] = 0;
         cdb_K8[0] = 0;
         cdb_K9[0] = 0;
#ifdef HAVE_LIBNCURSES
         init_terminfo(term);
#else
         init_termcap(term);
#endif
         fb_initopt();
         /*
          * all cdb_t_cols and cdb_t_lines re-calcs are done in winsize
          */
         fb_winsize();
      }

#ifndef HAVE_LIBNCURSES
   static init_termcap(term)
      char *term;

      {
         char *tgetstr(char *id, char **area);

         if (tgetent(Bp, term) != 1)
            fb_xerror(FB_MESSAGE, SYSMSG[S_BAD_TERMCAP], NIL);
         if (tgetstr(S_CL, &cdb_cl_area) == (char *) NULL)
            fb_xerror(FB_MESSAGE, SYSMSG[S_BAD_TERMCAP], S_CL);
         if (tgetstr(S_CD, &cdb_cd_area) == (char *) NULL)
            fb_xerror(FB_MESSAGE, SYSMSG[S_BAD_TERMCAP], S_CD);
         if (tgetstr(S_CE, &cdb_ce_area) == (char *) NULL)
            fb_xerror(FB_MESSAGE, SYSMSG[S_BAD_TERMCAP], S_CE);
         if (tgetstr(S_CM, &cdb_cm_area) == (char *) NULL)
            fb_xerror(FB_MESSAGE, SYSMSG[S_BAD_TERMCAP], S_CM);
         cdb_t_lines = tgetnum(S_LI);
         cdb_t_cols = tgetnum(S_CO);
         motion = 1;
         if (tgetstr(S_VB, &cdb_vb_area) == NULL)
            strcpy(cdb_VB, BELL);
         if (tgetstr(S_DL, &cdb_dl_area) == NULL)
            cdb_DL[0] = NULL;
         if (tgetstr(S_AL, &cdb_al_area) == NULL)
            cdb_AL[0] = NULL;
         if (tgetstr(S_DC, &cdb_dc_area) == NULL)
            cdb_DC[0] = NULL;
         if (tgetstr(S_IC, &cdb_ic_area) == NULL)
            cdb_IC[0] = NULL;
         if (tgetstr(S_KU, &cdb_ku_area) == NULL)
            cdb_KU[0] = NULL;
         if (tgetstr(S_KD, &cdb_kd_area) == NULL)
            cdb_KD[0] = NULL;
         if (tgetstr(S_KL, &cdb_kl_area) == NULL)
            cdb_KL[0] = NULL;
         if (tgetstr(S_KR, &cdb_kr_area) == NULL)
            cdb_KR[0] = NULL;
         if (tgetstr(S_K0, &cdb_k0_area) == NULL)
            cdb_K0[0] = NULL;
         if (tgetstr(S_K1, &cdb_k1_area) == NULL)
            cdb_K1[0] = NULL;
         if (tgetstr(S_K2, &cdb_k2_area) == NULL)
            cdb_K2[0] = NULL;
         if (tgetstr(S_K3, &cdb_k3_area) == NULL)
            cdb_K3[0] = NULL;
         if (tgetstr(S_K4, &cdb_k4_area) == NULL)
            cdb_K4[0] = NULL;
         if (tgetstr(S_K5, &cdb_k5_area) == NULL)
            cdb_K5[0] = NULL;
         if (tgetstr(S_K6, &cdb_k6_area) == NULL)
            cdb_K6[0] = NULL;
         if (tgetstr(S_K7, &cdb_k7_area) == NULL)
            cdb_K7[0] = NULL;
         if (tgetstr(S_K8, &cdb_k8_area) == NULL)
            cdb_K8[0] = NULL;
         if (tgetstr(S_K9, &cdb_k9_area) == NULL)
            cdb_K9[0] = NULL;
         if (cdb_standout){
            if ((tgetstr(S_SO, &cdb_so_area) == NULL) ||
                (tgetstr(S_SE, &cdb_se_area) == NULL))
               *cdb_SO = *cdb_SE = NULL;
            if ((tgetstr(S_RV, &cdb_rv_area) == NULL) ||
                (tgetstr(S_RE, &cdb_re_area) == NULL)){
               strcpy(cdb_RV, cdb_SO);
               strcpy(cdb_RE, cdb_SE);
               }
            else if (!cdb_reverse){
               strcpy(cdb_RV, cdb_SO);
               strcpy(cdb_RE, cdb_SE);
               }
            }
   }
#endif /* not HAVE_LIBNCURSES */

#ifdef HAVE_LIBNCURSES
   static int init_terminfo(term)
      char *term;

      {
         char *TSTR();
         int errret;

         setupterm(term, 1, &errret);
         if (errret == 0)
            fb_xerror(FB_MESSAGE, "Cannot init terminfo setting:", term);
         else if (errret == -1)
            fb_xerror(FB_MESSAGE, "Cannot find terminfo database.", NIL);
         strcpy(cdb_cl_area, TSTR(S_CL));
         strcpy(cdb_cd_area, TSTR(S_CD));
         strcpy(cdb_ce_area, TSTR(S_CE));
         strcpy(cdb_cm_area, TSTR(S_CM));
         cdb_t_lines = TNUM(S_LI);
         cdb_t_cols = TNUM(S_CO);
         motion = 1;
         strcpy(cdb_vb_area, TSTR(S_VB));
         if (cdb_VB[0] == NULL)
            strcpy(cdb_VB, BELL);
         strcpy(cdb_dl_area, TSTR(S_DL));
         strcpy(cdb_al_area, TSTR(S_AL));
         strcpy(cdb_dc_area, TSTR(S_DC));
         strcpy(cdb_ic_area, TSTR(S_IC));
         strcpy(cdb_ku_area, TSTR(S_KU));

         strcpy(cdb_kd_area, TSTR(S_KD));
         strcpy(cdb_kl_area, TSTR(S_KL));
         strcpy(cdb_kr_area, TSTR(S_KR));
         strcpy(cdb_k0_area, TSTR(S_K0));
         strcpy(cdb_k1_area, TSTR(S_K1));
         strcpy(cdb_k2_area, TSTR(S_K2));
         strcpy(cdb_k3_area, TSTR(S_K3));
         strcpy(cdb_k4_area, TSTR(S_K4));
         strcpy(cdb_k5_area, TSTR(S_K5));
         strcpy(cdb_k6_area, TSTR(S_K6));
         strcpy(cdb_k7_area, TSTR(S_K7));
         strcpy(cdb_k8_area, TSTR(S_K8));
         strcpy(cdb_k9_area, TSTR(S_K9));

         if (cdb_standout){
            strcpy(cdb_so_area, TSTR(S_SO));
            strcpy(cdb_se_area, TSTR(S_SE));
            strcpy(cdb_rv_area, TSTR(S_RV));
            strcpy(cdb_re_area, TSTR(S_RE));
            if (*cdb_rv_area == 0 && *cdb_re_area == 0){
               strcpy(cdb_RV, cdb_SO);
               strcpy(cdb_RE, cdb_SE);
               }
            else if (!cdb_reverse){
               strcpy(cdb_RV, cdb_SO);
               strcpy(cdb_RE, cdb_SE);
               }
            }
   }

   static char *TSTR(codename)
      char *codename;

      {
         char *p = 0;

         p = tigetstr(codename);
         if (p <= 0)
            p = NIL;
         return(p);
      }

   static int TNUM(codename)
      char *codename;

      {
         int val = 0;

         val = tigetnum(codename);
         return(val);
      }

#endif /* HAVE_LIBNCURSES */
