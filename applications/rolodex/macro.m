BEGIN
printf(".he '\\*(td'FirstBase Software'-Page %%-'\n")

BODY
printf(".lp\n")
printf(".sz 20\n")
printf("%s\n.br\n", $Company)
printf(".sz\n")
printf("%s %s\n.br\n", $LastName, $FirstName)
printf("%s FAX: %s\n.br\n", $WorkPhone, $FAXPhone)
printf("%s %s\n.br\n", $Addr1, $Addr2)
printf("%s, %s  %s\n.br\n", $City, $State, $Zip)
printf(".hl\n")
rec++

END
printf(".(f\nThere were %d records printed.\n.)f", rec)
