BEGIN{
      FS=" "
      row = 4
     }
{
 if (row > 22)
    {
     printf "$PAGE\n"
     row = 4;
    }
 printf "\"%s:\":%d,2\n", $1, row
 printf "%s@%d,17\n", $1, row
 row++ 
}
