# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
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



# include project feature configuration makefile
# ############################################################
# maybe enable in the future
# ############################################################
#include $(MTK_PROJECT_CONFIG)/$(PROJECT).mak

include mediatek/build/addon/core/definitions.mak
MTK_BUILD_SYSTEM := mediatek/build/addon/core
# ###############################################################
# Build System internal files
# ###############################################################

COMMON_DEFS_CONFIG           := $(MTK_BUILD_SYSTEM)/common_defs.mak
COMMON_DEP_RULE_CONFIG       := $(MTK_BUILD_SYSTEM)/common_dep_rule.mak
KERNEL_COMMON_DEFS_CONFIG    := $(MTK_BUILD_SYSTEM)/kernel_defs.mak
KERNEL_DEP_RULE_CONFIG       := $(MTK_BUILD_SYSTEM)/kernel_dep_rule.mak
ANDROID_COMMON_DEFS_CONFIG   := $(MTK_BUILD_SYSTEM)/android_defs.mak
ANDROID_DEP_RULE_CONFIG      := $(MTK_BUILD_SYSTEM)/android_dep_rule.mak
UBOOT_COMMON_DEFS_CONFIG     := $(MTK_BUILD_SYSTEM)/uboot_defs.mak
UBOOT_DEP_RULE_CONFIG        := $(MTK_BUILD_SYSTEM)/uboot_dep_rule.mak
PRELOADER_COMMON_DEFS_CONFIG := $(MTK_BUILD_SYSTEM)/preloader_defs.mak
PRELOADER_DEP_RULE_CONFIG    := $(MTK_BUILD_SYSTEM)/preloader_dep_rule.mak


INCLUDE_MAKEFILES := COMMON_DEP_RULE_CONFIG \
                     KERNEL_DEP_RULE_CONFIG \
                     ANDROID_DEP_RULE_CONFIG \
                     UBOOT_DEP_RULE_CONFIG \
                     PRELOADER_DEP_RULE_CONFIG

ifneq (,$(CUR_MODULE))
INCLUDE_MAKEFILES := $(filter $(call lower-to-upper,$(CUR_MODULE))_%_CONFIG COMMON_%_CONFIG,$(INCLUDE_MAKEFILES))
endif

INCLUDE_MAKEFILES := $(foreach file,$(INCLUDE_MAKEFILES),$($(file)))

include $(INCLUDE_MAKEFILES)

# ###############################################################
# include defs,include path, dep. rule configuration files
# ###############################################################

include $(COMMON_DEFS_CONFIG)
#include $(COMMON_DEP_RULE_CONFIG)
include $(KERNEL_COMMON_DEFS_CONFIG)
#include $(KERNEL_DEP_RULE_CONFIG)
include $(ANDROID_COMMON_DEFS_CONFIG)
#include $(ANDROID_DEP_RULE_CONFIG)
include $(UBOOT_COMMON_DEFS_CONFIG)
#include $(UBOOT_DEP_RULE_CONFIG)
include $(PRELOADER_COMMON_DEFS_CONFIG)
#include $(PRELOADER_DEP_RULE_CONFIG)


