#!/bin/bash

usage()
{
    echo "Usage: $0 filesdir searchstr"
    echo
    echo "Outputs the number of all existing files within filesdir and its subdirs"
    echo "Displays all lines that contain \"searchstr\""
    echo
    echo "filesdir	existing path to a directory"
    echo "searchstr	string that will be searched"
    #echo
    #echo "Options:"
    #echo "  -h            Show this help message."
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


NUM_INPUT_ARGS=2

# Check if the supplied num of args is correct
if [ $# -lt $NUM_INPUT_ARGS ] || [ $# -gt $NUM_INPUT_ARGS ]
then
	echo Wrong arguments. 1>&2 
	echo 1>&2
	usage 1>&2
	exit 1
fi

# Check if the supplied dir exists
if [ -d $1 ]
then
	FILESDIR=$1
else
	echo \"$1\" is not a valid path to a directory 1>&2
	echo 1>&2
	usage 1>&2
	exit 1
fi


# Assign searchstr
SEARCHSTR=$2

# Count all files within FILESDIR and subdirs
# (..) -> captures the output of the command inside the parentheses and splits it into an array, where each element corresponds to a file
# ls
# -p: appends a / to all dirs
# -R: recursive
# grep
# -v: inverts pattern matches
ALLFILES=($( ls -pR $FILESDIR | grep -v / ))
#echo ${ALLFILES[@]}

# get the number of files
# wc	 
# -w: word count
ALLFILESNUM=${#ALLFILES[@]}
#echo Number of files in selected folder: $ALLFILESNUM

# grep
# -r: recursive search
# -p: only output lines
MATCHES=($(grep -ro $SEARCHSTR $FILESDIR))
#echo Num of matching pattern: ${#MATCHES[@]}

# Official output of this script
echo "The number of files are ${ALLFILESNUM} and the number of matching lines are ${#MATCHES[@]}"









