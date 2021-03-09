BEGIN
#
# initialization phase of complete issue report. set nroff parameters.
#
# override default nroff top,bottom,left,and right margins.
printf(".m1 2v\n")
printf(".m2 0v\n")
printf(".m3 2v\n")
printf(".m4 13v\n")
# set up the nroff page header macro -- called at top of each page.
printf(".de $H\n")
printf(".po 2\n.in 0\n.ll 77\n")
printf(".nf\n.nj\n")
printf(".r\n")
printf(".ce 2\n")
printf("Rolodex Report Generated: %s\n", date(now()))
printf("%s\n", TITLE)
printf(".sp 1")
printf(".hl\n")
printf("..\n")
printf(".de $f\n")
printf(".po 2\n")
printf(".in 0\n")
printf(".ll 77\n")
printf(".nf\n")
printf(".nj\n")
printf(".r\n")
printf(".hl\n")
printf("..\n")

# first nroff commands 
printf(".po 2\n.in 0\n.ll 65\n")
printf(".bp\n")
printf(".nf\n.nj\n")
#

BODY
#
printf(" LastName: %-30s    FirstName: %s\n", $LastName, $FirstName)
printf("\n")
printf(" Company:  %s\n", $Company)
printf("\n")
printf("                        Phone Numbers\n")
printf("                        -------------\n")
printf("              Home: %s\n", $HomePhone)
printf("              Work: %s\n", $WorkPhone)
printf("\n")
printf("              Mailing Address\n")
printf("              ---------------\n")
printf("              Addr1: %s\n", $Addr1)
printf("              Addr2: %s\n", $Addr2)
printf("              City:  %-15s     State: %2s  Zip: %s\n", 
   $City, $State, $Zip)
printf("\n")
