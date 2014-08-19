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


# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# Rules for running apicheck to confirm that you haven't broken
# api compatibility or added apis illegally.
#

# skip api check for TINY_ANDROID and PDK buid
ifeq (,$(filter true, $(BUILD_TINY_ANDROID) $(TARGET_BUILD_PDK)))

.PHONY: checkapi

# eval this to define a rule that runs apicheck.
#
# Args:
#    $(1)  target
#    $(2)  stable api file
#    $(3)  api file to be tested
#    $(4)  arguments for apicheck
#    $(5)  command to run if apicheck failed
define check-api
$(TARGET_OUT_COMMON_INTERMEDIATES)/PACKAGING/$(strip $(1))-timestamp: $(2) $(3) $(APICHECK)
	@echo "Checking API:" $(1)
	$(hide) ( $(APICHECK_COMMAND) $(4) $(2) $(3) || ( $(5) ; exit 38 ) )
	$(hide) mkdir -p $$(dir $$@)
	$(hide) touch $$@
checkapi: $(TARGET_OUT_COMMON_INTERMEDIATES)/PACKAGING/$(strip $(1))-timestamp
endef

# Run the checkapi rules by default.
droidcore: checkapi

last_released_sdk_version := $(lastword $(call numerically_sort, \
            $(filter-out $(SRC_API_DIR)/current, \
                $(patsubst $(SRC_API_DIR)/%.txt,%, $(wildcard $(SRC_API_DIR)/*.txt)) \
             )\
        ))

# INTERNAL_PLATFORM_API_FILE is the one build by droiddoc.

# Check that the API we're building hasn't broken the last-released
# SDK version.
$(eval $(call check-api, \
	checkapi-last, \
	$(SRC_API_DIR)/$(last_released_sdk_version).txt, \
	$(INTERNAL_PLATFORM_API_FILE), \
	-hide 2 -hide 3 -hide 4 -hide 5 -hide 6 -hide 24 -hide 25 \
	-error 7 -error 8 -error 9 -error 10 -error 11 -error 12 -error 13 -error 14 -error 15 \
	-error 16 -error 17 -error 18 , \
	cat $(BUILD_SYSTEM)/apicheck_msg_last.txt \
	))

# Check that the API we're building hasn't changed from the not-yet-released
# SDK version.
$(eval $(call check-api, \
	checkapi-current, \
	$(SRC_API_DIR)/current.txt, \
	$(INTERNAL_PLATFORM_API_FILE), \
	-error 2 -error 3 -error 4 -error 5 -error 6 \
	-error 7 -error 8 -error 9 -error 10 -error 11 -error 12 -error 13 -error 14 -error 15 \
	-error 16 -error 17 -error 18 -error 19 -error 20 -error 21 -error 23 -error 24 \
	-error 25 , \
	cat $(BUILD_SYSTEM)/apicheck_msg_current.txt \
	))

.PHONY: update-api
update-api: $(INTERNAL_PLATFORM_API_FILE) | $(ACP)
	@echo Copying current.txt
	$(hide) $(ACP) $(INTERNAL_PLATFORM_API_FILE) $(SRC_API_DIR)/current.txt

# Add apicheck definitions for MediaTek API making.
ifeq ($(MTK_API_CHECK),yes)
ifneq ($(PARTIAL_BUILD),yes)
  include $(BUILD_SYSTEM_MTK_EXTENSION)/apicheck.mk
endif
endif
endif

