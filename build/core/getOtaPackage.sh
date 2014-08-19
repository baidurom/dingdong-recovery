#!/bin/bash
# $1 - project name : e.g.

#if [ $# -lt 1 ] ; then
#echo "Usage: ./otaPackage.sh projectName"
#echo " "
#echo "projectName:"
#echo "  one of available project."
#    exit
#fi

#if [ ! -d "out/target/product/$1" ]; then
#echo "no project $1, need one of available project"
#    exit
#fi

ROOTPATH="otaPackage"
mkdir -p $ROOTPATH
#bootable
mkdir -p  $ROOTPATH/bootable/recovery
cp -u bootable/recovery/Android.mk  $ROOTPATH/bootable/recovery/
#build
mkdir -p  $ROOTPATH/build/target/product/
cp -a build/target/product/security/  $ROOTPATH/build/target/product/
mkdir -p $ROOTPATH/build/tools/
cp -ur build/tools/releasetools/  $ROOTPATH/build/tools/
#out
mkdir -p $ROOTPATH/out/host/linux-x86/bin/
cp -u out/host/linux-x86/bin/bsdiff  out/host/linux-x86/bin/fs_config out/host/linux-x86/bin/imgdiff out/host/linux-x86/bin/minigzip  out/host/linux-x86/bin/mkbootfs  out/host/linux-x86/bin/mkbootimg  $ROOTPATH/out/host/linux-x86/bin/
mkdir -p $ROOTPATH/out/host/linux-x86/framework
cp -u out/host/linux-x86/framework/signapk.jar $ROOTPATH/out/host/linux-x86/framework/
#mediatek
mkdir -p $ROOTPATH/mediatek/misc
cp -u mediatek/misc/ota_scatter.txt  $ROOTPATH/mediatek/misc/
#ota.zip
#cp -u $1/*.zip $ROOTPATH/org.zip
#org.zip
cp -u $1/obj/PACKAGING/target_files_intermediates/*-target_files-*.zip  $ROOTPATH/ota.zip
#build.prop
cp -u $1/system/build.prop $ROOTPATH/build.prop

#cp -u $1/lk.bin $ROOTPATH/lk.bin
#cp -u $1/logo.bin $ROOTPATH/logo.bin

#configure.xml
echo "">$ROOTPATH/configure.xml
echo "<root>">>$ROOTPATH/configure.xml

#提取版本序号buildnumber
var=$(grep  "ro.adups.fota.version=" "$1/system/build.prop" )
buildnumber=${var##"ro.adups.fota.version="}
echo "<buildnumber>$buildnumber</buildnumber>">>$ROOTPATH/configure.xml

#提取语言
var=$(grep  "ro.product.locale.language=" "$1/system/build.prop" )
echo "<language>${var##"ro.product.locale.language="}</language>">>$ROOTPATH/configure.xml

#提取OEM厂商
var=$(grep  "ro.adups.fota.oem=" "$1/system/build.prop" )
echo "<oem>${var##"ro.adups.fota.oem="}</oem>">>$ROOTPATH/configure.xml

#提取运营商
var=$(grep  "ro.operator.optr=" "$1/system/build.prop")
if [ "$var" = "" ] ; then
  var=other
else
var=$(echo $var|tr A-Z a-z)
if [ ${var##"ro.operator.optr="} = op01 ] ; then
var=CMCC
elif [ ${var##"ro.operator.optr="} = op02 ] ; then
var=CU
else
var=other
fi
fi
echo "<operator>${var##"ro.operator.optr="}</operator>">>$ROOTPATH/configure.xml

#提取设备名称
var=$(grep  "ro.adups.fota.device=" "$1/system/build.prop" )
product=${var##"ro.adups.fota.device="}
echo "<product>$product</product>">>$ROOTPATH/configure.xml

#提取当前时间
echo "<publishtime>$(date +20%y%m%d%H%M%S)</publishtime>">>$ROOTPATH/configure.xml

#提取versionname
#var=$(grep  "ro.product.name=" "$1/system/build.prop" )
echo "<versionname>$buildnumber</versionname>">>$ROOTPATH/configure.xml
echo "</root>">>$ROOTPATH/configure.xml

#判断otapackage文件是否存在，存在则删除
#if [ -f "${product/ /_}"-ota-$buildnumber.zip ]; then
#echo "delete exist file: "${product/ /_}"-ota-$buildnumber.zip"
#rm -f "${product/ /_}"-ota-$buildnumber.zip
#fi

if [ -f $1/adups-otaPackage.zip ]; then
echo "delete exist file:$1/adups-otaPackage.zip"
rm -f $1/adups-otaPackage.zip
fi

#压缩文件
cd otaPackage
mv ota.zip ota_"${buildnumber/ /_}".zip
#mv org.zip org_$buildnumber.zip
zip -rq otaPackage.zip bootable build mediatek out configure.xml build.prop lk.bin logo.bin ota_"${buildnumber/ /_}".zip #org_$buildnumber.zip
cd ..
#mv otaPackage/otaPackage.zip "${product/ /_}"-ota-$buildnumber.zip
mv otaPackage/otaPackage.zip $1/adups-otaPackage.zip
rm -rf otaPackage

