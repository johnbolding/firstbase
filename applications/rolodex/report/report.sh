#!/bin/sh 
#
# include the common subroutines and definitions for Rolodex scripts
#
#
. $ROLOHOME/report/COMMON
#
# this is the view used by the dbvemit to capture the VEMIT FILE
#
REPORT_VIEW=${REPORT_DIRECTORY}report

trap \
   "rm -f $VEMIT_FILE $VEMIT_FILE2 ${INQIDX}.idx ${INQIDX}.idicti \
   ${INQIDX}.idict $SQL_SCRIPT \
   $OUTPUT $NROFF_FILE; exit " 0 1 2 3 4 5 6 7 8 9
#
# use dbvemit to capture print request in the VEMIT FILE
#
dbvemit -d $REPORT_DATABASE -v $REPORT_VIEW -a 1 $VEMIT_FILE -c \^ \
   CHOICEPAUSE=ON CHOICEADDPAUSE=ON ASKWRITE=ON VIPAUSE=ON
if [ ! -s $VEMIT_FILE ]; then
   NODATA
fi
clear
echo "$DATABASE^$INQIDX^$VEMIT_FILE" > $VEMIT_FILE2
cat $VEMIT_FILE >> $VEMIT_FILE2
#
#
echo generating index ...
#
# Now process the vemit file and generate a dbsql script
# that will itself generate a proper index for the request.
# Use UNIX awk to do this task
#
awk -f $AWKSCRIPT < $VEMIT_FILE2 > $SQL_SCRIPT
#
# again, the resulting DBSQL script will gen an index
dbsql -f $SQL_SCRIPT

#
# use these lines to debug the generated sql query, if needed
# echo File to examine are: $INQIDX and $SQL_SCRIPT
# read garbage
#

if [ ! -s $INDEXFILE ]; then
   NORECORDS
fi

#
# decide which type of report to do
# use shell itself to split apart the values
# this is dependent on the report.ddict file ordering
#
# report.ddict has the TYPE=9 ...
# sed is used to burst apart empty fields making them hold places too.
# (i.e. sh does not recognize ",," as a empty argument.
#
LINE=`cat $VEMIT_FILE | sed -e "s/\^\^/\^ \^/g" | sed -e "s/\^\^/\^ \^/g" | \
	sed -e "s/^\^/ \^/g"`
IFS="^"
set $LINE
SORTBY1=$1
SORTBY2=$2
TYPE=$3
#
# The following selections must match choices in choice/rep_type.ch
#
if [ $TYPE = 1 ] ; then
   MACRO_SCRIPT=${REPORT_DIRECTORY}macro/prt_comp.m
elif [ $TYPE = 2 ] ; then
   MACRO_SCRIPT=${REPORT_DIRECTORY}macro/prt_sum.m
else
   #
   # this is the default ... for now, the complete
   #
   MACRO_SCRIPT=${REPORT_DIRECTORY}macro/prt_comp.m
fi
#
# reset IFS to BLANK TAB NEWLINE
IFS="
"
if [ "$SORTBY1" = " " -o "$SORTBY1" = "" ] ; then
   SORTBY1="Name"
fi
TITLE="Sorted by: $SORTBY1"
if [ "$SORTBY2" != " " -o "$SORTBY" != "" ] ; then
   TITLE="${TITLE}, $SORTBY2"
fi
#
# generate and format the proper printout
#
echo generating printout ...
dbmacro -d $DATABASE -i $INQIDX -f $MACRO_SCRIPT \
   TITLE=$TITLE > $NROFF_FILE

echo formatting printout ...
nroff -me -Tlpr $NROFF_FILE > $OUTPUT
echo Done.
screenprint $OUTPUT
