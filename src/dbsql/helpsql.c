/*
 * Copyright (C) 2001 FirstBase Software, Inc. All rights reserved.
 *
 * $Id: helpsql.c,v 9.1 2001/01/16 02:46:46 john Exp $
 *
 * See the file LICENSE for conditions of use and distribution.
 *
 */

#ifndef LINT
static char Helpsql_sid[] = "@(#) $Id: helpsql.c,v 9.1 2001/01/16 02:46:46 john Exp $";
#endif

#include "dbsql_e.h"

static void help_dbase();
static help_com();
static help_select();
static help_create_view();
static help_drop_view();
static help_create_index();
static help_drop_index();

/*
 * helpsql - provide a fb_help facility
 */

   helpsql(com)
      char *com;

      {
         char buf[FB_MAXLINE], *p;
         int i;

         remove_newline(com);
         if ((p = strrchr(com, ';')) != 0)
            *p = NULL;
         if (com[0] == NULL || equal(com, "commands"))
            help_com();
         else if (equal(com, "select"))
            help_select();
         else if (equal(com, "create view"))
            help_create_view();
         else if (equal(com, "drop view"))
            help_drop_view();
         else if (equal(com, "create index"))
            help_create_index();
         else if (equal(com, "drop index"))
            help_drop_index();
         else{
            for (i = 1; (i = fb_getword(com, i, buf)) != 0; )
               help_dbase(buf);
            }
      }


   static void help_dbase(dname)
      char *dname;

      {
         fb_database *d, *fb_dballoc();
         int st, fld, row, i;

         d = fb_dballoc();
         fb_dbargs(dname, NIL, d);
         st = fb_opendb(d, READ, FB_NOINDEX, 0);
         if (st != FB_AOK){
            fb_serror(FB_MESSAGE, "Could not open database", dname);
            return;
            }
	 fprintf(stderr,
            "Database %s: Record Count = %ld, Delete Count = %ld\n\n",
            dname, d->reccnt, d->delcnt);
	 fprintf(stderr, "Fields of Database %s:\n\n", dname);
         for (fld = 0, row = 1; fld < d->nfields; row++){
            for (i = 1; i <= 3 && fld < d->nfields; i++, fld++)
               fprintf(stderr, "%10s ..... %c %4d   ",
                  d->kp[fld]->id,
                  d->kp[fld]->type,
                  d->kp[fld]->size);
            fprintf(stderr, "\n");
            if (row >= cdb_t_lines - 2){
               row = 0;
               pagebreak();
               }
            }
      }
      
   static help_com()
      {
         fprintf(stderr, "dbsql commands and help subjects:\n");
         fprintf(stderr, "   select\n");
         fprintf(stderr, "   create view\n");
         fprintf(stderr, "   drop view\n");
         fprintf(stderr, "   create index\n");
         fprintf(stderr, "   drop index\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "dbsql directives:\n");
         fprintf(stderr, "   help [subject] - general help/help on subject\n");
         fprintf(stderr, "   exit           - exit dbsql\n");
         fprintf(stderr, "   quit           - exit dbsql\n");
         fprintf(stderr, "   cd             - change directory\n");
         fprintf(stderr, "   pwd            - print working directory\n");
         fprintf(stderr, "   !command       - submit command to UNIX shell\n");
      }

   static help_select()
      {
         fprintf(stderr, "dbsql SELECT command Description:\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "The SELECT command is used to select, display,\n");
         fprintf(stderr, "and manipulate fields from databases. Records\n");
         fprintf(stderr, "from different databases (tables) can be joined\n");
         fprintf(stderr, "together, or related. Set functions and math\n");
         fprintf(stderr, "expressions can be specified. Limitations on the\n");
         fprintf(stderr, "records projected can also be specified.\n");
         fprintf(stderr, "\n");
         pagebreak();
         fprintf(stderr, "dbsql SELECT command Syntax:\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "SELECT [ALL | DISTINCT] <select list>\n");
         fprintf(stderr, "   <table expression>;\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "<select list> ::=\n");
         fprintf(stderr, "   <value expr> [ {, <value expr>} ... ] | *\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "<value expr>  ::= +,-,*,/ and primaries\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "<primary>     ::= <value spec> |\n");
         fprintf(stderr, "                  <column spec> |\n");
         fprintf(stderr, "                  <set function spec> |\n");
         fprintf(stderr, "                  ( <value expr> ) |\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "<table expression> ::= <from clause>\n");
         fprintf(stderr, "   [ <where clause> ]\n");
         fprintf(stderr, "   [ <order by clause> ]\n");
         fprintf(stderr, "   [ <group by clause> ]\n");
         fprintf(stderr, "   [ <having clause> ]\n");
         fprintf(stderr, "\n");
         pagebreak();
         fprintf(stderr, "dbsql SELECT command examples:\n");
         fprintf(stderr, "select name, phone from customer \n");
         fprintf(stderr, "   where name = \"Jones\";\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "select product, quantity from inventory \n");
         fprintf(stderr, "   order by product;\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "select * from sales;\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "select supplier.*, parts.* from supplier, parts\n");
         fprintf(stderr, "   where supplier.city = parts.city;\n");
      }

   static help_create_view()
      {
         fprintf(stderr, "dbsql CREATE VIEW command Description:\n");
         fprintf(stderr, "The CREATE VIEW command is used to create\n");
         fprintf(stderr, "a new database from existing databases.\n");
         fprintf(stderr, "Basically, this action physically saves the\n");
         fprintf(stderr, "result of a SELECT command.\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "dbsql CREATE VIEW command syntax:\n");
         fprintf(stderr, "CREATE VIEW table_name [ ( column_list ) ] as\n");
         fprintf(stderr, "   <SELECT statement> ;\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "dbsql CREATE VIEW command examples:\n");
         fprintf(stderr, "create view dbase (a, b, c) as \n");
         fprintf(stderr, "   select sn, sname, status from s; \n");
         fprintf(stderr, "\n");
         fprintf(stderr, "create view dbase as \n");
         fprintf(stderr, "   select weight, status from s\n");
         fprintf(stderr, "      where weight > 10;\n");
      }

   static help_drop_view()
      {
         fprintf(stderr, "dbsql DROP VIEW command Description:\n");
         fprintf(stderr, "The DROP VIEW command is used to create\n");
         fprintf(stderr, "a new database from existing databases.\n");
         fprintf(stderr, "Basically, this action physically saves the\n");
         fprintf(stderr, "result of a SELECT command.\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "dbsql DROP VIEW command syntax:\n");
         fprintf(stderr, "DROP VIEW table_name ;\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "dbsql DROP VIEW command examples:\n");
         fprintf(stderr, "drop view dbase;\n");
      }

   static help_create_index()
      {
         fprintf(stderr, "dbsql CREATE INDEX command Description:\n");
         fprintf(stderr, "The CREATE INDEX command is used to create\n");
         fprintf(stderr, "a new FirstBase index from an existing database.\n");
         fprintf(stderr, "Basically, this command uses SQL style WHERE\n");
         fprintf(stderr, "clauses to generate a standard FirstBase index.\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "dbsql CREATE INDEX command syntax:\n");
         fprintf(stderr, "CREATE INDEX index_name ON table_name \n");
         fprintf(stderr, " ( sort_spec_list ) WHERE where_clause ;\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "<sort_spec_list> ::=\n");
         fprintf(stderr, "   <sort_spec> [ {, <sort_spec>} ... ]\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "<sort_spec> ::=\n");
         fprintf(stderr, "   <table column> [ ASC | DESC ]\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "dbsql CREATE INDEX command examples:\n");
         fprintf(stderr, "create index newidx on s (sn, sname);\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "create index newidx on s (city DESC)\n");
         fprintf(stderr, "   where status > 20 ;\n");
         fprintf(stderr, "\n");
      }

   static help_drop_index()
      {
         fprintf(stderr, "dbsql DROP INDEX command Description:\n");
         fprintf(stderr, "The DROP INDEX command is used to create\n");
         fprintf(stderr, "a new database from existing databases.\n");
         fprintf(stderr, "Basically, this action physically saves the\n");
         fprintf(stderr, "result of a SELECT command.\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "dbsql DROP INDEX command syntax:\n");
         fprintf(stderr, "DROP INDEX index_name ;\n");
         fprintf(stderr, "\n");
         fprintf(stderr, "dbsql DROP INDEX command examples:\n");
         fprintf(stderr, "drop view newidx;\n");
      }

   pagebreak()
      {
         fprintf(stderr, "Hit <RETURN> for more:");
         getchar();
      }
