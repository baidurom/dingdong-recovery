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


# include release policy
RELEASE_FILE := $(BUILD_SYSTEM_MTK_EXTENSION)/release_file.mk
include $(RELEASE_FILE)

# word-colon not defined here; use our version
define _word-colon
$(word $(1),$(subst :,$(space),$(2)))
endef

$(foreach item,$(_release_policy), \
    $(eval _dir := $(call _word-colon,1,$(item))) \
    $(eval _policy := $(subst ;, ,$(call _word-colon,2,$(item)))) \
    $(eval $(_dir).RELEASE_POLICY := headquarter $(_policy)) \
)

DIR_WITH_RELEASE_POLICY := $(foreach item,$(_release_policy),$(call _word-colon,1,$(item)))

# <LOCAL_MODULE>:<SEMI-COLON SEPERATED LIBRARIES_LIST>
# (with be mapped to vendor/mediatek/yusu/jar/...)
_patch_additional_file :=

ifeq ($(PARTIAL_BUILD),true)
_patch_additional_file := $(subst /,:,$(subst $(ARTIFACT_DIR)/jar/,,$(shell find $(ARTIFACT_DIR)/jar -name "policy.jar")))
endif

$(foreach item,$(_patch_additional_file), \
    $(eval _module := $(call _word-colon,1,$(item))) \
    $(eval ARTIFACT.$(_module).JARS := $(call _word-colon,2,$(item))) \
)


# temporarily removed dependencies 
#  $(MTK_PATH_SOURCE)/external/mhal/inc:oversea \
#  $(MTK_PATH_SOURCE)/external/mhal/src/lib/inc:oversea \
#  $(MTK_PATH_SOURCE)/external/mhal/src/lib/lib3a: \
#  $(MTK_PATH_SOURCE)/external/mhal/src/lib/libidp: \
#  $(MTK_PATH_SOURCE)/external/mhal/src/lib/libisp: \
#  $(MTK_PATH_SOURCE)/external/mhal/src/lib/libmexif: \
#  $(MTK_PATH_SOURCE)/external/mhal/src/lib/libmhal: \
#  $(MTK_PATH_SOURCE)/external/mhal/src/lib/libmjpeg:oversea;tier1 \
#  $(MTK_PATH_SOURCE)/external/mhal/src/lib/libsensor: \
#  $(MTK_PATH_SOURCE)/external/mhal/src/test/camtest: \
#  $(MTK_PATH_SOURCE)/external/mhal/src/test/libcunit: \
#  $(MTK_PATH_SOURCE)/external/mhal/src/test/sensorUnitTest: \
#  $(MTK_PATH_SOURCE)/external/mhal/doc:oversea;tier1 \
#  $(MTK_PATH_SOURCE)/external/mhal/src/lib/libDetection: \
#  $(MTK_PATH_SOURCE)/external/mhal/src/lib/libmcu:oversea;tier1 \
