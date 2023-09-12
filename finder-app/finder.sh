#!/bin/bash


if [ $# -ne 2 ]
then
 echo "need args, finder.sh <path> <string>"
 exit 1
elif ! [ -d $1 ]
then
 echo "$1 is not directory"
 exit 1
fi

num_of_files=$(find $1 -type f | wc -l)
num_of_str=$(grep -r $2 $1 | wc -l)

echo "The number of files are $num_of_files and the number of matching lines are $num_of_str"
