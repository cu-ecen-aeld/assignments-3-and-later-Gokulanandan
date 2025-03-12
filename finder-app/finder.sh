#!/bin/bash


if [ $# -ne 2 ]; then
	echo "Invalid number of arguments"
	exit 1
fi

DIR_PATH=$1
SEARCH_STR=$2

if [ ! -e $DIR_PATH ]; then
	echo "path doesn't exist"
	exit 1;
fi

X=`find $DIR_PATH -type f | wc -l`
Y=`grep $SEARCH_STR -R $DIR_PATH | wc -l`

echo "The number of files are $X and the number of matching lines are $Y"
exit 0
