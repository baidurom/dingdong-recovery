#!/bin/bash

# unpack_boot_mtk.sh
# Unpack the boot.img or recovery.img of MTK format
#
# @author: duanqizhi01@baidu.com(duanqz)
#

BOOTIMG=$1
OUTPUT=$2

function usage()
{
	echo "Usage unpack_boot_mtk.sh BOOTIMG [OUTPUT]"
	echo "   BOOTIMG: the file path of the boot.img to be unpack"
	echo "   OUTPUT:  the output directory. if not present, the OUT/ directory will be used"
}

function init_tools()
{
	local old_pwd=`pwd`
	TOOL_DIR=`cd $(dirname $0); pwd`
	UNPACKBOOTIMG=$TOOL_DIR/unpack-mtk-bootimg.pl
	cd $old_pwd
}

function unpack_bootimg()
{
	local old_pwd=`pwd`
	mkdir -p $OUTPUT
	cp $BOOTIMG $OUTPUT/boot.img
	cd $OUTPUT

	# Unpack boot image
	$UNPACKBOOTIMG boot.img
	mv boot.img-ramdisk  RAMDISK
	mv boot.img-kernel   kernel
	rm -rf boot.img*

	cd $old_pwd
}


### Start Script ###

# Check parameters
[ $# -eq 0 ] && usage && exit 1;
[ -z $2 ] && OUTPUT="OUT/";

init_tools;
unpack_bootimg;
