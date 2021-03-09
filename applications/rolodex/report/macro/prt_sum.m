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
printf("Rolodex Summary Report Generated: %s\n", date(now()))
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
printf(" \(1\) LastName: %-30s    \(2\) FirstName: %s\n", $LastName, $FirstName)
printf(" \(3\) Company:  %s\n", $Company)
printf("\n")
