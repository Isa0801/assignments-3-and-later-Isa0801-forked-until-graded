#!/bin/bash

if [ $# -ne 2 ]
then
 echo "need args, writer.sh <filename> <string>"
 exit 1
fi

dir_path=$(dirname $1)

mkdir -p $dir_path

echo $2 > $1

