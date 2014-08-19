#!/bin/bash
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


MTK_PROJECT=$1

function change_mode
{
	local files=`echo $1 | sed 's/\[\(.*\)\]/\\\[\1\\\]/g'`
	for i in $files; do
		if [[ -e $i ]]; then
			chmod -R $2 $i
		fi
	done
}

files_to_chmod_x="
prebuilt/linux-x86/sdl/bin/*
prebuilt/linux-x86/flex/*
prebuilt/linux-x86/toolchain/arm-eabi-4.2.1/bin/*
prebuilt/linux-x86/toolchain/arm-eabi-4.2.1/arm-eabi/bin/*
prebuilt/linux-x86/toolchain/arm-eabi-4.2.1/libexec/gcc/arm-eabi/4.2.1/install-tools/*
prebuilt/linux-x86/toolchain/arm-eabi-4.2.1/libexec/gcc/arm-eabi/4.2.1/*
prebuilt/linux-x86/toolchain/arm-eabi-4.4.0/bin/*
prebuilt/linux-x86/toolchain/arm-eabi-4.4.0/arm-eabi/bin/*
prebuilt/linux-x86/toolchain/arm-eabi-4.4.0/libexec/gcc/arm-eabi/4.4.0/install-tools/*
prebuilt/linux-x86/toolchain/arm-eabi-4.4.0/libexec/gcc/arm-eabi/4.4.0/*
build/core/find-jdk-tools-jar.sh
build/tools/*.sh
build/tools/*.py
external/iptables/extensions/create_initext
external/webkit/WebCore/css/*.pl
external/webkit/WebCore/*.sh
external/dhcpcd/dhcpcd-run-hooks
kernel/build.sh
kernel/scripts/*.sh
kernel/scripts/*.pl
kernel/trace32/*.sh
kernel/trace32/tools/mkimage
frameworks/base/cmds/am/am
frameworks/base/cmds/dumpinfo/dumpinfo.sh
frameworks/base/cmds/ime/ime
frameworks/base/cmds/input/input
frameworks/base/cmds/pm/pm
frameworks/base/cmds/svc/svc
development/cmds/monkey/monkey
prebuilt/android-arm/gdbserver/gdbserver
mediatek/build/android/*.sh
mediatek/build/tools/mkimage
external/qemu/hxtool
external/qemu/feature_to_c.sh
external/qemu/android/tools/gen-hw-config.py
cts/tools/dx-tests/etc/compileall
build/tools/releasetools/ota_from_target_files
mediatek/build/tools/*
bootable/bootloader/preloader/build.sh
mediatek/source/preloader/build.sh
bootable/bootloader/uboot/build.sh
mediatek/source/dct/DrvGen
mediatek/custom/common/uboot/logo/update
mediatek/custom/common/uboot/logo/tool/*
mediatek/build/Makefile
mediatek/build/shell.sh
mediatek/build/tools/image_signning_tool/exe/*
"

files_to_chmod_w="
frameworks/base/api/current.xml
vendor/mediatek/*/artifacts/*.txt
mediatek/custom/${MTK_PROJECT}/preloader/custom_emi.c
mediatek/custom/${MTK_PROJECT}/preloader/MTK_Loader_Info_v4.tag
mediatek/custom/${MTK_PROJECT}/preloader/inc/custom_emi.h
mediatek/custom/${MTK_PROJECT}/secro/android-sec
mediatek/custom/common/secro/android-sec
mediatek/source/frameworks/featureoption/java/com/mediatek/featureoption/FeatureOption.java
kernel/drivers/net/wireless/mt592x/wlan/
mediatek/custom/common/uboot/logo/boot_logo
mediatek/source/external/bluetooth/blueangel/_bt_scripts/
mediatek/source/external/bluetooth/database/BTCatacherDB
"

for i in $files_to_chmod_x; do
	change_mode $i a+x
done
for i in $files_to_chmod_w; do
	change_mode $i a+w
done

files_to_touch="
external/webkit/WebCore/css/tokenizer.flex
"

for i in $files_to_touch; do
	touch $i
done
