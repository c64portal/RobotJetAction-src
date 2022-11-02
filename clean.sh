#!/bin/bash
#
# 
#
#

PWD=`pwd`

echo
echo "Cleaning source dir "

rm -rf *.bin
rm -rf *.d64
rm -rf *.prg
rm -rf *.b2
rm -rf *.sym
rm -rf main.asm 

rm -rf *.klog
rm -rf *.vs
rm -rf *.dbg

# echo "Cleaning bin/ dir "
# rm -rf bin/*

echo "Cleaning levels"
echo " w1 "
cd $PWD/data/w1
./clean.sh


