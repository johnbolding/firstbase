/*
* Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
*
* $Id: fb.h,v 9.2 2006/04/03 17:28:59 john Exp $
*
* See the file LICENSE for conditions of use and distribution.
*
*/

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#if HAVE_UNISTD_H
#include <sys/types.h>
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

/* simple macros */
#define FB_EDITMODE	1
#define FB_ENDMODE	0
#define FB_WITHROOT	1
#define FB_NOROOT	0
#define FB_FDIGITS	(SYSMSG[S_FORMAT_DIGITS])
#define FB_FSTRING	(SYSMSG[S_FORMAT_STRING])
#define FB_FCHAR	(SYSMSG[S_FORMAT_CHAR])
#define FB_F_READ	(SYSMSG[S_F_READ])
#define FB_F_WRITE	(SYSMSG[S_F_WRITE])
#define FB_FDATE	(SYSMSG[S_FDATE])
#define FB_BLANK	CHAR_BLANK
#define FB_NEWLINE	CHAR_NEWLINE
#define FB_HELP 	CHAR_QUESTION
#define FB_INTO 	CHAR_G_THAN
#define FB_OUTOF	CHAR_L_THAN
#define FB_DEREF	CHAR_AT
#define FB_DOT		CHAR_DOT
#define FB_UNDERSCORE 	CHAR_UNDERSCORE

/* firstbase macros */
#define FB_CHECKERROR(x) if ((x) == FB_ERROR) return(FB_ERROR);
#define FB_FLD(f,db)	((db)->kp[(f)]->fld)
#define FB_TYP(t,db)	((db)->kp[(t)]->type)
#define FB_ISDELETED(db) (db->kp[db->nfields]->fld[0] == '*')
#define FB_RECMOD(db)	((db->kp[db->nfields]->fld) + 9)
#define FB_RECUMOD(db)	(db->kp[db->nfields]->fld[9])
#define FB_RECGMOD(db)	(db->kp[db->nfields]->fld[10])
#define FB_RECOMOD(db)	(db->kp[db->nfields]->fld[11])

#define FB_OFALPHA(t)	((t == FB_ALPHA) || (t == FB_STRICTALPHA) || \
			   (t == FB_UPPERCASE))
#define FB_OFNUMERIC(t)	((t == FB_DOLLARS) || (t == FB_FLOAT) || \
			   (t == FB_NUMERIC) || (t == FB_POS_NUM))
#define FB_SLONG	(sizeof(long))
#define FB_MAPREC_SIZE 	(sizeof(long) * 2)

/* numeric constants */
#define FB_CN_LEN 	25		/* company name length */
#define FB_HEADELEM 	10		/* single header element size */

#define FB_HEADSTART	4L		/* position of 1st headelem */
#define FB_LONGFIELD 	300		/* longest field editable by FB */
#define FB_MAXBY 	20		/* maximum sortby fields */
#define FB_MAXIDXS 	FB_MAXBY	/* maximum number fields/index */
#define FB_MAXKEEP 	1000		/* max num fields in pgen/lgen */
#define FB_MAXLINE 	250		/* general mac line length */
#define FB_MAXNAME 	240		/* maximum PATHNAME length */
#define FB_MODESTART	24L		/* start of uid gid mode for record */
#define FB_NUMLENGTH 	25		/* width of numeric typed fields */
#define FB_PRINTLINE	501
#define FB_PSIZE 	10		/* password size */
#define FB_RECORDPTR	10		/* length of record pointer */
#define FB_SCREENFIELD 	50		/* screen window: normal fields */
#define FB_SEQSIZE 	4		/* sequence id number size */
#define FB_SEQSTART	0L		/* position of sequence id */
#define FB_TITLESIZE 	10		/* max size of fld title (id) */
#define FB_WAIT 	1		/* wait condition for lock */
#define FB_NOWAIT 	0		/* nowait condition for lock */
#define FB_AMERICAN	1		/* a date type */
#define FB_EUROPEAN	2		/* a date type */
#define FB_FADD 	16		/* just different than !@<> etc */

typedef struct fb_s_link fb_link;	/* defined link */
typedef struct fb_s_field fb_field;	/* defined field */
typedef struct fb_s_database fb_database;/* defined database */
typedef struct fb_s_bidx fb_bidx;	/* btree+ index set */
typedef struct fb_s_bseq fb_bseq;	/* btree+ sequence set */
typedef struct fb_s_ixauto fb_autoindex;/* autoindex structure */

struct fb_s_bidx {			/* btree+ index set node */
   long bi_left;			/* left side info */
   char bi_ltype;
   long bi_middle;			/* middle side info */
   char bi_mtype;
   long bi_right;			/* right side info */
   char bi_rtype;
   char *bi_key1;			/* separation keys */
   char *bi_key2;
   int bi_lock;
   char *bi_name;			/* index name */
   int bi_fd;				/* index set file descriptor */
   char *bi_rec;			/* storage for index record */
   int bi_ksize;			/* size of an index key */
   long bi_root;			/* index root */
   long bi_height;			/* index height */
   long bi_recsiz;			/* index record size */
   long bi_recno;			/* current record number */
   long bi_reccnt;			/* index record count */
   long bi_free;			/* free pointer */
   };

