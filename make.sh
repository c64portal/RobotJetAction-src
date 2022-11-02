#!/bin/bash


T=`date +"%Y%m%d_%H%M%S"`


# Add path to you kickc 0.82 compiler
KICK=~/c64/kickc082

DIR=`pwd`

if [ -z "$1" ]
	then
		echo
		echo "Available arguments: main, data"
		echo
	else
		if [ $1 = "main" ]
		then
			$KICK/bin/kickc.sh -Warraytype main.c -a -Sc $2
		fi

		if [ $1 = "data" ]
		then
			echo "--------------- Making levels in: ------- $DIR/data/w1"
			cd "$DIR/data/w1"
			./build.sh

		fi


fi




