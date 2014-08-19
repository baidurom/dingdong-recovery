#!/bin/bash

PLATFORM=$1
IMGDIR=$PWD/IMGDIR
RAMDISK=$IMGDIR/ramdisk

TOOL_PATH=../../unpack-tools
UNPACKMTKBOOTIMG=$TOOL_PATH/unpack-mtk-bootimg.pl
UNPACKBOOTIMGPL=$TOOL_PATH/unpack-bootimg.pl
UNPACKBOOTIMG=$TOOL_PATH/unpackbootimg

function unpack_qc_img()
{
	echo ">>> ==== platform qualcomm ===="
	echo ">>> unpack image ..."
	if [ -d $IMGDIR ];then
		rm -rf $IMGDIR
	fi
	mkdir -p $IMGDIR
	IMGNAME=`basename $1`
	cp -f $1 ./$IMGNAME > /dev/null
	$UNPACKBOOTIMG -i $IMGNAME -o $IMGDIR
	$UNPACKBOOTIMGPL $IMGNAME
	
	mv $IMGNAME-ramdisk  $IMGDIR/RAMDISK
	mv $IMGDIR/$IMGNAME-zImage $IMGDIR/kernel
	mv $IMGDIR/$IMGNAME-cmdline  $IMGDIR/cmdline
	mv $IMGDIR/$IMGNAME-base  $IMGDIR/base
	mv $IMGDIR/$IMGNAME-pagesize  $IMGDIR/pagesize
	rm -rf $IMGNAME-*
	rm -rf $IMGDIR/$IMGNAME-*
	
	#echo {$IMGNAME%.img} >> $FILEINFO
	echo ">>> unpack image done!"
}

function unpack_mtk_img()
{
	echo ">>> ==== platform rom-mtk ===="
	echo ">>> unpack img ..."
	IMGNAME=`basename $1`
	cp -f $1 ./$IMGNAME > /dev/null
	$UNPACKMTKBOOTIMG $IMGNAME
	mv $IMGNAME-ramdisk  RAMDISK
	mv $IMGNAME-kernel  kernel
	rm -rf $IMGNAME-*

	#echo {$IMGNAME%.img} >> $FILEINFO
	echo ">>> unpack image done!"
}


function usage()
{
	echo " usage: $0 -m/q recovery.img/boot.img"
	echo " -m :  mtk platform"
	echo " -q :  qualcomm platform"
	exit 0
}

if [[ "$#" == "2" ]] && [[ -f $2 ]];then
	if [ "$PLATFORM" = "-m" ];then
		unpack_mtk_img $2
	elif [ "$PLATFORM" = "-q" ];then
		unpack_qc_img $2
	else
		usage
	fi
else
	usage
fi

