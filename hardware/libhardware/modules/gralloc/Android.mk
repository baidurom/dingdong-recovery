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


LOCAL_PATH := $(call my-dir)

# HAL module implemenation stored in
# hw/<OVERLAY_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := liblog libcutils

LOCAL_SRC_FILES := 	\
	gralloc.cpp 	\
	framebuffer.cpp \
	mapper.cpp
	
LOCAL_MODULE := gralloc.default
LOCAL_CFLAGS := -DLOG_TAG=\"gralloc\"

# --- MediaTek ---------------------------------------------------------------
ifeq ($(MTK_ION_SUPPORT), yes)
LOCAL_SRC_FILES += gralloc_ion.cpp
LOCAL_CFLAGS += -DMTK_ION_SUPPORT -DMTK_GRALLOC_ION_DBG
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include
endif
# ----------------------------------------------------------------------------

include $(BUILD_SHARED_LIBRARY)
