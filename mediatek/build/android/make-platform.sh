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


PRODUCT_OUT=$1
TARGET_PRODUCT=$2

if [[ ${PRODUCT_OUT} == "" ]]; then 
  echo "usage: make-platform.sh <PRODUCT_OUT> <TARGET_PRODUCT>"
  exit -1;
fi

if [[ ! -e ${PRODUCT_OUT}/root || ! -e ${PRODUCT_OUT}/system ]]; then
  echo "$TARGET_PRODUCT platform is not built"
  exit -1
fi

PACK_PLATFORM=n
PACK_RAMDISK=n

if [ "${TARGET_PRODUCT}" == "mt6575_fpga" ]; then 
	PACK_RAMDISK=y
	PACK_PLATFORM=y
fi

if [ ${PACK_PLATFORM} == "y" ]; then 

	PACK_OUT=${PRODUCT_OUT}/.pack
	ARCHIVE_NAME=platform.tgz
	FULL_ARCHIVE_NAME=${PRODUCT_OUT}/${ARCHIVE_NAME}
	
	SYSTEM_FOLDER=${PRODUCT_OUT}/system

	echo -n "Packing Android platform \"${FULL_ARCHIVE_NAME}\"..."
	if [[ -e ${PACK_OUT} ]]; then
	  rm -rf ${PACK_OUT};
	fi
	if [[ ! -e ${PACK_OUT} ]]; then
	  mkdir ${PACK_OUT};
	fi
	if [[ ! -e ${PACK_OUT} || ! -d ${PACK_OUT} ]]; then
	  echo "can't create dir .pack. please check file permission.";
	  exit -1;
	fi
	cp -R ${SYSTEM_FOLDER}/* ${PACK_OUT}
	cp -R ${PRODUCT_OUT}/data ${PACK_OUT}/data
	cd ${PACK_OUT};
	  chmod -R a+x bin/*;
	  chmod -R a+x xbin/*;
	  tar -zcf ../${ARCHIVE_NAME} *;
	cd - > /dev/null
	echo "Done"

fi

if [ ${PACK_RAMDISK} == "y" ]; then 
	echo "Creating ramdisk.gz..."

	RAMDISK=ramdisk
	RAMDISK_GZ=ramdisk.gz
	rm -f ${PRODUCT_OUT}/$RAMDISK
	rm -f ${PRODUCT_OUT}/$RAMDISK.gz

	cd ${PRODUCT_OUT}/root
	rm -rf data
	find . -print | cpio -H newc -o > ../${RAMDISK}
	gzip -9 ../$RAMDISK
	echo Created ${PRODUCT_OUT}/$RAMDISK.gz
	cd - > /dev/null

	echo "Creating factory ramdisk.gz..."

	RAMDISK=ramdisk-factory
	RAMDISK_GZ=ramdisk-factory.gz
	rm -f ${PRODUCT_OUT}/$RAMDISK
	rm -f ${PRODUCT_OUT}/$RAMDISK.gz

	cd ${PRODUCT_OUT}/factory/root
	rm -rf data
	find . -print | cpio -H newc -o > ../../${RAMDISK}
	gzip -9 ../../$RAMDISK
	echo Created ${PRODUCT_OUT}/$RAMDISK.gz
	cd - > /dev/null
fi

