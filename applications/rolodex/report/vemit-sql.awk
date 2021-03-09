BEGIN \
{
   FS = "^"
}

{
   DATABASE   = $1         # database name
   INDEXNAME  = $2         # temp index name
   TEMPFILE   = $3         # passed in TEMPFILE NAME for values
   getline
   LastName   = $1         # LastName     a   30
   FirstName  = $2         # FirstName    a   15
   Company    = $3         # Company      a   35
   HomePhone  = $4         # HomePhone    a   17
   WorkPhone  = $5         # WorkPhone    a   17
   Addr1      = $6         # Addr1        a   40
   Addr2      = $7         # Addr2        a   40
   City       = $8         # City         a   15
   State      = $9         # State        a   2
   Zip        = $10        # Zip          a   10
   TYPE       = $11        # TYPE         a   1
   S_LastName = $12        # S_LastName   a   1
   S_FirstNam = $13        # S_FirstNam   a   1
   S_Company  = $14        # S_Company    a   1
   S_HomePhon = $15        # S_HomePhon   a   1
   S_WorkPhon = $16        # S_WorkPhon   a   1
   S_Addr1    = $17        # S_Addr1      a   1
   S_Addr2    = $18        # S_Addr2      a   1
   S_City     = $19        # S_City       a   1
   S_State    = $20        # S_State      a   1
   S_Zip      = $21        # S_Zip        a   1

   if (S_LastName == 1)
       SORTBY1 = "LastName"
   else if (S_FirstNam == 1)
       SORTBY1 = "FirstName"
   else if (S_Company == 1)
       SORTBY1 = "Company"
   else if (S_HomePhon == 1)
       SORTBY1 = "HomePhone"
   else if (S_WorkPhon == 1)
       SORTBY1 = "WorkPhone"
   else if (S_Addr1 == 1)
       SORTBY1 = "Addr1"
   else if (S_Addr2 == 1)
       SORTBY1 = "Addr2"
   else if (S_City == 1)
       SORTBY1 = "City"
   else if (S_State == 1)
       SORTBY1 = "State"
   else if (S_Zip == 1)
       SORTBY1 = "Zip"

   if (S_LastName == 2)
       SORTBY2 = "LastName"
   else if (S_FirstNam == 2)
       SORTBY2 = "FirstName"
   else if (S_Company == 2)
       SORTBY2 = "Company"
   else if (S_HomePhon == 2)
       SORTBY2 = "HomePhone"
   else if (S_WorkPhon == 2)
       SORTBY2 = "WorkPhone"
   else if (S_Addr1 == 2)
       SORTBY2 = "Addr1"
   else if (S_Addr2 == 2)
       SORTBY2 = "Addr2"
   else if (S_City == 2)
       SORTBY2 = "City"
   else if (S_State == 2)
       SORTBY2 = "State"
   else if (S_Zip == 2)
       SORTBY2 = "Zip"

   if (SORTBY1 == "" && SORTBY2 == "")
       SORTBY = "LastName"
   else if (SORTBY1 != "" && SORTBY2 == "")
      SORTBY = SORTBY1
   else if (SORTBY1 == "" && SORTBY2 != "")
      SORTBY = SORTBY2
   else if (SORTBY1 != "" && SORTBY2 != "")
      SORTBY = SORTBY1 ", " SORTBY2

   printf "%s^%s^%s\n", SORTBY1, SORTBY2, TYPE > TEMPFILE

   printf "CREATE INDEX %s\n", INDEXNAME
   printf "ON %s (%s)\n", DATABASE, SORTBY
   printf "WHERE "
   if (LastName != ""){
      if (aflag)
      printf " AND "
      printf "LastName LIKE \"%%%s%%\"\n", LastName
      aflag = 1
      }
   if (FirstName != ""){
      if (aflag)
      printf " AND "
      printf "FirstName LIKE \"%%%s%%\"\n", FirstName
      aflag = 1
      }
   if (Company != ""){
      if (aflag)
      printf " AND "
      printf "Company LIKE \"%%%s%%\"\n", Company
      aflag = 1
      }
   if (HomePhone != ""){
      if (aflag)
      printf " AND "
      printf "HomePhone = \"%s\"\n", HomePhone
      aflag = 1
      }
   if (WorkPhone != ""){
      if (aflag)
      printf " AND "
      printf "WorkPhone = \"%s\"\n", WorkPhone
      aflag = 1
      }
   if (Addr1 != ""){
      if (aflag)
      printf " AND "
      printf "Addr1 = \"%s\"\n", Addr1
      aflag = 1
      }
   if (Addr2 != ""){
      if (aflag)
      printf " AND "
      printf "Addr2 = \"%s\"\n", Addr2
      aflag = 1
      }
   if (City != ""){
      if (aflag)
      printf " AND "
      printf "City = \"%s\"\n", City
      aflag = 1
      }
   if (State != ""){
      if (aflag)
      printf " AND "
      printf "State = \"%s\"\n", State
      aflag = 1
      }
   if (Zip != ""){
      if (aflag)
      printf " AND "
      printf "Zip = \"%s\"\n", Zip
      aflag = 1
      }
   if (aflag == 0)
   printf "LastName IS NOT NULL\n"
   printf ";\n"
   exit
}
