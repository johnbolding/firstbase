/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: alarm.c,v 9.0 2001/01/09 02:56:23 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Alarm_sid[] = "@(#) $Id: alarm.c,v 9.0 2001/01/09 02:56:23 john Exp $";
#endif

#include <fb.h>

#if HAVE_FCNTL && HAVE_GETITIMER

#include <fb_ext.h>
#include <signal.h>
#include <sys/time.h>

/*
 * firstbase alarm mechanism - does not disturb pending alarms
 */

static struct itimerval t_val, ot_val;
static long orig_it_sec, orig_it_usec;
static RETSIGTYPE (*ohandler)(int disp) = NULL;
static long alarm_locktime;

   fb_set_alarm(locktime, nhandler)
      long locktime;
      void (*nhandler)(int disp);

      {
         /*
          * if the outside layer has a timer running that is less than
          * the one about to be set, don't set anything here, and assume
          * that there is an outside signal handler that will take over!
          */
         alarm_locktime = locktime;
         getitimer(ITIMER_REAL, &ot_val);
         ohandler = NULL;
         orig_it_sec = 0L;
         orig_it_usec = 0L;
         if (ot_val.it_value.tv_sec == 0 || locktime < ot_val.it_value.tv_sec){
            /* save outside signal handler - set inside signal handler */
            ohandler = signal(SIGALRM, nhandler);
            /*
             * set up the t_val structure -- allow outside layer
             * of timer to run concurrently as this timer by setting
             * the interval portion which reloads if timer expires.
             */
            t_val.it_interval.tv_sec = 0L;
            t_val.it_interval.tv_usec = 0L;
            if (ot_val.it_value.tv_sec > 0){
               t_val.it_interval.tv_sec =
                  ot_val.it_value.tv_sec - locktime;
               orig_it_sec = ot_val.it_value.tv_sec;
               orig_it_usec = ot_val.it_value.tv_usec;
               }
            t_val.it_interval.tv_usec = ot_val.it_value.tv_usec;
            t_val.it_value.tv_sec = locktime;
            t_val.it_value.tv_usec = 0;
            setitimer(ITIMER_REAL, &t_val, NULL);
            }
      }

   fb_jump_alarm()
      {
         signal(SIGALRM, ohandler);
      }

   fb_restore_alarm()
      {
         int set_timer = 1;

         if (ohandler != NULL){
            getitimer(ITIMER_REAL, &t_val);
            if (orig_it_sec > 0)
               orig_it_sec -= (alarm_locktime - t_val.it_value.tv_sec);
            ot_val.it_value.tv_sec = orig_it_sec;
            ot_val.it_value.tv_usec = orig_it_usec;
            signal(SIGALRM, ohandler);
            }
         else if (ot_val.it_value.tv_sec > 0){
            /*
             * in this case, the ot_val value must have been
             * less than the locktime. need to skip the setitimer below.
             */
            set_timer = 0;
            }
         else{
            ot_val.it_value.tv_sec = 0L;
            ot_val.it_value.tv_usec = 0L;
            ot_val.it_interval.tv_usec = 0L;
            ot_val.it_interval.tv_usec = 0L;
            }
         if (set_timer)
            setitimer(ITIMER_REAL, &ot_val, NULL);
      }

#endif /* HAVE_FCNTL && HAVE_GETITIMER */
