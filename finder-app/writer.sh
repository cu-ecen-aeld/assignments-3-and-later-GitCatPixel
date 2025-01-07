#!/bin/bash

usage()
{
    echo "Usage: $0 writefile writestr"
    echo
    echo "Creates a file @\"writefile\" and writes \"writestr\" to it."
    echo "Displays all lines that contain \"searchstr\""
    echo
    echo "writefile	path and filename that shall be created"
    echo "writestr	string that will be written"
}

# For now we won't use getopts
#
# Info: leading ':' silences automated error msg for illegal options
#while getopts :h opt
#do
#   case $opt in
#       h) usage
#	  # removes the -h from the list of args
#          shift     
#          ;;
#       ?) echo "Unknown option -$OPTARG."
#	  echo
#
#	  # Print help text
#          usage
#          echo
#	  ;;
#   esac
#done

DEBUG=0
NUM_INPUT_ARGS=2

# Check if the supplied num of args is correct
if [ $# -lt $NUM_INPUT_ARGS ] || [ $# -gt $NUM_INPUT_ARGS ]
then
	echo Wrong arguments. 1>&2 
	echo 1>&2
	usage 1>&2
	exit 1
fi

# Assigne variables
WRITEFILE=$1
WRITESTR=$2

# Create file 
# isolate file name & path
# cut:
# -d:   Select delimiter '/'
# -f1:  'f' = field, '1' = first, '-' = from behind
FILENAME=$(echo ${WRITEFILE} | cut -d'/' -f1)

if [ $DEBUG -eq 1 ]
then
    echo Filename: $FILENAME
fi

# grep
# -o:   only-match
# -P:   Pearl regex
# ^:    start of line
# .*:   any amount of any char
# \/:   until '/'
FILEPATH=$(echo ${WRITEFILE} | grep -oP '^(.*\/)')

if [ $DEBUG  -eq 1 ]
then
    echo Filepath: $FILEPATH
fi

# Create parent directories
mkdir -p $FILEPATH

# IF the result of the last command was not 0...
if [ $? -ne 0 ] 
then
    exit 1
fi

# Create the file
echo $WRITESTR > $WRITEFILE

# IF the result of the last command was not 0...
if [ $? -ne 0 ] 
then
    exit 1
else
    exit 0
fi



