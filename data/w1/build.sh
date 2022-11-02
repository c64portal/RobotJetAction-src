#!/bin/bash

KICKASM=/Applications/KickAssembler/KickAss.jar 
BB2=b2.exe


echo "********** Doing chars and colors and levels maps ************"

# ByteBoozer2 cruncher needs first 2 bytes to be start address of decrunch destination
# Let's add lo/hi bytes of start address to the files

cp 4000.bin chars.bin.prg
cat chars.bin >> chars.bin.prg
$BB2 chars.bin.prg

cp 2D00.bin colors.bin.prg
cat colors.bin >> colors.bin.prg
$BB2 colors.bin.prg


for i in `ls l*.asm`
do
	echo $i
	sed '/;/d' $i | sed '/map/d' >> $i.s
	java -jar $KICKASM $i.desc >> /dev/null
	$BB2 $i.prg >> /dev/null
done
