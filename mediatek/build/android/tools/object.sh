# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.

function Usage
{
    cat<<HELP
Usage: mediatek/build/android/tools/`basename $0` project
Description:
    project
        project represent the correct project name 
Example:
   mediatek/build/android/tools/`basename $0` oppo
   mediatek/build/android/tools/`basename $0` generic   
Attention:
    for emulator,please use generic for the project argument
HELP
}

while getopts "h" OPT;do
case $OPT in
h) Usage;exit 1 ;;
?) Usage;exit 1 ;;
esac
done
shift `expr $OPTIND - 1`
if [ $# != 1 ];then
    Usage
    exit 1
fi
prjConfig="./mediatek/config/$1/ProjectConfig.mk"
if [ ! -e "${prjConfig}" ]; then
    echo "Error!the argument $1 is not a correct project name!"
    echo "please enter -h for help infomation!"
    exit 1
fi

mkdir -p vendor/mediatek/$1/artifacts/obj/hardware/libhardware_legacy/gps
cp out/target/product/$1/obj/SHARED_LIBRARIES/libhardware_legacy_intermediates/gps/*.o vendor/mediatek/$1/artifacts/obj/hardware/libhardware_legacy/gps/
cp mediatek/build/android/tools/mkfiles/hardware_gps.mk vendor/mediatek/$1/artifacts/obj/hardware/libhardware_legacy/gps/Android.mk
mkdir -p vendor/mediatek/$1/artifacts/obj/hardware/libhardware_legacy/wifi
cp out/target/product/$1/obj/SHARED_LIBRARIES/libhardware_legacy_intermediates/wifi/*.o vendor/mediatek/$1/artifacts/obj/hardware/libhardware_legacy/wifi/
cp mediatek/build/android/tools/mkfiles/hardware_wifi.mk vendor/mediatek/$1/artifacts/obj/hardware/libhardware_legacy/wifi/Android.mk
mkdir -p vendor/mediatek/$1/artifacts/obj/$(MTK_PATH_SOURCE)/hardware/ril/mtk-ril
cp mediatek/build/android/tools/mkfiles/mtk-ril.mk vendor/mediatek/$1/artifacts/obj/$(MTK_PATH_SOURCE)/hardware/ril/mtk-ril/Android.mk
mkdir -p vendor/mediatek/$1/artifacts/obj/external/opencore
cp mediatek/build/android/tools/mkfiles/opencore.mk vendor/mediatek/$1/artifacts/obj/external/opencore/Android.mk
mkdir -p vendor/mediatek/$1/artifacts/out/target/common/obj/JAVA_LIBRARIES
cp -rf out/target/common/obj/JAVA_LIBRARIES/static_gemini_intermediates/ vendor/mediatek/$1/artifacts/out/target/common/obj/JAVA_LIBRARIES/

# copy export_include
mkdir -p vendor/mediatek/$1/artifacts/out/target/product/$1/obj/STATIC_LIBRARIES/libui-mtk_intermediates/
cp -vf out/target/product/$1/obj/STATIC_LIBRARIES/libui-mtk_intermediates/export_includes vendor/mediatek/$1/artifacts/out/target/product/$1/obj/STATIC_LIBRARIES/libui-mtk_intermediates/

mkdir -p vendor/mediatek/$1/artifacts/out/target/product/$1/obj/STATIC_LIBRARIES/libgui-mtk_intermediates/
cp -vf out/target/product/$1/obj/STATIC_LIBRARIES/libgui-mtk_intermediates/export_includes vendor/mediatek/$1/artifacts/out/target/product/$1/obj/STATIC_LIBRARIES/libgui-mtk_intermediates/

mkdir -p vendor/mediatek/$1/artifacts/out/target/product/$1/obj/STATIC_LIBRARIES/libsurfaceflinger-mtk_intermediates/
cp -vf out/target/product/$1/obj/STATIC_LIBRARIES/libsurfaceflinger-mtk_intermediates/export_includes vendor/mediatek/$1/artifacts/out/target/product/$1/obj/STATIC_LIBRARIES/libsurfaceflinger-mtk_intermediates/

mkdir -p vendor/mediatek/$1/artifacts/out/target/product/$1/obj/STATIC_LIBRARIES/libdex_intermediates/
cp -vf out/target/product/$1/obj/STATIC_LIBRARIES/libdex_intermediates/export_includes vendor/mediatek/$1/artifacts/out/target/product/$1/obj/STATIC_LIBRARIES/libdex_intermediates/

mkdir -p vendor/mediatek/$1/artifacts/out/host/linux-x86/obj/STATIC_LIBRARIES/libdex_intermediates/
cp -vf out/host/linux-x86/obj/STATIC_LIBRARIES/libdex_intermediates/export_includes vendor/mediatek/$1/artifacts/out/host/linux-x86/obj/STATIC_LIBRARIES/libdex_intermediates/

mkdir -p vendor/mediatek/$1/artifacts/out/target/product/$1/obj/SHARED_LIBRARIES/libdvm_intermediates/
cp -vf out/target/product/$1/obj/SHARED_LIBRARIES/libdvm_intermediates/export_includes vendor/mediatek/$1/artifacts/out/target/product/$1/obj/SHARED_LIBRARIES/libdvm_intermediates/

mkdir -p vendor/mediatek/$1/artifacts/out/target/product/$1/obj/SHARED_LIBRARIES/libstagefright_memutil_intermediates/
cp -vf out/target/product/$1/obj/SHARED_LIBRARIES/libstagefright_memutil_intermediates/export_includes vendor/mediatek/$1/artifacts/out/target/product/$1/obj/SHARED_LIBRARIES/libstagefright_memutil_intermediates/

mkdir -p vendor/mediatek/$1/artifacts/out/target/product/$1/obj/SHARED_LIBRARIES/libmtkbtextadpa2dp_intermediates/
cp -vf out/target/product/$1/obj/SHARED_LIBRARIES/libmtkbtextadpa2dp_intermediates/export_includes vendor/mediatek/$1/artifacts/out/target/product/$1/obj/SHARED_LIBRARIES/libmtkbtextadpa2dp_intermediates/

mkdir -p vendor/mediatek/$1/artifacts/out/target/product/$1/obj/SHARED_LIBRARIES/libextjsr82_intermediates/
cp -vf out/target/product/$1/obj/SHARED_LIBRARIES/libextjsr82_intermediates/export_includes vendor/mediatek/$1/artifacts/out/target/product/$1/obj/SHARED_LIBRARIES/libextjsr82_intermediates/

mkdir -p vendor/mediatek/$1/artifacts/out/target/product/$1/obj/SHARED_LIBRARIES/libeis_intermediates/
cp -vf out/target/product/$1/obj/SHARED_LIBRARIES/libeis_intermediates/export_includes vendor/mediatek/$1/artifacts/out/target/product/$1/obj/SHARED_LIBRARIES/libeis_intermediates/

mkdir -p vendor/mediatek/$1/artifacts/out/target/common/obj/JAVA_LIBRARIES
cp -rf out/target/common/obj/JAVA_LIBRARIES/com.mediatek.a3m-static_intermediates/ vendor/mediatek/$1/artifacts/out/target/common/obj/JAVA_LIBRARIES/

sharedlib="libexttestmode libeis"
for i in `echo $sharedlib`
do
  mkdir -p vendor/mediatek/$1/artifacts/out/target/product/$1/obj/SHARED_LIBRARIES/${i}_intermediates/
  cp -vf out/target/product/$1/obj/SHARED_LIBRARIES/${i}_intermediates/export_includes vendor/mediatek/$1/artifacts/out/target/product/$1/obj/SHARED_LIBRARIES/${i}_intermediates/
done


staticlib="libaudiodcrflt libmhalscenario_plat_autorama"
for i in `echo $staticlib`
do
  mkdir -p vendor/mediatek/$1/artifacts/out/target/product/$1/obj/STATIC_LIBRARIES/${i}_intermediates/
  cp -vf out/target/product/$1/obj/STATIC_LIBRARIES/${i}_intermediates/export_includes vendor/mediatek/$1/artifacts/out/target/product/$1/obj/STATIC_LIBRARIES/${i}_intermediates/
done
# release BT database
# Todo: 
# Use MTK_BT_SUPPORT switch to wrap here
mkdir -p vendor/mediatek/$1/artifacts/BTDataBase
cp mediatek/source/external/bluetooth/blueangel/_bt_scripts/database_win32/BTCatacherDB vendor/mediatek/$1/artifacts/BTDataBase/.

# prevent dive into deeper layer
echo "# empty "> vendor/mediatek/$1/artifacts/Android.mk