struct fb_s_bseq {			/* btree+ sequence set node */
   long bs_prev;			/* prev and next pointers */
   long bs_next;
   char *bs_key1;			/* index keys */
   char *bs_key2;
   char *bs_key3;
   int bs_curkey;			/* current key pointer */
   int bs_lock;
   char *bs_name;			/* sequence file name */
   int bs_fd;				/* sequence file descriptor */
   char *bs_rec;			/* storage for sequnce record */
   int bs_ksize;			/* size of a sequence key */
   long bs_head;			/* head of sequence list */
   long bs_tail;			/* tail of sequence list */
   long bs_recsiz;			/* size of sequence record */
   long bs_recno;			/* current sequence record number */
   long bs_reccnt;			/* sequence record count */
   long bs_free;			/* free pointer */
   };

struct fb_s_link {
   fb_database *f_dp;			/* pointer to linked database */
   fb_field *f_fix;			/* indexed value in 'from' database */
   fb_field *f_tix;			/* ix val in 'to' db providing link */
   fb_field *f_ffp;			/* field within 'from' database */
   fb_field *f_tfp;			/* field within 'to' database */
   char *f_fld;				/* storage area for field val */
   char *f_xfld;			/* index value creating field link */
   fb_link *f_next;			/* to link all flinks for searching */
   long f_absrec;			/* absolute record number to use */
   fb_database *f_basedp;		/* database which referred link */
   };

struct fb_s_ixauto {			/* auto indexed field struct */
   char *autoname;			/* for storing autoindex name */
   char *dup_fld;			/* for 'old' field value */
   int afd, hfd;			/* auto index and header fds */
   short uniq;				/* uniqness flag */

   int ix_tree;				/* btree flag */
   fb_bseq *ix_seq, *ix_seqtmp;		/* btree sequence set */
   fb_bidx *ix_idx, *ix_idxtmp;		/* btree index set */
   fb_field **ix_ip;			/* field pointers array for auto ix */
   long ix_bsmax, ix_bsend;		/* for btrees - for auto ix array */
   int ix_ifields;			/* number of fields in the ix array */
   char *ix_key_fld;			/* key space for index entry */
   };

struct fb_s_field {				/* a database field */
   char *id;				/* unique title labeling field */
   char type;				/* type of the field */
   int size;				/* maximum size of field */
   int loc;				/* location when arec is used */
   char *fld;				/* actual pointer to field */
   int incr;				/* to store incremental defaults */
   char *comment, *idefault, *help, *prev, *range, *a_template, *f_macro;
   char comloc, lock, choiceflag;	/* various flags */
   fb_autoindex *aid;			/* pointer to auto index data */
   fb_link *dflink;			/* link to other database */
   fb_link *xflink;			/* extended choice/link database */
   char *mode;				/* permissions settings per field */
   short int f_prec;			/* precision for field */
   };

struct fb_s_database {			/* FB database header record */
   char *dbase;				/* full database path names */
   char *dindex;
   char *dmap;
   char *ddict;
   char *idict;
   char *sdict;				/* simple screen name */
   char *dlog;				/* log name of database */
   long reccnt;				/* full record count */
   char dirty;				/* dirty bit == 0 or 1 */
   long delcnt;				/* deleted record count */
   long rec;				/* current record number */
   int recsiz;				/* approximate record size */
   int nfields;				/* number of fields */
   int fd;				/* database file descriptor */
   int ifields;				/* number of index fields */
   int ifd;				/* index file descriptor */
   int ihfd;				/* index header file descriptor */
   int logfd;				/* log file descriptor */
   int irecsiz;				/* index record size */
   long bsmax, bsend, bsrec;		/* search max/end points */
   int mfd;				/* record map file descriptor */
   fb_field **kp;			/* array of fields */
   fb_field **ip;			/* array of index fields */
   char *orec;				/* original copy of rec */
   char *arec;				/* alternate copy of rec */
   char *irec;				/* buffer for an index record */
   char *afld;				/* field buffers for this db */
   char *bfld;				/* field buffers for this db */
   int refcnt;				/* reference count for linktop list */

					/* B+tree index fields */
   int b_tree;				/* flag set if dbase is using btrees */
   fb_bseq *b_seq, *b_seqtmp;		/* sequence set (and temp space) */
   fb_bidx *b_idx, *b_idxtmp;		/* index set (and temp space) */
   fb_autoindex **b_autoindex;		/* autoindex ptr array */
   int b_maxauto;			/* max number of auto index */
   int b_curauto;			/* cur number of auto index array */
   short int fixedwidth;		/* fixed width flag */

   int b_sid;				/* server id - open via server */
   short int inside_bulkrec;		/* server id - open via server */
   int sequence;			/* sequence number - ala fb_getseq() */
   };

/* external data - always included - defined in sysmsg.c */
extern char *SYSMSG[];

#include <input.h>
#include <error.h>
#include <message.h>
#include <mdict.h>

#include <dbpwd.h>
#include <vdict.h>

/* Cdb to X header file */
#include <cx.h>

#include <libcc.h>
#include <libcdb.h>
#include <libdbd.h>
#include <libinit.h>
#include <liblic.h>
#include <librec.h>
#include <libscr.h>
#include <libwin.h>
#include <libsec.h>
#include <libedit.h>
#include <libclnt.h>
#include <interface.h>
