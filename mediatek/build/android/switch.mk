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


define add-install-target
$(shell if [ ! -e $(1) ]; then touch $(1); fi; \
  echo $(2) >> $(1) \
 )
endef

ARTIFACT_SWITCH_FILE := $(ARTIFACT_DIR)/switch.txt
ARTIFACT_DIR_CLS := $(ARTIFACT_DIR)/cls
ARTIFACT_DIR_JAR := $(ARTIFACT_DIR)/jar
SWITCH_DIRECTORY :=

#config the switch flag
# ----------------------------------------
SWITCH_GEMINI_FLAG := no


# ----------------------------------------

ifeq ($(SWITCH_GEMINI_FLAG),yes)
# #########################################
# gemini switch
# #########################################
GEMINI_SWITCH_DIR := $(ARTIFACT_DIR)
GEMINI_CLS_SSIM := cls_ssim
GEMINI_CLS_DSIM := cls_dsim
GEMINI_JAR_INI := $(ARTIFACT_DIR_JAR)
GEMINI_JAR_SSIM := jar_ssim
GEMINI_JAR_DSIM := jar_dsim
ifeq ($(GEMINI),yes)
SWITCH_DIRECTORY += $(ARTIFACT_DIR_CLS):$(GEMINI_SWITCH_DIR)/$(GEMINI_CLS_DSIM)
SWITCH_DIRECTORY += $(ARTIFACT_DIR_JAR):$(GEMINI_SWITCH_DIR)/$(GEMINI_JAR_DSIM)
else
SWITCH_DIRECTORY += $(ARTIFACT_DIR_CLS):$(GEMINI_SWITCH_DIR)/$(GEMINI_CLS_SSIM)
SWITCH_DIRECTORY += $(ARTIFACT_DIR_JAR):$(GEMINI_SWITCH_DIR)/$(GEMINI_JAR_SSIM)
endif # GEMINI
###########################################
endif # SWITCH_GEMINI_FLAG

#add the swith info into switch.txt
$(shell if [ -e $(ARTIFACT_SWITCH_FILE) ]; then rm -f $(ARTIFACT_SWITCH_FILE); fi)
$(foreach item,$(SWITCH_DIRECTORY),\
  $(eval $(call add-install-target,$(ARTIFACT_SWITCH_FILE),$(item))) \
 )


