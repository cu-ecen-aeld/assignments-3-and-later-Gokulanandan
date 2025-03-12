#!/bin/bash

if [ $# -ne 2 ]; then
	echo "Invalid number of arguments"
	exit 1
fi

FILE_PATH=$1
WRITE_STR="$2"

mkdir -p "$(dirname "$FILE_PATH")" && touch $FILE_PATH
if [ $? -ne 0 ]; then
	echo "File could not be created"
	exit 1
fi
echo $WRITE_STR > $FILE_PATH
