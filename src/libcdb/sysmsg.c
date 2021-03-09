/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: sysmsg.c,v 9.0 2001/01/09 02:56:30 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Sysmsg_sid[] = "@(#) $Id: sysmsg.c,v 9.0 2001/01/09 02:56:30 john Exp $";
#endif

/* global strings - can be removed from the data (code) section */

/* don't be foolish. the order here is mandatory */

char *SYSMSG[] ={
   "not found",                                         /* 0 */
   "%s",                                                /* 1 */
   "end of file",                                       /* 2 */
   "%c",                                                /* 3 */
   "<Space>=next page, Any other key=quit: ",           /* 4 */
   "r",                                                 /* 5 */
   "w",                                                 /* 6 */
   "%c%c/%c%c/%c%c",                                    /* 7 */
   "illegal command/reference: ",                       /* 8 */
   "help",                                              /* 9 */
   "(display@%d/%d)",                                   /* 10 */
   "Permission denied",                                 /* 11 */
   "I/O Error:",                                        /* 12 */
   "bad dictionary",                                    /* 13 */
   "bad index file",                                    /* 14 */
   "bad data",                                          /* 15 */
   "bad map file",                                      /* 16 */
   "bad formula",                                       /* 17 */
   "bad header",                                        /* 18 */
   "input error:",                                      /* 19 */
   "invalid entry",                                     /* 20 */
   "too long",                                          /* 21 */
   "bad format",                                        /* 22 */
   "no help",                                           /* 23 */
   "Fatal Error",                                       /* 24 */
   "termcap definition is missing",                     /* 25 */
   " <>", 	                                        /* 26 */
   "Abort.",                                            /* 27 */
   "task not done",                                     /* 28 */
   "** HIT ANY KEY TO CONTINUE **",                     /* 29 */
   "Command Level",                                     /* 30 */
   "Record Level",                                      /* 31 */
   "Field Level",                                       /* 32 */
   "<CTL>-H=Help, -=End",				/* 33 */
   "Working...",                                        /* 34 */
   "",  /* was S_SEARCH == Search */                    /* 35 */
   "Add Mode",                                          /* 36 */
   "Not Created",                                       /* 37 */
   "DELETED",                                           /* 38 */
   "Do You Want To RESTORE This Record (y,<cr>) ? ",    /* 39 */
   "Not Restored",                                      /* 40 */
   "Record",                                            /* 41 */
   "Field # (<CTL>-H=Help, -=End):",                    /* 42 */
   "Not Updated",                                       /* 43 */
   "Field",                                             /* 44 */
   "Creating",                                          /* 45 */
   "Writing",                                           /* 46 */
   "%d",                                                /* 47 */
   "@%05ld:%s",                                         /* 48 */
   "%s:%.10s",                                          /* 49 */
   "NEW",                                               /* 50 */
   "*%05ld",                                            /* 51 */
   "@%05ld",                                            /* 52 */
   "del",                                               /* 53 */
   "put",                                               /* 54 */
   "locked",                                            /* 55 */
   "",		/* was FLD_MSG - now in line */	        /* 56 */
   "That Entry Is in Use. UNIQUE entry Required!",      /* 57 */
   "%3d> %s",                                           /* 58 */
   " %s",                                               /* 59 */
   "%s ",                                               /* 60 */
   "<%c%c%6d>",                                         /* 61 */
   "Select Choice",                                     /* 62 */
   "Enter choice: ",                                    /* 63 */
   "Available Choices for",                             /* 64 */
   "Choice",                                            /* 65 */
   "Meaning",                                           /* 66 */
   "Comment",                                           /* 67 */
   "Are You Sure You Want To Delete This Record (y/<cr>) ? ", /* 68 */
   "nonnumeric item",                                   /* 69 */
   "Enter Positive OR Negative Number: ",               /* 70 */
   "%49ld",                                             /* 71 */
   "%49.2f",                                            /* 72 */
   " ", /* string blank */                              /* 73 */
   "OK? (<cr>=yes,any_other=n) ",                       /* 74 */
   "FieldCalc Level",                                   /* 75 */
   "Firstbase NOT multi-user!",                         /* 76 */
   "%slock/",                                           /* 77 */
   "%s %s",                                             /* 78 */
   "/", /* string slash */                              /* 79 */
   "%s.lck%ld",                                         /* 80 */
   "%s.%s",                                             /* 81 */
   "LOCKED Record: Retry? (n=no, <other>=yes)? ",       /* 82 */
   "Retrying...",                                       /* 83 */
   "AutoDefault Mode",                                  /* 84 */
   "Security Check",                                    /* 85 */
   "Password: ",                                        /* 86 */
   "Sorry.",                                            /* 87 */
   ".cdb",                                              /* 88 */
   ".ddict",                                            /* 89 */
   ".map",                                              /* 90 */
   ".idx",                                              /* 92 */
   ".idict",                                            /* 91 */
   "%s%s",                                              /* 93 */
   "%04d",                                              /* 94 */
   "Searching...",                                      /* 95 */
   "Restored",                                          /* 96 */
   "Any Change? (y=Yes, <return>=no) ?",                /* 97 */
   "ON",                                                /* 98 */
   "[None]",                                            /* 99 */
   "\n",  /* string newline */                          /* 100 */
   ".idicti",                                           /* 101 */
   "%s/%s",                                             /* 102 */
   "Auto Regeneration...",                              /* 103 */
   "%*s",                                               /* 104 */
   "y", /* string y */                                  /* 105 */
   "n", /* string n */                                  /* 106 */
   "?", /* string ques */                               /* 107 */
   "out of memory",                                     /* 108 */
   "execution error",                                   /* 109 */
   "%s%s%s",                                            /* 110 */
   "%s%ld",                                             /* 111 */
   "dirty database - use dbrestor to fix.",		/* 112 */
   ".log",						/* 113 */
   "bad log file",                                      /* 114 */
   0
   };
